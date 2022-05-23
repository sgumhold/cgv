#pragma once

#include <string>

#include <cgv/render/render_types.h>
#include <cgv/utils/file.h>
#include <cgv/utils/xml.h>

#include "color_map.h"

//#include "lib_begin.h"

namespace cgv {
namespace glutil {

class color_map_reader : cgv::render::render_types {
private:
	struct point_info {
		float x = -1.0f;
		float o = -1.0f;
		float r = -1.0f;
		float g = -1.0f;
		float b = -1.0f;
	};

	static point_info read_point(const cgv::utils::xml_tag& tag) {
		point_info pi;
		const auto end = tag.attributes.end();

		auto it = tag.attributes.find("x");
		if(it != end) pi.x = std::strtof((*it).second.c_str(), nullptr);
		it = tag.attributes.find("o");
		if(it != end) pi.o = std::strtof((*it).second.c_str(), nullptr);
		it = tag.attributes.find("r");
		if(it != end) pi.r = std::strtof((*it).second.c_str(), nullptr);
		it = tag.attributes.find("g");
		if(it != end) pi.g = std::strtof((*it).second.c_str(), nullptr);
		it = tag.attributes.find("b");
		if(it != end) pi.b = std::strtof((*it).second.c_str(), nullptr);
		
		return pi;
	}

	static bool parse_xml(const std::vector<cgv::utils::xml_tag>& tags, std::vector<std::string>& names, std::vector<color_map>& color_maps) {
		std::string name = "";
		color_map cm;
		bool read_cm = false;

		for(const auto& tag : tags) {
			if(tag.name == "")
				continue;

			const auto end = tag.attributes.end();

			if(tag.name == "ColorMaps") {
				// is probably a color map file
			} else if(tag.name == "ColorMap") {
				if(tag.type == cgv::utils::XTT_OPEN) {
					if(read_cm) {
						if(!cm.empty()) {
							names.push_back(name);
							color_maps.push_back(cm);
						}
						name = "";
						cm = color_map();
					}
					// a new color map
					auto it = tag.attributes.find("name");
					if(it != end) {
						name = (*it).second;
					}
					read_cm = true;
				} else if(tag.type == cgv::utils::XTT_CLOSE) {
					// a new color map
					if(read_cm) {
						if(!cm.empty()) {
							names.push_back(name);
							color_maps.push_back(cm);
						}
						name = "";
						cm = color_map();
						read_cm = false;
					}
				}
			} else if(tag.name == "Point" || tag.name == "ColorPoint" || tag.name == "OpacityPoint") {
				if(read_cm) {
					point_info pi = read_point(tag);

					if(!(pi.x < 0.0f)) {
						if(!(pi.r < 0.0f || pi.g < 0.0f || pi.b < 0.0f)) {
							rgb col(0.0f);
							col[0] = cgv::math::clamp(pi.r, 0.0f, 1.0f);
							col[1] = cgv::math::clamp(pi.g, 0.0f, 1.0f);
							col[2] = cgv::math::clamp(pi.b, 0.0f, 1.0f);
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
	static bool read_from_xml(const std::vector<cgv::utils::xml_tag>& tags, std::vector<std::string>& names, std::vector<color_map>& color_maps) {
		// clear previous data
		names.clear();
		color_maps.clear();
		return parse_xml(tags, names, color_maps);
	}

	static bool read_from_xml(const std::vector<std::string>& lines, std::vector<std::string>& names, std::vector<color_map>& color_maps) {
		std::vector<cgv::utils::xml_tag> tags;
		for(const auto& line : lines)
			tags.push_back(cgv::utils::xml_read_tag(line));
			
		return read_from_xml(tags, names, color_maps);
	}

	static bool read_from_xml(const std::string& file_name, std::vector<std::string>& names, std::vector<color_map>& color_maps) {
		if(!cgv::utils::file::exists(file_name) || cgv::utils::to_upper(cgv::utils::file::get_extension(file_name)) != "XML")
			return false;

		std::string content;
		cgv::utils::file::read(file_name, content, true);

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

		return read_from_xml(tags, names, color_maps);
	}
};

}
}

//#include <cgv/config/lib_end.h>
