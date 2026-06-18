#include "transfer_function_writer.h"

//#include <string>
//
//#include <cgv/media/color.h>
//#include <cgv/media/image/image_writer.h>
//#include <cgv/media/transfer_function.h>
//#include <cgv/utils/file.h>
//
//#include <tinyxml2/tinyxml2.h>

namespace cgv {
namespace media {

void transfer_function_writer::to_xml_printer(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag) {
	std::vector<transfer_function_writer_entry> entries = { { name, transfer_function } };
	to_xml_printer(printer, entries, put_parent_tag);
}

void transfer_function_writer::to_xml_printer(tinyxml2::XMLPrinter& printer, const std::vector<transfer_function_writer_entry>& entries, bool put_parent_tag) {
	if(put_parent_tag)
		printer.OpenElement("ColorMaps");

	for(const auto& entry : entries)
		write_color_map(printer, entry.name, entry.transfer_function);

	if(put_parent_tag)
		printer.CloseElement();
}

std::string transfer_function_writer::to_xml(const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag) {
	std::vector<transfer_function_writer_entry> entries = { { name, transfer_function } };
	return to_xml(entries, put_parent_tag);
}

std::string transfer_function_writer::to_xml(const std::vector<transfer_function_writer_entry>& entries, bool put_parent_tag) {
	tinyxml2::XMLPrinter printer;
	to_xml_printer(printer, entries, put_parent_tag);
	std::string xml = printer.CStr();
	return xml;
}

bool transfer_function_writer::write_to_xml_file(const std::string& file_name, const std::string& name, const cgv::media::transfer_function* transfer_function, bool put_parent_tag) {
	std::vector<transfer_function_writer_entry> entries = { { name, transfer_function } };
	return write_to_xml_file(file_name, entries, put_parent_tag);
}

bool transfer_function_writer::write_to_xml_file(const std::string& file_name, const std::vector<transfer_function_writer_entry>& entries, bool put_parent_tag) {
	return cgv::utils::file::write(file_name, to_xml(entries, put_parent_tag), true);
}

bool transfer_function_writer::write_to_png_file(const std::string& file_name, const cgv::media::transfer_function* transfer_function, size_t resolution) {
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

void transfer_function_writer::write_color_map(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::media::transfer_function* transfer_function) {
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

} // namespace media
} // namespace cgv
