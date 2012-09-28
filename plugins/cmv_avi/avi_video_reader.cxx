#include "avi_video_reader.h"
#include <cgv/type/variant.h>
#include <Vfw.h>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::type;

extern HWND get_main_window();
//HWND AVIStreamBeginStreaming(int,int,int,int);

avi_video_reader::avi_video_reader() : pfile(0), ps(0), psCompressed(0), getFrame(0)
{
	_fmemset(&opts, 0, sizeof(opts));
	opts.fccType = streamtypeVIDEO;
	frame_index = 0;
	n = 0;
	/*
	input_format = 0;
	output_format = 0;
	decompressor = 0;
	*/
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
		// close get frame 
		if (getFrame) {
			AVIStreamGetFrameClose(getFrame);
			getFrame = 0;
		}
#ifdef _MSC_VER
		HRESULT hr = AVIStreamBeginStreaming(ps, fi, nr_frames, 1000);
		if (hr != AVIERR_OK) {
			last_error = "streaming from given location not possible";
			return false;
		}
#endif
		getFrame = AVIStreamGetFrameOpen(ps, NULL);
		if (getFrame == NULL) {
			last_error = "no decompressor found";
			return false;
		}
		frame_index = fi;
		return true;
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
	AVICOMPRESSOPTIONS FAR * aopts[1] = {&opts};
	AVIFileInit();
	frame_index = 0;
	// read avi
	HRESULT hr = AVIFileOpen(&pfile, file_name.c_str(), OF_READ | OF_SHARE_DENY_WRITE,NULL);
	if (hr != AVIERR_OK) {
		last_error = std::string("couldn't open avi file ")+file_name;
		return false;
	}
	psCompressed = 0;
	// get the file info
	AVIFILEINFO fileInfo;
	hr = AVIFileInfo(pfile, &fileInfo, sizeof(fileInfo));
	if (hr != AVIERR_OK) {
		last_error = "no file info available";
		close();
		return false;
	}
//	new (&df) cgv::data::data_format(fileInfo.dwWidth, fileInfo.dwHeight);
	fps = float(fileInfo.dwRate)/fileInfo.dwScale;
	// get the stream
	hr = AVIFileGetStream(pfile, &ps, streamtypeVIDEO, 0);
	if (hr != AVIERR_OK) {
		last_error = "no video stream found";
		close();
		return false;
	}
	// get the stream info
	hr = AVIStreamInfo(ps, &strhdr, sizeof(strhdr));
	if (hr != AVIERR_OK) {
		last_error = "no stream info provided";
		close();
		return false;
	}
	df.~data_format();
	new (&df) cgv::data::data_format(strhdr.rcFrame.right-strhdr.rcFrame.left, 
												strhdr.rcFrame.bottom-strhdr.rcFrame.top, 
												cgv::type::info::TI_UINT8, CF_RGB);
	n = df.get_size();
	nr_frames = AVIStreamLength(ps);
	if (nr_frames == -1) {
		last_error = "no stream length provided";
		close();
		return false;
	}
	/*
		char fmt[5];
		fmt[4] = 0;
		((DWORD&) fmt[0]) = strhdr.fccHandler;
		std::cout << "format: " << fmt << std::endl;
   */
#ifdef _MSC_VER
	hr = AVIStreamBeginStreaming(ps, 0, nr_frames, 1000);
	if (hr != AVIERR_OK) {
		last_error = "start of streaming not possible";
		close();
		return false;
	}
#endif
	return open_frame();
}

bool avi_video_reader::open_frame()
{
	getFrame = AVIStreamGetFrameOpen(ps, NULL);
	if (getFrame != NULL) 
		return true;
	close();
	return false;
/*	HRESULT hr;
	AVISTREAMINFO info;
	hr = AVIStreamInfo(ps, &info, sizeof(info));
	if (hr != AVIERR_OK) {
		last_error = "no stream info provided";
		close();
		return false;
	}
	LONG format_size;
	hr = AVIStreamReadFormat(ps,0,NULL,&format_size);
	if (hr != AVIERR_OK) {
		last_error = "could not determine size of format";
		close();
		return false;
	}
	input_format = (LPBITMAPINFOHEADER) new unsigned char[format_size];
	hr = AVIStreamReadFormat(ps,0,input_format,&format_size);
	if (hr != AVIERR_OK) {
		last_error = "could not read input format";
		close();
		return false;
	}

	int i;
	ICINFO icinfo;
	for (i=0; ICInfo(ICTYPE_VIDEO, i, &icinfo); i++) {
		HIC hic = ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY);
		if (hic) {
			ICGetInfo(hic, &icinfo, sizeof(icinfo));
			std::string codec_name = cgv::utils::wstr2str(std::wstring(icinfo.szDescription));
			std::cout << "query to codec " << codec_name.c_str() << std::endl;
			DWORD hr = ICDecompressQuery(hic,input_format,NULL);
			if (hr == ICERR_OK) {
				std::cout << "query succeeded" << std::endl;
			}
		}
	}


	decompressor = ICDecompressOpen(
		ICTYPE_VIDEO,info.fccHandler,input_format,NULL);
	if (decompressor == NULL) {
		last_error = "could not find decompressor";
		close();
		return false;
	}
	format_size = ICDecompressGetFormat(decompressor, input_format, NULL);
	output_format = (LPBITMAPINFOHEADER) new unsigned char[format_size];
	hr = ICDecompressGetFormat(decompressor, input_format, output_format);
	if (hr != AVIERR_OK) {
		last_error = "could not get output format";
		close();
		return false;
	}
	hr = ICDecompressBegin(decompressor,input_format,output_format);
	if (hr != AVIERR_OK) {
		last_error = "could not start decompression";
		close();
		return false;
	}
	return true;
	*/
}

/// read a frame and return whether this was successful
bool avi_video_reader::read_frame(const cgv::data::data_view& dv)
{
	if (pfile == 0) {
		last_error = "attempt to read frame of not opened file";
		return false;
	}
	// read frame
	unsigned int i;
	if (getFrame != NULL) {
		unsigned char* buffer = (unsigned char*) AVIStreamGetFrame(getFrame, frame_index);
		if (buffer == 0) return false;
		// toggle red and blue components back
		buffer += sizeof(BITMAPINFOHEADER);
		unsigned char* ptr = dv.get_ptr<unsigned char>();
		for (i=0; i<n; ++i, ptr += 3, buffer += 3) {
			ptr[0] = buffer[2];
			ptr[1] = buffer[1];
			ptr[2] = buffer[0];
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
	if (ps) AVIStreamClose(ps);
	if (psCompressed) AVIStreamClose(psCompressed);
	if (pfile) AVIFileClose(pfile);
	if (getFrame) AVIStreamGetFrameClose(getFrame);
	ps = 0;
	psCompressed = 0;
	pfile = 0;
	getFrame = 0;
	AVIFileExit();
	return true;
}


#include <cgv/base/register.h>

cgv::base::object_registration<avi_video_reader> avi_read_reg("");

