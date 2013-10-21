#include "gl_context.h"
#ifdef WIN32
#include <windows.h>
#include <Tchar.h>
#include <cgv/utils/convert.h>
#include <iostream>

using namespace cgv::utils;

namespace cgv {
	namespace render {
		namespace gl {

typedef std::basic_string<_TCHAR> tstring;

struct win32_gl_context : public gl_context
{
	// pointer to modul instance
	HINSTANCE hinst; 
	// handle of window instance
	HWND hwnd;
	// device context
	HDC hdc;
	//
	HGLRC glc;
	//
	unsigned int width;
	//
	unsigned int height;

	// Function prototypes.  
	//static int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int); 
	static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM); 

	bool init_application(); 
	bool ensure_init_application(); 
	bool init_instance(const tstring& title, int); 
	bool set_pixel_format();
	bool create(const std::string& title, bool show);

	win32_gl_context(unsigned int w = -1, unsigned int h = -1);
	~win32_gl_context();

	/// return the current render pass
	RenderPass get_render_pass() const { return RP_NONE; }
	/// return the current render pass flags
	RenderPassFlags get_render_pass_flags() const { return RPF_NONE; }
	/// perform the given render task
	void render_pass(RenderPass render_pass = RP_MAIN, 
		RenderPassFlags render_pass_flags = RPF_ALL) {}
	/// return the default render pass flags
	RenderPassFlags get_default_render_pass_flags() const { return RPF_NONE; }
	/// return the default render pass flags
	void set_default_render_pass_flags(RenderPassFlags) { }
	/// return whether the context is currently in process of rendering
	bool in_render_process() const { return false; }
	/// return whether the context is created
	bool is_created() const { return true; }
	/// return whether the context is current
	bool is_current() const { return true; }
	/// make the current context current
	bool make_current() const;
	/// clear the current context, typically used in multi-threaded rendering to allow usage of context in several threads
	void clear_current() const;

	//@}

	/// return the width of the window
	unsigned int get_width() const { return width; }
	/// return the height of the window
	unsigned int get_height() const { return height; }
	/// resize the context to the given dimensions
	void resize(unsigned int width, unsigned int height) { }
	/// set a user defined background color
	void set_bg_color(float r, float g, float b, float a) {}
	/// the context will be redrawn when the system is idle again
	void post_redraw() {}
	/// the context will be redrawn right now. This method cannot be called inside the following methods of a drawable: init, init_frame, draw, finish_draw
	void force_redraw() {}
	bool is_alpha_buffer_attached() const { return true; }
	void attach_alpha_buffer() {}
	void detach_alpha_buffer() {}
	bool is_stencil_buffer_attached() const { return false; }
	void attach_stencil_buffer() {}
	void detach_stencil_buffer() {}
	bool is_quad_buffer_supported() const { return false; }
	bool is_quad_buffer_attached() const { return false; }
	void attach_quad_buffer() {}
	void detach_quad_buffer() {}
	bool is_accum_buffer_attached() const { return false; }
	void attach_accum_buffer() {}
	void detach_accum_buffer() {}
	bool is_multisample_enabled() const { return false; }
	void enable_multisample() {}
	void disable_multisample() {}

	/**@name font selection and measure*/
	//@{
	/// enable the given font face with the given size in pixels
	void enable_font_face(cgv::media::font::font_face_ptr font_face, float font_size) {}
	/// return the size in pixels of the currently enabled font face
	float get_current_font_size() const { return 12; }
	/// return the currently enabled font face
	cgv::media::font::font_face_ptr get_current_font_face() const { return cgv::media::font::font_face_ptr();
	}
	//@}
	/// returns an output stream whose output is printed at the current cursor location
	std::ostream& output_stream() { return std::cout; }
};

/* Application entry point. 
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow) 
{ 
    MSG msg; 
 
    if (!InitApplication(hinstance)) 
        return FALSE; 
 
    if (!InitInstance(hinstance, nCmdShow)) 
        return FALSE; 
 
    BOOL fGotMessage;
    while ((fGotMessage = GetMessage(&msg, (HWND) NULL, 0, 0)) != 0 && fGotMessage != -1) 
    { 
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    } 
    return (int)msg.wParam; 
        UNREFERENCED_PARAMETER(lpCmdLine); 
} 
*/

bool win32_gl_context::init_application()
{ 
    WNDCLASSEX wcx;  
    // Fill in the window class structure with parameters 
    // that describe the main window. 
    wcx.cbSize = sizeof(wcx);          // size of structure 
    wcx.style = CS_HREDRAW | 
        CS_VREDRAW;                    // redraw if size changes 
    wcx.lpfnWndProc = MainWndProc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = hinst;         // handle to instance 
    wcx.hIcon = LoadIcon(NULL, 
        IDI_APPLICATION);              // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, 
        IDC_ARROW);                    // predefined arrow 
    wcx.hbrBackground = NULL;                  // white background brush 
    wcx.lpszMenuName =  _T("MainMenu");    // name of menu resource 
    wcx.lpszClassName = _T("MainWClass");  // name of window class 
    wcx.hIconSm = NULL; 
 
    // Register the window class. 
 
	 ATOM atom = RegisterClassEx(&wcx);
    return atom != NULL; 
} 

bool win32_gl_context::init_instance(const tstring& title, int nCmdShow)
{  
    // Create the main window.  
    hwnd = CreateWindow( 
        _T("MainWClass"),        // name of window class 
        title.c_str(),            // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
        width == -1 ? CW_USEDEFAULT : width,       // default width 
		  height == -1 ? CW_USEDEFAULT : height,       // default height 
        (HWND) NULL,         // no owner window 
        (HMENU) NULL,        // use class menu 
        hinst,           // handle to application instance 
        (LPVOID) NULL);      // no window-creation data 
 
    if (!hwnd) 
        return false; 
    // Show the window and send a WM_PAINT message to the window 
    // procedure. 
    ShowWindow(hwnd, nCmdShow); 
    UpdateWindow(hwnd); 
    return true; 
} 

LRESULT CALLBACK win32_gl_context::MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{ 
    switch (uMsg) 
    { 
        case WM_CREATE: 
            // Initialize the window. 
            return 0; 
 
        case WM_PAINT: 
            // Paint the window's client area. 
            return 0; 
 
        case WM_SIZE: 
            // Set the size and position of the window. 
            return 0; 
 
        case WM_DESTROY: 
            // Clean up window-specific data objects. 
            return 0; 
 
        // 
        // Process other messages. 
        // 
 
        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 
    return 0; 
}


bool win32_gl_context::make_current() const
{
	if (glc == NULL)
		return false;
	if (!wglMakeCurrent(hdc,glc)) {
		DWORD error = GetLastError();
		std::cerr << "failed to make current [hdc=" << hdc << ", glc=" << glc << "] with error " << error << std::endl;
		return false;
	}
	return true;
}

void win32_gl_context::clear_current() const
{
	wglMakeCurrent(NULL,NULL);
}

win32_gl_context::win32_gl_context(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
	hinst = NULL;
	hwnd = NULL;
	hdc = NULL;
	glc = NULL;
}

win32_gl_context::~win32_gl_context()
{
	wglMakeCurrent(NULL,NULL);
	DestroyWindow(hwnd);
	hinst = NULL;
	hwnd = NULL;
	hdc = NULL;
	glc = NULL;
}

bool win32_gl_context::set_pixel_format()
{
	PIXELFORMATDESCRIPTOR pfd = { 
		 sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd 
		 1,                     // version number 
		 PFD_DRAW_TO_WINDOW |   // support window 
		 PFD_SUPPORT_OPENGL |   // support OpenGL 
		 PFD_DOUBLEBUFFER,      // double buffered 
		 PFD_TYPE_RGBA,         // RGBA type 
		 24,                    // 24-bit color depth 
		 0, 0, 0, 0, 0, 0,      // color bits ignored 
		 0,                     // no alpha buffer 
		 0,                     // shift bit ignored 
		 0,                     // no accumulation buffer 
		 0, 0, 0, 0,            // accum bits ignored 
		 32,                    // 32-bit z-buffer 
		 0,                     // no stencil buffer 
		 0,                     // no auxiliary buffer 
		 PFD_MAIN_PLANE,        // main layer 
		 0,                     // reserved 
		 0, 0, 0                // layer masks ignored 
	}; 
	int  iPixelFormat; 
	 
	// get the best available match of pixel format for the device context  
	iPixelFormat = ChoosePixelFormat(hdc, &pfd); 
	 
	// make that the pixel format of the device context 
	return SetPixelFormat(hdc, iPixelFormat, &pfd) == TRUE;
}

bool win32_gl_context::ensure_init_application()
{
	static bool is_initialized = false;
	static bool result = false;
	if (!is_initialized) {
		result = init_application();
		is_initialized = true;
	}
	return result;
}

bool win32_gl_context::create(const std::string& title, bool show)
{
	hinst = GetModuleHandle(NULL);
	if (!ensure_init_application()) {
		std::cout << "failed to init application" << std::endl;
		return false;
	}
#ifdef _UNICODE
	if (!init_instance(str2wstr(title), show ? SW_SHOW : SW_HIDE)) {
#else
	if (!init_instance(title, show ? SW_SHOW : SW_HIDE)) {
#endif
		std::cout << "failed to init instance" << std::endl;
		return false;
	}
	hdc = GetDC(hwnd);
	if (hdc == NULL) {
		std::cout << "failed to get DC" << std::endl;
		return false;
	}
	if (!set_pixel_format()) {
		std::cout << "failed to choose pixel format" << std::endl;
		return false;
	}
	glc = wglCreateContext(hdc);
	if (glc == NULL) {
		std::cout << "failed to create gl context" << std::endl;
		return false;
	}
	return true;
}

context* create_win32_gl_context(RenderAPI api, unsigned int w, unsigned int h, 
									const std::string& title, bool show)
{
	if (api != RA_OPENGL)
		return 0;
	win32_gl_context* ctx_ptr = new win32_gl_context(w,h);
	if (!ctx_ptr->create(title, show)) {
		delete ctx_ptr;
		return 0;
	}
	return ctx_ptr;
}

		}

context_factory_registration create_win32_gl_context_registration(gl::create_win32_gl_context);

	}
}


#endif
