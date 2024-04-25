#include "cmdline_tools.h"
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/progression.h>
#include <stdio.h>

#if WIN32
#define popen(...) _popen(__VA_ARGS__);
#define pclose(...) _pclose(__VA_ARGS__);
#endif

namespace cgv {
	namespace os {

		bool expand_archive(const std::string& file_path, const std::string& destination_path)
		{
			std::string command;
#ifdef WIN32
			command = "PowerShell -Command \"Expand-Archive -Path '";
			command += file_path + "' -DestinationPath '" + destination_path + "'\"";
			cgv::utils::replace(command, '/', '\\');
#else
			command = "unzip '";
			command += file_path + "' -d '" + destination_path + "'";
			command = cgv::utils::file::clean_path(command);
#endif
			int result = system(command.c_str());
			return result != -1;
		}

		bool compress_archive(const std::string& file_filter_path, const std::string& archive_path)
		{
			std::string command;
#ifdef WIN32
			command = "PowerShell -Command \"Compress-Archive -Path '";
			command += file_filter_path + "' -DestinationPath '" + archive_path + "'\"";
#else
			command = "zip '";
			command += archive_path + "' '" + file_filter_path + "'";
#endif
			int result = system(command.c_str());
			return result != -1;
		}


		std::string query_system_output(std::string cmd, bool cerr)
		{
			std::string data;
			FILE* stream;
			const int max_buffer = 256;
			char buffer[max_buffer];
			if (cerr)
				cmd.append(" 2>&1");

			stream = popen(cmd.c_str(), "r");

			if (stream) {
				while (!feof(stream))
					if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
				pclose(stream);
			}
			return data;
		}
		size_t read_system_output(std::string cmd, uint8_t* buffer, size_t buffer_size, const char* progression_text, bool use_cerr, void (*on_progress_update)(int, void*), void* user_data, size_t block_size, bool cycle_till_eof)
		{
			cgv::utils::progression* prog_ptr = 0;
			if (progression_text != 0)
				prog_ptr = new cgv::utils::progression(progression_text, buffer_size / block_size, 20);
			FILE* fp;
			if (use_cerr)
				cmd.append(" 2>&1");
#ifdef WIN32
			const char* mode = "rb";
#else
			const char* mode = "r";
#endif
			fp = popen(cmd.c_str(), mode);
			size_t nr_bytes_read = 0;
			int block_cnt = 0;
			if (fp) {
				while ((nr_bytes_read < buffer_size || cycle_till_eof) && !feof(fp)) {
					size_t nr_bytes = block_size;
					if (!cycle_till_eof)
						nr_bytes = std::min(nr_bytes, buffer_size - nr_bytes_read);
					size_t offset = nr_bytes_read % buffer_size;
					size_t nr_read = fread(buffer + offset, 1, nr_bytes, fp);
					if (prog_ptr)
						prog_ptr->step();
					if (on_progress_update)
						on_progress_update(++block_cnt, user_data);
					nr_bytes_read += nr_read;
					if (nr_read < nr_bytes)
						break;
				}
				pclose(fp);
			}
			return nr_bytes_read;
		}
		FILE* open_system_input(const std::string& cmd, bool in_binary_mode)
		{
#ifdef WIN32
			const char* mode = in_binary_mode ? "wb" : "w";
#else
			const char* mode = "w";
#endif
			return popen(cmd.c_str(), mode);
		}
		int close_system_input(FILE* fp)
		{
			return pclose(fp);
		}

	}
}