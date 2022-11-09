#pragma once

#include <string>
#include <stdio.h>

#include <cgv/utils/lib_begin.h>


namespace cgv {
	namespace utils {
		namespace file {
			/// open file with c-mode, buffer and buffer length
			CGV_API void* open(const std::string& file_name, const std::string& mode, void* buf, int length);
			/// check, whether file exists
			CGV_API bool exists(const std::string& file_name);
			/// remove file
			CGV_API bool remove(const std::string& file_name);
			/// rename file
			CGV_API bool rename(const std::string& before, const char* after);
			/// find a file in the given paths
			CGV_API std::string find_in_paths(
				const std::string& file_name,      // file name without path
				const std::string& path_list,      // semicolon separated list of paths
				bool recursive = true,             // whether to search subdirectories of paths
				const std::string& sub_dir = "");  // sub directory to which search in paths is restricted
			/// copy file
			CGV_API bool copy(const std::string& from, const std::string& to);
			/// receive the length of a file in bytes (works for files larger than 4GB)
			CGV_API size_t size(const std::string& file_name, bool ascii = false);
			/// read the file into the content string
			CGV_API bool read(const std::string& file_name, std::string& content, bool ascii = false);
			/// read the file in a newly allocated returned buffer and set the file size
			CGV_API char* read(const std::string& file_name, bool ascii = false, size_t* size_ptr = 0, size_t add_nr_bytes_to_buffer=0);
			/// read from a file into a memory block, the optional argument \c ascii tells whether to write in ascii or binary (default) mode
			CGV_API bool read(const std::string& filename, char* ptr, size_t size, bool ascii = false, size_t file_offset = 0);
			/// write a file from the content string, the optional argument \c ascii tells whether to write in ascii or binary (default) mode
			CGV_API bool write(const std::string& filename, const std::string& content, bool ascii = false);
			/// write a file from a memory block, the optional argument \c ascii tells whether to write in ascii or binary (default) mode
			CGV_API bool write(const std::string& filename, const char* ptr, size_t size, bool ascii = false);
			/// append a memory block to a given file or create new file, the optional argument \c ascii tells whether to write in ascii or binary (default) mode
			CGV_API bool append(const std::string& filename, const char* ptr, size_t size, bool ascii = false);
			/// append second file at the end of first file (works for files larger than 4GB)
			CGV_API bool append(const std::string& file_name_1, const std::string& file_name_2, bool ascii = false);
			/// result type of file comparison
			enum Result { EQUAL, DIFFERENT, FILE_ERROR };
			/// binary comparison of two files
			CGV_API Result cmp(const std::string& what, const std::string& with);
			/// find first matching file, return 0 if non found
			CGV_API void* find_first(const std::string& filter);
			///  find next and return 0 if no next exists
			CGV_API void* find_next(void* handle);
			/// return name of currently found file without path
			CGV_API std::string find_name(void* handle);
			/// return the time of the last write time
			CGV_API long long find_last_write_time(const void* handle);
			/// check if file is readonly
			CGV_API bool find_read_only(const void* handle);
			/// check if we have directory
			CGV_API bool find_directory(const void* handle);
			/// check if system file
			CGV_API bool find_system(const void* handle);
			/// check if hidden file
			CGV_API bool find_hidden(const void* handle);
			/// check if to be archived file
			CGV_API bool find_archive(const void* handle);
			/// check if normal file
			CGV_API bool find_normal(const void* handle);
			/// return file size, but careful, this does not work for file sizes larger than 2GB. To support large files use cgv::utils::file::size(path+"/"+cgv::utils::file::find_name(h)) instead.
			CGV_API size_t find_size(void* handle);
			/// return the time of last write of a given file
			CGV_API long long get_last_write_time(const std::string& file_path);
			/// return the extension of a file name without the dot
			CGV_API std::string get_extension(const std::string& file_path);
			/// return the file path and name without extension and without dot
			CGV_API std::string drop_extension(const std::string& file_path);
			/// return the name with extension of a file path
			CGV_API std::string get_file_name(const std::string& file_path);
			/// return the path of a file name
			CGV_API std::string get_path(const std::string& file_path);
			/// check whether a path is relative
			CGV_API bool is_relative_path(const std::string& file_path);
			/// clean up path such that it conforms to platform specific path
			CGV_API std::string platform_path(const std::string& file_path);
			/// clean up the path such that all back slashes are replaced by /, no multiple / arise and no trailing / arises
			CGV_API std::string clean_path(const std::string& file_path);
			/// remove the prefix_path from the file_path in case that it is a prefix and return true whether it was a prefix
			CGV_API bool shorten_path(std::string& file_path, const std::string& prefix_path);
			/// read string from a binary file
			CGV_API bool read_string_bin(std::string& s, FILE* fp);
			/// write string to a binary file
			CGV_API bool write_string_bin(const std::string& s, FILE* fp);
		}
	}
}

#include <cgv/config/lib_end.h>

