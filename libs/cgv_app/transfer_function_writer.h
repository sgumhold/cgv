#pragma once

#include <string>

#include <cgv/media/image/image_writer.h>
#include <cgv/media/transfer_function.h>
#include <cgv/utils/file.h>

#include <tinyxml2/tinyxml2.h>

namespace cgv {
namespace app {

class transfer_function_writer {
private:
	static void write_color_map(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::media::transfer_function* transfer_function) {

		printer.OpenElement("ColorMap");
		printer.PushAttribute("name", name.c_str());

		for(const auto& color_point : transfer_function->get_color_points()) {
			printer.OpenElement("ColorPoint");
			printer.PushAttribute("x", color_point.first);
			printer.PushAttribute("r", color_point.second.R());
			printer.PushAttribute("g", color_point.second.G());
			printer.PushAttribute("b", color_point.second.B());
			printer.CloseElement();
		}

		for(const auto& opacity_point : transfer_function->get_opacity_points()) {
			printer.OpenElement("OpacityPoint");
			printer.PushAttribute("x", opacity_point.first);
			printer.PushAttribute("o", opacity_point.second);
			printer.CloseElement();
		}
		
		printer.CloseElement();
	}

public:
	static void to_xml_printer(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag = true) {
		to_xml_printer(printer, { name }, { transfer_function }, put_parent_tag);
	}

	static void to_xml_printer(tinyxml2::XMLPrinter& printer, const std::vector<std::string>& names, const std::vector<const cgv::media::transfer_function*>& transfer_functions, bool put_parent_tag = true) {

		if (put_parent_tag)
			printer.OpenElement("ColorMaps");

		if (names.size() == transfer_functions.size()) {
			for (size_t i = 0; i < names.size(); ++i)
				write_color_map(printer, names[i], transfer_functions[i]);
		}

		if (put_parent_tag)
			printer.CloseElement();
	}

	static std::string to_xml(const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag = true) {
		return to_xml({ name }, { transfer_function }, put_parent_tag);
	}

	static std::string to_xml(const std::vector<std::string>& names, const std::vector<const cgv::media::transfer_function*>& transfer_functions, bool put_parent_tag = true) {
		tinyxml2::XMLPrinter printer;
		to_xml_printer(printer, names, transfer_functions, put_parent_tag);
		std::string xml = printer.CStr();

		return xml;
	}

	static bool write_to_xml_file(const std::string& file_name, const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag = true) {
		return write_to_xml_file(file_name, { name }, { transfer_function }, put_parent_tag);
	}

	static bool write_to_xml_file(const std::string& file_name, const std::vector<std::string>& names, const std::vector<const cgv::media::transfer_function*>& transfer_functions, bool put_parent_tag = true) {
		return cgv::utils::file::write(file_name, to_xml(names, transfer_functions, put_parent_tag), true);
	}

	static bool write_to_png_file(const std::string& file_name, const cgv::media::transfer_function* transfer_function, size_t resolution) {
		std::vector<rgba> data = transfer_function->quantize(resolution);

		std::vector<rgba8> data8;
		std::transform(data.begin(), data.end(), std::back_inserter(data8), [](const rgba& color) {
			return rgba8(color);
		});

		cgv::data::data_format data_format(resolution, 1, cgv::type::info::TI_UINT8, cgv::data::CF_RGBA);
		cgv::data::data_view data_view(&data_format, data8.data());
		cgv::media::image::image_writer writer(file_name);
		return writer.write_image(data_view);
	}
};

}
}
