#include "e57.h"
#include "xml.h"
#include <cgv/utils/xml.h>
#include <regex>
#include <cstdint>
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

std::array<uint32_t,256> generate_crc32_table(uint32_t trunc_polynomial)
{
	std::array<uint32_t, 256> table;
	const uint32_t mask = (1 << (32 - 1));

	for (int32_t i = 0; i < table.size(); i++) {
		uint32_t crc = i << 24;
		for (uint8_t bit = 0; bit < 8; bit++) {
			if (crc & mask) // check for msb
				crc = (crc << 1) ^ trunc_polynomial;
			else
				crc <<= 1;
		}
		table[i] = crc;
	}
	return table;
}

uint32_t crc32(const uint8_t* data, size_t data_length)
{
	std::array<uint32_t, 256> crc32_table = generate_crc32_table(0x1EDC6F41);
	static constexpr uint32_t final_xor = 0xFFFFFFFFu;

	uint32_t crc32 = 0xFFFFFFFFu;

	for (size_t i = 0; i < data_length; i++) {
		const uint32_t ix = (crc32 ^ data[i]) & 0xff;
		crc32 = (crc32 >> 8) ^ crc32_table[ix];
	}

	crc32 ^= final_xor;
	return crc32;
}

} // namespace
namespace cgv {
namespace pointcloud {
namespace file_parser {



void e57_data_set::read(const std::string& file_name)
{

	checked_file file;
	file.open(file_name.c_str(), std::fstream::binary | std::fstream::in);
	
	std::array<char, checked_file::logical_page_size> header_page;
	if (!file.read_physical_page(header_page.data(), 0)) {
		throw e57_parsing_error(e57_error_code::FILE_TO_SMALL,
								"can't read header");
	}

	this->header = read_header(header_page.data(), checked_file::logical_page_size);
	
	/*

	if (this->header.xml_physical_offset > file_content.size()) {
		throw e57_parsing_error(e57_error_code::FILE_TO_SMALL, "can't get xml data, file is to small for offset and length in header file");
	}

	
	std::string xml_content =
		  file_content.substr(this->header.xml_physical_offset, this->header.xml_logical_length);
	


	std::unique_ptr<xml_node> xml_root = read_xml(xml_content);
	*/
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

checked_file::checked_file() : s_file(nullptr){}

void checked_file::open(const char* filename, std::ios_base::openmode mode)
{
	if (s_file) {
		if (s_file->is_open())
			s_file->close();
	}
	s_file = std::make_unique<std::fstream>(filename, mode);
}

bool checked_file::read_physical_page(char* page_buffer, const size_t page) {
	bool good = false;
	if (s_file) {
		std::streamsize start = s_file->tellg();
		if (start != page_offset(page)) {
			s_file->seekg(page_offset(page));
		}

		s_file->read((char*)physical_page_buffer.data(), physical_page_size);
		//size_t read_bytes = s_file->tellg() - start;
		good = (bool)*s_file;
		memcpy(page_buffer, physical_page_buffer.data(), logical_page_size);
		uint32_t crc_sum = crc32(physical_page_buffer.data(), logical_page_size);
		uint32_t stored_sum = *reinterpret_cast<uint32_t*>(&page_buffer[logical_page_size]);
		if (stored_sum != crc_sum) {
			std::stringstream ss;
			ss << "bad checksum for page beginning at " << start;
			throw e57_parsing_error(e57_error_code::BAD_CHECKSUM, ss.str());
		}
	}
	return good;
}

} // namespace file_parser
} // namespace pointcloud
} // namespace cgv