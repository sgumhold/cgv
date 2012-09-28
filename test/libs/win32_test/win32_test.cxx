#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/media/text/convert.h>
#include <cgv_win32/win32_gl_view.h>
#include <cgv_win32/win32_trigger_server.h>
#include <cgv/utils/console.h>
#include <windows.h>

using namespace cgv::base;
using namespace cgv::utils;
using namespace cgv::media::text;
using namespace cgv::gui;

struct win32_window
{
	// pointer to modul instance
	HINSTANCE hinst; 
	// handle of window instance
	HWND hwnd;
	// device context
	HDC hdc;
	//
	unsigned int width;
	//
	unsigned int height;

	static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM); 

	win32_window(unsigned int w, unsigned int h);
	~win32_window();
	bool init_application(); 
	bool init_instance(const std::wstring& title, int); 
	bool set_pixel_format();
	bool create(const std::string& title, bool show);

};

bool win32_window::init_application()
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
    wcx.lpszMenuName =  L"MainMenu";    // name of menu resource 
    wcx.lpszClassName = L"MainWClass";  // name of window class 
    wcx.hIconSm = NULL; 
 
    // Register the window class. 
 
	 ATOM atom = RegisterClassEx(&wcx);
    return atom != NULL; 
} 
 
bool win32_window::init_instance(const std::wstring& title, int nCmdShow)
{  
    // Create the main window.  
    hwnd = CreateWindow( 
        L"MainWClass",        // name of window class 
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

LRESULT CALLBACK win32_window::MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{ 
	PAINTSTRUCT ps; 
    HDC hdc;
    switch (uMsg) 
    { 
        case WM_CREATE: 
            // Initialize the window. 
            return 0; 
 
        case WM_PAINT: 
			hdc = BeginPaint(hwnd, &ps); 
            TextOut(hdc, 0, 0, L"Hello, Windows!", 15); 
            EndPaint(hwnd, &ps); 
            return 0L; 
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

win32_window::win32_window(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
	hinst = NULL;
	hwnd = NULL;
	hdc = NULL;
}

win32_window::~win32_window()
{
	DestroyWindow(hwnd);
	hinst = NULL;
	hwnd = NULL;
	hdc = NULL;
}

bool win32_window::set_pixel_format()
{
	PIXELFORMATDESCRIPTOR pfd = { 
		 sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd 
		 1,                     // version number 
		 PFD_DRAW_TO_WINDOW |   // support window 
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

bool win32_window::create(const std::string& title, bool show)
{
	hinst = GetModuleHandle(NULL);
	if (!init_application()) {
		std::cout << "failed to init application" << std::endl;
		return false;
	}
	if (!init_instance(str2wstr(title), show ? SW_SHOW : SW_HIDE)) {
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
	return true;
}







int main(int argc, char** argv)
{

	win32_trigger_server wts;
	register_trigger_server(trigger_server_ptr(new win32_trigger_server ));

	win32_window w(800,600);
	w.create("viewer", true);
	cgv::data::ref_ptr<win32_gl_view> swgv_ptr(new win32_gl_view("viewer"));
	swgv_ptr->attach(w.hwnd);

	disable_registration_event_cleanup();
	enable_permanent_registration();
	enable_registration();
	register_object(swgv_ptr);
	register_object(console::get_console());
	process_command_line_args(argc, argv);
	enable_registration_event_cleanup();

	MSG msg;
    BOOL bRet;
	// Start the message loop. 
    while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
    { 
        if (bRet == -1)
        {
            // handle the error and possibly exit
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    } 
 
    // Return the exit code to the system. 
	swgv_ptr->detach();
    return msg.wParam; 
}
