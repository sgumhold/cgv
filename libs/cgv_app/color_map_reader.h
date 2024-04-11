#pragma once

#include <string>

#include <cgv/base/import.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/color_map.h>
#include <cgv/utils/file.h>

#include <tinyxml2/tinyxml2.h>
#include <cgv_xml/query.h>

namespace cgv {
namespace app {

class color_map_reader {
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
	static const identifier_config& default_identifier_config() {
		static const identifier_config default_ic;
		return default_ic;
	}

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

	static void extract_value(const tinyxml2::XMLElement& elem, const std::string& name, bool as_float, float& value) {

		if(as_float) {
			elem.QueryFloatAttribute(name.c_str(), &value);
		} else {
			int temp;
			if(elem.QueryIntAttribute(name.c_str(), &temp) == tinyxml2::XML_SUCCESS)
				value = static_cast<float>(temp) / 255.0f;
		}
	}

	static point_info extract_control_point(const tinyxml2::XMLElement& elem, const identifier_config& config) {

		point_info pi;
		extract_value(elem, config.position_value_id, true, pi.x);
		extract_value(elem, config.opactiy_value_id, config.opacity_value_type_float, pi.o);
		extract_value(elem, config.red_value_id, config.color_value_type_float, pi.r);
		extract_value(elem, config.green_value_id, config.color_value_type_float, pi.g);
		extract_value(elem, config.blue_value_id, config.color_value_type_float, pi.b);
		return pi;
	}

	static void extract_color_map(const tinyxml2::XMLElement& elem, result& entries, const identifier_config& config) {

		std::string name = elem.Attribute("name");
		cgv::render::color_map cm;

		auto child = elem.FirstChildElement();
		while(child) {
			if(strcmp(child->Name(), config.point_tag_id.c_str()) == 0 ||
				strcmp(child->Name(), config.color_point_tag_id.c_str()) == 0 ||
				strcmp(child->Name(), config.opacity_point_tag_id.c_str()) == 0) {
				point_info pi = extract_control_point(*child, config);

				if(!(pi.x < 0.0f)) {
					if(!(pi.r < 0.0f || pi.g < 0.0f || pi.b < 0.0f)) {
						rgb col(0.0f);
						col[0] = std::min(pi.r, 1.0f);
						col[1] = std::min(pi.g, 1.0f);
						col[2] = std::min(pi.b, 1.0f);
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

			child = child->NextSiblingElement();
		}

		if(!cm.empty())
			entries.push_back({ name, cm });
	}

	static void extract_color_maps(const tinyxml2::XMLElement& elem, result& entries, const identifier_config& config) {

		cgv::xml::FindElementByNameVisitor findElementByName("ColorMaps");
		elem.Accept(&findElementByName);

		if (auto color_maps_elem = findElementByName.Result()) {
			auto color_map_elem = color_maps_elem->FirstChildElement();

			while (color_map_elem) {
				if (strcmp(color_map_elem->Name(), config.color_map_tag_id.c_str()) == 0)
					extract_color_map(*color_map_elem, entries, config);
				color_map_elem = color_map_elem->NextSiblingElement();
			}
		}
	}

public:
	static bool read_from_xml(const tinyxml2::XMLElement& elem, result& entries, const identifier_config& config = default_identifier_config()) {

		entries.clear();
		extract_color_maps(elem, entries, config);

		return !entries.empty();
	}

	static bool read_from_xml(const tinyxml2::XMLDocument& doc, result& entries, const identifier_config& config = default_identifier_config()) {

		if(const tinyxml2::XMLElement* elem = doc.RootElement())
			return read_from_xml(*elem, entries, config);

		return false;
	}

	static bool read_from_xml_string(const std::string& xml, result& entries, const identifier_config& config = default_identifier_config()) {
		
		tinyxml2::XMLDocument doc;
		if(doc.Parse(xml.c_str()) == tinyxml2::XML_SUCCESS)
			return read_from_xml(doc, entries, config);

		return false;
	}

	static bool read_from_xml_file(const std::string& file_name, result& entries, const identifier_config& config = default_identifier_config()) {
		
		tinyxml2::XMLDocument doc;
		if(doc.LoadFile(file_name.c_str()) == tinyxml2::XML_SUCCESS)
			return read_from_xml(doc, entries, config);

		return false;
	}

	static bool read_from_image_file(const std::string& file_name, result& entries) {

		entries.clear();

		if(!cgv::utils::file::exists(file_name))
			return false;

		cgv::data::data_format format;
		cgv::data::data_view data;
		cgv::media::image::image_reader image_reader(format);

		const std::string name = cgv::utils::file::drop_extension(cgv::utils::file::get_file_name(file_name));

		if(image_reader.read_image(file_name, data)) {
			auto type_id = format.get_component_type();

			float max_value = 1.0f;
			if(cgv::type::info::is_integral(type_id)) {
				switch(cgv::type::info::get_type_size(type_id)) {
				case 1: max_value = static_cast<float>(0x000000FF); break;
				case 2: max_value = static_cast<float>(0x0000FFFF); break;
				case 3: max_value = static_cast<float>(0x00FFFFFF); break;
				case 4: max_value = static_cast<float>(0xFFFFFFFF); break;
				default: break;
				}
			}

			cgv::render::color_map cm;
			float step = 1.0f / static_cast<float>(format.get_width() - 1);
			float t = 0.0f;

			if(format.get_width() > 0 && format.get_height() > 0) {
				switch(format.get_nr_components()) {
				case 1u:
					for(size_t x = 0; x < format.get_width(); ++x) {
						cm.add_color_point(t, cgv::rgb(data.get<float>(0, 0, x) / max_value));
						t += step;
					}
					break;
				case 3u:
				case 4u:
					for(size_t x = 0; x < format.get_width(); ++x) {
						float r = data.get<float>(0, 0, x) / max_value;
						float g = data.get<float>(1, 0, x) / max_value;
						float b = data.get<float>(2, 0, x) / max_value;
						cm.add_color_point(t, cgv::rgb(r, g, b));
						t += step;
					}

					if(format.get_nr_components() == 4u) {
						t = 0.0f;
						for(size_t x = 0; x < format.get_width(); ++x) {
							cm.add_opacity_point(t, data.get<float>(3, 0, x) / max_value);
							t += step;
						}
					}
					break;
				default:
					break;
				}
			}

			entries.push_back({ name, cm });
		}

		return !entries.empty();
	}
};

}
}
