#include <iostream>
#include <fstream>
#include <cgv/render/context.h>
#include <cgv/render/shader_code.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv/render/texture.h>
#include <cgv/render/textured_material.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>

#ifndef WIN32
#include <fltk/GlWindow.h>
#include <fltk/run.h>
#endif

using namespace cgv::render;
using namespace cgv::utils::file;
using namespace cgv::utils;

void perform_test();

#ifndef WIN32
struct fltk_gl_context : public gl::gl_context, public fltk::GlWindow
{
	fltk_gl_context(unsigned int _w = -1, unsigned int _h = -1) : fltk::GlWindow(_w,_h) {
		show();
	}

	bool is_created() const { return true; }
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


	/// return the current render pass
	RenderPass get_render_pass() const { return RP_NONE; }
	/// return the current render pass flags
	RenderPassFlags get_render_pass_flags() const { return RPF_NONE; }
	/// perform the given render task
	void render_pass(RenderPass render_pass = RP_MAIN, 
		RenderPassFlags render_pass_flags = RPF_ALL) {}
	/// return whether the context is currently in process of rendering
	bool in_render_process() const { return false; }
	/// return whether the context is current
	bool is_current() const { return true; }
	/// make the current context current
	bool make_current() const { const_cast<fltk_gl_context*>(this)->fltk::GlWindow::make_current(); return true; }
	//@}

	/// return the width of the window
	unsigned int get_width() const { return w(); }
	/// return the height of the window
	unsigned int get_height() const { return h(); }
	/// resize the context to the given dimensions
	void resize(unsigned int width, unsigned int height) { fltk::GlWindow::resize(width,height); }
	/// set a user defined background color
	void set_bg_color(float r, float g, float b, float a) {}
	/// the context will be redrawn when the system is idle again
	void post_redraw() { redraw(); }
	/// the context will be redrawn right now. This method cannot be called inside the following methods of a drawable: init, init_frame, draw, finish_draw
	void force_redraw() { redraw(); }

	/**@name font selection and measure*/
	//@{
	/// enable the given font face with the given size in pixels
	void enable_font_face(cgv::media::font::font_face_ptr font_face, float font_size) {}
	/// return the size in pixels of the currently enabled font face
	float get_current_font_size() const { return 12; }
	/// return the currently enabled font face
	cgv::media::font::font_face_ptr get_current_font_face() const { return cgv::media::font::font_face_ptr(); }
	//@}
	/// returns an output stream whose output is printed at the current cursor location
	std::ostream& output_stream() { return std::cout; }
	void draw() 
	{
		perform_test();
		exit(0);
	}
};
#endif

bool convert_to_string(const std::string& in_fn, const std::string& out_fn)
{
	std::string content = shader_code::read_code_file(in_fn);
	if (content.empty())
		return false;
	if (to_upper(get_extension(in_fn)[0]) == 'P')
		write(drop_extension(out_fn)+"."+get_extension(in_fn).substr(1), content.c_str(), content.length(), true);

	std::ofstream os(out_fn.c_str());
	if (os.fail())
		return false;
	std::string sn = get_file_name(in_fn);
	replace(sn, '.', '_');
	os << "const char* " << sn.c_str() << " =\"\\\n";

	for (unsigned int i=0; i<content.size(); ++i) {
		switch (content[i]) {
		case '\n' : os << "\\n\\\n"; break;
		case '\t' : os << "\\t"; break;
		case '"' : os << "\\\""; break;
		default: os << content[i]; break;
		}
	}
	os << "\";\n";
	return true;
}

int g_argc;
char** g_argv;
context* g_ctx_ptr;

void perform_test()
{
	bool shader_developer = false;
	char* options = getenv("CGV_OPTIONS");
	if (options)
		shader_developer = is_element("SHADER_DEVELOPER", to_upper(options), ';');

	if (getenv("CGV_DIR") != 0)
		get_shader_config()->shader_path = std::string(getenv("CGV_DIR"))+"/libs/cgv_gl/glsl";
	// check input file extension
	std::string ext = to_lower(get_extension(g_argv[1]));
	if (ext == "glpr") {
		// in case of shader program, build it from the file
		shader_program prog(true);
		if (prog.build_program(*g_ctx_ptr, g_argv[1], true)) {
			write(g_argv[2], "ok", 2, true);
			std::cout << "shader program ok (" << g_argv[1] << ")" << std::endl;
		}
		else {
			std::cerr << g_argv[1] << " (1) : glsl program error" << std::endl;
			if (!shader_developer)
				write(g_argv[2], "error", 2, true);
		}
	}
	else {
//		if (ext[0] != 'p') {
			// otherwise read and compile code
			shader_code code;
			if (code.read_and_compile(*g_ctx_ptr, g_argv[1])) {
				// convert the input file to a string declaration with the string
				convert_to_string(g_argv[1],g_argv[2]);
				//write(g_argv[2], "ok", 2, true);
				std::cout << "shader code ok (" << g_argv[1] << ")" << std::endl;
			}
			else {
				if (!shader_developer)
					convert_to_string(g_argv[1],g_argv[2]);
			}
//		}
/*		else {
			// convert the input file to a string declaration with the string
			convert_to_string(g_argv[1],g_argv[2]);
			//write(g_argv[2], "ok", 2, true);
			std::cout << "pre-shader code transformed to log file only (" << g_argv[1] << ")" << std::endl;
		}
		*/
	}
}

int main(int argc, char** argv)
{
	// check command line arguments
	if (argc != 3) {
		std::cout << "usage: shader_test.exe input_file log_file" << std::endl;
		return -1;
	}

	// create a context without window
#ifdef WIN32
	context* ctx_ptr = create_context();
#else
	context* ctx_ptr = new fltk_gl_context(200,200);
#endif
	if (!ctx_ptr) {
		std::cerr << "could not create context!" << std::endl;
		return -1;
	}
	g_argc= argc;
	g_argv = argv;
	g_ctx_ptr = ctx_ptr;
#ifdef WIN32
	perform_test();
#else
	fltk::run();
#endif
	// destroy context
	delete ctx_ptr;
	return 0;
}

namespace cgv {
	namespace render {
		namespace gl {

shader_program& ref_textured_material_prog(context& ctx)
{
	static shader_program prog;
	return prog;
}
/// set the program variables needed by the lighting.glsl shader
void set_lighting_parameters(context& ctx, shader_program& prog)
{
}
		}
void texture::ensure_state(context& ctx) const
{
	if (state_out_of_date) {
		ctx.texture_set_state(*this);
		state_out_of_date = false;
	}
}

bool texture::enable(context& ctx, int _tex_unit)
{
	if (!handle) {
		render_component::last_error = "attempt to enable texture that is not created";
		return false;
	}
	ensure_state(ctx);
	tex_unit = _tex_unit;
	return ctx.texture_enable(*this, tex_unit, get_nr_dimensions());
}
/// disable texture and restore state before last enable call
bool texture::disable(context& ctx)
{
	return ctx.texture_disable(*this, tex_unit, get_nr_dimensions());
}

/// change component format and clear internal format
void texture::set_component_format(const std::string& description)
{
	component_format::set_component_format(description);
	internal_format = 0;
	state_out_of_date = true;
}

/// return pointer to diffuse texture or 0 if non created
texture* textured_material::get_diffuse_texture() const
{
	return diffuse_texture;
}

/// return pointer to diffuse texture or 0 if non created
texture* textured_material::get_bump_texture() const
{
	return bump_texture;
}


/// return the currently set alpha test function
textured_material::AlphaTestFunc textured_material::get_alpha_test_func() const
{
	return alpha_test_func;
}


/// return the currently used alpha threshold used by the comparison alpha test functions
float textured_material::get_alpha_threshold() const
{
	return alpha_threshold;
}

/// return reference to currently set alpha test function
textured_material::AlphaTestFunc& textured_material::ref_alpha_test_func()
{
	return alpha_test_func;
}
/// return reference to currently used alpha threshold used by the comparison alpha test functions
float& textured_material::ref_alpha_threshold()
{
	return alpha_threshold;
}

	}
}
