#include "file.h"

#include <cgv/type/standard_types.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <string.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <glob.h>
#endif

namespace cgv {
	namespace utils {
		namespace file {
#if _MSC_VER > 1400
#pragma warning (disable:4996)
#endif

void* open(const std::string& file_name, const std::string& mode, void* buf, int length)
{
	FILE *file = fopen(file_name.c_str(), mode.c_str());
	if ( (file != 0) && (setvbuf((FILE*) file, (char*) buf, _IOFBF, length) == 0) )
		return file;
	else
		return 0;
}


bool exists(const std::string& file_name)
{
	void* handle = find_first(file_name);
	return handle != 0;
	/*
	FILE* fp = fopen(file_name.c_str(), "r");
	if (fp) 
		fclose(fp);
	return fp != NULL;
	*/
}

std::string find_recursive(const std::string& path, const std::string& file_name)
{
	if (cgv::utils::file::exists(path+'/'+file_name))
		return path+'/'+file_name;
	void* h = find_first(path+"/*");
	while (h) {
		if (find_directory(h) && find_name(h) != "." && find_name(h) != "..") {
			std::string res = find_recursive(path+'/'+find_name(h), file_name);
			if (!res.empty())
				return res;
		}
		h = find_next(h);
	}
	return "";
}

/// find a file in the given paths
std::string find_in_paths(const std::string& file_name, const std::string& path_list,
								  bool recursive, const std::string& sub_dir)
{
	// first check file name itself
	if (cgv::utils::file::exists(file_name))
		return file_name;

	// then check in sub_dir
	std::string sfn;
	if (sub_dir.empty())
		sfn = file_name;
	else {
		sfn = sub_dir+'/'+file_name;
		if (cgv::utils::file::exists(sfn))
			return sfn;
	}

	// iterate all entries in path list and check for file_name in sub_dir
	size_t pos = 0;
	do {
		size_t end_pos = path_list.find_first_of(';', pos);
		std::string fn;
		if (end_pos == std::string::npos) {
			fn = path_list.substr(pos)+'/'+sfn;
			pos = path_list.length();
		}
		else {
			fn = path_list.substr(pos,end_pos-pos)+'/'+sfn;
			pos = end_pos+1;
		}
		if (cgv::utils::file::exists(fn))
			return fn;
	} while (pos < path_list.length());

	// recursive iteration
	pos = 0;
	do {
		size_t end_pos = path_list.find_first_of(';', pos);
		std::string fn;
		if (end_pos == std::string::npos) {
			fn = path_list.substr(pos);
			pos = path_list.length();
		}
		else {
			fn = path_list.substr(pos, end_pos-pos);
			pos = end_pos+1;
		}
		fn = find_recursive(fn, sfn);
		if (!fn.empty())
			return fn;
	} while (pos < path_list.length());
	return "";
}

bool remove(const std::string& file_name)
{
	return ::remove(file_name.c_str()) == 0;
}

bool rename(const std::string& before, const std::string& after)
{
	return ::rename(before.c_str(), after.c_str()) == 0;
}

bool copy(const std::string& from, const std::string& to)
{
	bool  success = false;
	FILE* fp = ::fopen(from.c_str(), "rb");
	FILE* gp = ::fopen(to.c_str(), "wb");
	if (fp && gp) {
		success = true;
		char buffer[4096];
		while (!feof(fp)) {
			size_t count  = fread (buffer, sizeof(char), 4096, fp);
			if (count != fwrite(buffer, sizeof(char), count, gp) ) {
				success = false;
				break;
			}
		}
	}
	if (fp) fclose(fp);
	if (gp) fclose(gp);
	return success;
}

Result cmp(const std::string& what, const std::string& with)
{
	Result result = file::EQUAL;
	FILE* fp = ::fopen(what.c_str(), "rb");
	FILE* gp = ::fopen(with.c_str(), "rb");
	if (fp && gp) {
		char bufferf[4096];
		char bufferg[4096];
		while (!feof(fp)) {
			size_t countf = fread (bufferf, sizeof(char), 4096, fp);
			size_t countg = fread (bufferg, sizeof(char), 4096, gp);
			if ( (countf != countg) || 
				 (::memcmp(bufferf, bufferg, sizeof(char)*countf) != 0) ) {
				result = file::DIFFERENT;
				break;
			}
		}
	}
	else 
		result = file::FILE_ERROR;
	if (fp) fclose(fp);
	if (gp) fclose(gp);
	return result;
}

size_t size(const std::string& file_name, bool ascii)
{
	void* handle = find_first(file_name);
	if (handle == 0)
		return (size_t)-1;
	return find_size(handle);
#ifdef _WIN32
	int fh = _open(file_name.c_str(), ascii ? _O_RDONLY : (_O_BINARY | _O_RDONLY) );
	if (fh == -1) return (size_t)-1;
	size_t l = _filelength(fh);
	_close(fh);
	return l;
#else
	int fh = ::open(file_name.c_str(), O_RDONLY);
	if (fh == -1)
		return -1;
	struct stat fileinfo;
	fstat(fh, &fileinfo);
	close(fh);
	return fileinfo.st_size;
#endif
}

/// read from a file into a memory block, the optional argument \c ascii tells whether to write in ascii or binary (default) mode
bool read(const std::string& filename, char* ptr, size_t size, bool ascii, size_t file_offset)
{
	FILE* fp = ::fopen(filename.c_str(), ascii ? "r" : "rb");
	if (fp == 0)
		return false;
	if (file_offset != 0) {
		if (::fseek(fp, (long)file_offset, SEEK_SET) != 0)
			return false;
	}
	size_t n = ::fread(ptr, 1, size, fp);
	fclose(fp);
	return n == size;
}

/// read the file into the content string
bool read(const std::string& file_name, std::string& content, bool ascii)
{
	size_t l = size(file_name, ascii);
	if (l == (size_t)-1) return false;
/*	char* buffer = new char[l];
	FILE* fp = ::fopen(file_name.c_str(), ascii ? "r" : "rb");
	unsigned int n = ::fread(buffer, 1, l, fp);
	content.resize(n);
	memcpy(&content[0],buffer,n);
	delete [] buffer;
	return true;*/
	content.resize(l);
	FILE* fp = ::fopen(file_name.c_str(), ascii ? "r" : "rb");
	size_t n = ::fread(&content[0], 1, l, fp);
	content.resize(n);
	::fclose(fp);
	return true;
}

char* read(const std::string& file_name, bool ascii, size_t* size_ptr, size_t add_nr_bytes_to_buffer)
{
	size_t l = size(file_name);
	if (l == (size_t)-1) return 0;
	char* buffer = new char[l+ add_nr_bytes_to_buffer];
	FILE* fp = ::fopen(file_name.c_str(), ascii ? "r" : "rb");
	l = ::fread(buffer, 1, l, fp);
	::fclose(fp);
	if (size_ptr) *size_ptr = l;
	return buffer;
}

bool write(const std::string& file_name, const char* ptr, size_t size, bool ascii)
{
	bool res = false;
	FILE* fp = ::fopen(file_name.c_str(), ascii ? "w" : "wb");
	if (fp) {
		res = ::fwrite(ptr, 1, size, fp) == size;
		::fclose(fp);
	}
	return res;
}

bool append(const std::string& file_name, const char* ptr, size_t size, bool ascii)
{
	bool res = false;
	FILE* fp = ::fopen(file_name.c_str(), ascii ? "a" : "ab");
	if (fp) {
		res = ::fwrite(ptr, 1, size, fp) == size;
		::fclose(fp);
	}
	return res;
}


#ifdef _WIN32
struct FileInfo
{
	_finddata_t fileinfo;
	intptr_t    handle;
};
#else
struct FileInfo
{
	glob_t* globResults;
	int index;
};
#endif

/// return the time of the last write time
long long find_last_write_time(const void* handle)
{
	FileInfo *fi = (FileInfo*) handle;
#ifdef _WIN32
	return fi->fileinfo.time_write;
#else
	struct stat statBuffer;
	int result =stat(find_name(fi).c_str(),&statBuffer);
        if (result == 0)
        {
		return statBuffer.st_ctime;
        }

	return 0;
#endif
}

/// return the time of last write of a given file
long long get_last_write_time(const std::string& file_path)
{
	void* handle = find_first(file_path);
	if (!handle)
		return -1;
	long long time = find_last_write_time(handle);
	// make sure that internal data structure is removed
	find_next(handle);
	return time;
}

/// find first matching file, return 0 if non found
void* find_first(const std::string& filter)
{
	FileInfo *fi = new FileInfo;
#ifdef _WIN32
	fi->handle = _findfirst(filter.c_str(), &fi->fileinfo);
	if (fi->handle == -1) {
		delete fi;
		return 0;
	}
	return fi;
#else
	fi->globResults=new glob_t();
	fi->index=0;
	if(glob(filter.c_str(),GLOB_NOSORT,NULL,(fi->globResults))!=0)
	{
		delete fi->globResults;
		delete fi;
		return 0;
	}
	//printf("filter: %s",filter.c_str());
	//for(int i=0;i<fi->globResults->gl_pathc;i++) printf("file: %s\n",get_file_name(fi->globResults->gl_pathv[i]).c_str());
	return fi;
//	std::cerr << "Not Implemented" << std::endl;
//	return NULL;
#endif
}

///  find next and return 0 if no next exists
void* find_next(void* handle)
{
	FileInfo *fi = (FileInfo*) handle;
#ifdef _WIN32
	if (_findnext(fi->handle, &fi->fileinfo) == -1) {
		delete fi;
		return 0;
	}
	return fi;
#else
	fi->index+=1;
	if(fi->globResults->gl_pathc > fi->index) return fi;
	//std::cerr << "Not Implemented" << std::endl;
	return NULL;
#endif
}
/// return name of currently found file without path
std::string find_name(void* handle)
{
	FileInfo *fi = (FileInfo*) handle;
#ifdef _WIN32
	return std::string(fi->fileinfo.name);
#else
	if(fi->globResults->gl_pathc > fi->index)
		return get_file_name(std::string(fi->globResults->gl_pathv[fi->index]));
//	std::cerr << "Not Implemented" << std::endl;
	return std::string("");
#endif
}
/// check if file is readonly
bool find_read_only(const void* handle)
{
#ifdef _WIN32
	FileInfo *fi = (FileInfo*) handle;
	return fi->fileinfo.attrib & _A_RDONLY;
#else
	std::cerr << "Not Implemented" << std::endl;
	return false;
#endif
}

/// check if we have directory
bool find_directory(const void* handle)
{
	FileInfo *fi = (FileInfo*) handle;
#ifdef _WIN32
	return (fi->fileinfo.attrib & _A_SUBDIR) != 0;
#else
	if(fi->globResults->gl_pathc > fi->index)
	{
		struct stat stat_buf;
		stat(fi->globResults->gl_pathv[fi->index],&stat_buf); 
		return S_ISDIR(stat_buf.st_mode);
	}
	return false;
#endif
}
/// check if system file
bool find_system(const void* handle)
{
#ifdef _WIN32
	FileInfo *fi = (FileInfo*) handle;
	return (fi->fileinfo.attrib & _A_SYSTEM) != 0;
#else
	std::cerr << "Not Implemented" << std::endl;
	return false;
#endif
}
/// check if hidden file
bool find_hidden(const void* handle)
{
#ifdef _WIN32
	FileInfo *fi = (FileInfo*) handle;
	return (fi->fileinfo.attrib & _A_HIDDEN) != 0;
#else
	std::cerr << "Not Implemented" << std::endl;
	return false;
#endif
}
/// check if to be archived file
bool find_archive(const void* handle)
{
#ifdef _WIN32
	FileInfo *fi = (FileInfo*) handle;
	return (fi->fileinfo.attrib & _A_ARCH) != 0;
#else
	std::cerr << "Not Implemented" << std::endl;
	return false;
#endif
}
/// check if normal file
bool find_normal(const void* handle)
{
	FileInfo *fi = (FileInfo*) handle;
#ifdef _WIN32
	return fi->fileinfo.attrib  == _A_NORMAL;
#else
	if(fi->globResults->gl_pathc > fi->index)
	{
		struct stat stat_buf;
		stat(fi->globResults->gl_pathv[fi->index],&stat_buf); 
		return S_ISREG(stat_buf.st_mode);
	}
	return false;
#endif
}
/// return file size
size_t find_size(void* handle)
{
	FileInfo *fi = (FileInfo*) handle;
#ifdef _WIN32
	return fi->fileinfo.size;
#else
	if(fi->globResults->gl_pathc > fi->index)
	{
		struct stat stat_buf;
		stat(fi->globResults->gl_pathv[fi->index],&stat_buf); 
		return stat_buf.st_size;
	}
	return 0;
#endif
}

/// return the extension of a file name
std::string get_extension(const std::string& file_path)
{
	size_t pos = file_path.find_last_of('.');
	if (pos == std::string::npos)
		return std::string();
	return file_path.substr(pos+1);
}

/// return the file path and name without extension
std::string drop_extension(const std::string& file_path)
{
	return file_path.substr(0,file_path.find_last_of('.'));
}

/// return the name with extension of a file path
std::string get_file_name(const std::string& file_path)
{
	size_t pos[3] = {
		file_path.find_last_of('/'), 
		file_path.find_last_of('\\'),
		file_path.find_last_of(':') 
	};
	size_t p = std::string::npos;
	for (unsigned int i=0; i<3; ++i)
		if (pos[i] != std::string::npos)
			if (p == std::string::npos || pos[i] > p)
				p = pos[i];
	if (p == std::string::npos)
		return file_path;
	return file_path.substr(p+1);
}

/// return the path of a file name
std::string get_path(const std::string& file_path)
{
	size_t pos[3] = { 
		file_path.find_last_of('/'), 
		file_path.find_last_of('\\'),
		file_path.find_last_of(':') 
	};
	size_t p = std::string::npos;
	for (unsigned int i=0; i<3; ++i)
		if (pos[i] != std::string::npos)
			if (p == std::string::npos || pos[i] > p)
				p = pos[i];
	if (p == std::string::npos)
		return "";
	return file_path.substr(0,p);
}

char to_lower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return (c-'A')+'a';
	switch (c) {
		case 'Ä' : return 'ä';
		case 'Ö' : return 'ö';
		case 'Ü' : return 'ü';
		default: return c;
	}
}

bool is_letter(char c)
{
	c = to_lower(c);
	return c >= 'a' && c <= 'z';
}
/// check whether a path is relative
bool is_relative_path(const std::string& file_path)
{
	if (file_path.size() == 0)
		return true;
	if (file_path[0] == '/')
		return false;
	if (file_path[0] == '\\')
		return false;
	if (file_path.size() == 1)
		return true;
	if (file_path[1] == ':' && is_letter(file_path[0]))
		return false;
	return true;
}

/// clean up the path such that all back slashes are replaced by /, no multiple / arise and no trailing / arises
std::string clean_path(const std::string& file_path)
{
	std::string cleaned_path = file_path;
	unsigned i;
	for (i=0; i<cleaned_path.size(); ++i) {
		char c = cleaned_path[i];
		if (c == '\\')
			c = '/';
#ifdef _WIN32
		else c = to_lower(c);
#endif
		cleaned_path[i] = c;
	}
	bool last_is_slash = false;
	unsigned n = (unsigned int) cleaned_path.size();
	unsigned m = n;
	for (unsigned int i=0, j=0; i<n; ++i, ++j) {
		if (j < i)
			cleaned_path[j] = cleaned_path[i];
		if (cleaned_path[i] == '/')
			if (last_is_slash) {
				--j;
				--m;
			}
			else
				last_is_slash = true;
		else
			last_is_slash = false;
	}
	if (cleaned_path[m-1] == '/')
		--m;
	return cleaned_path.substr(0,m);
}

/// clean up path such that it conforms to platform specific path
std::string platform_path(const std::string& file_path)
{
	std::string cleaned_path = clean_path(file_path);
	unsigned i;
	for (i = 0; i < cleaned_path.size(); ++i) {
		char c = cleaned_path[i];
#ifdef _WIN32
		if (c == '/')
			c = '\\';
#else
		if (c == '\\')
			c = '/';
#endif
		cleaned_path[i] = c;
	}
	return cleaned_path;
}

/// remove the prefix_path from the file_path in case that it is a prefix and return true whether it was a prefix
bool shorten_path(std::string& file_path, const std::string& prefix_path)
{
	std::string cleaned_prefix_path = clean_path(prefix_path);
	std::string cleaned_file_path = clean_path(file_path);
	if (cleaned_file_path.size() < cleaned_prefix_path.size()) {
		file_path = cleaned_file_path;
		return false;
	}
	if (cleaned_file_path.substr(0,cleaned_prefix_path.size()) != cleaned_prefix_path) {
		file_path = cleaned_file_path;
		return false;
	}
	if (cleaned_file_path.size() == cleaned_prefix_path.size()) {
		file_path = "";
		return true;
	}
	unsigned int o = (unsigned int) cleaned_prefix_path.size();
	if (cleaned_file_path[o] == '/')
		++o;
	file_path = cleaned_file_path.substr(o);
	return true;
}



bool read_string_bin(std::string& s, FILE* fp)
{
	cgv::type::uint16_type length;
	if (fread(&length, sizeof(cgv::type::uint16_type), 1, fp) != 1)
		return false;
	s.resize(length);
	if (length == 0)
		return true;
	return fread(&s[0], sizeof(char), length, fp) == length;
}

bool write_string_bin(const std::string& s, FILE* fp)
{
	cgv::type::uint16_type length = (cgv::type::uint16_type)s.size();
	if (fwrite(&length, sizeof(cgv::type::uint16_type), 1, fp) != 1)
		return false;
	if (length == 0)
		return true;
	return fwrite(&s[0], sizeof(char), length, fp) == length;
}


		}
	}
}
