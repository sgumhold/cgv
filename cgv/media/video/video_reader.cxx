#include "video_reader.h"
#include <cgv/base/register.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/file.h>
#include <vector>

using namespace cgv::base;
using namespace cgv::utils;
using namespace cgv::data;

namespace cgv {
	namespace media {
		namespace video {


/// interfaces that allows to listen to registration events
struct video_reader_listener : public cgv::base::base, public cgv::base::registration_listener
{
	static std::vector<abst_video_reader*>& ref_readers()
	{
		static std::vector<abst_video_reader*> readers;
		return readers;
	}
	std::string get_type_name() const 
	{
		return "video_reader_listener";
	}
	void register_object(base_ptr object, const std::string&)
	{
		abst_video_reader* rd = object->get_interface<abst_video_reader>();
		if (rd)
			ref_readers().push_back(rd);
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

/// construct an image reader from a file name
video_reader::video_reader(data_format& image_format) 
	: image_format_ptr(&image_format), rd(0)
{
	fps = 25;
}

/// destruct reader and implementation
video_reader::~video_reader()
{
	close();
}


/// return a string with a list of supported extensions, where the list entries are separated with the passed character that defaults to a semicolon
const std::string& video_reader::get_supported_extensions(char sep)
{
	static std::string exts;
	exts.clear();
	std::vector<abst_video_reader*>& readers = video_reader_listener::ref_readers();
	for (unsigned int i=0; i<readers.size(); ++i) {
		if (!exts.empty())
			exts += ';';
		exts += readers[i]->get_supported_extensions();
	}
	if (sep != ';') 
		cgv::utils::replace(exts,';',sep);
	return exts;
}

std::string video_reader::construct_filter_string()
{
	std::string text = "Video Files (";
	std::string exts = "*.";
	text += get_supported_extensions(',');
	text += "):";
	exts += get_supported_extensions(',');
	replace(exts,",",";*.");
	return text+exts;
}

/// return a reference to the last error message
const std::string& video_reader::get_last_error() const
{
	static std::string dummy;
	if (!rd)
		return dummy;
	return rd->get_last_error();
}

/// open the file and read the video header in order to determine the image format
bool video_reader::open(const std::string& file_name)
{
	if (!image_format_ptr) {
		last_error = "no space provided to store the image format";
		return false;
	}
	if (rd)
		close();
	std::string ext = to_lower(cgv::utils::file::get_extension(file_name));
	std::vector<abst_video_reader*>& readers = video_reader_listener::ref_readers();
	for (unsigned int i=0; i<readers.size(); ++i) {
		if (cgv::utils::is_element(ext, readers[i]->get_supported_extensions())) {
			rd = readers[i]->clone();
			return rd->open(file_name, *image_format_ptr, fps);
		}
	}
	last_error  = "no video writer found for extension: ";
	last_error += ext;
	return false;
}

/// return the image format of the video file
const cgv::data::data_format& video_reader::get_image_format() const
{
	return *image_format_ptr;
}

/// return the frame rate
float video_reader::get_fps() const
{
	return fps;
}

/** read the next frame to the given data view, if this is empty
    recreate it with a newly allocated data pointer, which is owned
	 by the view afterwards. This method returns false if an error
	 occurs or if there are no more frames in the file. */
bool video_reader::read_frame(cgv::data::data_view& dv)
{
	if (!image_format_ptr) {
		last_error = "no image format available";
		return false;
	}
	if (dv.empty())
		new (&dv) data_view(image_format_ptr);
	if (rd)
		return rd->read_frame(dv);
	last_error = "no video reader available";
	return false;
}
/** same as other read_frame version, but the data_view must point
    to allocated data of the correct format. */
bool video_reader::read_frame(const cgv::data::data_view& dv)
{
	if (rd)
		return rd->read_frame(dv);
	last_error = "no video reader available";
	return false;
}
/// close the video file
bool video_reader::close()
{
	if (rd) {
		bool res = rd->close();
		delete rd;
		rd = 0;
		return res;
	}
	last_error = "no video reader available";
	return false;
}

/// overload to return the type name of this object
std::string video_reader::get_type_name() const
{
	if (rd)
		return rd->get_type_name();
	return "cmv::video_reader";
}
/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
std::string video_reader::get_property_declarations()
{
	if (rd)
		return rd->get_property_declarations();
	return cgv::base::base::get_property_declarations();
}

/// abstract interface for the setter, by default it simply returns false
bool video_reader::set_void(const std::string& property, const std::string& type, const void* value)
{
	if (rd)
		return rd->set_void(property, type, value);
	return cgv::base::base::set_void(property, type, value);
}
/// abstract interface for the getter, by default it simply returns false
bool video_reader::get_void(const std::string& property, const std::string& type, void* value)
{
	if (rd)
		return rd->get_void(property, type, value);
	return cgv::base::base::get_void(property, type, value);
}

object_registration<video_reader_listener> video_reader_listener_registration("");

		}
	}
}