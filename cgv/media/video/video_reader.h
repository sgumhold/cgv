#pragma once

#include <string>
#include <vector>
#include <cgv/base/base.h>
#include <cgv/data/data_view.h>

#include "lib_begin.h"

namespace cgv {
	namespace media {
		/// namespace for image processing
		namespace video {

class CGV_API video_reader;

/// abstract interface for a video reader
class CGV_API abst_video_reader : public cgv::base::base
{
public:
	friend class video_reader;
	/// construct a copy of the video reader
	virtual abst_video_reader* clone() const = 0;
	/// return a reference to the last error message
	virtual const std::string& get_last_error() const = 0;
	/// return a string containing a colon separated list of extensions that can be read with this reader
	virtual const char* get_supported_extensions() const = 0;
	/// open the file and read the header in order to determine the image format and the fps
	virtual bool open(const std::string& file_name, cgv::data::data_format& df, float& fps) = 0;
	/// read a frame and return whether this was successful
	virtual bool read_frame(const cgv::data::data_view& dv) = 0;
	/// close the video file
	virtual bool close() = 0;

};

/// the video reader chooses a specific reader automatically based on the extension of the given file name 
class CGV_API video_reader : public cgv::base::base
{
protected:
	/// store the image format
	cgv::data::data_format* image_format_ptr;
	/// store a pointer to the chosen reader
	abst_video_reader* rd;
	/// store the fps of the opened video file
	float fps;
	/// store the last error not resulting from video writer implementations
	std::string last_error;
	/// abstract interface for the setter, by default it simply returns false
	bool set_void(const std::string& property, const std::string& type, const void* value);
	/// abstract interface for the getter, by default it simply returns false
	bool get_void(const std::string& property, const std::string& type, void* value);
public:
	/** construct a video reader from a reference to the image format, which
	    will be set after opening a file. */
	video_reader(cgv::data::data_format& image_format);
	/// destruct reader and implementation
	~video_reader();
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// return a string with a list of supported extensions, where the list entries are separated with the passed character that defaults to a semicolon
	static const std::string& get_supported_extensions(char sep = ';');
	/// use this to generate a file_open_dialog or file_save_dialog
	static std::string construct_filter_string();
	/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
	std::string get_property_declarations();
	/// return a reference to the last error message
	const std::string& get_last_error() const;
	/// open the file and read the video header in order to determine the image format
	bool open(const std::string& file_name);
	/// return the image format of the video file
	const cgv::data::data_format& get_image_format() const;
	/// return the frame rate
	float get_fps() const;
	/** read the next frame to the given data view, if this is empty
	    recreate it with a newly allocated data pointer, which is owned
		 by the view afterwards. This method returns false if an error
		 occurs or if there are no more frames in the file. */
	bool read_frame(cgv::data::data_view& dv);
	/** same as other read_frame version, but the data_view must point
	    to allocated data of the correct format. */
	bool read_frame(const cgv::data::data_view& dv);
	/// close the video file
	bool close();
};

		}
	}
}

#include <cgv/config/lib_end.h>
