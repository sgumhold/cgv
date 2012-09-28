#include "video_writer.h"
#include <cgv/base/register.h>
#include <cgv/utils/scan.h>
#include <vector>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::utils;

namespace cgv {
	namespace media {
		namespace video {


/// dummy implementation provided that does nothing and returns false
bool abst_video_writer::scan_codecs(std::vector<std::string>&) const
{
	return false;
}

/// dummy empty implementation provided that does nothing
bool abst_video_writer::set_codec(const std::string&)
{
	return false;
}

/// return the currently selected codec, default implementation returns empty string
std::string abst_video_writer::get_codec() const
{
	return "";
}

/// interfaces that allows to listen to registration events
struct video_writer_listener : public cgv::base::base, public cgv::base::registration_listener
{
	static std::vector<abst_video_writer*>& ref_writers()
	{
		static std::vector<abst_video_writer*> writers;
		return writers;
	}
	std::string get_type_name() const 
	{
		return "video_writer_listener";
	}
	void register_object(base_ptr object, const std::string&)
	{
		abst_video_writer* rd = object->get_interface<abst_video_writer>();
		if (rd)
			ref_writers().push_back(rd);
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

/** return a string with a list of supported extensions, where the list entries are separated 
    with the passed character that defaults to a semicolon */
const std::string& video_writer::get_supported_extensions(char sep)
{
	static std::string exts;
	exts.clear();
	std::vector<abst_video_writer*>& writers = video_writer_listener::ref_writers();
	for (unsigned int i=0; i<writers.size(); ++i) {
		if (!exts.empty())
			exts += ';';
		exts += writers[i]->get_supported_extensions();
	}
	if (sep != ';') 
		cgv::utils::replace(exts,';',sep);
	return exts;
}

std::string video_writer::construct_filter_string()
{
	std::string text = "Video Files (";
	std::string exts = "*.";
	text += get_supported_extensions(',');
	text += "):";
	exts += get_supported_extensions(',');
	replace(exts,",",";*.");
	return text+exts;
}



/// construct a video writer from a file extension and choose an implementation based on the extension
video_writer::video_writer(const std::string& file_extension)
{
	wr = 0;
	std::string ext = to_lower(file_extension);
	std::vector<abst_video_writer*>& writers = video_writer_listener::ref_writers();
	for (unsigned int i=0; i<writers.size(); ++i) {
		if (cgv::utils::is_element(ext, writers[i]->get_supported_extensions())) {
			wr = writers[i]->clone();
			return;
		}
	}
	last_error  = "no video writer found for extension: ";
	last_error += ext;
}

/// return a reference to the last error message
const std::string& video_writer::get_last_error() const
{
	if (!wr)
		return last_error;
	return wr->get_last_error();
}

/// returns the type name of the chosen video writer implementation
std::string video_writer::get_type_name() const
{
	if (wr)
		return wr->get_type_name();
	return "cmv::video_writer";
}

/// return a list of supported codecs in text format
bool video_writer::scan_codecs(std::vector<std::string>& codec_names) const
{
	if (wr)
		return wr->scan_codecs(codec_names);
	return false;
}

/// select a specific codec. This will chance the properties available with the property interface.
bool video_writer::set_codec(const std::string& codec_name)
{
	if (wr)
		return wr->set_codec(codec_name);
	return false;
}

/// return the currently selected codec
std::string video_writer::get_codec() const
{
	if (wr)
		return wr->get_codec();
	return "";
}

/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
std::string video_writer::get_property_declarations()
{
	if (wr)
		return wr->get_property_declarations();
	return cgv::base::base::get_property_declarations();
}

/// abstract interface for the setter, by default it simply returns false
bool video_writer::set_void(const std::string& property, const std::string& type, const void* value)
{
	if (wr)
		return wr->set_void(property, type, value);
	return cgv::base::base::set_void(property, type, value);
}
/// abstract interface for the getter, by default it simply returns false
bool video_writer::get_void(const std::string& property, const std::string& type, void* value)
{
	if (wr)
		return wr->get_void(property, type, value);
	return cgv::base::base::get_void(property, type, value);
}

/** open a video file for writing images in the given format with the given fps. Use the currently 
	 selected codec and codec properties as set with the property interface. */
bool video_writer::open(const std::string& file_name, const data_format& image_format, float fps, bool interactive)
{
	if (wr)
		return wr->open(file_name, image_format, fps, interactive);
	last_error = "no video writer available";
	return false;
}
/** write the image data stored in the data view as next video frame to the previously
    opened video file. Return whether this was successful. */
bool video_writer::write_frame(const const_data_view& image_data)
{
	if (wr)
		return wr->write_frame(image_data);
	last_error = "no video writer available";
	return false;
}

/// close the video file
bool video_writer::close()
{
	if (wr)
		return wr->close();
	last_error = "no video writer available";
	return false;
}


object_registration<video_writer_listener> video_writer_listener_registration("");

		}
	}
}

