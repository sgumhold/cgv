#include "cmdline_tools.h"

#include <stdio.h>

namespace cgv {
	namespace os {

		bool expand_archive(const std::string& file_path, const std::string& destination_path)
		{
			std::string command;
#ifdef WIN32
			command = "PowerShell -Command {Expand-Archive -Path '";
			command += file_path + "' -DestinationPath '" + destination_path + "'";
#else
			command = "unzip '";
			command += file_path + "' -d '" + destination_path + "'";
#endif
			int result = system(command.c_str());
			return result != -1;
		}

		bool compress_archive(const std::string& file_filter_path, const std::string& archive_path)
		{
			std::string command;
#ifdef WIN32
			command = "PowerShell -Command {Compress-Archive -Path '";
			command += file_filter_path + "' -DestinationPath '" + archive_path + "'";
#else
			command = "zip '";
			command += archive_path + "' '" + file_filter_path + "'";
#endif
			int result = system(command.c_str());
			return result != -1;
		}

	}
}