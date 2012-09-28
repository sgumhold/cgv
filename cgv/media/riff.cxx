#include "riff.h"

#pragma warning (disable:4996)

namespace cgv {
	namespace media {


unsigned fourcc::make_id(const std::string& ascii)
{
	std::string s = ascii;
	while (s.size() < 4)
		s += ' ';
	unsigned id = (unsigned&)(s[0]);
	return id;
}

std::string fourcc::to_string() const
{
	return std::string((char*)&id, 4);
}

bool fourcc::operator == (const std::string& s) const
{
	return id == make_id(s);
}

bool fourcc::operator != (const std::string& s) const
{
	return id != make_id(s);
}

std::ostream& operator << (std::ostream& os, const fourcc& f)
{
	return os << f.to_string();
}

bool riff_handler::begin_list_chunk(fourcc id, unsigned size, fourcc hdr) { return true; }
bool riff_handler::process_chunk_header(fourcc id, unsigned size, void*& data_ptr) { return true; }
void riff_handler::process_chunk_data(fourcc id, unsigned size, void* data_ptr) { }
void riff_handler::end_list_chunk(fourcc id, unsigned size, fourcc hdr) { }

riff_reader::riff_reader(riff_handler* _rh) : rh(_rh)
{
}

bool riff_reader::read_chunk_info(fourcc& id, unsigned& size)
{
	if (fread(&id, 1, 4, fp) != 4) {
		fclose(fp);
		return false;
	}
	if (fread(&size, 1, 4, fp) != 4) {
		fclose(fp);
		return false;
	}
	return true;
}

bool riff_reader::read_chunk_list(unsigned list_size)
{
	while (list_size > 0) {

		fourcc id;
		unsigned size;
		if (!read_chunk_info(id, size))
			return false;
		unsigned size_correct = size;
		if ((size & 1) == 1)
			size_correct += 1;
			
		list_size -= 8+size_correct;
		
		if (id == "LIST") {
			fourcc hdr;
			if (fread(&hdr, 1, 4, fp) != 4) {
				close();
				return false;
			}
			if (rh->begin_list_chunk(id, size, hdr)) {
				if (!read_chunk_list(size-4))
					return false;
				rh->end_list_chunk(id, size, hdr);
			}
			else {
				if (fseek(fp, size-4, SEEK_CUR) != 0) {
					close();
					return false;
				}
			}
		}
		else {
			void* data_ptr = 0;
			unsigned char* data;
			if (id != "JUNK" && rh->process_chunk_header(id, size, data_ptr)) {
				if (data_ptr == 0)
					data = new unsigned char [size];
				else
					data = (unsigned char*) data_ptr;
				
				if (fread(data, 1, size, fp) != size) {
					close();
					return false;
				}
				
				if (size_correct > size)
					fseek(fp, 1, SEEK_CUR);
				
				rh->process_chunk_data(id, size, data);
				
				if (data_ptr == 0)
					delete [] data;
			}
			else {
				if (fseek(fp, size, SEEK_CUR) != 0) {
					close();
					return false;
				}
			}
		}
	}
	return true;
}

void riff_reader::close()
{
	fclose(fp);
	fp = 0;
}

bool riff_reader::read(const std::string& file_name)
{
	if (!rh)
		return false;
		
	fp = fopen(file_name.c_str(), "rb");
	if (fp == 0)
		return false;

	fourcc id;
	unsigned size;
	if (!read_chunk_info(id, size))
		return false;
		
	if (id != "RIFF") {
		close();
		return false;
	}
	
	fourcc hdr;
	if (fread(&hdr, 1, 4, fp) != 4) {
		close();
		return false;
	}
	
	if (!rh->begin_list_chunk(id, size, hdr)) {
		close();
		return false;
	}
	
	if (read_chunk_list(size-4))
		close();
	
	rh->end_list_chunk(id, size, hdr);
	return true;
}

	}
}