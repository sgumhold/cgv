#include "avi_video_reader.h"
#include <cgv/type/variant.h>
#include <Vfw.h>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::type;

extern HWND get_main_window();
//HWND AVIStreamBeginStreaming(int,int,int,int);

avi_video_reader::avi_video_reader() : pfile(0), ps(0), getFrame(0)
{
	_fmemset(&opts, 0, sizeof(opts));
	opts.fccType = streamtypeVIDEO;
	frame_index = 0;
	n = 0;
}

///
avi_video_reader::~avi_video_reader()
{
	close();
}


/// returns the type name of the chosen video writer implementation
std::string avi_video_reader::get_type_name() const
{
	return "avi_video_reader";
}

/// construct a copy of the video reader
abst_video_reader* avi_video_reader::clone() const
{
	return new avi_video_reader();
}

std::string avi_video_reader::get_property_declarations()
{
	return "frame:uint32;nr_frames:uint32";
}


/// abstract interface for the getter, by default it simply returns false
bool avi_video_reader::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "frame") {
		set_variant(frame_index, value_type, value_ptr);
		return true;
	}
	else if (property == "nr_frames") {
		set_variant(nr_frames, value_type, value_ptr);
		return true;
	}
	return false;
}

bool avi_video_reader::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "frame") {
		uint32_type fi = variant<uint32_type>::get(value_type, value_ptr);
		if (fi >= nr_frames) {
			last_error = "frame number larger or equal to nr of frames";
			return false;
		}
		end_streaming();
		return start_streaming(fi, nr_frames);
	}
	return false;
}
/// return a reference to the last error message
const std::string& avi_video_reader::get_last_error() const
{
	return last_error;
}
/// return a string containing a colon separated list of extensions that can be read with this reader
const char* avi_video_reader::get_supported_extensions() const
{
	return "avi";
}
/// open the file and read the header in order to determine the image format and the fps
bool avi_video_reader::open(const std::string& file_name,
	cgv::data::data_format& df, float& fps)
{
	AVISTREAMINFO strhdr;
	AVICOMPRESSOPTIONS FAR* aopts[1] = { &opts };
	AVIFileInit();
	// read avi
	HRESULT hr = AVIFileOpen(&pfile, file_name.c_str(), OF_READ | OF_SHARE_DENY_WRITE, NULL);
	if (hr != AVIERR_OK) {
		last_error = std::string("couldn't open avi file ") + file_name;
		return false;
	}
	// get the file info
	AVIFILEINFO fileInfo;
	hr = AVIFileInfo(pfile, &fileInfo, sizeof(fileInfo));
	if (hr != AVIERR_OK) {
		last_error = "no file info available";
		close();
		return false;
	}
	//	new (&df) cgv::data::data_format(fileInfo.dwWidth, fileInfo.dwHeight);
	fps = float(fileInfo.dwRate) / fileInfo.dwScale;
	// get the stream
	hr = AVIFileGetStream(pfile, &ps, streamtypeVIDEO, 0);
	if (hr != AVIERR_OK) {
		last_error = "no video stream found";
		close();
		return false;
	}
	// get the stream info
	hr = AVIStreamInfo(ps, &strhdr, sizeof(strhdr));
	char fccType[4]; (DWORD&)(fccType[0]) = strhdr.fccType;
	char fccHandler[4]; (DWORD&)(fccHandler[0]) = strhdr.fccHandler;
	if (hr != AVIERR_OK) {
		last_error = "no stream info provided";
		close();
		return false;
	}
	df.~data_format();
	new (&df) cgv::data::data_format(strhdr.rcFrame.right - strhdr.rcFrame.left,
		strhdr.rcFrame.bottom - strhdr.rcFrame.top,
		cgv::type::info::TI_UINT8, CF_RGB);
	n = unsigned(df.get_nr_entries());
	nr_frames = AVIStreamLength(ps);
	if (nr_frames == -1) {
		last_error = "no stream length provided";
		close();
		return false;
	}

	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biBitCount = 24;
	info.biWidth = LONG(df.get_width());
	info.biHeight = LONG(df.get_height());
	info.biPlanes = 1;
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 200;
	info.biYPelsPerMeter = 200;
	info.biClrUsed = 0;
	info.biClrImportant = 0;
	return start_streaming(0, nr_frames);
}

bool avi_video_reader::start_streaming(LONG start, LONG end)
{
#ifdef _MSC_VER
	HRESULT hr = AVIStreamBeginStreaming(ps, start, end, 1000);
	if (hr != AVIERR_OK) {
		last_error = "start of streaming not possible";
		close();
		return false;
	}
#endif
	getFrame = AVIStreamGetFrameOpen(ps, &info);
	if (getFrame != NULL) {
		frame_index = start;
		return true;
	}
	close();
	return false;
}

void avi_video_reader::end_streaming()
{
	if (getFrame) {
		HRESULT hr = AVIStreamGetFrameClose(getFrame);
		if (hr != AVIERR_OK) {
			last_error = "close frame not possible";
		}
		getFrame = 0;
		hr = AVIStreamEndStreaming(ps);
		if (hr != AVIERR_OK) {
			last_error = "end of streaming not possible";
		}
	}
}

/// read a frame and return whether this was successful
bool avi_video_reader::read_frame(const cgv::data::data_view& dv)
{
	if (pfile == 0) {
		last_error = "attempt to read frame of not opened file";
		return false;
	}
	// read frame
	if (getFrame != NULL) {
		unsigned char* buffer = (unsigned char*) AVIStreamGetFrame(getFrame, frame_index);
		if (buffer == 0) {
			end_streaming();
			return false;
		}
		// toggle red and blue components back
		const BITMAPINFOHEADER& info = reinterpret_cast<const BITMAPINFOHEADER&>(*buffer);
		unsigned stride = ((((info.biWidth * info.biBitCount) + 31) & ~31) >> 3) - (info.biWidth * info.biBitCount >> 3);
		buffer += sizeof(BITMAPINFOHEADER);
		unsigned char* ptr = dv.get_ptr<unsigned char>();
		for (int j = 0; j < info.biHeight; ++j) {
			for (int i = 0; i < info.biWidth; ++i) {
				ptr[0] = buffer[2];
				ptr[1] = buffer[1];
				ptr[2] = buffer[0];
				ptr += 3;
				buffer += 3;
			}
			buffer += stride;
		}
	}
	else {
		return false;
	}
	++frame_index;
	return true;
}
/// close the video file
bool avi_video_reader::close()
{
	end_streaming();
	if (ps) 
		AVIStreamClose(ps);
	if (pfile) 
		AVIFileClose(pfile);
	ps = 0;
	pfile = 0;
	AVIFileExit();
	return true;
}


#include <cgv/base/register.h>

cgv::base::object_registration<avi_video_reader> avi_read_reg("");

