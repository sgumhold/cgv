#pragma once

#include <string>
#include <vector>

#include "lib_begin.h"

///
namespace cgv {
	///
	namespace base {

/** first search the current directory, then check the resource files and finally the registered data paths. 
    The data paths are initialized to the environment variable "CGV_DATA" being a semicolon separated list
	of paths. The list of data paths can be referenced with ref_data_path_list. If recurse 
	is given, the current directory and the registered data paths are scanned recursively and all sub directories 
	are also scanned. If sub_path is given, not current directory is searched first, but "./"+sub_path. sub_path does not
	influence the search in the resource files. In the registered paths the sub_path parameter reduces the search
	to data_path[i]+"/"+sub_path. If a file is found as a resource of the current program or plugin, it is preceeded
	with the prefix "res://". */
extern CGV_API std::string find_data_file(const std::string& file_name, bool recurse = false, const std::string& sub_path = "");

/// return a reference to the data path list, which is constructed from the environment variable CGV_DATA
extern CGV_API std::vector<std::string>& ref_data_path_list();

/// open a file with fopen supporting resource files, that have the prefix "res://"
extern CGV_API FILE* open_data_file(const std::string& file_name, const char* mode);

/// read ascii file into a string
extern CGV_API bool read_data_file(const std::string& file_name, std::string& content, bool ascii);

/// return the file size of a given file with support for resource files, that have the prefix "res://"
extern CGV_API unsigned int data_file_size(const std::string& file_name);

/// find the offset of the given data block in the given file
extern CGV_API unsigned int find_file_offset(const std::string& file_name, const char* data, unsigned int data_size);

	}
}

#include <cgv/config/lib_end.h>