#pragma once

#include <string>
#include <vector>
#include <cgv/base/base.h>
#include <cgv/data/data_view.h>

#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace video {

class CGV_API video_writer;

/// abstract interface for video writers
class CGV_API abst_video_writer : public cgv::base::base
{
public:
	friend class video_writer;
	/// construct a copy of the video writer
	virtual abst_video_writer* clone() const = 0;
	/// return a string containing a colon separated list of extensions that can be read with this video writer
	virtual const char* get_supported_extensions() const = 0;
	/// return a reference to the last error message
	virtual const std::string& get_last_error() const = 0;
	/// dummy implementation provided that does nothing and returns false
	virtual bool scan_codecs(std::vector<std::string>& codec_names) const;
	/// dummy empty implementation provided that does nothing and returns false
	virtual bool set_codec(const std::string& codec_name);
	/// return the currently selected codec, default implementation returns empty string
	virtual std::string get_codec() const;
	/// open file from given file name, format and fps
	virtual bool open(const std::string& file_name, const cgv::data::data_format& image_format, float fps, bool interactive) = 0;
	/// write the next frame
	virtual bool write_frame(const cgv::data::const_data_view& image_data) = 0;
	/// close the video file
	virtual bool close() = 0;
};

/// the image writer chooses a specific writer automatically based on the extension of the given file name 
class CGV_API video_writer : public cgv::base::base
{
protected:
	/// store a pointer to the chosen reader
	abst_video_writer* wr;
	/// store the last error not resulting from video writer implementations
	std::string last_error;
	/// abstract interface for the setter, uses the interface of the selected writer implementation
	bool set_void(const std::string& property, const std::string& type, const void* value);
	/// abstract interface for the getter, uses the interface of the selected writer implementation
	bool get_void(const std::string& property, const std::string& type, void* value);
public:
	/** return a string with a list of supported extensions, where the list entries are separated 
	    with the passed character that defaults to a semicolon */
	static const std::string& get_supported_extensions(char sep = ';');
	/// use this to generate a file_open_dialog or file_save_dialog
	static std::string construct_filter_string();
	/// construct a video writer from a file extension and choose an implementation based on the extension
	video_writer(const std::string& file_extension);
	/// return a reference to the last error message
	const std::string& get_last_error() const;
	/// returns the type name of the chosen video writer implementation
	std::string get_type_name() const;
	/// return a list of supported codecs in text format
	bool scan_codecs(std::vector<std::string>& codec_names) const;
	/// select a specific codec. This will chance the properties available with the property interface.
	bool set_codec(const std::string& codec_name);
	/// return the currently selected codec
	std::string get_codec() const;
	/** return a semicolon separated list of property declarations supported by the selected codec.
		 Each property declaration is of the form "name:type", where type is in correcpondence with
		 the cgv::type::info::type_name. */
	std::string get_property_declarations();
	/** open a video file for writing images in the given format with the given fps. Use the currently 
		 selected codec and codec properties as set with the property interface. */
	bool open(const std::string& file_name, 
				 const cgv::data::data_format& image_format, 
				 float fps = 25,
				 bool interactive = false);
	/** write the image data stored in the data view as next video frame to the previously
	    opened video file. Return whether this was successful. */
	bool write_frame(const cgv::data::const_data_view& image_data);
	/// close the video file
	bool close();
};

		}
	}
}

#include <cgv/config/lib_end.h>
