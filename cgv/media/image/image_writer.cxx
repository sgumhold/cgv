#include "image_writer.h"
#include <cgv/base/register.h>
#include <cgv/utils/scan.h>
#include <vector>

using namespace cgv::base;
using namespace cgv::utils;

namespace cgv {
	namespace media {
		namespace image {

/// interfaces that allows to listen to registration events
struct writer_listener : public cgv::base::base, public cgv::base::server, public cgv::base::registration_listener
{
	static std::vector<base_ptr>& ref_writers()
	{
		static std::vector<base_ptr> writers;
		return writers;
	}
	std::string get_type_name() const 
	{
		return "image_writer_listener";
	}
	void register_object(base_ptr object, const std::string&)
	{
		if (object->get_interface<abst_image_writer>())
			ref_writers().push_back(object);
	}
	void unregister_object(base_ptr object, const std::string&)
	{
		for (unsigned int i=0; i<ref_writers().size(); ++i) {
			if (object == ref_writers()[i]) {
				ref_writers().erase(ref_writers().begin()+i);
				++i;
			}
		}
	}
};

/// return whether multiple images are supported, default implementation returns false
bool abst_image_writer::supports_multiple_images() const
{
	return false;
}

/// construct an image image_writer from a file name and choose a image_writer from the extension
image_writer::image_writer(const std::string& _file_name) : file_name(_file_name)
{
	is_opened = false;
	std::string::size_type pos = file_name.find_last_of('.');
	if (pos == std::string::npos) {
		last_error = "file name without extension specified";
		return;
	}
	std::string ext = to_lower(file_name.substr(pos+1));
	std::vector<base_ptr>& writers = writer_listener::ref_writers();
	for (unsigned int i=0; i<writers.size(); ++i) {
		abst_image_writer* aiw = writers[i]->get_interface<abst_image_writer>();
		if (cgv::utils::is_element(ext, aiw->get_supported_extensions())) {
			wr = aiw->clone();
			return;
		}
	}
	wr = 0;
	last_error  = "no image image_writer found for extension: ";
	last_error += ext;
}

/// return a string with a list of supported extensions, where the list entries are separated with the passed character that defaults to a semicolon
const std::string& image_writer::get_supported_extensions(char sep)
{
	static std::string exts;
	exts.clear();
	std::vector<base_ptr>& writers = writer_listener::ref_writers();
	for (unsigned int i=0; i<writers.size(); ++i) {
		if (!exts.empty())
			exts += ';';
		exts += writers[i]->get_interface<abst_image_writer>()->get_supported_extensions();
	}
	if (sep != ';') 
		cgv::utils::replace(exts,';',sep);
	return exts;
}

///
std::string image_writer::construct_filter_string()
{
	std::string text = "Image Files (";
	std::string exts = "*.";
	text += get_supported_extensions(',');
	text += "):";
	exts += get_supported_extensions(',');
	replace(exts,",",";*.");
	return text+exts;
}

/// check if the chosen image_writer supports the given component format
bool image_writer::is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats) const
{
	if (wr)
		return wr->is_format_supported(cf, palette_formats);
	return false;
}

/// return a reference to the last error message
const std::string& image_writer::get_last_error() const
{
	if (!wr)
		return last_error;
	return wr->get_last_error();
}

/// close file on destruction
image_writer::~image_writer()
{
	close();
}

/// close image file;
bool image_writer::close()
{
	bool res = false;
	if (is_opened && wr)
		res = wr->close();
	is_opened = false;
	return res;
}

/** write the data stored in the data view to a file with the file name given in the constructor. */
bool image_writer::write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes, double duration)
{
	if (wr) {
		if (!is_opened) {
			if (!wr->open(file_name))
				return false;
			is_opened = true;
		}
		return wr->write_image(dv, palettes, duration);
	}
	return false;
}


/// return whether multiple images are supported, default implementation returns false
bool image_writer::supports_multiple_images() const
{
	if (wr)
		return wr->supports_multiple_images();
	return false;
}

/// overload to return the type name of this object
std::string image_writer::get_type_name() const
{
	if (wr)
		return wr->get_type_name();
	return "cmi::image_writer";
}
/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
std::string image_writer::get_property_declarations()
{
	if (wr)
		return wr->get_property_declarations();
	return cgv::base::base::get_property_declarations();
}

/// abstract interface for the setter, by default it simply returns false
bool image_writer::set_void(const std::string& property, const std::string& type, const void* value)
{
	if (wr)
		return wr->set_void(property, type, value);
	return cgv::base::base::set_void(property, type, value);
}
/// abstract interface for the getter, by default it simply returns false
bool image_writer::get_void(const std::string& property, const std::string& type, void* value)
{
	if (wr)
		return wr->get_void(property, type, value);
	return cgv::base::base::get_void(property, type, value);
}


object_registration<writer_listener> wlr("register image writer listener");

		}
	}
}