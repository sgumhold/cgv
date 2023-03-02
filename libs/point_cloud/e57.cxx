#include "e57.h"
#include "xml.h"
#include "crc.h"
#include <regex>
#include <limits>
#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <functional>

namespace {

// check if machine uses the big endian byte order
bool is_big_endian()
{
	uint16_t test = 0xFF00;
	uint8_t* first_byte = (uint8_t*)&test;
	return *first_byte;
}

void reverse_byte_order_32(uint32_t& b)
{
	b = (b & 0xFFFF0000) >> 16 | (b & 0x0000FFFF) << 16;
	b = (b & 0xFF00FF00) >> 8 | (b & 0x00FF00FF) << 8;
}

void reverse_byte_order_16(uint16_t& b)
{
	b = (b & 0xFF00) >> 8 | (b & 0x00FF) << 8;
}




} // namespace
namespace cgv {
namespace pointcloud {
namespace file_parser {

namespace {

class tag_finder
{
	std::string tag_name;

public:
	tag_finder(const std::string& name) : tag_name(name) {}

	bool operator()(xml_node* node)
	{
		return (xml_tag_node*)node != nullptr && ((xml_tag_node*)node)->tag.name == tag_name;
	}
};
//finds the first xml tag for that pred returns true
template <typename Iterator, typename UnaryPredicate>
xml_tag_node* find_tag_node_if(Iterator begin, Iterator end, UnaryPredicate pred)
{
	auto it = std::find_if(begin, end, pred);
	if (it != end) {
		return dynamic_cast<xml_tag_node*>(*it);
	}
	return nullptr;
}

template <typename Container, typename UnaryPredicate> xml_tag_node* find_tag_node_if(Container c, UnaryPredicate pred)
{
	return find_tag_node_if(c.begin(), c.end(), pred);
}

//maps types to channel setup code
const std::unordered_map<std::string, std::function<e57::data_channel(const e57::structure_element& element_ptr)>> type_parser_map{
	{"Float", [](const e57::structure_element& element) {
		size_t float_size = 4;
		auto precision_attr = element.node_ptr->tag.attributes.find("precision");
		if (precision_attr != element.node_ptr->tag.attributes.end()) {
			if (precision_attr->second == "double") {
				return e57::data_channel(e57::data_type::DOUBLE);
			}
			else if (precision_attr->second == "single") {
				return e57::data_channel(e57::data_type::FLOAT);
			}
			else {
				std::cerr << "e57_data_set::read ignored unknown precision " << precision_attr->second
							<< " using default precision for" << element.name << "\n";
			}
		}
		return e57::data_channel(e57::data_type::FLOAT);
	}},
	{"Integer", [](const e57::structure_element& element) {
		return e57::data_channel(e57::data_type::INT);
	}}};
} // namespace



void e57_data_set::read(const std::string& file_name)
{
	// e57 files are protected by crc checksums
	e57::checked_file file;
	file.open(file_name.c_str(), std::fstream::binary | std::fstream::in);
	
	//read the header
	std::array<char, e57::checked_file::logical_page_size> header_page;
	if (!file.read_page(header_page.data(), 0)) {
		throw e57_parsing_error(e57_error_code::FILE_TO_SMALL,
								"can't read header");
	}

	this->header = read_header(header_page.data(), e57::checked_file::logical_page_size);
	
	
	// get the xml data
	std::vector<char> xml_content(this->header.xml_logical_length+1,'\0');
	file.seek(this->header.xml_physical_offset);
	if (!file.read(xml_content.data(), this->header.xml_logical_length)) {
		throw e57_parsing_error(e57_error_code::FILE_TO_SMALL,
								"can't get xml data, file is to small for offset and length in header file");
	}


	std::string xml_content_str(xml_content.data());
	//std::cout << xml_content_str;
	std::unique_ptr<xml_tag_node> xml_root = read_xml(xml_content_str);

	//interpret xml tree
	if (xml_root->tag.name != "e57Root") {
		// not critical, wrong name for root tag 
	}
	
	// find the 3d point data
	xml_tag_node* node_data_3d = find_tag_node_if(xml_root->childs, tag_finder("data3D"));
	if (node_data_3d) {
		//has 3d data
		xml_tag_node* vector_child =
			  find_tag_node_if(node_data_3d->childs, tag_finder("vectorChild"));
		if (vector_child) {
			xml_tag_node* points = find_tag_node_if(vector_child->childs, tag_finder("points"));
			if (points == nullptr) {
				throw e57_parsing_error(e57_error_code::XML_STRUCTURE_ERROR, "missing points tag");
			}
			std::string type = points->tag.attributes["type"];
			uint64_t offset = stoul(points->tag.attributes["fileOffset"]);
			uint64_t record_count = stoul(points->tag.attributes["recordCount"]);

			if (type == "CompressedVector") {
				xml_tag_node* prototype_xml_node = find_tag_node_if(points->childs, tag_finder("prototype"));
				if (prototype_xml_node == nullptr) {
					throw e57_parsing_error(e57_error_code::XML_STRUCTURE_ERROR, "missing prototype tag");
				}
				e57::structure_node_adapter prototype(prototype_xml_node);
				// read compressed vector header

				file.seek(offset);
				e57::compressed_vector_section_header section_header;
				file.read(&section_header, sizeof(e57::compressed_vector_section_header));

				// setup data structures based on prototype

				std::unordered_map<std::string, e57::data_channel> channels; // name -> vector,type
				// create channels
				for (auto& e : prototype.members()) {
					auto tfm_it = type_parser_map.find(e.type);
					if (tfm_it != type_parser_map.end()) {
						channels.emplace(e.name, type_parser_map.at(e.type)(e));
					}
					else  {
						std::cerr << "e57_data_set::read ignored unknown type " << e.type << "\n";	
					}
				}

				//TODO read packets
			}

		}
	}
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

namespace e57 {

checked_file::checked_file() : s_file(nullptr) {}

void checked_file::open(const char* filename, std::ios_base::openmode mode)
{
	if (s_file) {
		if (s_file->is_open())
			s_file->close();
	}
	s_file = std::make_unique<std::fstream>(filename, mode);
	file_size = s_file->tellg();
	page_offset_cursor = 0;
	page_cursor = 0;
}

bool checked_file::read_page(char* page_buffer, const size_t page)
{
	bool good = false;
	if (s_file) {
		// reposition cursor if needed
		std::streamsize start = s_file->tellg();
		if (start != page_offset(page)) {
			s_file->seekg(page_offset(page));
		}
		// read physical page
		good = peek_physical_page((char*)physical_page_buffer.data());
		buffer_page_nr = page;
		// copy to buffer
		memcpy(page_buffer, physical_page_buffer.data(), logical_page_size);

		// set cursor to next page
		page_cursor = page + 1;
		page_offset_cursor = 0;
	}
	return good;
}

bool checked_file::peek_physical_page(char* phys_page_buffer)
{
	bool good = false;
	if (s_file) {
		s_file->read((char*)phys_page_buffer, physical_page_size);
		// size_t read_bytes = s_file->tellg() - start;
		good = (bool)*s_file;
		uint32_t crc_sum = crc32<0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF>(physical_page_buffer.data(), logical_page_size);
		uint32_t stored_sum = *reinterpret_cast<uint32_t*>(physical_page_buffer.data() + logical_page_size);
		reverse_byte_order_32(stored_sum); // checksum is stored with big endian byteorder
		if (stored_sum != crc_sum) {
			std::stringstream ss;
			ss << "bad checksum for page " << page_cursor;
			throw e57_parsing_error(e57_error_code::BAD_CHECKSUM, ss.str());
		}
	}
	return good;
}

void checked_file::seek(size_t phys_offset)
{
	page_cursor = page_nr(phys_offset);
	page_offset_cursor = phys_offset - page_offset(page_cursor);
	assert(page_offset_cursor < logical_page_size);
}

size_t checked_file::read(char* buffer, size_t bytes)
{
	size_t logical_bytes_read = 0;
	while (0 < bytes) {
		if (buffer_page_nr != page_cursor) {
			std::streamsize start = s_file->tellg();
			if (start != page_offset(page_cursor)) {
				s_file->seekg(page_offset(page_cursor));
			}
			bool good = peek_physical_page(physical_page_buffer.data());
			buffer_page_nr = page_cursor;
		}
		size_t page_bytes = std::min<size_t>(bytes, logical_page_size - page_offset_cursor);
		memcpy(buffer, physical_page_buffer.data() + page_offset_cursor, page_bytes);
		bytes -= page_bytes;
		buffer += page_bytes;
		logical_bytes_read += page_bytes;
		if ((page_offset_cursor + page_bytes) >= logical_page_size) {
			page_offset_cursor = 0;
			page_cursor += 1;
		}
	}
	return logical_bytes_read;
}

size_t checked_file::read(void* buffer, size_t bytes)
{
	return read((char*)buffer, bytes);
}

compressed_vector_section_header::compressed_vector_section_header()
{
	memset(this, 0, sizeof(compressed_vector_section_header));
}


structure_node_adapter::structure_node_adapter(xml_tag_node* xml)
{
	if (xml) {
		parse_xml_node(xml);
	}
}

const std::vector<structure_element>& structure_node_adapter::members()
{
	return elements_cache;
}

void structure_node_adapter::parse_xml_node(xml_tag_node* node)
{
	this->elements_cache.clear();
	this->struct_node_ptr = node;

	for (auto& ch : node->childs) {
		xml_tag_node* struct_member = dynamic_cast<xml_tag_node*>(ch);
		if (struct_member) {
			elements_cache.emplace_back();
			structure_element& element = elements_cache.back();
			auto& tag_attrs = struct_member->tag.attributes;
			element.name = struct_member->tag.name;
			element.type = tag_attrs["type"];
			element.node_ptr = struct_member;
			if (ch->childs.size() == 1 && (xml_string_node*)ch->childs[0]) {
				element.value = ((xml_string_node*)ch->childs[0])->content;
			}
		}
	}
}
structure_element::structure_element() : node_ptr(nullptr) {}
data_channel::data_channel(e57::data_type type) : type(type) {
	switch (type) {
	case e57::data_type::FLOAT:
		element_size = 4;
		vector_ptr = new std::vector<float>();
		break;
	case e57::data_type::INT:
		element_size = 4;
		vector_ptr = new std::vector<int32_t>();
		break;
	case e57::data_type::DOUBLE:
		element_size = 8;
		vector_ptr = new std::vector<double>();
		break;
	default:
		element_size = 0;
	}
}

} // namespace e57
} // namespace file_parser
} // namespace pointcloud
} // namespace cgv