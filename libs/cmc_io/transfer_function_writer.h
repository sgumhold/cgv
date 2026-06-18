#pragma once

#include <string>

#include <cgv/media/color.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/media/transfer_function.h>
#include <cgv/utils/file.h>

#include <tinyxml2/tinyxml2.h>

#include "lib_begin.h"

namespace cgv {
namespace media {

struct transfer_function_writer_entry {
	std::string name;
	const cgv::media::transfer_function* transfer_function = nullptr;
};

class CGV_API transfer_function_writer {
public:
	static void to_xml_printer(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag = true);

	static void to_xml_printer(tinyxml2::XMLPrinter& printer, const std::vector<transfer_function_writer_entry>& entries, bool put_parent_tag = true);

	static std::string to_xml(const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag = true);

	static std::string to_xml(const std::vector<transfer_function_writer_entry>& entries, bool put_parent_tag = true);

	static bool write_to_xml_file(const std::string& file_name, const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag = true);

	static bool write_to_xml_file(const std::string& file_name, const std::vector<transfer_function_writer_entry>& entries, bool put_parent_tag = true);

	static bool write_to_png_file(const std::string& file_name, const cgv::media::transfer_function* transfer_function, size_t resolution);

private:
	static void write_color_map(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::media::transfer_function* transfer_function);
};

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
