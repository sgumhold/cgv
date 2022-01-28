#pragma once

#include <string>

#include <cgv/render/render_types.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>

#include "color_map.h"

//#include "lib_begin.h"

namespace cgv {
namespace glutil{

class color_map_reader : cgv::render::render_types {
private:
	// TODO: move these three methods to some string lib
	static std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {

		str.erase(0, str.find_first_not_of(chars));
		return str;
	}

	static std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {

		str.erase(str.find_last_not_of(chars) + 1);
		return str;
	}

	static std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {

		return ltrim(rtrim(str, chars), chars);
	}

	static std::string xml_attribute_value(const std::string& attribute) {

		size_t pos_start = attribute.find_first_of("\"");
		size_t pos_end = attribute.find_last_of("\"");

		if(pos_start != std::string::npos &&
			pos_end != std::string::npos &&
			pos_start < pos_end &&
			attribute.length() > 2) {
			return attribute.substr(pos_start + 1, pos_end - pos_start - 1);
		}

		return "";
	}

	static std::pair<std::string, std::string> xml_attribute_pair(const std::string& attribute) {

		std::string name = "";
		std::string value = "";

		size_t pos = attribute.find_first_of('=');

		if(pos != std::string::npos) {
			name = attribute.substr(0, pos);
		}

		size_t pos_start = attribute.find_first_of("\"", pos);
		size_t pos_end = attribute.find_last_of("\"");

		if(pos_start != std::string::npos &&
			pos_end != std::string::npos &&
			pos_start < pos_end &&
			attribute.length() > 2) {
			value = attribute.substr(pos_start + 1, pos_end - pos_start - 1);
		}

		return { name, value };
	}

	static bool xml_attribute_to_int(const std::string& attribute, int& value) {

		std::string value_str = xml_attribute_value(attribute);

		if(!value_str.empty()) {
			int value_i = 0;

			try {
				value_i = stoi(value_str);
			} catch(const std::invalid_argument&) {
				return false;
			} catch(const std::out_of_range&) {
				return false;
			}

			value = value_i;
			return true;
		}

		return false;
	}

	static bool xml_attribute_to_float(const std::string& attribute, float& value) {

		std::string value_str = xml_attribute_value(attribute);

		if(!value_str.empty()) {
			float value_f = 0.0f;

			try {
				value_f = stof(value_str);
			} catch(const std::invalid_argument&) {
				return false;
			} catch(const std::out_of_range&) {
				return false;
			}

			value = value_f;
			return true;
		}

		return false;
	}

	static bool parse_xml(const std::string& file_name, std::string& name, color_map& cm) {

		if(!cgv::utils::file::exists(file_name) || cgv::utils::to_upper(cgv::utils::file::get_extension(file_name)) != "XML")
			return false;

		std::string content;
		cgv::utils::file::read(file_name, content, true);

		bool read = true;
		size_t nl_pos = content.find_first_of("\n");
		size_t line_offset = 0;
		bool first_line = true;

		int active_stain_idx = -1;

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

			trim(line);

			if(line.length() < 3)
				continue;

			line = line.substr(1, line.length() - 2);

			std::vector<cgv::utils::token> tokens;
			cgv::utils::split_to_tokens(line, tokens, "", true, "", "");

			if(tokens.size() == 1 && to_string(tokens[0]) == "ColorMaps") {
				// is probably a paraview color map file
			}

			if(tokens.size() == 3 && to_string(tokens[0]) == "ColorMap") {
				name = xml_attribute_value(to_string(tokens[1]));
			}

			if(tokens.size() == 6 && to_string(tokens[0]) == "Point") {
				float x = -1.0f;
				float o = 0.0f;
				float r = 0.0f;
				float g = 0.0f;
				float b = 0.0f;

				xml_attribute_to_float(to_string(tokens[1]), x);
				xml_attribute_to_float(to_string(tokens[2]), o);
				xml_attribute_to_float(to_string(tokens[3]), r);
				xml_attribute_to_float(to_string(tokens[4]), g);
				xml_attribute_to_float(to_string(tokens[5]), b);

				if(!(x < 0.0f)) {
					rgb col(0.0f);
					float opacity = 0.0f;

					col[0] = cgv::math::clamp(r, 0.0f, 1.0f);
					col[1] = cgv::math::clamp(g, 0.0f, 1.0f);
					col[2] = cgv::math::clamp(b, 0.0f, 1.0f);
					
					opacity = cgv::math::clamp(o, 0.0f, 1.0f);
					
					cm.add_color_point(x, col);
					cm.add_opacity_point(x, opacity);
				}
			}
		}

		return true;
	}

public:
	static bool read_from_xml(const std::string& file_name, color_map& cm, std::string& name) {
		// clear previous data
		cm.clear();
		//std::string name = "";
		return parse_xml(file_name, name, cm);
	}
};

}
}

//#include <cgv/config/lib_end.h>
