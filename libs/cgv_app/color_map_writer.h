#pragma once

#include <string>

#include <cgv/media/image/image_writer.h>
#include <cgv/render/color_map.h>
#include <cgv/utils/file.h>

#include <tinyxml2/tinyxml2.h>

namespace cgv {
namespace app {

class color_map_writer {
private:
	static void write_color_map(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::render::color_map& color_map) {

		printer.OpenElement("ColorMap");
		printer.PushAttribute("name", name.c_str());

		for(const auto& color_point : color_map.ref_color_points()) {
			printer.OpenElement("ColorPoint");
			printer.PushAttribute("x", color_point.first);
			printer.PushAttribute("r", color_point.second.R());
			printer.PushAttribute("g", color_point.second.G());
			printer.PushAttribute("b", color_point.second.B());
			printer.CloseElement();
		}

		for(const auto& opacity_point : color_map.ref_opacity_points()) {
			printer.OpenElement("OpacityPoint");
			printer.PushAttribute("x", opacity_point.first);
			printer.PushAttribute("o", opacity_point.second);
			printer.CloseElement();
		}
		
		printer.CloseElement();
	}

public:
	static void to_xml_printer(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::render::color_map& color_map, bool put_parent_tag = true) {

		std::vector<std::string> names = {name};
		std::vector<cgv::render::color_map> color_maps = {color_map};
		to_xml_printer(printer, names, color_maps, put_parent_tag);
	}

	static void to_xml_printer(tinyxml2::XMLPrinter& printer, const std::vector<std::string>& names, const std::vector<cgv::render::color_map>& color_maps, bool put_parent_tag = true) {

		if (put_parent_tag)
			printer.OpenElement("ColorMaps");

		if (names.size() == color_maps.size()) {
			for (size_t i = 0; i < names.size(); ++i)
				write_color_map(printer, names[i], color_maps[i]);
		}

		if (put_parent_tag)
			printer.CloseElement();
	}

	static std::string to_xml(const std::string& name, const cgv::render::color_map& color_map, bool put_parent_tag = true) {
		
		std::vector<std::string> names = { name };
		std::vector<cgv::render::color_map> color_maps = { color_map };

		return to_xml(names, color_maps, put_parent_tag);
	}

	static std::string to_xml(const std::vector<std::string>& names, const std::vector<cgv::render::color_map>& color_maps, bool put_parent_tag = true) {
		
		tinyxml2::XMLPrinter printer;
		to_xml_printer(printer, names, color_maps, put_parent_tag);
		std::string xml = printer.CStr();

		return xml;
	}

	static bool write_to_xml_file(const std::string& file_name, const std::string& name, const cgv::render::color_map& color_map, bool put_parent_tag = true) {
	
		std::vector<std::string> names = { name };
		std::vector<cgv::render::color_map> color_maps = { color_map };

		return write_to_xml_file(file_name, names, color_maps, put_parent_tag);
	}

	static bool write_to_xml_file(const std::string& file_name, const std::vector<std::string>& names, const std::vector<cgv::render::color_map>& color_maps, bool put_parent_tag = true) {

		return cgv::utils::file::write(file_name, to_xml(names, color_maps, put_parent_tag), true);
	}

	static bool write_to_png_file(const std::string& file_name, const cgv::render::color_map& color_map, size_t resolution) {
	
		std::vector<rgba> data = color_map.interpolate(static_cast<size_t>(resolution));

		bool has_opacity = !color_map.ref_opacity_points().empty();

		std::vector<uint8_t> data_8(4 * data.size());
		for(unsigned i = 0; i < data.size(); ++i) {
			rgba col = data[i];
			data_8[4 * i + 0] = static_cast<uint8_t>(255.0f * col.R());
			data_8[4 * i + 1] = static_cast<uint8_t>(255.0f * col.G());
			data_8[4 * i + 2] = static_cast<uint8_t>(255.0f * col.B());
			data_8[4 * i + 3] = has_opacity ? static_cast<uint8_t>(255.0f * col.alpha()) : 255;
		}

		cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format((unsigned)resolution, 1, TI_UINT8, cgv::data::CF_RGBA), data_8.data());

		cgv::media::image::image_writer writer(file_name);

		return writer.write_image(dv);
	}
};

}
}
