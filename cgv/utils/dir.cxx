#include <cgv/utils/dir.h>
#include <cgv/utils/file.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>


#endif

namespace cgv {
	namespace utils {
		namespace dir {

bool exists(const std::string& dir_name)
{
#ifdef _WIN32
	void* handle = file::find_first(dir_name+"\\*.*");
	if (handle == 0)
		return false;
	file::find_quit(handle);
	return true;
#else
	struct stat myStat;
	return ((stat(dir_name.c_str(), &myStat) == 0) && (((myStat.st_mode) & S_IFMT) == S_IFDIR));
#endif
}

bool chdir(const std::string& dir_name)
{
#ifdef _WIN32
	return _chdir(dir_name.c_str()) == 0;
#else
	return ::chdir(dir_name.c_str());
//	std::cerr << "Not Implemented\n" << std::endl;
#endif
	return false;
}

bool mkdir(const std::string& dir_name)
{
#ifdef _WIN32
	return _mkdir(dir_name.c_str()) == 0;
#else
	return ::mkdir(dir_name.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
//	std::cerr << "Not Implemented\n" << std::endl;
#endif
}

bool rmdir(const std::string& dir_name)
{
#ifdef _WIN32
	return _rmdir(dir_name.c_str()) == 0;
#else
	return ::rmdir(dir_name.c_str());
//	std::cerr << "Not Implemented\n" << std::endl;
#endif
	return false;
}


std::string current()
{
#ifdef _WIN32
	char buf[FILENAME_MAX];
	char * pbuf = _getcwd(buf,FILENAME_MAX);
	std::string s = (pbuf) ? buf : "";
#else
	char* r = getcwd(NULL,0);//buff,256);
	std::string s="";
	if (r == NULL) {
		std::cerr<< "failed"<<std::endl;
	}
	else {
		s = r;
		free(r);
	}
#endif
	return s;
}

bool recursive_glob(const std::string& dir_name, const std::string& sub_path, std::vector<std::string>& file_names, const std::string& filter, bool recursive, bool short_file_names, std::vector<std::string>* subdir_names)
{
	std::string search_path = dir_name + sub_path + filter;
	void* handle = file::find_first(search_path);
	while (handle != 0) {
		std::string file_name = (short_file_names ? sub_path : dir_name + sub_path) + file::find_name(handle);
		if (file::find_directory(handle)) {
			if (file::find_name(handle) != "." && file::find_name(handle) != "..") {
				if (subdir_names)
					subdir_names->push_back(file_name);
				if (recursive)
					recursive_glob(dir_name, sub_path + file::find_name(handle) + "/", file_names, filter, true, short_file_names, subdir_names);
			}			
		}
		else {
			file_names.push_back(file_name);
		}
		handle = file::find_next(handle);
	}
	if (recursive) {
		search_path = dir_name + sub_path + "*";
		void* handle = file::find_first(search_path);
		while (handle != 0) {
			std::string file_name = (short_file_names ? sub_path : dir_name + sub_path) + file::find_name(handle);
			if (file::find_directory(handle) && file::find_name(handle) != "." && file::find_name(handle) != "..") {
				if (subdir_names)
					subdir_names->push_back(file_name);
				if (recursive)
					recursive_glob(dir_name, sub_path + file::find_name(handle) + "/", file_names, filter, true, short_file_names, subdir_names);
			}			
			handle = file::find_next(handle);
		}
	}
	return true;
}

/// collect all files in a directory optionally collect files recursively and subdirectory names
bool glob(const std::string& dir_name, std::vector<std::string>& file_names, const std::string& filter, bool recursive, bool short_file_names, std::vector<std::string>* subdir_names)
{
	if (!exists(dir_name))
		return false;
	recursive_glob(dir_name+"/", "", file_names, filter, recursive, short_file_names, subdir_names);
	return true;
}
		}
	}
}
