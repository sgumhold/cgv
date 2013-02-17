#include <cgv/base/register.h>

#include "import.h"

#include <cgv/utils/dir.h>
#include <cgv/utils/file.h>
#include <cgv/utils/tokenizer.h>


#include <stdlib.h>
#include <map>

using namespace cgv::utils;
using namespace cgv::utils::file;

namespace cgv {
	namespace base {

std::string find_data_file_rec(const std::string& path, const std::string& file_name)
{
	if (cgv::utils::file::exists(path+'/'+file_name))
		return path+'/'+file_name;
	void* h = find_first(path+"*");
	while (h) {
		if (find_directory(h) && find_name(h) != "." && find_name(h) != "..") {
			std::string fn = path+find_name(h)+'/';
			if (exists(fn+file_name))
				return fn+file_name;
			fn = find_data_file_rec(fn, file_name);
			if (!fn.empty())
				return fn;
		}
		h = find_next(h);
	}
	return "";
}

std::string find_data_file_1(const std::string& base_path, const std::string& sub_path, const std::string& file_name, bool recurse)
{
	std::string base_path_prefix = base_path;
	if (!base_path_prefix.empty())
		base_path_prefix += "/";
	std::string dir_name = base_path_prefix+sub_path;
	if (!dir_name.empty()) {
		if (!dir::exists(dir_name))
			return "";
		dir_name += "/";
	}
	if (!dir_name.empty())
		dir_name += "/";
	if (exists(dir_name+file_name))
		return dir_name+file_name;
	if (!recurse)
		return "";
	return find_data_file_rec(dir_name, file_name);
}

std::string find_data_file(const std::string& file_name, bool recurse, const std::string& sub_path)
{
	std::string fn = find_data_file_1("", sub_path, file_name, recurse);
	if (!fn.empty())
		return fn;

	std::map<std::string, resource_file_info>::const_iterator it = ref_resource_file_map().find(file_name);
	if (it != ref_resource_file_map().end())
		return std::string("res://")+file_name;

	const std::vector<std::string>& path_list = ref_data_path_list();
	for (unsigned int i=0; i<path_list.size(); ++i) {
		fn = find_data_file_1(path_list[i], sub_path, file_name, recurse);
		if (!fn.empty())
			return fn;
	}
	return std::string();
}

std::string clean_data_path(const std::string& data_path)
{
	if (data_path.size() == 0)
		return data_path;
	unsigned int p = (unsigned int) data_path.size()-1;
	if (data_path[p] == '/' || data_path[p] == '\\')
		return clean_data_path(data_path.substr(0,p));
	return data_path;
}

/// return a reference to the data path list, which is constructed from the environment variable CGV_DATA
std::vector<std::string>& ref_data_path_list()
{
	static std::vector<std::string> data_path_list;
	static bool initialized = false;
	if (!initialized) {
		char* cgv_data = getenv("CGV_DATA");
		if (cgv_data) {
			std::string cgv_data_str(cgv_data);
			std::vector<token> data_path_tokens;
			bite_all(tokenizer(cgv_data_str).set_ws(";"),data_path_tokens);
			for (unsigned int i=0; i<data_path_tokens.size(); ++i) {
				std::string data_path = to_string(data_path_tokens[i]);
				data_path_list.push_back(clean_data_path(data_path));
			}
		}
		initialized = true;
	}
	return data_path_list;
}

/// open a file with fopen supporting resource files, that have the prefix "res://"
FILE* open_data_file(const std::string& file_name, const char* mode)
{
	if (file_name.substr(0,6) != "res://")
		return fopen(file_name.c_str(), mode);
	std::map<std::string, resource_file_info>::const_iterator i = ref_resource_file_map().find(file_name.substr(6));
	if (i == ref_resource_file_map().end())
		return 0;
	std::string source_file = i->second.source_file;
	if (source_file.empty())
		source_file = ref_prog_name();
	else
		source_file = file::get_path(ref_prog_name())+"/"+source_file;
	unsigned int off = i->second.file_offset;
	if (off == (unsigned int)-1)
		off = find_file_offset(source_file, i->second.file_data, i->second.file_length);
	if (off == (unsigned int)-1)
		return 0;
	FILE* fp = fopen(source_file.c_str(), mode);
	if (!fp)
		return 0;
	fseek(fp, off, SEEK_SET);
	return fp;
}

/// read ascii file into a string
bool read_data_file(const std::string& file_name, std::string& content, bool ascii)
{
	if (file_name.substr(0,6) != "res://")
		return file::read(file_name, content, ascii);
	unsigned int n = data_file_size(file_name);
	content.resize(n);
	FILE* fp = open_data_file(file_name, ascii?"ra":"rb");
	if (!fp)
		return false;
	if (!fread(&content[0], 1, n, fp)) {
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

/// return the file size of a given file with support for resource files, that have the prefix "res://"
unsigned int data_file_size(const std::string& file_name)
{
	if (file_name.substr(0,6) != "res://")
		return (unsigned int) file::size(file_name.c_str());
	return ref_resource_file_map()[file_name.substr(6)].file_length;
}

/// find the offset of the given data block in the given file
unsigned int find_file_offset(const std::string& file_name, const char* data, unsigned int data_size)
{
	size_t file_size;
	char* file_data = read(file_name, false, &file_size);
	if (!file_data || file_size < data_size)
		return -1;
	unsigned int n = (unsigned int) (file_size-data_size);
	char* fdata = file_data;
	for (unsigned int off = 0; off < n; ++off, ++fdata) {
		bool found = true;
		for (unsigned int i = 0; i < data_size; ++i) {
			if (data[i] != fdata[i]) {
				found = false;
				break;
			}
		}
		if (found) {
			delete [] file_data;
			return off;
		}
	}
	delete [] file_data;
	return -1;
}


	}
}