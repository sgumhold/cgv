#pragma once

#include <setjmp.h>
#include <stdio.h>
#include <jpeglib.h>

#include <cgv/media/image/image_writer.h>

using namespace cgv::media::image;

#include "lib_begin.h"

struct my_error_mgr
{
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

/// implements the image reader interface for jpg files
class jpg_writer : public abst_image_writer
{
protected:
	/* This struct contains the JPEG compression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 * It is possible to have several such structures, representing multiple
	 * compression/decompression processes, in existence at once.  We refer
	 * to any one struct (and its associated working data) as a "JPEG object".
	 */
	struct jpeg_compress_struct cinfo;
	my_error_mgr jerr;
	/* More stuff */
	FILE* outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	mutable std::string last_error;
	std::string file_name;
	unsigned quality;
public:
	/// default constructor
	jpg_writer();
	/// close file in destructor
	~jpg_writer();
	///
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// construct a copy of the reader
	abst_image_writer* clone() const;
	/// return a string containing a colon separated list of extensions that can be read with this reader
	const char* get_supported_extensions() const;
	/// check if the chosen writer supports the given component format
	bool is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats) const;
	/// return a reference to the last error message
	const std::string& get_last_error() const;
	/// open image file to write
	bool open(const std::string& file_name);
	/** write the data stored in the data view to a file with the file name given in the constructor. */
	bool write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes, double duration);
	/// close image [stream]
	bool close();
};

#include <cgv/config/lib_end.h>
