#pragma once

#include <setjmp.h>
#include <stdio.h>
#include <jpeglib.h>

#include <cgv/media/image/image_reader.h>

using namespace cgv::media::image;


struct my_error_mgr 
{
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

#include "lib_begin.h"

/// implements the image reader interface for bmp files
class CGV_API jpg_reader : public abst_image_reader
{
protected:
	FILE* fp;
	struct jpeg_decompress_struct cinfo;
	my_error_mgr jerr;

	std::string last_error;
public:
	/// default constructor
	jpg_reader();
	/// close file in destructor
	~jpg_reader();
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// return the last error message
	const std::string& get_last_error() const;
	/// construct a copy of the reader
	abst_image_reader* clone() const;
	/// return a string containing a colon separated list of extensions that can be read with this reader
	const char* get_supported_extensions() const;
	/// open the file and read the image header in order to determine the data format
	bool open(const std::string& file_name, data_format& df, std::vector<data_format>* palette_formats);
	/// whether the reader supports per line reading (only valid after successful opening an image file
	bool supports_per_line_read() const;
	/// read the next line into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done
	bool read_line(const data_format& df, const data_view& dv);
	/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
	bool read_image(const data_format& df, const data_view& dv);
	/// close the image file
	bool close();
};

#include <cgv/config/lib_end.h>
