#include <cstdint>
#include <string>
#include <istream>
#include <cstdio>
#include <fstream>
#include "point_cloud.h"

namespace cgv {
namespace pointcloud {
namespace file_parser {
	
enum class e57_error_code { 
	FILE_TO_SMALL, 
	XML_ERROR,
	BAD_CHECKSUM,
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
	void read(const std::string& file_name); 

	e57_file_header read_header(const char* data, const size_t data_length);
};

//file with build in error correction, has logical and physical pages that are protected by checksums
class checked_file
{
  public:
	static constexpr int physical_page_size = 1 << 10;
	static constexpr int logical_page_size = physical_page_size-sizeof(uint32_t);

  private:
	std::unique_ptr<std::fstream> s_file;

	std::array<char, physical_page_size> physical_page_buffer;
	unsigned buffer_page_nr;

	size_t page_cursor;
	unsigned page_offset_cursor;

	size_t file_size;
  public:

	checked_file();

	void open(const char* filename, std::ios_base::openmode mode = std::fstream::binary | std::fstream::in | std::fstream::out);

	inline size_t page_nr(size_t physical_offset) {
		return physical_offset >> 10;
	}

	inline size_t page_offset(size_t page) {
		return page << 10;
	}

	// reads logical page in file, stores content in page_buffer, the page_buffer must have at least the size of a logical page
	bool read_page(char* page_buffer, const size_t page);

	// copies next physical page into phys_page_buffer
	bool peek_physical_page(char* phys_page_buffer);

	// move cursor
	void seek(size_t phys_offset);

	// start reading from current cursor position, returns number of bytes read
	size_t read(char* buffer, size_t bytes);
};

}}}