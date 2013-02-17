#pragma once

#include <cgv/base/base.h>
#include <cgv/data/data_view.h>

#include "lib_begin.h"

using namespace cgv::data;

namespace cgv {
	namespace media {
		namespace image {

class CGV_API image_writer;

/// abstract interface for image readers
class CGV_API abst_image_writer : public cgv::base::base
{
public:
	friend class image_writer;
	/// construct a copy of the reader
	virtual abst_image_writer* clone() const = 0;
	/// return a string containing a colon separated list of extensions that can be read with this reader
	virtual const char* get_supported_extensions() const = 0;
	/// check if the chosen writer supports the given component format
	virtual bool is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats) const = 0;
	/// return a reference to the last error message
	virtual const std::string& get_last_error() const = 0;
	/// return whether multiple images are supported, default implementation returns false
	virtual bool supports_multiple_images() const;
	/// open image file to write
	virtual bool open(const std::string& file_name) = 0;
	/// write one image
	virtual bool write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes, double duration) = 0;
	/// close image [stream]
	virtual bool close() = 0;
};

/// the image writer chooses a specific writer automatically based on the extension of the given file name 
class CGV_API image_writer : public cgv::base::base
{
protected:
	/// store a pointer to the chosen reader
	abst_image_writer* wr;
	/// whether file has been opened
	bool is_opened;
	/// store the file name
	std::string file_name;
	/// store the last error not resulting from writer implementations
	std::string last_error;
	/// abstract interface for the setter, by default it simply returns false
	bool set_void(const std::string& property, const std::string& type, const void* value);
	/// abstract interface for the getter, by default it simply returns false
	bool get_void(const std::string& property, const std::string& type, void* value);
public:
	/// construct an image writer from a file name and choose a writer from the extension
	image_writer(const std::string& file_name);
	/// close file on destruction
	~image_writer();
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// return a string with a list of supported extensions, where the list entries are separated with the passed character that defaults to a semicolon
	static const std::string& get_supported_extensions(char sep = ';');
	/// use this to generate a file_open_dialog or file_save_dialog
	static std::string construct_filter_string();
	/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
	std::string get_property_declarations();
	/// check if the chosen writer supports the given component format
	bool is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats = 0) const;
	/// return a colon separated list of supported options
	std::string get_options() const;
	/// return a reference to the last error message
	const std::string& get_last_error() const;
	/** write the data stored in the data view to a file with the file name given in the constructor. 
	    In case a vector of data views for palettes is given, write the image in paletted format. */
	bool write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes = 0, double duration = 0);
	/// close image file;
	bool close();
	/// return whether multiple images are supported, default implementation returns false
	bool supports_multiple_images() const;
};

		}
	}
}

#include <cgv/config/lib_end.h>
