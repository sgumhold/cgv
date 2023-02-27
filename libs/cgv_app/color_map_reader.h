#pragma once

#include <string>

#include <cgv/base/import.h>
#include <cgv/render/color_map.h>
#include <cgv/render/render_types.h>
#include <cgv/utils/file.h>
#include <cgv/utils/xml.h>

namespace cgv {
namespace app {

class color_map_reader : cgv::render::render_types {
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

		identifier_config() {}
	};

	// data structure of loaded color maps result
	typedef std::vector<std::pair<std::string, cgv::render::color_map>> result;

private:
	struct point_info {
		float x = -1.0f;
		float o = -1.0f;
		float r = -1.0f;
		float g = -1.0f;
		float b = -1.0f;
	};

	static float parse_value(const std::string& value, bool as_float) {
		if(as_float)
			return std::strtof(value.c_str(), nullptr);
		else
			return static_cast<float>(std::strtol(value.c_str(), nullptr, 10)) / 255.0f;
	}

	static point_info read_point(const cgv::utils::xml_tag& tag, const identifier_config& config = identifier_config()) {
		point_info pi;
		const auto end = tag.attributes.end();

		auto it = tag.attributes.find(config.position_value_id);
		if(it != end) pi.x = parse_value((*it).second, true);
		it = tag.attributes.find(config.opactiy_value_id);
		if(it != end) pi.o = parse_value((*it).second, config.opacity_value_type_float);
		it = tag.attributes.find(config.red_value_id);
		if(it != end) pi.r = parse_value((*it).second, config.color_value_type_float);
		it = tag.attributes.find(config.green_value_id);
		if(it != end) pi.g = parse_value((*it).second, config.color_value_type_float);
		it = tag.attributes.find(config.blue_value_id);
		if(it != end) pi.b = parse_value((*it).second, config.color_value_type_float);
		
		return pi;
	}

	static bool parse_xml(const std::vector<cgv::utils::xml_tag>& tags, result& entries, const identifier_config& config = identifier_config()) {
		std::string name = "";
		cgv::render::color_map cm;
		bool read_cm = false;

		for(const auto& tag : tags) {
			if(tag.name == "")
				continue;

			const auto end = tag.attributes.end();

			if(tag.name == "ColorMaps") {
				// is probably a color map file
			} else if(tag.name == config.color_map_tag_id) {
				if(tag.type == cgv::utils::XTT_OPEN) {
					if(read_cm) {
						if(!cm.empty())
							entries.push_back({ name, cm });
						name = "";
						cm = cgv::render::color_map();
					}
					// a new color map
					auto it = tag.attributes.find(config.name_value_id);
					if(it != end) {
						name = (*it).second;
					}
					read_cm = true;
				} else if(tag.type == cgv::utils::XTT_CLOSE) {
					// a new color map
					if(read_cm) {
						if(!cm.empty())
							entries.push_back({ name, cm });
						name = "";
						cm = cgv::render::color_map();
						read_cm = false;
					}
				}
			} else if(tag.name == config.point_tag_id || tag.name == config.color_point_tag_id || tag.name == config.opacity_point_tag_id) {
				if(read_cm) {
					point_info pi = read_point(tag, config);

					if(!(pi.x < 0.0f)) {
						if(!(pi.r < 0.0f || pi.g < 0.0f || pi.b < 0.0f)) {
							rgb col(0.0f);
							col[0] = cgv::math::clamp(pi.r, 0.0f, 1.0f);
							col[1] = cgv::math::clamp(pi.g, 0.0f, 1.0f);
							col[2] = cgv::math::clamp(pi.b, 0.0f, 1.0f);
							// apply gamma correction if requested
							if(config.apply_gamma) {
								col[0] = pow(col[0], 2.2f);
								col[1] = pow(col[1], 2.2f);
								col[2] = pow(col[2], 2.2f);
							}
							cm.add_color_point(pi.x, col);
						}

						if(!(pi.o < 0.0f)) {
							cm.add_opacity_point(pi.x, cgv::math::clamp(pi.o, 0.0f, 1.0f));
						}
					}
				}
			}
		}

		return true;
	}

public:
	static bool read_from_xml(const std::vector<cgv::utils::xml_tag>& tags, result& entries, const identifier_config& config = identifier_config()) {
		// clear previous data
		entries.clear();
		return parse_xml(tags, entries, config);
	}

	static bool read_from_xml(const std::vector<std::string>& lines, result& entries, const identifier_config& config = identifier_config()) {
		std::vector<cgv::utils::xml_tag> tags;
		for(const auto& line : lines)
			tags.push_back(cgv::utils::xml_read_tag(line));
			
		return read_from_xml(tags, entries, config);
	}

	static bool read_from_xml(const std::string& file_name, result& entries, const identifier_config& config = identifier_config()) {
		std::string content;
		if(cgv::utils::to_upper(cgv::utils::file::get_extension(file_name)) != "XML" || !cgv::base::read_data_file(file_name, content, true))
			return false;

		bool read = true;
		size_t nl_pos = content.find_first_of("\n");
		size_t line_offset = 0;

		std::vector<cgv::utils::xml_tag> tags;

		while(read) {
			std::string line = "";

			if(nl_pos == std::string::npos) {
				read = false;
				line = content.substr(line_offset, std::string::npos);
			} else {
				size_t next_line_offset = nl_pos;
				line = content.substr(line_offset, next_line_offset - line_offset);
				line_offset = next_line_offset + 1;
				nl_pos = content.find_first_of('\n', line_offset);
			}

			tags.push_back(cgv::utils::xml_read_tag(line));
		}

		return read_from_xml(tags, entries, config);
	}
};

}
}
