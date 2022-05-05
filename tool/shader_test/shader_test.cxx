#include <iostream>
#include <fstream>
#include <cgv/base/register.h>
#include <cgv/render/context.h>
#include <cgv/render/shader_code.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv/render/texture.h>
#include <cgv/render/textured_material.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>
#include <cgv/utils/options.h>

#ifndef WIN32
#include <fltk/GlWindow.h>
#include <fltk/run.h>
#endif

using namespace cgv::base;
using namespace cgv::render;
using namespace cgv::utils::file;
using namespace cgv::utils;


// Disable registration debugging for more concise shader_test output
struct global_regdebug_disabler
{
	global_regdebug_disabler()
	{
		disable_registration_debugging();
	}
} grdd_instance;


int perform_test();

#ifndef WIN32
struct fltk_gl_context : public gl::gl_context, public fltk::GlWindow
{
	fltk_gl_context(unsigned int _w = -1, unsigned int _h = -1) : fltk::GlWindow(_w,_h) {
		show();
	}

	bool is_created() const { return true; }
	void attach_depth_buffer(bool attach = true) {}
	bool is_alpha_buffer_attached() const { return true; }
	void attach_alpha_buffer(bool attach = true) {}
	void detach_alpha_buffer() {}
	bool is_stencil_buffer_attached() const { return false; }
	void attach_stencil_buffer(bool attach = true) {}
	void detach_stencil_buffer() {}
	bool is_quad_buffer_supported() const { return false; }
	bool is_quad_buffer_attached() const { return false; }
	void attach_quad_buffer() {}
	void detach_quad_buffer() {}
	bool is_accum_buffer_attached() const { return false; }
	void attach_accumulation_buffer(bool attach = true) {}
	void detach_accumulation_buffer() {}
	bool is_multisample_enabled() const { return false; }
	void attach_multi_sample_buffer(bool attach = true) {}
	void enable_multisample() {}
	void disable_multisample() {}
	bool is_stereo_buffer_supported() const { return false; }
	void attach_stereo_buffer(bool attach = true) {}


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
	///
	void clear_current() const {}
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
		int exit_code = perform_test();
		exit(exit_code);
	}
};
#endif

bool convert_to_string(const std::string& in_fn, const std::string& out_fn, bool skip_comments = true)
{
	std::string content = shader_code::read_code_file(in_fn);
	if (content.empty())
		return false;
	if (to_upper(get_extension(in_fn)[0]) == 'P')
		write(drop_extension(out_fn)+"."+get_extension(in_fn).substr(1), content.c_str(), content.length(), true);
	// try to open output file
	std::ofstream os(out_fn.c_str());
	if (os.fail())
		return false;
	// encode in base64 if this a cgv option
	if (cgv::utils::has_option("ENCODE_SHADER_BASE64")) {
		// prepend a 'paragraph' char (ANSI hexadecimal code A7)
		static const unsigned char prefix[2] = {0xA7, 0x00};
		static const std::string prefix_str((char*)prefix);
		content = prefix_str + cgv::utils::encode_base64(content);
	}
	// stream out the string declaration
	std::string sn = get_file_name(in_fn);
	replace(sn, '.', '_');
	os << "const char* " << sn.c_str() << " =\"\\\n";
	// write out the content in form of a string
	bool last_is_newline = false;
	bool last_is_slash = false;
	bool last_is_hashtag = false;
	size_t written_chars = 0;
	for (unsigned int i=0; i<content.size(); ++i) {
		bool new_last_is_newline = false;
		bool new_last_is_slash = false;
		bool new_last_is_hashtag = false;
		switch (content[i]) {
		case '/':
			// in case of single line comment
			if (last_is_slash)
				// skip till end of line or end of content
				do { ++i; } while (i < content.size() && content[i] != '\n');
			else
				new_last_is_slash = true;
			break;
		case '*':
			// in case of single line comment
			if (last_is_slash) {
				// skip till end of line or end of content
				bool last_is_star = false;
				if (++i < content.size()) {
					do {
						last_is_star = content[i] == '*';
						++i;
					} while (i < content.size() && !(last_is_star && content[i] == '/'));
				}
			}
			else
			{
				os << content[i];
				++written_chars;
			}
			break;
		case '#':
		{
			// make sure defines are always placed at the start of a new line
			bool next_is_hashtag = false;
			if(i < content.size() - 1)
				if(content[i + 1] == '#')
					next_is_hashtag = true;

			if(!last_is_newline && !last_is_hashtag && !next_is_hashtag) {
				os << "\\n\\\n";
				written_chars += 3;
			}
			os << content[i];
			++written_chars;

			new_last_is_hashtag = true;
		}
			break;
		default:
			if (last_is_slash)
				os << '/';
			switch (content[i]) {
			case '\\': os << "\\\\"; written_chars += 2; break;
			case '\n': os << "\\n\\\n"; written_chars += 3; new_last_is_newline = true; break;
			case '\t': os << "\\t"; ++written_chars; break;
			case '"': os << "\\\""; written_chars += 2; break;
			default: os << content[i]; ++written_chars; break;
			}
		}
		// respect the Visual Studio C++ Compiler maximum string literal size of 16kB (with some tolerance)
		if(written_chars > 16000) {
			os << "\"\\\n\"";
			written_chars = 0;
		}
		last_is_newline = new_last_is_newline;
		last_is_slash = new_last_is_slash;
		last_is_hashtag = new_last_is_hashtag;
	}
	os << "\";\n";
	return true;
}

int g_argc;
char** g_argv;
context* g_ctx_ptr;

void stream_out_prog(std::ostream& os, const std::string& fn, const cgv::render::shader_define_map& defines)
{
	os << fn;
	if (!defines.empty())
		for (const auto& d : defines)
			os << "|" << d.first << "=" << d.second;
}

int perform_test()
{
	bool shader_developer = cgv::utils::has_option("SHADER_DEVELOPER");
	int exit_code = 0;
	if (get_shader_config()->shader_path.empty() && getenv("CGV_DIR") != 0) {
		get_shader_config()->shader_path = 
			std::string(getenv("CGV_DIR")) + "/libs/cgv_gl/glsl;" +
			std::string(getenv("CGV_DIR")) + "/libs/cgv_glutil/glsl;" +
			std::string(getenv("CGV_DIR")) + "/libs/cgv_glutil/glsl/2d;" +
			std::string(getenv("CGV_DIR")) + "/libs/plot/glsl;" +
			std::string(getenv("CGV_DIR")) + "/libs/cgv_proc";
	}
	// check input file extension
	std::string ext = to_lower(get_extension(g_argv[1]));
	if (ext == "glpr") {
		std::vector<shader_define_map> define_maps = shader_program::extract_instances(g_argv[1]);
		if (define_maps.empty())
			define_maps.push_back({});
		for (auto defines : define_maps) {
			// in case of shader program, build it from the file
			shader_program prog(true);
			if (prog.build_program(*g_ctx_ptr, g_argv[1], shader_developer, defines)) {
				convert_to_string(g_argv[1], g_argv[2]);
				//write(g_argv[2], "ok", 2, true);
				std::cout << "shader program ok (";
				stream_out_prog(std::cout, g_argv[1], defines);
				std::cout << ")" << std::endl;
			}
			else {
				if (!shader_developer)
					convert_to_string(g_argv[1], g_argv[2]);
				else {
					std::cout << "error:";
					stream_out_prog(std::cout, g_argv[1], defines);
					std::cout << " (1) : glsl program error" << std::endl;
					exit_code = 1;
				}
			}
		}
	}
	else {
		shader_code code;
		if (code.read_and_compile(*g_ctx_ptr, g_argv[1], cgv::render::ST_DETECT, shader_developer)) {
			// convert the input file to a string declaration with the string
			convert_to_string(g_argv[1], g_argv[2]);
			// write(g_argv[2], "ok", 2, true);
			std::cout << "shader code ok (" << g_argv[1] << ")" << std::endl;
		} else {
			if (!shader_developer)
				convert_to_string(g_argv[1], g_argv[2]);
			else
				exit_code = 1;
		}
	}
	return exit_code;
}

int main(int argc, char** argv)
{
	int exit_code = 0;
	get_render_config()->show_error_on_console = false;
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
		std::cout << "error: could not create context!" << std::endl;
		return -1;
	}
	g_argc = argc;
	g_argv = argv;
	g_ctx_ptr = ctx_ptr;
#ifdef WIN32
	exit_code = perform_test();
#else
	exit_code = fltk::run();
#endif
	// destroy context
	delete ctx_ptr;
	return exit_code;
}
#ifndef SHADER_TEST_APP
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
void texture::ensure_state(const context& ctx) const
{
	if (state_out_of_date) {
		ctx.texture_set_state(*this);
		state_out_of_date = false;
	}
}

bool texture::enable(const context& ctx, int _tex_unit)
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
bool texture::disable(const context& ctx)
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
texture* textured_material::get_texture(int ti) const
{
	return 0;
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

void textured_material::enable_textures(context&)
{
}
void textured_material::disable_textures(context&)
{
}

	}
}
#endif