#include "image_reader.h"
#include <cgv/base/register.h>
#include <cgv/utils/scan.h>
#include <vector>

using namespace cgv::base;
using namespace cgv::utils;

namespace cgv {
	namespace media {
		namespace image {


/// whether the file can contain several images
bool abst_image_reader::supports_multiple_images() const
{
	return false;
}

/// return the number of images in the file, what can cause the whole file to be scanned
unsigned abst_image_reader::get_nr_images() const
{
	return 1;
}


/// return the duration of the current image in seconds
float abst_image_reader::get_image_duration() const
{
	return 0;
}

/// return the index of the current image
unsigned abst_image_reader::get_current_image() const
{
	return 0;
}

/// jump to a specific image and return whether this was successful
bool abst_image_reader::seek_image(unsigned idx)
{
	return idx == 0;
}

/// read the i-th palette in case of a paletted file format, the standard implementation returns false
bool abst_image_reader::read_palette(unsigned int, const data_view&)
{
	return false;
}

/// interfaces that allows to listen to registration events
struct reader_listener : public cgv::base::base, public cgv::base::registration_listener
{
	static std::vector<base_ptr>& ref_readers()
	{
		static std::vector<base_ptr> readers;
		return readers;
	}
	std::string get_type_name() const { return "image_reader_listener"; }

	void register_object(base_ptr object, const std::string&)
	{
		if (object->get_interface<abst_image_reader>())
			ref_readers().push_back(object);
	}
	void unregister_object(base_ptr object, const std::string&)
	{
		for (unsigned int i=0; i<ref_readers().size(); ++i) {
			if (object == ref_readers()[i]) {
				ref_readers().erase(ref_readers().begin()+i);
				++i;
			}
		}
	}
};

/// construct an image image_reader from a file name
image_reader::image_reader(data_format& file_format, std::vector<data_format>* _palette_formats) 
	: file_format_ptr(&file_format), palette_formats(_palette_formats), rd(0)
{
}

/// return a string with a list of supported extensions, where the list entries are separated with the passed character that defaults to a semicolon
const std::string& image_reader::get_supported_extensions(char sep)
{
	static std::string exts;
	exts.clear();
	std::vector<base_ptr>& readers = reader_listener::ref_readers();
	for (unsigned int i=0; i<readers.size(); ++i) {
		if (!exts.empty())
			exts += ';';
		exts += readers[i]->get_interface<abst_image_reader>()->get_supported_extensions();
	}
	if (sep != ';') 
		cgv::utils::replace(exts,';',sep);
	return exts;
}

///
std::string image_reader::construct_filter_string()
{
	std::string text = "Image Files (";
	std::string exts = "*.";
	text += get_supported_extensions(',');
	text += "):";
	exts += get_supported_extensions(',');
	replace(exts,",",";*.");
	return text+exts;
}

/// return a reference to the last error message
const std::string& image_reader::get_last_error() const
{
	if (!rd)
		return last_error;
	return rd->get_last_error();
}

/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
bool image_reader::read_image(const std::string& file_name, data_view& dv, std::vector<data_view> *palettes)
{
	if (!open(file_name) || file_format_ptr->empty())
		return false;
	if (!read_image(dv, palettes) || !close())
		return false;
	return true;
}

/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
bool image_reader::read_image(const std::string& file_name, const data_view& dv, const std::vector<data_view> *palettes)
{
	if (dv.empty() || !open(file_name) || file_format_ptr->empty())
		return false;
	if (!read_image(dv, palettes) || !close())
		return false;
	return true;
}

/// open the file and read the image header in order to determine the data format
bool image_reader::open(const std::string& file_name)
{
	if (!file_format_ptr)
		return false;
	std::string::size_type pos = file_name.find_last_of('.');
	if (pos == std::string::npos)
		return false;
	std::string ext = to_lower(file_name.substr(pos+1));
	std::vector<base_ptr>& readers = reader_listener::ref_readers();
	std::stringstream all_supported_extensions("");
	for (unsigned int i=0; i<readers.size(); ++i) {
		if (i != readers.size() - 1) {
            all_supported_extensions << ":";
		}
		const std::string &supported_extensions = readers[i]->get_interface<abst_image_reader>()->get_supported_extensions();
		all_supported_extensions << supported_extensions;
		if (cgv::utils::is_element(ext, supported_extensions)) {
			rd = readers[i]->get_interface<abst_image_reader>()->clone();
			return rd->open(file_name, *file_format_ptr, palette_formats);
		}
	}
	last_error = "could not find a suitable reader for " + file_name + " (supported formats are " +
				 all_supported_extensions.str() + ")";
	return false;
}

/// return the image info
data_format* image_reader::get_file_format() const
{
	return file_format_ptr;
}

/// whether the file can contain several images
bool image_reader::supports_multiple_images() const
{
	return false;
}

/// return the number of images in the file, what can cause the whole file to be scanned
unsigned image_reader::get_nr_images() const
{
	if (!rd)
		return 1;
	return rd->get_nr_images();
}

/// return the duration of the current image in seconds
float image_reader::get_image_duration() const
{
	if (!rd)
		return 0;
	return rd->get_image_duration();
}

/// return the index of the current image
unsigned image_reader::get_current_image() const
{
	if (!rd)
		return 0;
	return rd->get_current_image();
}

/// jump to a specific image and return whether this was successful
bool image_reader::seek_image(unsigned idx)
{
	if (!rd)
		return idx == 0;
	return rd->seek_image(idx);
}

/// read the i-th palette in case of a paletted file format, the standard implementation returns false
bool image_reader::read_palette(unsigned int i, data_view& dv)
{
	if (!rd || !palette_formats || i > palette_formats->size())
		return false;
	if (dv.empty())
		new(&dv) data_view(&palette_formats->at(i));
	return rd->read_palette(i,dv);
}

/// read the i-th palette in case of a paletted file format, the standard implementation returns false
bool image_reader::read_palette(unsigned int i, const data_view& dv)
{
	if (!rd || !palette_formats || i > palette_formats->size() || dv.empty())
		return false;
	return rd->read_palette(i,dv);
}

/// whether the image_reader supports per line reading (only valid after successful opening an image file
bool image_reader::supports_per_line_read() const
{
	if (!rd)
		return false;
	return rd->supports_per_line_read();
}

/// read the next line into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done
bool image_reader::read_line(data_view& dv)
{
	if (!rd)
		return false;
	if (dv.empty()) {
		data_view dv_all(file_format_ptr,0);
		dv = dv_all(0);
		dv.set_ptr(new unsigned char[file_format_ptr->get_width()*
											  file_format_ptr->get_entry_size()], true);
	}
	return rd->read_line(*file_format_ptr, dv);
}
/// read the next line into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done
bool image_reader::read_line(const data_view& dv)
{
	if (!rd || dv.empty())
		return false;
	return rd->read_line(*file_format_ptr, dv);
}

/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
bool image_reader::read_image(data_view& dv, std::vector<data_view> *palettes)
{
	if (!rd)
		return false;
	if (dv.empty())
		new(&dv) data_view(file_format_ptr);

	if (palette_formats && palettes) {
		for (unsigned i=0; i<palette_formats->size(); ++i) {
			palettes->push_back(data_view());
			if (!read_palette(i, palettes->back()))
				return false;
		}
	}
	return rd->read_image(*file_format_ptr, dv);
}

/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
bool image_reader::read_image(const data_view& dv, const std::vector<data_view> *palettes)
{
	if (!rd || dv.empty())
		return false;

	if (palette_formats) {
		if (!palettes)
			return false;
		if (palettes->size() != palette_formats->size())
			return false;
		for (unsigned i=0; i<palette_formats->size(); ++i) {
			if (!read_palette(i, palettes->at(i)))
				return false;
		}
	}
	return rd->read_image(*file_format_ptr, dv);
}

/// close the image file
bool image_reader::close()
{
	if (!rd)
		return false;
	return rd->close();
}

/// overload to return the type name of this object
std::string image_reader::get_type_name() const
{
	if (rd)
		return rd->get_type_name();
	return "cmi::image_reader";
}
/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
std::string image_reader::get_property_declarations()
{
	if (rd)
		return rd->get_property_declarations();
	return cgv::base::base::get_property_declarations();
}

/// abstract interface for the setter, by default it simply returns false
bool image_reader::set_void(const std::string& property, const std::string& type, const void* value)
{
	if (rd)
		return rd->set_void(property, type, value);
	return cgv::base::base::set_void(property, type, value);
}
/// abstract interface for the getter, by default it simply returns false
bool image_reader::get_void(const std::string& property, const std::string& type, void* value)
{
	if (rd)
		return rd->get_void(property, type, value);
	return cgv::base::base::get_void(property, type, value);
}

object_registration<reader_listener> rlr("register image reader registry");

		}
	}
}
