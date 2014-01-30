#pragma once

#include <string>
#include <vector>

#include "lib_begin.h"

///
namespace cgv {
	///
	namespace base {

/// extract a valid path from the given argument and push it onto the stack of parent paths. This should always be paired with a call to pop_file_parent().
extern CGV_API void push_file_parent(const std::string& path_or_file_name);

/// pop the latestly pushed parent path from the parent path stack.
extern CGV_API void pop_file_parent();

/** Find a file with the given strategy and return the file name extended by the necessary path.
    The strategy defines in which search paths the file should be looked for, whether the search 
	paths should be searched recursively and in which order the search paths should be searched
	(see details below).
    Optional arguments are a sub_directory and a master_path. For all non recursive searches the
	sub_directory will be added to each search path and the file is first looked up in the sub_directory.
	The master_path just defines another search path that can be used by the search strategy.
	
	The search strategy is a string containing one letter for each search command, which are processed
	in the order of the string. Each capital letter causes a recursive search. The following search 
	commands are available:
	- r/R ... search resource files (no recursive search necessary as resource files do not have paths) and if found add the prefix "res://" to the file_name
	- c/C ... search current directory (here no path prefix is used)
	- m/M ... search master path
	- d/D ... search all data paths registered in the environment variable CGV_DATA
	- p/P ... search the path of the parent on the parent path stack that is controlled with push_file_parent() and pop_file_parent()
	- a/A ... search all anchester paths of the all parents on the parent path stack that is controlled with push_file_parent() and pop_file_parent()
	As an example the strategy "cpmD" first looks in the current directory, then in the parent directory, then in the master path and finally recursively
	in each of the registered data paths.
*/
extern CGV_API std::string find_data_file(const std::string& file_name, const std::string& strategy, const std::string& sub_directory = "", const std::string& master_path = "");

/// return a reference to the data path list, which is constructed from the environment variable CGV_DATA
extern CGV_API std::vector<std::string>& ref_data_path_list();

/// return a reference to the parent file stack controlled with push_file_parent() and pop_file_parent(), where the last vector element is the latestly pushed one
extern CGV_API std::vector<std::string>& ref_parent_file_stack();

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