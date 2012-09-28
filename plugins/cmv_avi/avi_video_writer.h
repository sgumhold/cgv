#pragma once

#include <cgv/media/video/video_writer.h>

#ifdef WIN32
#undef UNICODE
#define  STRICT
#define  INC_OLE2
#include <windows.h>
#include <windowsx.h>
#include <memory.h>
#include <mmsystem.h>
#include <vfw.h>
#include <wingdi.h>
#endif

using namespace cgv::media::video;

#include "lib_begin.h"

/// abstract interface for video writers
class CGV_API avi_video_writer : public abst_video_writer
{
protected:
	PAVIFILE pfile;
	PAVISTREAM ps;
	PAVISTREAM psCompressed;
	AVICOMPRESSOPTIONS opts;
	mutable std::string last_error;
	cgv::data::data_view temp_frame;
	unsigned int n;
	unsigned int frame_index;
	bool auto_complete_options();
	bool find_best_codec(std::string& codec_name) const;
	DWORD get_fccHandler(const std::string& codec_name, DWORD* quality=0) const;
	/// abstract interface for the setter of a dynamic property, by default it simply returns false
	bool set_void(const std::string& property, const std::string& type, const void* value);
	/// abstract interface for the getter of a dynamic property, by default it simply returns false
	bool get_void(const std::string& property, const std::string& type, void* value);
	/** return a semicolon separated list of property declarations of the form 
	    "name:type", by default an empty list is returned. The types should by
		 consistent with the names returned by cgv::type::info::type_name::get_name. */
	std::string get_property_declarations();
public:
	/// construct empty writer
	avi_video_writer();
	/// returns the type name of the chosen video writer implementation
	std::string get_type_name() const;
	/// construct a copy of the video writer
	abst_video_writer* clone() const;
	/// return a string containing a colon separated list of extensions that can be read with this video writer
	const char* get_supported_extensions() const;
	/// return a reference to the last error message
	const std::string& get_last_error() const;
	/// 
	bool scan_codecs(std::vector<std::string>& codec_names) const;
	/// dummy empty implementation provided that does nothing and returns false
	bool set_codec(const std::string& codec_name);
	/// return the currently selected codec, default implementation returns empty string
	std::string get_codec() const;
	/// open file from given file name, format and fps
	bool open(const std::string& file_name, const cgv::data::data_format& image_format, float fps, bool interactive);
	/// write the next frame
	bool write_frame(const cgv::data::const_data_view& image_data);
	/// close the video file
	bool close();
};

#include <cgv/config/lib_end.h>
