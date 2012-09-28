#pragma once

#include <cgv/media/video/video_reader.h>

#ifdef WIN32
#undef UNICODE
#define  STRICT
#define  INC_OLE2
#include <windows.h>
#include <windowsx.h>
#include <memory.h>
#include <mmsystem.h>
#include <Vfw.h>
#include <wingdi.h>
#endif

using namespace cgv::media::video;

#include "lib_begin.h"

/// abstract interface for a video reader
class CGV_API avi_video_reader : public abst_video_reader
{
protected:
	PAVIFILE pfile;
	PAVISTREAM ps;
	PAVISTREAM psCompressed;
	PGETFRAME  getFrame;
	AVICOMPRESSOPTIONS opts;
	/*
	LPBITMAPINFOHEADER input_format;
	LPBITMAPINFOHEADER output_format;
	HIC decompressor;
	*/
	unsigned int frame_index;
	unsigned int nr_frames;
	unsigned int n;
	mutable std::string last_error;
	bool get_void(const std::string& property, const std::string& type, void* value);
	bool set_void(const std::string& property, const std::string& type, const void* value);
	bool open_frame();

public:
	///
	avi_video_reader();
	///
	~avi_video_reader();
	/// returns the type name of the chosen video writer implementation
	std::string get_type_name() const;
	/// construct a copy of the video reader
	abst_video_reader* clone() const;
	/// return a reference to the last error message
	const std::string& get_last_error() const;
	/** two properties are support: 
		 frame:uint32 ... index of frame, which is read next. If this is set,
		                  the file pointer is moved to the given frame index,
								which must be in the range [0,nr_frames].
		 nr_frames:uint32 ... read only property that provides access to the
		                  total number of frames in the movie file. */
	std::string get_property_declarations();
	/// return a string containing a colon separated list of extensions that can be read with this reader
	const char* get_supported_extensions() const;
	/// open the file and read the header in order to determine the image format and the fps
	bool open(const std::string& file_name, 
				 cgv::data::data_format& df, float& fps);
	/// read a frame and return whether this was successful
	bool read_frame(const cgv::data::data_view& dv);
	/// close the video file
	bool close();
};

#include <cgv/config/lib_end.h>
