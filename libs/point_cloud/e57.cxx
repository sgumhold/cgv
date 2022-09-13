#include "e57.h"
#include "xml.h"
#include <cgv/utils/xml.h>
#include <regex>
namespace {

// check if machine uses the big endian byte order
bool is_big_endian()
{
	uint16_t test = 0xFF00;
	uint8_t* first_byte = (uint8_t*)&test;
	return *first_byte;
}

bool get_tag(const std::string& xml_data, int offset, int& start, int& end)
{
	start = offset;
	end = -1;
	while (start < xml_data.size() && xml_data[start] != '<')
		++start;

	int i = start;
	while (i < xml_data.size() && xml_data[i] != '>')
		++i;
	end = i;
	return i < xml_data.size();
}

} // namespace
namespace cgv {
namespace pointcloud {
namespace file_parser {



void e57_data_set::read(const std::string& file_content) {

	this->header = read_header(file_content.data(), file_content.size());
	
	if (this->header.xml_physical_offset > file_content.size()) {
		throw e57_parsing_error(e57_error_code::FILE_TO_SMALL, "can't get xml data, file is to small for offset and length in header file");
	}

	std::string xml_content =
		  file_content.substr(this->header.xml_physical_offset, file_content.size() - this->header.xml_physical_offset);

	std::unique_ptr<xml_node> xml_root = read_xml(xml_content);
	//TODO interpret xml tree
}

e57_file_header cgv::pointcloud::file_parser::e57_data_set::read_header(const char* data, const size_t data_length)
{
	if (data_length < sizeof(e57_file_header))
		throw e57_parsing_error(e57_error_code::FILE_TO_SMALL, "can't parse header, file is to small");

	if (is_big_endian()) {
		throw e57_parsing_error(e57_error_code::UNSUPPORTED_OPERATION, "not implemented for big endian machines");
	}

	e57_file_header header;
	memcpy(&header, data, sizeof(e57_file_header));
	return header;
}

cgv::pointcloud::file_parser::e57_parsing_error::e57_parsing_error(const e57_error_code ec,
																   const std::string m) noexcept
	: err_code(ec), msg(m)
{
}

} // namespace file_parser
} // namespace pointcloud
} // namespace cgv