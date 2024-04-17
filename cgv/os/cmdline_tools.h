#pragma once

#include <cstdint>
#include <string>
#include <stdio.h>

#include "lib_begin.h"

namespace cgv {
	namespace os {

/// expand all files from zip archive \c file_path into folder \c destination_path
extern CGV_API bool expand_archive(const std::string& file_path, const std::string& destination_path);

/// compress all files specified by \c file_filter_path into zip archive \c archive_path
extern CGV_API bool compress_archive(const std::string& file_filter_path, const std::string& archive_path);

/// execute command and return string with command output to stdout or stderr in case \c cerr is true
extern CGV_API std::string query_system_output(std::string cmd, bool cerr);

/// <summary>
/// start system command and read output of system command in binary format into given buffer
/// </summary>
/// <param name="cmd">system command to be executed including all commandline parameters and options</param>
/// <param name="buffer">buffer to which the system command output is written</param>
/// <param name="buffer_size">size of buffer</param>
/// <param name="progression_text">if specified (or not 0) show progress on console</param>
/// <param name="use_cerr">whether to read output of system command from stderr - otherwise read from stdout</param>
/// <param name="on_progress_update">callback function called once per block size with invocation id</param>
/// <param name="user_data">user data passed on to \c on_progress_update function</param>
/// <param name="block_size">block size used for callback invocations</param>
/// <param name="cycle_till_eof">whether to process complete output of system command even if buffer is already full, if false stop at end of buffer</param>
/// <returns>return number of bytes read which can be more than the size of the buffer</returns>
extern CGV_API size_t read_system_output(std::string cmd, uint8_t* buffer, size_t buffer_size, const char* progression_text = 0, bool use_cerr = false, void (*on_progress_update)(int, void*) = 0, void* user_data = 0, size_t block_size = 4096, bool cycle_till_eof = false);

/// <summary>
/// start a child process with a system command and return file handle that allows to write to standard input of system command
/// </summary>
/// <param name="cmd">to be executed system command</param>
/// <returns>file handle or null_ptr in case of failure; use close_system_input() function to close</returns>
extern CGV_API FILE* open_system_input(const std::string& cmd, bool in_binary_mode = true);
/// <summary>
/// function to close a system command standard input file handle
/// </summary>
/// <param name="fp">file handle created with open_system_input() function</param>
/// <returns>return value returned by system command</returns>
extern CGV_API int close_system_input(FILE* fp);

	}
}

#include <cgv/config/lib_end.h>
