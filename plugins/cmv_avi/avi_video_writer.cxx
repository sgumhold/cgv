#include "avi_video_writer.h"
#include <algorithm>
#include <cgv/utils/convert.h>
#include <cgv/type/variant.h>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::type;

char* best_codecs[] = {
	"DivX",
	"ffdshow",
	"Indeo",
	"Cinepak",
	"H.263",
	"H.261",
	"Microsoft Video",
	NULL
};

BOOL CALLBACK myEnumFunc(HWND hwnd, LPARAM lParam)
{
	std::vector<HWND>* hwnds = (std::vector<HWND>*) lParam;
	hwnds->push_back(hwnd);
	return TRUE;
}

HWND get_main_window()
{
	std::vector<HWND> hwnds;
	LPARAM lParam = 0;
	(std::vector<HWND>*&)lParam = &hwnds;
	EnumThreadWindows(GetCurrentThreadId(), &myEnumFunc, lParam);
	/*
	std::cout << "found " << hwnds.size() << " windows" << std::endl;
	for (unsigned int i=0; i<hwnds.size(); ++i) {
		char buffer[1024];
		int n = GetWindowText(hwnds[i],buffer,1024);
		if (n == 0) {
			std::cout << "window " << i << " has no title" << std::endl;
		}
		else {
			std::cout << "window " << i << " : " << buffer << std::endl;
		}
	}
	*/
	if (hwnds.size() > 0)
		return hwnds[0];
	return GetTopWindow(NULL);
}
/// abstract interface for the setter of a dynamic property, by default it simply returns false
bool avi_video_writer::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "quality") {
		opts.dwFlags |= AVICOMPRESSF_VALID;
		opts.dwQuality = (DWORD)(10000*variant<flt32_type>::get(value_type,value_ptr));
		return true;
	}
	else if (property == "bytes_per_sec") {
		opts.dwFlags |= AVICOMPRESSF_VALID;
		opts.dwFlags |= AVICOMPRESSF_DATARATE;
		opts.dwBytesPerSecond = variant<uint32_type>::get(value_type,value_ptr);
		return true;
	}
	else if (property == "key_frame_step") {
		opts.dwFlags |= AVICOMPRESSF_VALID;
		opts.dwFlags |= AVICOMPRESSF_KEYFRAMES;
		opts.dwKeyFrameEvery = variant<uint32_type>::get(value_type,value_ptr);
		return true;
	}
	return false;
}

/// abstract interface for the getter of a dynamic property, by default it simply returns false
bool avi_video_writer::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "quality") {
		flt32_type q = (flt32_type)opts.dwQuality;
		q *= 0.0001f;
		set_variant(q, value_type, value_ptr);
		return true;
	}
	else if (property == "bytes_per_sec") {
		set_variant(opts.dwBytesPerSecond, value_type, value_ptr);
		return true;
	}
	else if (property == "key_frame_step") {
		set_variant(opts.dwKeyFrameEvery, value_type, value_ptr);
		return true;
	}
	return false;
}

/** return a semicolon separated list of property declarations of the form 
    "name:type", by default an empty list is returned. The types should by
	 consistent with the names returned by cgv::type::info::type_name::get_name. */
std::string avi_video_writer::get_property_declarations()
{
	return "quality:flt32;bytes_per_sec:uint32;key_frame_step:uint32";
}

bool avi_video_writer::find_best_codec(std::string& codec_name) const
{
	std::vector<std::string> codec_names;
	if (!scan_codecs(codec_names) || codec_names.size() == 0) {
		last_error = "could not choose a codec";
		return false;
	}
	for (char** best_codec = best_codecs; *best_codec != NULL; ++best_codec) {
		for (unsigned int i=0; i<codec_names.size(); ++i)
			if (codec_names[i].find(*best_codec) != std::string::npos) {
				codec_name = codec_names[i];
				return true;
			}
	}
	codec_name = codec_names[0];
	return true;
}

DWORD avi_video_writer::get_fccHandler(const std::string& codec_name, DWORD* quality) const
{
	int i;
	ICINFO icinfo;
	for (i=0; ICInfo(ICTYPE_VIDEO, i, &icinfo); i++) {
		HIC hic = ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY);
		if (hic) {
			ICGetInfo(hic, &icinfo, sizeof(icinfo));
			if (codec_name == cgv::utils::wstr2str(std::wstring(icinfo.szDescription))) {
				if (quality)
					*quality = ICGetDefaultQuality(hic);
				return icinfo.fccHandler;
			}
		}
	}
	return 0;
}


bool avi_video_writer::auto_complete_options()
{
	if (opts.fccHandler == 0) {
		std::string codec_name;
		return find_best_codec(codec_name) && set_codec(codec_name);
	}
	return true;
}

/// construct empty writer
avi_video_writer::avi_video_writer() : pfile(0), ps(0), psCompressed(0)
{
	_fmemset(&opts, 0, sizeof(opts));
	opts.fccType = streamtypeVIDEO;
}

/// returns the type name of the chosen video writer implementation
std::string avi_video_writer::get_type_name() const
{
	return "avi_video_writer";
}

/// construct a copy of the video writer
abst_video_writer* avi_video_writer::clone() const
{
	return new avi_video_writer();
}

/// return a string containing a colon separated list of extensions that can be read with this video writer
const char* avi_video_writer::get_supported_extensions() const
{
	return "avi";
}
/// return a reference to the last error message
const std::string& avi_video_writer::get_last_error() const
{
	return last_error;
}

/// dummy implementation provided that does nothing
bool avi_video_writer::scan_codecs(std::vector<std::string>& codec_names) const
{
	int i;
	ICINFO icinfo;
	icinfo.dwSize = sizeof(ICINFO);
	for (i=0; ICInfo(ICTYPE_VIDEO, i, &icinfo); i++) {
		char fccType[4], fccHandler[4];
		(cgv::type::uint32_type&)fccType[0] = icinfo.fccType;
		(cgv::type::uint32_type&)fccHandler[0] = icinfo.fccHandler;
		HIC hic = ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY);
		if (hic) {
			ICGetInfo(hic, &icinfo, sizeof(icinfo));
			codec_names.push_back(cgv::utils::wstr2str(std::wstring(icinfo.szDescription)));
			ICClose(hic);
		}
	}
	return true;
}

/// dummy empty implementation provided that does nothing
bool avi_video_writer::set_codec(const std::string& codec_name)
{
	opts.fccHandler = get_fccHandler(codec_name, &opts.dwQuality);				
	opts.dwFlags |= AVICOMPRESSF_VALID;
	return opts.fccHandler != 0;
}

/// return the currently selected codec, default implementation returns empty string
std::string avi_video_writer::get_codec() const
{
	if (opts.fccHandler == 0) {
		std::string codec_name;
		find_best_codec(codec_name);
		return codec_name;
	}
	ICINFO icinfo;
	HIC hic = ICOpen(opts.fccType, opts.fccHandler, ICMODE_QUERY);
	if (hic) {
		ICGetInfo(hic, &icinfo, sizeof(icinfo));
		return cgv::utils::wstr2str(std::wstring(icinfo.szDescription));
	}
	return "";
}


/// open file from given file name, format and fps
bool avi_video_writer::open(
		const std::string& file_name, 
		const cgv::data::data_format& image_format, 
		float fps, bool interactive)
{
	AVISTREAMINFO strhdr;
	AVICOMPRESSOPTIONS FAR * aopts[1] = {&opts};
	AVIFileInit();
	// open movie file
	HRESULT hr = AVIFileOpen(&pfile, file_name.c_str(), OF_WRITE | OF_CREATE,NULL);
	if (hr != AVIERR_OK) {
		last_error = "could not open avi file for write";
		return false;
	}
	// fill header for the video stream....
	_fmemset(&strhdr, 0, sizeof(strhdr));
	strhdr.fccType                = streamtypeVIDEO;// stream type
	strhdr.fccHandler             = 0;
	strhdr.dwScale                = 1;
	strhdr.dwRate                 = (DWORD)fps;		    // 15 fps
	strhdr.dwSuggestedBufferSize  = DWORD(image_format.get_nr_bytes());
	SetRect(&strhdr.rcFrame, 0, 0, image_format.get_width(), image_format.get_height());

	// create the stream;
	hr = AVIFileCreateStream(pfile,	&ps, &strhdr);
	if (hr != AVIERR_OK) {
		last_error = "could not create avi stream";
		close();
		return false;
	}
	// ask for options
	if (interactive) {
		if (opts.fccHandler == 0) {
			std::string codec_name;
			if (find_best_codec(codec_name))
				set_codec(codec_name);
		}
		if (!AVISaveOptions(get_main_window(), 0, 1, &ps, (LPAVICOMPRESSOPTIONS FAR *) &aopts)) {
			last_error = "user canceled options dialog";
			close();
			return false;
		}
		else {
			if (!auto_complete_options()) {
				close();
				return false;
			}
		}
	}
	// make compressed stream
	hr = AVIMakeCompressedStream(&psCompressed, ps, &opts, NULL);
	if (hr != AVIERR_OK) {
		last_error = "could not construct compressed avi stream";
		close();
		return false;
	}

	BITMAPINFOHEADER bhi;
	bhi.biSize = sizeof(BITMAPINFOHEADER);
	bhi.biWidth = image_format.get_width();
	bhi.biHeight = image_format.get_height();
	bhi.biPlanes = 1;
	bhi.biBitCount = image_format.get_entry_size()*8;
	bhi.biCompression = BI_RGB;
	bhi.biSizeImage = 0;
	bhi.biXPelsPerMeter = 0 ;
	bhi.biYPelsPerMeter = 0 ;
	bhi.biClrUsed = 0;
	bhi.biClrImportant = 0;

	// create an image buffer
	temp_frame.~data_view();
	new (&temp_frame) data_view(&image_format);
	n = unsigned(image_format.get_nr_entries());
	frame_index = 0;

	hr = AVIStreamSetFormat(psCompressed, 0, &bhi, bhi.biSize);
	if (hr != AVIERR_OK) {
		last_error = "could not set image format for avi stream";
		close();
		return false;
	}
	return true;
}

/// write the next frame
bool avi_video_writer::write_frame(const cgv::data::const_data_view& image_data)
{
	if (pfile == 0) {
		last_error = "attempt to write to avi file which has not been opened yet.";
		return false;
	}
	// toggle red and blue component in frame
	unsigned int i;
	const unsigned char* src = image_data.get_ptr<const unsigned char>();
	unsigned char* dst       = temp_frame.get_ptr<unsigned char>();
	for (i=0; i<n; ++i, src += 3, dst += 3) {
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
	}
	// write frame
	HRESULT hr = AVIStreamWrite(psCompressed, frame_index, 1, temp_frame.get_ptr<unsigned char>(), 3*n, AVIIF_KEYFRAME, 0, 0);
	if (hr != AVIERR_OK) {
		last_error = "could not write frame to avi file";
		return false;
	}
	++frame_index;
	return true;
}

/// close the video file
bool avi_video_writer::close()
{
	if (ps) {
		AVIStreamClose(ps);
		ps = 0;
	}
	if (psCompressed) {
		AVIStreamClose(psCompressed);
		psCompressed = 0;
	}
	if (pfile) {
		AVIFileClose(pfile);
		pfile = 0;
	}
	AVIFileExit();
	temp_frame.~data_view();
	new (&temp_frame)data_view();
	return true;
}

#include <cgv/base/register.h>

cgv::base::object_registration<avi_video_writer> avi_write_reg("");
