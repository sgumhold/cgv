#pragma once

#include <string>
#include <stdio.h>
#include <iostream>

#include "lib_begin.h"

namespace cgv {
	namespace media {

/// represents fourcc ids as used in the riff format to identify chunks
struct CGV_API fourcc
{
	static unsigned make_id(const std::string& ascii);
	/// store fourcc as 32 bit integer
	unsigned id;
	/// construct from unsigned integer
	fourcc(unsigned _id = 0) : id(_id) {}
	/// construct from string
	fourcc(const std::string& s) : id(make_id(s)) {}
	/// convert to string
	std::string to_string() const;
	/// compare with string
	bool operator == (const std::string& s) const;
	/// compare with string
	bool operator != (const std::string& s) const;
};

/// stream out fourcc structure
extern CGV_API std::ostream& operator << (std::ostream& os, const fourcc& f);

/// callback handler passed to riff reader
struct CGV_API riff_handler
{
	/// this is called when riff or list chunk is started, return whether to process list elements (default return is true)
	virtual bool begin_list_chunk(fourcc id, unsigned size, fourcc hdr);
	//! this is called before data of chunk is read, return whether to process chunk data (default return is true)
	/*! If the chunk data should be stored in memory allocated by the handler, set \c data_ptr to this memory block
	    that must provide space for \c size bytes. If \c data_ptr is not set, the reader automatically allocates 
		a new data block, which is also deallocated by the reader after the corresponding call to \c process_chunk_data. */
	virtual bool process_chunk_header(fourcc id, unsigned size, void*& data_ptr);
	/// if \c process_chunk_header returned \c true, this callback is called to process chunk data
	virtual void process_chunk_data(fourcc id, unsigned size, void* data_ptr);
	/// called to mark the end of a list chunk
	virtual void end_list_chunk(fourcc id, unsigned size, fourcc hdr);
};

/// reader class for riff files such as .ani or .avi
class CGV_API riff_reader
{
protected:
	FILE* fp;
	riff_handler* rh;
	bool read_chunk_info(fourcc& f, unsigned& size);
	bool read_chunk_list(unsigned size);
	void close();
public:
	/// construct from riff_handler
	riff_reader(riff_handler* _rh);
	/// read given file and generate callbacks to the riff_handler, return whether read was successful
	bool read(const std::string& file_name);
};

	}
}

#include <cgv/config/lib_end.h>