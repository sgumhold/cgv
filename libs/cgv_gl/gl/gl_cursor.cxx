#include "gl_cursor.h"
#include <iostream>
#include <cgv/media/riff.h>
#include "gl.h"

#ifdef WIN32
#include <windows.h>

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

    // Retrieve the bitmap color format, width, and height.  
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
        return 0;

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

     if (cClrBits < 24) 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER) + 
                    sizeof(RGBQUAD) * (size_t(1) << cClrBits)); 

     // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

     else 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER)); 

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight; 
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
     pbmi->bmiHeader.biClrImportant = 0; 
     return pbmi; 
} 

unsigned char* get_bitmap_data(HBITMAP hbmp, unsigned& tex_w, unsigned& tex_h)
{
	BITMAP bmp; 
	if (!GetObject(hbmp, sizeof(BITMAP), (LPSTR)&bmp)) 
		return 0;
	PBITMAPINFO pbi = CreateBitmapInfoStruct(hbmp);
	PBITMAPINFOHEADER pbih = (PBITMAPINFOHEADER) pbi;
	unsigned my_size = 4 * pbih->biWidth * pbih->biHeight;
	if (pbih->biSizeImage >= my_size)
		my_size = pbih->biSizeImage;
	unsigned char* lpBits = new unsigned char[my_size];
    if (!lpBits)
		return 0;
	if (!GetDIBits(GetDC(NULL), hbmp, 0, (WORD) pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
		return 0;
	tex_w = bmp.bmWidth;
	tex_h = bmp.bmHeight;
	return lpBits;
}

unsigned char* get_cursor_bitmap(HCURSOR hc, unsigned& w, unsigned& h, int anim_step = 0)
{
	ICONINFO ii;
	GetIconInfo(hc, &ii);
	BITMAP bmp;
	if (!GetObject(ii.hbmColor ? ii.hbmColor : ii.hbmMask, sizeof(BITMAP), (LPSTR)&bmp)) 
		return 0;
	w = bmp.bmWidth;
	h = bmp.bmHeight;
	if (h == 2*w)
		h = w;
	HDC dc = GetDC(NULL);
	HDC mem_dc = CreateCompatibleDC(dc);
	HBITMAP hbm = CreateCompatibleBitmap (dc, w, h);
	HBITMAP old_bm = (HBITMAP)SelectObject(mem_dc, hbm);
	RECT r;
	r.left = 0;
	r.bottom = 0;
	r.right = w;
	r.top = h;
	HBRUSH black = CreateSolidBrush(RGB(0,0,0));

	FillRect(mem_dc,&r, black);
	if (DrawIconEx(mem_dc,0,0,hc,w,h,anim_step,NULL,DI_NORMAL | DI_COMPAT) == FALSE) {
		SelectObject(mem_dc, old_bm);
		DeleteObject(black);
		DeleteObject(hbm);
		DeleteDC(mem_dc);
		ReleaseDC(NULL,dc);
		return 0;
	}

	SelectObject(mem_dc, old_bm);
	unsigned char* data_b = get_bitmap_data(hbm,w,h);
	
	old_bm = (HBITMAP)SelectObject(mem_dc, hbm);
	HBRUSH white = CreateSolidBrush(RGB(255,255,255));
	FillRect(mem_dc,&r, white);
	DrawIconEx(mem_dc,0,0,hc,w,h,anim_step,NULL,DI_NORMAL | DI_COMPAT);
	SelectObject(mem_dc, old_bm);
	unsigned char* data_w = get_bitmap_data(hbm,w,h);

	unsigned n=w*h;
	for (unsigned i=0; i<n; ++i) {
		int sum_alpha = 0;
		for (unsigned j=0; j<3; ++j)
			sum_alpha += 255+(int)data_b[4*i+j]-(int)data_w[4*i+j];
		if (sum_alpha > 3*255)
			sum_alpha = 3*255;
		unsigned char alpha = (unsigned char)(sum_alpha/3);
		data_b[4*i+3] = alpha;
		if (alpha > 0) {
			for (unsigned j=0; j<3; ++j) {
				float v = data_b[4*i+j]*255.0f/alpha;
				if (v > 255)
					v = 255;
				data_b[4*i+j] = (unsigned char) v;
			}
		}
	}
	
	delete [] data_w;
	
	DeleteObject(white);
	DeleteObject(black);
	DeleteObject(hbm);
	DeleteDC(mem_dc);
	ReleaseDC(NULL,dc);
	return data_b;
}

unsigned create_texture_from_cursor(HCURSOR hc, unsigned& tex_w, unsigned& tex_h, int& hot_x, int& hot_y, int anim_step = 0)
{
	unsigned tex_id;
	ICONINFO ii;
	GetIconInfo(hc, &ii);
/*	ICONINFOEX iie;
	iie.cbSize = sizeof(ICONINFOEX);
	GetIconInfoEx(hc, &iie);*/
	unsigned char* bgra_data = get_cursor_bitmap(hc, tex_w, tex_h, anim_step);
	if (!bgra_data)
		return -1;
	if (anim_step == 1) {
		unsigned w,h;
		unsigned char* first_bgra_data = get_cursor_bitmap(hc, w, h, 0);
		int m=4*tex_w*tex_h;
		bool found_diff = false;
		for (int j=0; !found_diff && (j<m); ++j)
			if (first_bgra_data[j] != bgra_data[j])
				found_diff = true;
		delete [] first_bgra_data;
		if (!found_diff) {
			delete [] bgra_data;
			return -1;
		}
	}
	hot_x = ii.xHotspot;
	hot_y = ii.yHotspot;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bgra_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	delete [] bgra_data;
	return tex_id;
}
#endif

namespace cgv {
	namespace render {
		namespace gl {

gl_cursor::gl_cursor()
{
	w=h=hot_x=hot_y=0;
	duration = 1;
}

bool gl_cursor::is_created() const
{
	return tex_ids.size() > 0;
}

void gl_cursor::clear()
{
	while (tex_ids.size() > 0) {
		glDeleteTextures(1, &tex_ids.back());
		tex_ids.pop_back();
	}
	periods.clear();
	frames.clear();
}

void gl_cursor::add_frame(unsigned tex_id)
{
	append_step((unsigned)tex_ids.size(),3);
	tex_ids.push_back(tex_id);
}

#ifdef WIN32

#include <windows.h>
#include <Winnls.h>

std::wstring str2wstr(const std::string& s)
{
	std::wstring ws;
	ws.resize(s.size());
#ifdef WIN32
	int n = MultiByteToWideChar(CP_ACP,0,s.c_str(),(int)s.size(),&ws[0],(int)ws.size());
	ws.resize(n);
#else
	std::cerr << "str2wstr(const std::string& s) not implemented" << std::endl;
#endif
	return ws;
}


bool create_from_cursor(HCURSOR hc, gl_cursor* c)
{
	if (!hc)
		return false;
	c->clear();
	unsigned tex_id, i = 0;
	do {
		tex_id = create_texture_from_cursor(hc, c->w, c->h, c->hot_x, c->hot_y, i);
		if (tex_id != -1)
			c->add_frame(tex_id);
		++i;
	} while (tex_id != -1);
	return c->is_created();
}

bool gl_cursor::create(const std::string& id)
{
	static struct { const char* id; LPCWSTR cid; } lookup[] =
	{
		{ "startup", IDC_APPSTARTING },
		{ "arrow", IDC_ARROW },
		{ "cross", IDC_CROSS },
		{ "wait", IDC_WAIT },
		{ "hand", IDC_HAND },
		{ "help", IDC_HELP },
		{ "no", IDC_NO },
		{ "ns", IDC_SIZENS },
		{ "we", IDC_SIZEWE },
		{ "move", IDC_SIZEALL },
		{ "nesw", IDC_SIZENESW },
		{ "nwse", IDC_SIZENWSE }
	};
	LPCWSTR cid = 0;
	for (unsigned i=0; i<10; ++i)
		if (id == lookup[i].id) {
			cid = lookup[i].cid;
			break;
		}
	if (!cid)
		return false;
	return create_from_cursor(LoadCursor(NULL, cid), this);
}

#include <cgv/type/standard_types.h>

using namespace cgv::type;

struct anih_structure
{
	uint32_type size;
	uint32_type nr_frames;
	uint32_type nr_steps;
	uint32_type width;
	uint32_type height;
	uint32_type nr_bits_per_pixel;
	uint32_type nr_planes;
	uint32_type display_rate;
	uint32_type flags;
};

struct ani_riff_handler : public media::riff_handler
{
	unsigned nr_frames;
	unsigned nr_steps;
	unsigned period;
	gl_cursor* c;
	ani_riff_handler(gl_cursor* _c) : c(_c)
	{
		nr_frames = 0;
		nr_steps = 0;
		period = 3;
	}
	void process_chunk_data(media::fourcc id, unsigned size, void* data_ptr)
	{
		if (id == "anih") {
			anih_structure& as = *((anih_structure*)data_ptr);
			nr_frames = as.nr_frames;
			nr_steps = as.nr_steps;
			period = as.display_rate == 0 ? 3 : as.display_rate;
			c->tex_ids.resize(nr_frames);
			c->frames.resize(nr_steps);
			c->periods.resize(nr_steps);
			unsigned i;
			for (i=0; i<nr_steps; ++i) {
				c->frames[i] = i % nr_frames;
				c->periods[i] = period;
			}
			for (i=0; i<nr_frames; ++i)
				c->tex_ids[i] = -1;
			c->update_duration();
		}
		else if (id == "seq") {
			unsigned count = size/4;
			unsigned *values = (unsigned*)data_ptr;
			c->frames.resize(count);
			c->periods.resize(count);
			if (count != c->periods.size())
				std::cerr << "found wrong number of seq entries" << std::endl;
			else
				for (unsigned i=0; i<count; ++i)
					c->frames[i] = values[i];
		}
		else if (id == "rate") {
			unsigned count = size/4;
			unsigned *values = (unsigned*)data_ptr;
			if (count != c->periods.size())
				std::cerr << "found wrong number of rate entries" << std::endl;
			else
				for (unsigned i=0; i<count; ++i)
					c->periods[i] = values[i];
			c->update_duration();
		}
	}
};

bool gl_cursor::create_from_file(const std::string& file_name)
{
	// extract upper case extension
	unsigned dot = (unsigned)file_name.find_last_of('.');
	if (dot == std::string::npos)
		return false;
	std::string ext = file_name.substr(dot+1);
	for (unsigned i=0; i<ext.size(); ++i)
		if (ext[i] >= 'a' && ext[i] <= 'z') {
			ext[i] -= 'a';
			ext[i] += 'A';
		}

	// load static cursors
	if (ext == "CUR") {
		return create_from_cursor(LoadCursorFromFile(str2wstr(file_name).c_str()), this);
	}
	else if (ext == "ANI") {
		// open file and get seq and rate chunks
		clear();
		ani_riff_handler arh(this);
		media::riff_reader rr(&arh);
		if (!rr.read(file_name))
			return false;
		//
		HCURSOR hc = LoadCursorFromFile(str2wstr(file_name).c_str());
		if (!hc)
			return false;
		for (unsigned i = 0; i<get_nr_steps(); ++i) {
			if (tex_ids[frames[i]] == -1) {
				tex_ids[frames[i]] = create_texture_from_cursor(hc, w, h, hot_x, hot_y, i);
				if (tex_ids[frames[i]] == -1) {
					clear();
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

#else
bool gl_cursor::create(const std::string& id)
{
	std::cerr << "gl_cursor::create only implemented under WIN32" << std::endl;
	return false;
}
bool gl_cursor::create_from_file(const std::string& file_name)
{
	std::cerr << "gl_cursor::create_from_file only implemented under WIN32" << std::endl;
	return false;
}
#endif


void gl_cursor::update_duration()
{
	duration = 0;
	for (unsigned i=0; i<periods.size(); ++i)
		duration += periods[i];
}

/// return the number of animation steps
unsigned gl_cursor::get_nr_steps() const
{
	return (unsigned)frames.size();
}

/// return the number of animation frames
unsigned gl_cursor::get_nr_frames() const
{
	return (unsigned)tex_ids.size();
}

/// return the gl texture id of the given frame
unsigned gl_cursor::get_texture_id(unsigned frame_idx) const
{
	return tex_ids[frame_idx];
}

/// return the frame index of given step
unsigned gl_cursor::get_step_frame(unsigned step_idx) const
{
	return frames[step_idx];
}

/// set the frame index of given step
void gl_cursor::set_step_frame(unsigned step_idx, unsigned frame_idx)
{
	frames[step_idx] = frame_idx;
}

/// return the period of given step in 1/60th of a second
unsigned gl_cursor::get_step_period(unsigned step_idx) const
{
	return periods[step_idx];
}

/// return the period of given step in 1/60th of a second
void gl_cursor::set_step_period(unsigned step_idx, unsigned period)
{
	periods[step_idx] = period;
	update_duration();
}

/// append an animation step given by frame index and step period
void gl_cursor::append_step(unsigned frame_idx, unsigned period)
{
	frames.push_back(frame_idx);
	periods.push_back(period);
	update_duration();
}

/// find the step index of a given elapsed time in seconds
unsigned gl_cursor::find_step_index(double elapsed_seconds) const
{
	unsigned i, period = ((int)(elapsed_seconds * 60))%duration;
	for (i=0; i<periods.size(); ++i) {
		if (periods[i] > period)
			return i;
		period -= periods[i];
	}
	return i % periods.size();
}

void gl_cursor::draw(int x, int y, bool use_color, unsigned frame_idx)
{
	if (frame_idx >= tex_ids.size())
		return;
	if (tex_ids[frame_idx] == -1) 
		return;

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_TEXTURE_BIT);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glAlphaFunc(GL_GREATER, 0.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, use_color ? GL_MODULATE : GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, tex_ids[frame_idx]);
	x -= hot_x;
	y -= hot_y;
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(1,1);
		glVertex2i(x+w,y);
		glTexCoord2f(0,1);
		glVertex2i(x,y);
		glTexCoord2f(0,0);
		glVertex2i(x,y+h);
		glTexCoord2f(1,0);
		glVertex2i(x+w,y+h);
	glEnd();
	glPopAttrib();
}

		}
	}
}
