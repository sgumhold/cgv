#include "transfer_function_reader.h"

#include <cgv/media/color.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/utils/file.h>
#include <cgv_xml/query.h>

namespace cgv {
namespace media {

transfer_function_reader_result transfer_function_reader::read_from_xml(const tinyxml2::XMLElement& elem, const identifier_config& config) {
	transfer_function_reader_result result;
	result.entries = extract_color_maps(elem, config);
	result.success = !result.entries.empty();
	return result;
}

transfer_function_reader_result transfer_function_reader::read_from_xml(const tinyxml2::XMLDocument& doc, const identifier_config& config) {
	if(const tinyxml2::XMLElement* elem = doc.RootElement())
		return read_from_xml(*elem, config);
	return {};
}

transfer_function_reader_result transfer_function_reader::read_from_xml_string(const std::string& xml, const identifier_config& config) {
	tinyxml2::XMLDocument doc;
	if(doc.Parse(xml.c_str()) == tinyxml2::XML_SUCCESS)
		return read_from_xml(doc, config);
	return {};
}

transfer_function_reader_result transfer_function_reader::read_from_xml_file(const std::string& file_name, const identifier_config& config) {
	tinyxml2::XMLDocument doc;
	if(doc.LoadFile(file_name.c_str()) == tinyxml2::XML_SUCCESS)
		return read_from_xml(doc, config);
	return {};
}

transfer_function_reader_result transfer_function_reader::read_from_image_file(const std::string& file_name) {
	if(!cgv::utils::file::exists(file_name))
		return {};

	transfer_function_reader_result result;

	cgv::data::data_format format;
	cgv::data::data_view data;
	cgv::media::image::image_reader image_reader(format);

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

		auto transfer_function = std::make_shared<cgv::media::transfer_function>();
		float step = 1.0f / static_cast<float>(format.get_width() - 1);
		float t = 0.0f;

		if(format.get_width() > 0 && format.get_height() > 0) {
			switch(format.get_nr_components()) {
			case 1u:
				for(size_t x = 0; x < format.get_width(); ++x) {
					transfer_function->add_color_point(t, cgv::rgb(data.get<float>(0, 0, x) / max_value));
					t += step;
				}
				break;
			case 3u:
			case 4u:
				for(size_t x = 0; x < format.get_width(); ++x) {
					float r = data.get<float>(0, 0, x) / max_value;
					float g = data.get<float>(1, 0, x) / max_value;
					float b = data.get<float>(2, 0, x) / max_value;
					transfer_function->add_color_point(t, cgv::rgb(r, g, b));
					t += step;
				}

				if(format.get_nr_components() == 4u) {
					t = 0.0f;
					for(size_t x = 0; x < format.get_width(); ++x) {
						transfer_function->add_opacity_point(t, data.get<float>(3, 0, x) / max_value);
						t += step;
					}
				}
				break;
			default:
				break;
			}
		}

		const std::string name = cgv::utils::file::drop_extension(cgv::utils::file::get_file_name(file_name));
		result.entries.push_back({ name, transfer_function });
	}

	result.success = !result.entries.empty();
	return result;
}

void transfer_function_reader::extract_value(const tinyxml2::XMLElement& elem, const std::string& name, bool as_float, float& value) {
	if(as_float) {
		elem.QueryFloatAttribute(name.c_str(), &value);
	} else {
		int temp;
		if(elem.QueryIntAttribute(name.c_str(), &temp) == tinyxml2::XML_SUCCESS)
			value = static_cast<float>(temp) / 255.0f;
	}
}

transfer_function_reader::point_info transfer_function_reader::extract_control_point(const tinyxml2::XMLElement& elem, const identifier_config& config) {
	point_info pi;
	extract_value(elem, config.position_value_id, true, pi.x);
	extract_value(elem, config.opactiy_value_id, config.opacity_value_type_float, pi.o);
	extract_value(elem, config.red_value_id, config.color_value_type_float, pi.r);
	extract_value(elem, config.green_value_id, config.color_value_type_float, pi.g);
	extract_value(elem, config.blue_value_id, config.color_value_type_float, pi.b);
	return pi;
}

transfer_function_reader_result::entry transfer_function_reader::extract_color_map(const tinyxml2::XMLElement& elem, const identifier_config& config) {
	std::string name = elem.Attribute("name");
	auto transfer_function = std::make_shared<cgv::media::transfer_function>();

	auto child = elem.FirstChildElement();
	while(child) {
		if(strcmp(child->Name(), config.point_tag_id.c_str()) == 0 ||
			strcmp(child->Name(), config.color_point_tag_id.c_str()) == 0 ||
			strcmp(child->Name(), config.opacity_point_tag_id.c_str()) == 0) {
			point_info pi = extract_control_point(*child, config);

			if(!(pi.r < 0.0f || pi.g < 0.0f || pi.b < 0.0f)) {
				rgb col(0.0f);
				col[0] = std::min(pi.r, 1.0f);
				col[1] = std::min(pi.g, 1.0f);
				col[2] = std::min(pi.b, 1.0f);
				// apply gamma correction if requested
				if(config.apply_gamma)
					col = cgv::media::pow(col, 2.2f);
				transfer_function->add_color_point(pi.x, col);
			}

			if(!(pi.o < 0.0f)) {
				transfer_function->add_opacity_point(pi.x, cgv::math::clamp(pi.o, 0.0f, 1.0f));
			}
		}

		child = child->NextSiblingElement();
	}

	return { name, transfer_function };
}

std::vector<transfer_function_reader_result::entry> transfer_function_reader::extract_color_maps(const tinyxml2::XMLElement& elem, const identifier_config& config) {
	cgv::xml::FindElementByNameVisitor findElementByName("ColorMaps");
	elem.Accept(&findElementByName);

	std::vector<transfer_function_reader_result::entry> entries;

	if(auto color_maps_elem = findElementByName.Result()) {
		auto color_map_elem = color_maps_elem->FirstChildElement();

		while(color_map_elem) {
			if(strcmp(color_map_elem->Name(), config.color_map_tag_id.c_str()) == 0) {
				transfer_function_reader_result::entry entry = extract_color_map(*color_map_elem, config);
				if(!entry.transfer_function->empty())
					entries.push_back(entry);
			}
			color_map_elem = color_map_elem->NextSiblingElement();
		}
	}

	return entries;
}

} // namespace media
} // namespace cgv
