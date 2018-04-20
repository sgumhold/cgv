#pragma once

#include <stdio.h>
#include <tiffconf.h>
#include <tiffio.h>

#include <cgv/media/image/image_writer.h>

using namespace cgv::media::image;

#include "lib_begin.h"

enum TiffCompression {
	TC_NONE,
	TC_LZW,
	TC_JPEG
};

/// implements the image reader interface for bmp files
class tiff_writer : public abst_image_writer
{
protected:
	TIFF *fp;
	bool allows_row_based;
	unsigned row;
	unsigned nr_images;
	TiffCompression tiff_compression;
	int  jpeg_quality;
	bool jpeg_raw_color;
	std::string last_error;
public:
	/// default constructor
	tiff_writer();
	/// close file in destructor
	~tiff_writer();
	/// reflect options
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// return the last error message
	const std::string& get_last_error() const;
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// construct a copy of the reader
	abst_image_writer* clone() const;
	/// return a string containing a colon separated list of extensions that can be read with this reader
	const char* get_supported_extensions() const;
	/// check if the chosen writer supports the given component format
	bool is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats) const;
	/// return whether multiple images are supported, default implementation returns false
	bool supports_multiple_images() const;
	/// open image file to write
	bool open(const std::string& file_name);
	/** write the data stored in the data view to a file with the file name given in the constructor. */
	bool write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes, double duration);
	/// close image [stream]
	bool close();
};

#include <cgv/config/lib_end.h>
