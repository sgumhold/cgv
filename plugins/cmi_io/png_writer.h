#pragma once

#include <stdio.h>
#include <png.h>
#include <zlib.h>

#include <cgv/media/image/image_writer.h>

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#define png_voidp_NULL (void*)NULL
#define png_error_ptr_NULL (png_error_ptr)NULL

using namespace cgv::media::image;

#include "lib_begin.h"

/// implements the image reader interface for bmp files
class png_writer : public abst_image_writer
{
protected:
	FILE* fp;
	png_structp png_ptr;
	png_infop info_ptr;
	bool interlacing;
	mutable std::string last_error;
	std::string file_name;
	unsigned compression_level;
	bool determine_params(const component_format& cf, int& color_type, int& bit_depth) const;
public:
	/// default constructor
	png_writer();
	/// close file in destructor
	~png_writer();
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
