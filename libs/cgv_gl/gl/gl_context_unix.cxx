#ifndef WIN32
#include <gl/wgl.h>
#include "gl_context.h"
#include <iostream>

using namespace cgv::utils;


namespace cgv {
	namespace render {
		namespace gl {

struct unix_gl_context : public gl_context
{
	Display   *d_dpy;
	Window     d_win;
	GLXContext d_ctx;
	//
	unsigned int width;
	//
	unsigned int height;

	// Function prototypes.  

	bool init_application(); 
	bool ensure_init_application(); 
	bool init_instance(); 
	bool set_pixel_format();
	bool create(const std::string& title, bool show);

	unix_gl_context(unsigned int w = -1, unsigned int h = -1);
	~unix_gl_context();

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
	///
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

bool unix_gl_context::make_current() const
{
	bool res;
	XLockDisplay(d_dpy);
	res = glXMakeCurrent(d_dpy, d_win, d_ctx);
	XUnlockDisplay(d_dpy);
	
	if (!res) {
		std::cerr << "failed to make current" << std::endl;
		return false;
	}
	return true;
}

void unix_gl_context::clear_current() const
{
	XLockDisplay(d_dpy);
	glXMakeCurrent(d_dpy, 0, 0);
	XUnlockDisplay(d_dpy);
}

unix_gl_context::unix_gl_context(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
	d_dpy = 0;
	d_win = 0;
	d_ctx = 0;
}

unix_gl_context::~unix_gl_context()
{
	XLockDisplay(d_dpy);
	glXMakeCurrent(d_dpy, d_win, d_ctx);
	glXDestroyContext(d_dpy, d_ctx);
	XDestroyWindow(d_dpy, d_win);
	XUnlockDisplay(d_dpy);
	XCloseDisplay(d_dpy);
	d_dpy = 0;
	d_win = 0;
	d_ctx = 0;
}

bool unix_gl_context::create(const std::string& title, bool show)
{

	// Set to NULL for getting it from the environment variable
	const char *display = ":0"; 

	if (!(d_dpy = XOpenDisplay(display))) {
		std::cerr << "Couldn't open X11 display" << std::endl;
		return false;
	}
	XLockDisplay(d_dpy);

	int attr[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 16,
		None
	};

	int scrnum = DefaultScreen(d_dpy);
	
	Window root = RootWindow(d_dpy, scrnum);
	
	/*
	// We need this for OpenGL3
	int elemc;
	GLXFBConfig *fbcfg = glXChooseFBConfig(d_dpy, scrnum, NULL, &elemc);
	if (!fbcfg) {
		std::cerr << "Couldn't get FB configs" << std::endl;
		return false;
	}
	else {
		std::cout << "Got " << elemc << " FB configs" << std::endl;
	}
	*/
	
	XVisualInfo *visinfo = glXChooseVisual(d_dpy, scrnum, attr);

	if (!visinfo) {
		std::cerr << "Couldn't get a visual" << std::endl;
	}
	else {
		d_ctx = glXCreateContext(d_dpy, visinfo, NULL, True);
		
		
		// Window parameters
		XSetWindowAttributes winattr;
		winattr.background_pixel = 0;
		winattr.border_pixel = 0;
		winattr.colormap = XCreateColormap(d_dpy, root, visinfo->visual, AllocNone);
		winattr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
		unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

		//std::cout << "Window depth " << visinfo->depth << ", w " << width << "x" << height << std::endl;
		d_win = XCreateWindow(d_dpy, root, 0, 0, width, height, 0, 
				visinfo->depth, InputOutput, visinfo->visual, mask, &winattr);

		/*
		// OpenGL 3.2
		int gl3attr[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 2,
			GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			None
		};

		d_ctx = glXCreateContextAttribsARB(d_dpy, fbcfg[0], NULL, true, gl3attr);
		*/
		

		XMapWindow(d_dpy, d_win);
		glXMakeCurrent(d_dpy, d_win, d_ctx);
		XUnmapWindow(d_dpy, d_win);
		/*
		std::cout << "OpenGL:\n\tvendor " << glGetString(GL_VENDOR) 
				<< "\n\trenderer " << glGetString(GL_RENDERER)
				<< "\n\tversion " << glGetString(GL_VERSION)
				<< "\n\tshader language " << glGetString(GL_SHADING_LANGUAGE_VERSION)
				<< "\n" << std::endl;
				*/
		/*
		int extCount;
		glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
		for (int i = 0; i < extCount; ++i)
			std::cout << "Extension " << i+1 << "/" << extCount << ": " << glGetStringi(GL_EXTENSIONS, i) << std::endl;
		*/
		glViewport(0, 0, width, height);
	}
	
	XFree(visinfo);
	XUnlockDisplay(d_dpy);
	return true;
}

context* create_unix_gl_context(RenderAPI api, unsigned int w, unsigned int h, 
									const std::string& title, bool show)
{
	if (api != RA_OPENGL)
		return 0;
	unix_gl_context* ctx_ptr = new unix_gl_context(w,h);
	if (!ctx_ptr->create(title, show)) {
		delete ctx_ptr;
		return 0;
	}
	return ctx_ptr;
}

		}
	}
}

#include "lib_begin.h"

namespace cgv {
	namespace render {
	
extern CGV_API context_factory_registration create_unix_gl_context_registration(gl::create_unix_gl_context);

	}
}


#endif
