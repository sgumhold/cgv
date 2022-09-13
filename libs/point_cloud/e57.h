#include <cstdint>
#include <string>
#include <istream>
#include "point_cloud.h"

namespace cgv {
namespace pointcloud {
namespace file_parser {
	
enum class e57_error_code { 
	FILE_TO_SMALL, 
	XML_ERROR,
	UNSUPPORTED_OPERATION
};

enum class e57_data_type {
	FLOAT,
	INTEGER,
	STRING,
	VECTOR,
	STRUCTURE
};

class e57_node
{
public:
	virtual e57_data_type type() = 0;
};

//e57 header structure
struct e57_file_header
{
	char file_signature[8];
	uint32_t major_version;
	uint32_t minor_version;
	uint64_t file_physical_length;
	uint64_t xml_physical_offset;
	uint64_t xml_logical_length;
	uint64_t page_size;
};


struct e57_parsing_error
{
	std::string msg;
	e57_error_code err_code;

	e57_parsing_error(const e57_error_code ec, const std::string m) noexcept;
};

class e57_data_set
{
	e57_file_header header;

public:
	void read(const std::string& file_content); 

	e57_file_header read_header(const char* data, const size_t data_length);
};

}}}