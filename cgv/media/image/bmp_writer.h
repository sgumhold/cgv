#pragma once

#include "image_writer.h"
#include <stdio.h>
#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace image {

/// implements the image writer interface for bmp files
class CGV_API bmp_writer : public abst_image_writer
{
protected:
	std::string last_error;
	FILE* fp;
public:
	/// default constructor
	bmp_writer();
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// construct a copy of the reader
	abst_image_writer* clone() const;
	/// return a string containing a colon separated list of extensions that can be read with this reader
	const char* get_supported_extensions() const;
	/// check if the chosen writer supports the given component format
	bool is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats) const;
	/// return a colon separated list of supported options
	std::string get_options() const;
	/// return a reference to the last error message
	const std::string& get_last_error() const;
	/// open image file to write
	bool open(const std::string& file_name);
	/** write the data stored in the data view to a file with the file name given in the constructor. */
	bool write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes, double duration);
	/// close image [stream]
	bool close();
};

		}
	}
}

#include <cgv/config/lib_end.h>
