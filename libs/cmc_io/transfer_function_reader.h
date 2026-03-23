#pragma once

#include <string>

#include <cgv/media/transfer_function.h>

#include <tinyxml2/tinyxml2.h>

#include "lib_begin.h"

namespace cgv {
namespace media {

struct transfer_function_reader_result {
	struct entry {
		std::string name;
		std::shared_ptr<cgv::media::transfer_function> transfer_function;
	};
	bool success = false;
	std::vector<entry> entries;
};


class CGV_API transfer_function_reader {
public:
	// configuration for xml tag and attribute name identifiers
	struct identifier_config {
		std::string color_map_tag_id = "ColorMap";
		std::string point_tag_id = "Point";
		std::string color_point_tag_id = "ColorPoint";
		std::string opacity_point_tag_id = "OpacityPoint";
		std::string name_value_id = "name";
		std::string position_value_id = "x";
		std::string red_value_id = "r";
		std::string green_value_id = "g";
		std::string blue_value_id = "b";
		std::string opactiy_value_id = "o";
		bool color_value_type_float = true;
		bool opacity_value_type_float = true;
		bool apply_gamma = false;
	};

public:
	static transfer_function_reader_result read_from_xml(const tinyxml2::XMLElement& elem, const identifier_config& config = {});

	static transfer_function_reader_result read_from_xml(const tinyxml2::XMLDocument& doc, const identifier_config& config = {});

	static transfer_function_reader_result read_from_xml_string(const std::string& xml, const identifier_config& config = {});

	static transfer_function_reader_result read_from_xml_file(const std::string& file_name, const identifier_config& config = {});

	static transfer_function_reader_result read_from_image_file(const std::string& file_name);

private:
	struct point_info {
		float x = -1.0f;
		float o = -1.0f;
		float r = -1.0f;
		float g = -1.0f;
		float b = -1.0f;
	};

	static void extract_value(const tinyxml2::XMLElement& elem, const std::string& name, bool as_float, float& value);

	static point_info extract_control_point(const tinyxml2::XMLElement& elem, const identifier_config& config);

	static transfer_function_reader_result::entry extract_color_map(const tinyxml2::XMLElement& elem, const identifier_config& config);

	static std::vector<transfer_function_reader_result::entry> extract_color_maps(const tinyxml2::XMLElement& elem, const identifier_config& config);
};

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
