#include <cgv/base/group.h>
#include "gl_context.h"
#include "gl_tools.h"
#include <cgv_gl/gl/wgl.h>
#ifdef _WIN32
#undef TA_LEFT
#undef TA_TOP
#undef TA_RIGHT
#undef TA_BOTTOM
#endif
#include <cgv/base/base.h>
#include <cgv/base/action.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/textured_material.h>
#include <cgv/utils/scan.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/gui/event_handler.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/geom.h>
#include <cgv/math/inv.h>
#include <cgv/type/standard_types.h>
#include <cgv/os/clipboard.h>

using namespace cgv::base;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::os;
using namespace cgv::math;
using namespace cgv::media::font;
using namespace cgv::media::illum;

namespace cgv {
	namespace render {
		namespace gl {
			
GLenum map_to_gl(PrimitiveType primitive_type)
{
	static const GLenum gl_primitive_type[] = {
		GLenum(-1),
		GL_POINTS,
		GL_LINES,
		GL_LINES_ADJACENCY,
		GL_LINE_STRIP,
		GL_LINE_STRIP_ADJACENCY,
		GL_LINE_LOOP,
		GL_TRIANGLES,
		GL_TRIANGLES_ADJACENCY,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_STRIP_ADJACENCY,
		GL_TRIANGLE_FAN,
		GL_QUADS,
		GL_QUAD_STRIP,
		GL_POLYGON,
		GL_PATCHES
	};
	return gl_primitive_type[primitive_type];
}

GLenum map_to_gl(MaterialSide material_side)
{
	static const GLenum gl_material_side[] = {
		GLenum(0),
		GL_FRONT,
		GL_BACK,
		GL_FRONT_AND_BACK
	};
	return gl_material_side[material_side];
}

GLenum map_to_gl(AccessType access_type)
{
	static const GLenum gl_access_type[] = {
		GL_READ_ONLY,
		GL_WRITE_ONLY,
		GL_READ_WRITE
	};
	return gl_access_type[access_type];
}

GLenum map_to_gl(BlendFunction blend_function)
{
	static const GLenum gl_blend_func[] = {
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_CONSTANT_COLOR,
		GL_ONE_MINUS_CONSTANT_COLOR,
		GL_CONSTANT_ALPHA,
		GL_ONE_MINUS_CONSTANT_ALPHA,
		GL_SRC_ALPHA_SATURATE,
		GL_SRC1_COLOR,
		GL_ONE_MINUS_SRC1_COLOR,
		GL_SRC1_ALPHA,
		GL_ONE_MINUS_SRC1_ALPHA
	};
	return gl_blend_func[blend_function];
}

GLenum map_to_gl(CompareFunction compare_func)
{
	static const GLenum gl_compare_func[] = {
		GL_LEQUAL,
		GL_GEQUAL,
		GL_LESS,
		GL_GREATER,
		GL_EQUAL,
		GL_NOTEQUAL,
		GL_ALWAYS,
		GL_NEVER
	};
	return gl_compare_func[compare_func];
}

static const GLenum gl_depth_format_ids[] =
{
	GL_DEPTH_COMPONENT,
	GL_DEPTH_COMPONENT16,
	GL_DEPTH_COMPONENT24,
	GL_DEPTH_COMPONENT32
};

static const GLenum gl_color_buffer_format_ids[] =
{
	GL_RGB,
	GL_RGBA
};

static const char* depth_formats[] =
{
	"[D]",
	"uint16[D]",
	"uint32[D:24]",
	"uint32[D]",
	0
};

static const char* color_buffer_formats[] =
{
	"[R,G,B]",
	"[R,G,B,A]",
	0
};

GLenum get_tex_dim(TextureType texture_type)
{
	static const GLenum gl_texture_type[] = {
		GLenum(0),
		GL_TEXTURE_1D,
		GL_TEXTURE_2D,
		GL_TEXTURE_3D,
		GL_TEXTURE_1D_ARRAY,
		GL_TEXTURE_2D_ARRAY,
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_2D_MULTISAMPLE,
		GL_TEXTURE_2D_MULTISAMPLE_ARRAY
	};
	return gl_texture_type[texture_type];
}

GLenum get_tex_bind(TextureType texture_type)
{
	static const GLenum gl_tex_binding[] = {
		GLenum(0),
		GL_TEXTURE_BINDING_1D,
		GL_TEXTURE_BINDING_2D,
		GL_TEXTURE_BINDING_3D,
		GL_TEXTURE_BINDING_1D_ARRAY,
		GL_TEXTURE_BINDING_2D_ARRAY,
		GL_TEXTURE_BINDING_CUBE_MAP,
		GL_TEXTURE_BUFFER,
		GL_TEXTURE_BINDING_2D_MULTISAMPLE,
		GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY
	};
	return gl_tex_binding[texture_type];
}

GLenum map_to_gl(TextureWrap texture_wrap)
{
	static const GLenum gl_texture_wrap[] = {
		GL_REPEAT,
		GL_CLAMP,
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_BORDER,
		GL_MIRROR_CLAMP_EXT,
		GL_MIRROR_CLAMP_TO_EDGE_EXT,
		GL_MIRROR_CLAMP_TO_BORDER_EXT,
		GL_MIRRORED_REPEAT
	};
	return gl_texture_wrap[texture_wrap];
}

GLenum map_to_gl(TextureFilter filter_type)
{
	static const GLenum gl_texture_filter[] = {
		GL_NEAREST,
		GL_LINEAR,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};
	return gl_texture_filter[filter_type];
}

GLboolean map_to_gl(bool flag) {
	return flag ? GL_TRUE : GL_FALSE;
}

GLuint get_gl_id(const void* handle)
{
	return (const GLuint&)handle - 1;
}

void* get_handle(GLuint id)
{
	void* handle = 0;
	(GLuint&)handle = id + 1;
	return handle;
}

void gl_context::put_id(void* handle, void* ptr) const
{
	*static_cast<GLuint*>(ptr) = get_gl_id(handle);
}

/// set a very specific texture format. This should be called after the texture is constructed and before it is created.
void set_gl_format(texture& tex, GLuint gl_format, const std::string& component_format_description)
{
	tex.set_component_format(component_format_description);
	(GLuint&)tex.internal_format = gl_format;
}

/// return the texture format used for a given texture. If called before texture has been created, the function returns, which format would be chosen by the automatic format selection process.
GLuint get_gl_format(const texture& tex)
{
	if (tex.internal_format)
		return (const GLuint&)tex.internal_format;
	cgv::data::component_format best_cf;
	GLuint gl_format = find_best_texture_format(tex, &best_cf);
	return gl_format;
}


void gl_set_material_color(GLenum side, const cgv::media::illum::phong_material::color_type& c, float alpha, GLenum type)
{
	GLfloat v[4] = { c[0],c[1],c[2],c[3] * alpha };
	glMaterialfv(side, type, v);
}

/// enable a material without textures
void gl_set_material(const cgv::media::illum::phong_material& mat, MaterialSide ms, float alpha)
{
	if (ms == MS_NONE)
		return;
	unsigned side = map_to_gl(ms);
	gl_set_material_color(side, mat.get_ambient(), alpha, GL_AMBIENT);
	gl_set_material_color(side, mat.get_diffuse(), alpha, GL_DIFFUSE);
	gl_set_material_color(side, mat.get_specular(), alpha, GL_SPECULAR);
	gl_set_material_color(side, mat.get_emission(), alpha, GL_EMISSION);
	glMaterialf(side, GL_SHININESS, mat.get_shininess());
}

/// construct gl_context and attach signals
gl_context::gl_context()
{
	frame_buffer_stack.top()->handle = get_handle(0);
	max_nr_indices = 0;
	max_nr_vertices = 0;
	info_font_size = 14;
	show_help = false;
	show_stats = false;

	// set initial GL state from stack contents
	set_bg_color(get_bg_color());
	set_bg_depth(get_bg_depth());
	set_bg_stencil(get_bg_stencil());
	set_bg_accum_color(get_bg_accum_color());

	set_depth_test_state(get_depth_test_state());
	set_cull_state(get_cull_state());
	set_blend_state(get_blend_state());
	set_buffer_mask(get_buffer_mask());
}

/// return the used rendering API
RenderAPI gl_context::get_render_api() const
{
	return RA_OPENGL;
}

//GL_STACK_OVERFLOW, "stack overflow"
//GL_STACK_UNDERFLOW, "stack underflow",
std::string get_source_tag_name(GLenum tag)
{
	static std::map<GLenum, const char*> source_tags = {
			{ GL_DEBUG_SOURCE_API,             "api" },
			{ GL_DEBUG_SOURCE_WINDOW_SYSTEM,   "window system" },
			{ GL_DEBUG_SOURCE_SHADER_COMPILER, "shader compiler" },
			{ GL_DEBUG_SOURCE_THIRD_PARTY,     "3rd party" },
			{ GL_DEBUG_SOURCE_APPLICATION,     "application" },
			{ GL_DEBUG_SOURCE_OTHER,           "other" }
	};
	return source_tags[tag];
}

std::string get_type_tag_name(GLenum tag)
{
	static std::map<GLenum, const char*> type_tags = {
			{ GL_DEBUG_TYPE_ERROR,               "error" },
			{ GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "deprecated behavior" },
			{ GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,  "undefinded behavior" },
			{ GL_DEBUG_TYPE_PORTABILITY,         "portability" },
			{ GL_DEBUG_TYPE_PERFORMANCE,         "performance" },
			{ GL_DEBUG_TYPE_OTHER,               "other" },
			{ GL_DEBUG_TYPE_MARKER,              "marker" },
			{ GL_DEBUG_TYPE_PUSH_GROUP,          "push group" },
			{ GL_DEBUG_TYPE_POP_GROUP,           "pop group" }
	};
	return type_tags[tag];
}

std::string get_severity_tag_name(GLenum tag)
{
	static std::map<GLenum, const char*> severity_tags = {
			{ GL_DEBUG_SEVERITY_NOTIFICATION, "notification" },
			{ GL_DEBUG_SEVERITY_HIGH,         "high" },
			{ GL_DEBUG_SEVERITY_MEDIUM,       "medium" },
			{ GL_DEBUG_SEVERITY_LOW,          "low" }
	};
	return severity_tags[tag];
};

void GLAPIENTRY debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;
	const gl_context* ctx = reinterpret_cast<const gl_context*>(userParam);
	std::string msg(message, length);
	msg = std::string("GLDebug Message[") + cgv::utils::to_string(id) + "] from " + get_source_tag_name(source) + " of type " + get_type_tag_name(type) + " of severity " + get_severity_tag_name(severity) + "\n" + msg;
	ctx->error(msg);
}

/// define lighting mode, viewing pyramid and the rendering mode
bool gl_context::configure_gl()
{
	if (!ensure_glew_initialized()) {
		error("gl_context::configure_gl could not initialize glew");
		return false;
	}
	const GLubyte* version_string = glGetString(GL_VERSION);
	version_major = version_string[0] - '0';
	version_minor = version_string[2] - '0';
	if (version_major >= 3) {
		GLint context_flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
#ifdef WIN32
		// weird behavior under windows or just nvidia or just my laptop (Stefan)??
		debug = (context_flags & WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB) != 0;
		forward_compatible = (context_flags & WGL_CONTEXT_DEBUG_BIT_ARB) != 0;
#else
		debug = (context_flags & GLX_CONTEXT_DEBUG_BIT_ARB) != 0;
		forward_compatible = (context_flags & GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB) != 0;
#endif
	}
	else {
		debug = false;
		forward_compatible = false;
	}
	int version = 10 * version_major + version_minor;
	if (version >= 32) {
		GLint context_profile;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &context_profile);
#ifdef WIN32
		core_profile = (context_profile & WGL_CONTEXT_CORE_PROFILE_BIT_ARB) != 0;
#else
		core_profile = (context_profile & GLX_CONTEXT_CORE_PROFILE_BIT_ARB) != 0;
#endif
	}
	else
		core_profile = false;

	const GLubyte* vendor_c_string = glGetString(GL_VENDOR);
	std::string vendor_string(reinterpret_cast<const char*>(vendor_c_string));
	vendor_string = cgv::utils::to_upper(vendor_string);
	
	if (vendor_string.find("NVIDIA"))
		gpu_vendor = GPU_VENDOR_NVIDIA;
	else if (vendor_string.find("INTEL"))
		gpu_vendor = GPU_VENDOR_INTEL;
	else if (vendor_string.find("AMD") || vendor_string.find("ATI"))
		gpu_vendor = GPU_VENDOR_AMD;
	
#ifdef _DEBUG
	std::cout << "OpenGL version " << version_major << "." << version_minor << (core_profile?" (core)":"") << (debug?" (debug)":"") << (forward_compatible?" (forward_compatible)":"") << std::endl;
	const GLubyte* renderer_c_string = glGetString(GL_RENDERER);
	const GLubyte* glslversion_c_string = glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (vendor_c_string)
		std::cout << "   vendor     : " << vendor_c_string << std::endl;
	if (renderer_c_string)
		std::cout << "   renderer   : " << renderer_c_string << std::endl;
	if (glslversion_c_string)
		std::cout << "   glslversion: " << glslversion_c_string << std::endl;
#endif
	if (debug) {
		glEnable(GL_DEBUG_OUTPUT);
		if (version >= 43) {
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			if (!check_gl_error("gl_context::configure() debug output"))
				glDebugMessageCallback(debug_callback, this);
		}
	}
	//enable_font_face(info_font_face, info_font_size);
	// use the eye location to compute the specular lighting
	if (!core_profile) {
		glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
		// this makes opengl normalize all surface normals before lighting calculations,
		// which is essential when using scaling to deform tesselated primities
		glEnable(GL_NORMALIZE);

		// should be initialized by the driver, but better be safe than risk errors later
		glMatrixMode(GL_MODELVIEW);
	}
	set_viewport(ivec4(0, 0, get_width(), get_height()));
//	if (check_gl_error("gl_context::configure_gl before init of children"))
//		return false;
	
	group_ptr grp(dynamic_cast<group*>(this));
	single_method_action<cgv::render::drawable, bool, cgv::render::context&> sma(*this, &drawable::init, false, false);
	for (unsigned i = 0; i<grp->get_nr_children(); ++i)
		traverser(sma, "nc").traverse(grp->get_child(i));

//	if (check_gl_error("gl_context::configure_gl after init of children."))
//		return false;

	return true;
}

void gl_context::resize_gl()
{
	group_ptr grp(dynamic_cast<group*>(this));
	set_viewport(ivec4(0, 0, get_width(), get_height()));
	if (grp) {
		single_method_action_2<drawable, void, unsigned int, unsigned int> sma(get_width(), get_height(), &drawable::resize);
		traverser(sma).traverse(grp);
	}
}

void gl_context::set_bg_color(vec4 rgba) {
	glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
	context::set_bg_color(rgba);
}

void gl_context::set_bg_depth(float d) {
	glClearDepth(d);
	context::set_bg_depth(d);
}

void gl_context::set_bg_stencil(int s) {
	glClearStencil(s);
	context::set_bg_stencil(s);
}

void gl_context::set_bg_accum_color(vec4 rgba) {
	if(!core_profile)
		glClearAccum(rgba[0], rgba[1], rgba[2], rgba[3]);
	context::set_bg_accum_color(rgba);
}

void gl_context::clear_background(bool color_flag, bool depth_flag, bool stencil_flag, bool accum_flag) {
	GLenum bits = 0;
	if(color_flag)
		bits |= GL_COLOR_BUFFER_BIT;
	if(depth_flag)
		bits |= GL_DEPTH_BUFFER_BIT;
	if(stencil_flag)
		bits |= GL_STENCIL_BUFFER_BIT;
	if(accum_flag && !core_profile)
		bits |= GL_ACCUM_BUFFER_BIT;
	if(bits)
		glClear(bits);
}

/// overwrite function to return info font size in case no font is currently selected
float gl_context::get_current_font_size() const
{
	if (current_font_size == 0)
		return info_font_size;
	return current_font_size;
}
/// overwrite function to return info font face in case no font is currently selected
media::font::font_face_ptr gl_context::get_current_font_face() const
{
	if (current_font_face.empty())
		return info_font_face;
	return current_font_face;
}

void gl_context::init_render_pass()
{
	if (info_font_face.empty()) {
		font_ptr info_font = default_font(true);
		if (!info_font.empty()) {
			info_font_face = info_font->get_font_face(FFA_REGULAR);
			info_font_face->enable(this, info_font_size);
			if (current_font_face.empty())
				enable_font_face(info_font_face, info_font_size);
		}
	}
#ifdef WIN32
	wglSwapIntervalEXT(enable_vsync ? 1 : 0);
#else
	glXSwapIntervalEXT(glXGetCurrentDisplay(), glXGetCurrentDrawable(), enable_vsync ? 1 : 0);
#endif
	if (sRGB_framebuffer)
		glEnable(GL_FRAMEBUFFER_SRGB);
	else
		glDisable(GL_FRAMEBUFFER_SRGB);

	static cgv::render::RenderPassFlags last_render_pass_flags = get_default_render_pass_flags();
	cgv::render::RenderPassFlags current_render_pass_flags = get_render_pass_flags();
	if (current_render_pass_flags & RPF_SET_LIGHTS) {
		for (unsigned i = 0; i < nr_default_light_sources; ++i)
			set_light_source(default_light_source_handles[i], default_light_source[i], false);

		for (unsigned i = 0; i < nr_default_light_sources; ++i)
			if (current_render_pass_flags & RPF_SET_LIGHTS_ON)
				enable_light_source(default_light_source_handles[i]);
			else
				disable_light_source(default_light_source_handles[i]);
	}
	else if ((last_render_pass_flags & RPF_SET_LIGHTS) == 0) {
		for (unsigned i = 0; i < nr_default_light_sources; ++i)
			if (is_light_source_enabled(default_light_source_handles[i]))
				disable_light_source(default_light_source_handles[i]);
	}
	last_render_pass_flags = current_render_pass_flags;

	if (get_render_pass_flags()&RPF_SET_MATERIAL) {
		set_material(default_material);
	}
	if ((get_render_pass_flags()&RPF_ENABLE_MATERIAL) && !core_profile) {
		// this mode allows to define the ambient and diffuse color of the surface material
		// via the glColor commands
		glEnable(GL_COLOR_MATERIAL);
	}
	if (get_render_pass_flags()&RPF_SET_STATE_FLAGS) {
		// set some default settings
		enable_depth_test();
		set_cull_state(CM_BACKFACE);
		if (!core_profile)
			glEnable(GL_NORMALIZE);
	}
	if ((get_render_pass_flags()&RPF_SET_PROJECTION) != 0)
		set_projection_matrix(cgv::math::perspective4<double>(45.0, (double)get_width()/get_height(),0.001,1000.0));

	if ((get_render_pass_flags()&RPF_SET_MODELVIEW) != 0)
		set_modelview_matrix(cgv::math::look_at4<double>(vec3(0,0,10), vec3(0,0,0), vec3(0,1,0)));
	
	if (check_gl_error("gl_context::init_render_pass before init_frame"))
		return;

	group* grp = dynamic_cast<group*>(this);
	if (grp && (get_render_pass_flags()&RPF_DRAWABLES_INIT_FRAME)) {
		single_method_action<drawable,void,cgv::render::context&> sma(*this, &drawable::init_frame, true, true);
		traverser(sma).traverse(group_ptr(grp));
	}

	if (check_gl_error("gl_context::init_render_pass after init_frame"))
		return;
	// this defines the background color to which the frame buffer is set by glClear
	if(get_render_pass_flags() & RPF_SET_CLEAR_COLOR)
		set_bg_color(get_bg_color());
	// this defines the background depth buffer value set by glClear
	if(get_render_pass_flags() & RPF_SET_CLEAR_DEPTH)
		set_bg_depth(get_bg_depth());
	// this defines the background depth buffer value set by glClear
	if(get_render_pass_flags() & RPF_SET_CLEAR_STENCIL)
		set_bg_stencil(get_bg_stencil());
	// this defines the background color to which the accum buffer is set by glClear
	if(get_render_pass_flags() & RPF_SET_CLEAR_ACCUM && !core_profile)
		set_bg_accum_color(get_bg_accum_color());
	clear_background(
		get_render_pass_flags() & RPF_CLEAR_COLOR,
		get_render_pass_flags() & RPF_CLEAR_DEPTH,
		get_render_pass_flags() & RPF_CLEAR_STENCIL,
		get_render_pass_flags() & RPF_CLEAR_ACCUM
	);
}

///
void gl_context::finish_render_pass()
{
}

struct format_callback_handler : public traverse_callback_handler
{
	std::ostream& os;
	format_callback_handler(std::ostream& _os) : os(_os) 
	{
	}
	/// called before the children of a group node g are processed, return whether these should be skipped. If children are skipped, the on_leave_children callback is still called.
	bool on_enter_children(group*)
	{
		os << "\a";
		return false;
	}
	/// called when the children of a group node g have been left, return whether to terminate traversal
	bool on_leave_children(group*)
	{
		os << "\b";
		return false;
	}

};

void gl_context::draw_textual_info()
{
	if (show_help || show_stats) {
		rgba tmp = current_color;
		push_depth_test_state();
		disable_depth_test();

		push_pixel_coords();
		enable_font_face(info_font_face, info_font_size);

		set_cursor(20, get_height()-1-20);

		vec4 bg = get_bg_color();
		//if (bg_r + bg_g + bg_b < 1.5f)
		if (bg[0] + bg[1] + bg[2] < 1.5f)
			set_color(rgba(1, 1, 1, 1));
		else
			set_color(rgba(0, 0, 0, 1));

		// traverse objects for show_stats callback
		format_callback_handler fch(output_stream());
		group_ptr grp(dynamic_cast<group*>(this));
		if (grp && show_stats) {
			single_method_action<cgv::base::base, void, std::ostream&> sma(output_stream(), &cgv::base::base::stream_stats, false, false);
			traverser(sma, "nc").traverse(grp, &fch);
			output_stream() << std::endl;
		}
		//if (bg_r + bg_g + bg_b < 1.5f)
		if(bg[0] + bg[1] + bg[2] < 1.5f)
			set_color(rgba(1, 1, 0, 1));
		else
			set_color(rgba(0.4f, 0.3f, 0, 1));

		if (grp && show_help) {
			// collect help from myself and all children
			single_method_action<event_handler, void, std::ostream&> sma(output_stream(), &event_handler::stream_help, false, false);
			traverser(sma, "nc").traverse(grp, &fch);
			output_stream().flush();
		}
		pop_pixel_coords();
		pop_depth_test_state();
	}
}

void gl_context::perform_screen_shot()
{
	glFlush();
	data::data_view dv;
	if (!read_frame_buffer(dv))
		return;
	std::string ext("bmp");
	std::string exts = cgv::media::image::image_writer::get_supported_extensions();
	if (cgv::utils::is_element("png",exts))
		ext = "png";
	else if (cgv::utils::is_element("tif",exts))
		ext = "tif";
	cgv::media::image::image_writer wr(std::string("screen_shot.")+ext);
	if (wr.is_format_supported(*dv.get_format()))
		wr.write_image(dv);
}

/// get list of program uniforms
void gl_context::enumerate_program_uniforms(shader_program& prog, std::vector<std::string>& names, std::vector<int>* locations_ptr, std::vector<int>* sizes_ptr, std::vector<int>* types_ptr, bool show) const
{
	GLint count;
	glGetProgramiv(get_gl_id(prog.handle), GL_ACTIVE_UNIFORMS, &count);
	for (int i = 0; i < count; ++i) {
		GLchar name[1000];
		GLsizei length;
		GLint size;
		GLenum type;
		glGetActiveUniform(get_gl_id(prog.handle), i, 1000, &length, &size, &type, name);
		std::string name_str(name, length);
		names.push_back(name_str);
		if (sizes_ptr)
			sizes_ptr->push_back(size);
		if (types_ptr)
			types_ptr->push_back(type);
		int loc = glGetUniformLocation(get_gl_id(prog.handle), name_str.c_str());
		if (locations_ptr)
			locations_ptr->push_back(loc);
		if (show)
			std::cout << i << " at " << loc << " = " << name_str << ":" << type << "[" << size << "]" << std::endl;
	}
}

/// get list of program uniforms
void gl_context::enumerate_program_attributes(shader_program& prog, std::vector<std::string>& names, std::vector<int>* locations_ptr, std::vector<int>* sizes_ptr, std::vector<int>* types_ptr, bool show) const
{
	GLint count;
	glGetProgramiv(get_gl_id(prog.handle), GL_ACTIVE_ATTRIBUTES, &count);
	for (int i = 0; i < count; ++i) {
		GLchar name[1000];
		GLsizei length;
		GLint size;
		GLenum type;
		glGetActiveAttrib(get_gl_id(prog.handle), i, 1000, &length, &size, &type, name);
		std::string name_str(name, length);
		names.push_back(name_str);
		if (sizes_ptr)
			sizes_ptr->push_back(size);
		if (types_ptr)
			types_ptr->push_back(type);
		int loc = glGetAttribLocation(get_gl_id(prog.handle), name_str.c_str());
		if (locations_ptr)
			locations_ptr->push_back(loc);
		if (show)
			std::cout << i << " at " << loc << " = " << name_str << ":" << type << "[" << size << "]" << std::endl;
	}
}

/// set the current color
void gl_context::set_color(const rgba& clr)
{
	current_color = clr;
	if (support_compatibility_mode && !core_profile) {
		glColor4fv(&clr[0]);
	}
	if (shader_program_stack.empty())
		return;
	cgv::render::shader_program& prog = *static_cast<cgv::render::shader_program*>(shader_program_stack.top());
	if (!prog.does_context_set_color())
		return;
	int clr_loc = prog.get_color_index();
	if (clr_loc == -1)
		return;
	prog.set_attribute(*this, clr_loc, clr);
}

/// set the current material 
void gl_context::set_material(const cgv::media::illum::surface_material& material)
{
	if (support_compatibility_mode && !core_profile) {
		unsigned side = map_to_gl(MS_FRONT_AND_BACK);
		float alpha = 1.0f - material.get_transparency();
		gl_set_material_color(side, material.get_ambient_occlusion()*material.get_diffuse_reflectance(), alpha, GL_AMBIENT);
		gl_set_material_color(side, material.get_diffuse_reflectance(), alpha, GL_DIFFUSE);
		gl_set_material_color(side, material.get_specular_reflectance(), alpha, GL_SPECULAR);
		gl_set_material_color(side, material.get_emission(), alpha, GL_EMISSION);
		glMaterialf(side, GL_SHININESS, 1.0f/(material.get_roughness()+1.0f/128.0f));
	}
	context::set_material(material);
}

/// enable a material with textures
void gl_context::enable_material(textured_material& mat)
{
	set_textured_material(mat);
	mat.enable_textures(*this);	
}

/// disable a material with textures
void gl_context::disable_material(textured_material& mat)
{
	mat.disable_textures(*this);
	current_material_ptr = 0;
	current_material_is_textured = false;
}

void gl_context::destruct_render_objects()
{
	for (unsigned i = 0; i < 4; ++i)
		progs[i].destruct(*this);
}

/// return a reference to a shader program used to render without illumination
shader_program& gl_context::ref_default_shader_program(bool texture_support)
{
	if (!texture_support) {
		if (!progs[0].is_created()) {
			if (!progs[0].build_program(*this, "default.glpr")) {
				error("could not build default shader program from default.glpr");
				exit(0);
			}
			progs[0].specify_standard_uniforms(true, false, false, true);
			progs[0].specify_standard_vertex_attribute_names(*this, true, false, false);
			progs[0].allow_context_to_set_color(true);
		}
		return progs[0];
	}
	if (!progs[1].is_created()) {
		if (!progs[1].build_program(*this, "textured_default.glpr")) {
			error("could not build default shader program with texture support from textured_default.glpr");
			exit(0);
		}
		progs[1].set_uniform(*this, "texture", 0);
		progs[1].specify_standard_uniforms(true, false, false, true);
		progs[1].specify_standard_vertex_attribute_names(*this, true, false, true);
		progs[1].allow_context_to_set_color(true);
	}
	return progs[1];
}

/// return a reference to the default shader program used to render surfaces without textures
shader_program& gl_context::ref_surface_shader_program(bool texture_support)
{
	if (!texture_support) {
		if (!progs[2].is_created()) {
			if (!progs[2].build_program(*this, "default_surface.glpr")) {
				error("could not build surface shader program from default_surface.glpr");
				exit(0);
			}
			progs[2].specify_standard_uniforms(true, true, true, true);
			progs[2].specify_standard_vertex_attribute_names(*this, true, true, false);
			progs[2].allow_context_to_set_color(true);
		}
		return progs[2];
	}
	if (!progs[3].is_created()) {
		if (!progs[3].build_program(*this, "textured_surface.glpr")) {
			error("could not build surface shader program with texture support from textured_surface.glpr");
			exit(0);
		}
		progs[3].specify_standard_uniforms(true, true, true, true);
		progs[3].specify_standard_vertex_attribute_names(*this, true, true, true);
		progs[3].allow_context_to_set_color(true);
	}
	return progs[3];
}

void gl_context::on_lights_changed()
{
	if (support_compatibility_mode && !core_profile) {
		GLint max_nr_lights;
		glGetIntegerv(GL_MAX_LIGHTS, &max_nr_lights);
		for (GLint light_idx = 0; light_idx < max_nr_lights; ++light_idx) {
			if (light_idx >= int(get_nr_enabled_light_sources())) {
				glDisable(GL_LIGHT0 + light_idx);
				continue;
			}
			GLfloat col[4] = { 1,1,1,1 };
			const cgv::media::illum::light_source& light = get_light_source(get_enabled_light_source_handle(light_idx));
			(rgb&)(col[0]) = light.get_emission()*light.get_ambient_scale();
			glLightfv(GL_LIGHT0 + light_idx, GL_AMBIENT, col);
			(rgb&)(col[0]) = light.get_emission();
			glLightfv(GL_LIGHT0 + light_idx, GL_DIFFUSE, col);
			(rgb&)(col[0]) = light.get_emission();
			glLightfv(GL_LIGHT0 + light_idx, GL_SPECULAR, col);

			GLfloat pos[4] = { 0,0,0,light.get_type() == cgv::media::illum::LT_DIRECTIONAL ? 0.0f : 1.0f };
			(vec3&)(pos[0]) = light.get_position();
			glLightfv(GL_LIGHT0 + light_idx, GL_POSITION, pos);
			if (light.get_type() != cgv::media::illum::LT_DIRECTIONAL) {
				glLightf(GL_LIGHT0 + light_idx, GL_CONSTANT_ATTENUATION, light.get_constant_attenuation());
				glLightf(GL_LIGHT0 + light_idx, GL_LINEAR_ATTENUATION, light.get_linear_attenuation());
				glLightf(GL_LIGHT0 + light_idx, GL_QUADRATIC_ATTENUATION, light.get_quadratic_attenuation());
			}
			else {
				glLightf(GL_LIGHT0 + light_idx, GL_CONSTANT_ATTENUATION, 1);
				glLightf(GL_LIGHT0 + light_idx, GL_LINEAR_ATTENUATION, 0);
				glLightf(GL_LIGHT0 + light_idx, GL_QUADRATIC_ATTENUATION, 0);
			}
			if (light.get_type() == cgv::media::illum::LT_SPOT) {
				glLightf(GL_LIGHT0 + light_idx, GL_SPOT_CUTOFF, light.get_spot_cutoff());
				glLightf(GL_LIGHT0 + light_idx, GL_SPOT_EXPONENT, light.get_spot_exponent());
				glLightfv(GL_LIGHT0 + light_idx, GL_SPOT_DIRECTION, light.get_spot_direction());
			}
			else {
				glLightf(GL_LIGHT0 + light_idx, GL_SPOT_CUTOFF, 180.0f);
				glLightf(GL_LIGHT0 + light_idx, GL_SPOT_EXPONENT, 0.0f);
				static float dir[3] = { 0,0,1 };
				glLightfv(GL_LIGHT0 + light_idx, GL_SPOT_DIRECTION, dir);
			}
			glEnable(GL_LIGHT0 + light_idx);
		}
	}
	context::on_lights_changed();
}

void gl_context::tesselate_arrow(double length, double aspect, double rel_tip_radius, double tip_aspect, int res, bool edges)
{
	double cyl_radius = length*aspect;
	double cone_radius = rel_tip_radius*cyl_radius;
	double cone_length = cone_radius/tip_aspect;
	double cyl_length = length - cone_length;
	push_modelview_matrix();
	mul_modelview_matrix(cgv::math::scale4(cyl_radius,cyl_radius,0.5*cyl_length));
		push_modelview_matrix();
			mul_modelview_matrix(cgv::math::rotate4(180.0,1.0,0.0,0.0));
			tesselate_unit_disk(res, false, edges);
		pop_modelview_matrix();

		mul_modelview_matrix(cgv::math::translate4(0.0,0.0,1.0));
		tesselate_unit_cylinder(res, false, edges);

	mul_modelview_matrix(cgv::math::translate4(0.0, 0.0, 1.0));
	mul_modelview_matrix(cgv::math::scale4(rel_tip_radius, rel_tip_radius, cone_length / cyl_length));
		push_modelview_matrix();
			mul_modelview_matrix(cgv::math::rotate4(180.0, 1.0, 0.0, 0.0));
				tesselate_unit_disk(res, false, edges);
		pop_modelview_matrix();
	mul_modelview_matrix(cgv::math::translate4(0.0, 0.0, 1.0));
		tesselate_unit_cone(res, false, edges);
	pop_modelview_matrix();
}

/// helper function that multiplies a rotation to modelview matrix such that vector is rotated onto target
void gl_context::rotate_vector_to_target(const dvec3& vector, const dvec3& target)
{
	double angle;
	dvec3 axis;
	compute_rotation_axis_and_angle_from_vector_pair(vector, target, axis, angle);
	mul_modelview_matrix(cgv::math::rotate4<double>(180.0 / M_PI * angle, axis));
}

///
void gl_context::tesselate_arrow(const dvec3& start, const dvec3& end, double aspect, double rel_tip_radius, double tip_aspect, int res, bool edges)
{
	if ((start - end).length() < 1e-8) {
		error("ignored tesselate arrow called with start and end closer then 1e-8");
		return;
	}
	push_modelview_matrix();
		mul_modelview_matrix(cgv::math::translate4<double>(start));
		rotate_vector_to_target(dvec3(0, 0, 1), end - start);
		tesselate_arrow((end-start).length(), aspect, rel_tip_radius, tip_aspect, res, edges);
	pop_modelview_matrix();
}

void gl_context::draw_light_source(const light_source& l, float i, float light_scale)
{	
	set_color(i*l.get_emission());
	push_modelview_matrix();
	switch (l.get_type()) {
	case LT_DIRECTIONAL :
		mul_modelview_matrix(cgv::math::scale4<double>(light_scale, light_scale, light_scale));
		tesselate_arrow(vec3(0.0f), l.get_position(), 0.1f,2.0f,0.5f);
		break;
	case LT_POINT :
		mul_modelview_matrix(
			cgv::math::translate4<double>(l.get_position())*
			cgv::math::scale4<double>(vec3(0.3f*light_scale)));
		tesselate_unit_sphere();
		break;
	case LT_SPOT :
		mul_modelview_matrix(
			cgv::math::translate4<double>(l.get_position())*
			cgv::math::scale4<double>(vec3(light_scale))
		);
		rotate_vector_to_target(dvec3(0, 0, -1), l.get_spot_direction());
		{
			float t = tan(l.get_spot_cutoff()*(float)M_PI/180);
			if (l.get_spot_cutoff() > 45.0f)
				mul_modelview_matrix(cgv::math::scale4<double>(1, 1, 0.5f / t));
			else
				mul_modelview_matrix(cgv::math::scale4<double>(t, t, 0.5f));
			mul_modelview_matrix(cgv::math::translate4<double>(0, 0, -1));
			push_cull_state();
			set_cull_state(CM_OFF);
			tesselate_unit_cone();
			pop_cull_state();
		}
	}
	pop_modelview_matrix();
}

void gl_context::announce_external_frame_buffer_change(void*& fbo_handle)
{
	if (frame_buffer_stack.empty()) {
		error("gl_context::announce_external_frame_buffer_change() called with empty frame buffer stack");
		return;
	}
	GLint fbo_id = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo_id);
	fbo_handle = frame_buffer_stack.top()->handle;
	frame_buffer_stack.top()->handle = get_handle(fbo_id);
}

void gl_context::recover_from_external_frame_buffer_change(void* fbo_handle)
{
	glBindFramebuffer(GL_FRAMEBUFFER, get_gl_id(fbo_handle));
	if (frame_buffer_stack.empty())
		return;
	frame_buffer_stack.top()->handle = fbo_handle;
}

void gl_context::announce_external_viewport_change(ivec4& cgv_viewport_storage)
{
	if (window_transformation_stack.empty()) {
		error("gl_context::announce_external_viewport_change() called with empty window transformation stack");
		return;
	}
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	cgv_viewport_storage = window_transformation_stack.top().front().viewport;
	window_transformation_stack.top().front().viewport = ivec4(vp[0], vp[1], vp[2], vp[3]);
}

void gl_context::recover_from_external_viewport_change(const ivec4& cgv_viewport_storage)
{
	glViewport(cgv_viewport_storage[0], cgv_viewport_storage[1], cgv_viewport_storage[2], cgv_viewport_storage[3]);
	if (window_transformation_stack.empty())
		return;
	window_transformation_stack.top().front().viewport = cgv_viewport_storage;
}

/// use this to push transformation matrices on the stack such that x and y coordinates correspond to window coordinates
void gl_context::push_pixel_coords()
{
	push_projection_matrix();
	push_modelview_matrix();

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	if (support_compatibility_mode && !core_profile) {
		// push projection matrix
		glMatrixMode(GL_PROJECTION);
		// set orthogonal projection
		glLoadIdentity();
		glOrtho(vp[0], vp[0]+vp[2], vp[1], vp[1]+vp[3], -1, 1);
		// push modelview matrix
		glMatrixMode(GL_MODELVIEW);
		// use identity for modelview
		glLoadIdentity();
	}
	set_modelview_matrix(cgv::math::identity4<double>());
	set_projection_matrix(cgv::math::ortho4<double>(vp[0], vp[0] + vp[2], vp[1], vp[1]+vp[3], -1, 1));
}

/// pop previously changed transformation matrices 
void gl_context::pop_pixel_coords()
{
	pop_modelview_matrix();
	pop_projection_matrix();
}

/// implement according to specification in context class
bool gl_context::read_frame_buffer(data::data_view& dv, 
											  unsigned int x, unsigned int y, FrameBufferType buffer_type, 
											  TypeId type, data::ComponentFormat cf, int w, int h)
{
	const cgv::data::data_format* df = dv.get_format();
	if (df) {
		w = int(df->get_width());
		h = int(df->get_height());
		type = df->get_component_type();
		cf = df->get_standard_component_format();
		if (w < 1 || h < 1) {
			error(std::string("read_frame_buffer: received invalid dimensions (") + cgv::utils::to_string(w) + "," + cgv::utils::to_string(h) + ")");
			return false;
		}
	}
	else {
		if (w < 1 || h < 1) {
			GLint vp[4];
			glGetIntegerv(GL_VIEWPORT,vp);
			w = w < 1 ? vp[2] : w;
			h = h < 1 ? vp[3] : h;
		}
	}
	GLuint gl_type = map_to_gl(type);
	if (gl_type == 0) {
		error(std::string("read_frame_buffer: could not make component type ")+cgv::type::info::get_type_name(df->get_component_type())+" to gl type");
		return false;
	}
	GLuint gl_format = GL_DEPTH_COMPONENT;
	if (cf != cgv::data::CF_D) {
		gl_format = GL_STENCIL_INDEX;
		if (cf != cgv::data::CF_S) {
			gl_format = map_to_gl(cf);
			if (gl_format == GL_RGB && cf != cgv::data::CF_RGB) {
				error(std::string("read_frame_buffer: could not match component format ") + cgv::utils::to_string(df->get_component_format()));
				return false;
			}
		}
	}
	GLenum gl_buffer = GL_BACK;
	if (buffer_type < FB_BACK)
		gl_buffer = GL_COLOR_ATTACHMENT0+buffer_type;
	else {
		switch (buffer_type) {
		case FB_FRONT :       gl_buffer = GL_FRONT; break;
		case FB_BACK  :       gl_buffer = GL_BACK; break;
		case FB_FRONT_LEFT :  gl_buffer = GL_FRONT_LEFT; break;
		case FB_FRONT_RIGHT : gl_buffer = GL_FRONT_RIGHT; break;
		case FB_BACK_LEFT  :  gl_buffer = GL_BACK_LEFT; break;
		case FB_BACK_RIGHT  : gl_buffer = GL_BACK_RIGHT; break;
		default:
			error(std::string("invalid buffer type ")+cgv::utils::to_string(buffer_type));
			return false;
		}
	}
	// after all necessary information could be extracted, ensure that format 
	// and data view are created
	if (!df) {
		df = new cgv::data::data_format(w,h,type,cf);
		dv.~data_view();
		new(&dv) data::data_view(df);
		dv.manage_format(true);
	}
	GLint old_buffer, old_pack;
	glGetIntegerv(GL_READ_BUFFER, &old_buffer);
	glGetIntegerv(GL_PACK_ALIGNMENT, &old_pack);
	glReadBuffer(gl_buffer);
	glPixelStorei(GL_PACK_ALIGNMENT, df->get_component_alignment());
	glReadPixels(x,y,w,h,gl_format,gl_type,dv.get_ptr<void>());
	glReadBuffer(old_buffer);
	glPixelStorei(GL_PACK_ALIGNMENT, old_pack);
	dv.reflect_horizontally();
	return true;
}


void render_vertex(int k, const float* vertices, const float* normals, const float* tex_coords,
						  const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, bool flip_normals)
{
	if (normals && normal_indices) {
		if (flip_normals) {
			float n[3] = { -normals[3*normal_indices[k]],-normals[3*normal_indices[k]+1],-normals[3*normal_indices[k]+2] };
			glNormal3fv(n);
		}
		else
			glNormal3fv(normals+3*normal_indices[k]);
	}
	if (tex_coords && tex_coord_indices)
		glTexCoord2fv(tex_coords+2*tex_coord_indices[k]);
	glVertex3fv(vertices+3*vertex_indices[k]);
}

attribute_array_binding*& get_aab_ptr()
{
	static attribute_array_binding* aab_ptr = 0;
	return aab_ptr;
}

vertex_buffer*& get_vbo_ptr()
{
	static vertex_buffer* vbo_ptr = 0;
	return vbo_ptr;
}

bool gl_context::release_attributes(const float* normals, const float* tex_coords, const int* normal_indices, const int* tex_coord_indices) const
{
	shader_program* prog_ptr = static_cast<shader_program*>(get_current_program());
	if (!prog_ptr || prog_ptr->get_position_index() == -1)
		return false;

	attribute_array_binding*& aab_ptr = get_aab_ptr();
	if (!aab_ptr) {
		attribute_array_binding::disable_global_array(*this, prog_ptr->get_position_index());
		if (prog_ptr->get_normal_index() != -1 && normals && normal_indices)
			attribute_array_binding::disable_global_array(*this, prog_ptr->get_normal_index());
		if (prog_ptr->get_texcoord_index() != -1 && tex_coords && tex_coord_indices)
			attribute_array_binding::disable_global_array(*this, prog_ptr->get_texcoord_index());
	}
	else {
		aab_ptr->disable(const_cast<gl_context&>(*this));
		aab_ptr->disable_array(*this, prog_ptr->get_position_index());
		if (prog_ptr->get_normal_index() != -1 && normals && normal_indices)
			aab_ptr->disable_global_array(*this, prog_ptr->get_normal_index());
		if (prog_ptr->get_texcoord_index() != -1 && tex_coords && tex_coord_indices)
			aab_ptr->disable_global_array(*this, prog_ptr->get_texcoord_index());
		vertex_buffer*& vbo_ptr = get_vbo_ptr();
		vbo_ptr->destruct(*this);
		delete vbo_ptr;
		vbo_ptr = 0;
	}
	return true;
}

bool gl_context::prepare_attributes(std::vector<vec3>& P, std::vector<vec3>& N, std::vector<vec2>& T, unsigned nr_vertices,
	const float* vertices, const float* normals, const float* tex_coords,
	const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, bool flip_normals) const
{
	unsigned i;
	shader_program* prog_ptr = static_cast<shader_program*>(get_current_program());
	if (!prog_ptr || prog_ptr->get_position_index() == -1)
		return false;
	P.resize(nr_vertices);
	for (i = 0; i < nr_vertices; ++i)
		P[i] = *reinterpret_cast<const vec3*>(vertices + 3 * vertex_indices[i]);

	if (prog_ptr->get_normal_index() != -1 && normals && normal_indices) {
		N.resize(nr_vertices);
		for (i = 0; i < nr_vertices; ++i) {
			N[i] = *reinterpret_cast<const vec3*>(normals + 3 * normal_indices[i]);
			if (flip_normals)
				N[i] = -N[i];
		}
	}
	if (prog_ptr->get_texcoord_index() != -1 && tex_coords && tex_coord_indices) {
		T.resize(nr_vertices);
		for (i = 0; i < nr_vertices; ++i)
			T[i] = *reinterpret_cast<const vec2*>(tex_coords + 2 * tex_coord_indices[i]);
	}

	attribute_array_binding*& aab_ptr = get_aab_ptr();
	if (core_profile && !aab_ptr) {
		aab_ptr = new attribute_array_binding();
		aab_ptr->create(*this);
	}
	if (!aab_ptr) {
		attribute_array_binding::set_global_attribute_array<>(*this, prog_ptr->get_position_index(), P);
		attribute_array_binding::enable_global_array(*this, prog_ptr->get_position_index());
		if (prog_ptr->get_normal_index() != -1) {
			if (normals && normal_indices) {
				attribute_array_binding::set_global_attribute_array<>(*this, prog_ptr->get_normal_index(), N);
				attribute_array_binding::enable_global_array(*this, prog_ptr->get_normal_index());
			}
			else
				attribute_array_binding::disable_global_array(*this, prog_ptr->get_normal_index());
		}
		if (prog_ptr->get_texcoord_index() != -1) {
			if (tex_coords && tex_coord_indices) {
				attribute_array_binding::set_global_attribute_array<>(*this, prog_ptr->get_texcoord_index(), T);
				attribute_array_binding::enable_global_array(*this, prog_ptr->get_texcoord_index());
			}
			else
				attribute_array_binding::disable_global_array(*this, prog_ptr->get_texcoord_index());
		}
	}
	else {
		vertex_buffer*& vbo_ptr = get_vbo_ptr();
		if (!vbo_ptr)
			vbo_ptr = new vertex_buffer();
		else
			vbo_ptr->destruct(*this);
		vbo_ptr->create(*this, P.size() * sizeof(vec3) + N.size() * sizeof(vec3) + T.size() * sizeof(vec2));
		vbo_ptr->replace(const_cast<gl_context&>(*this), 0, &P.front(), P.size());
		size_t nml_off = P.size() * sizeof(vec3);
		size_t tex_off = nml_off;
		if (!N.empty()) {
			vbo_ptr->replace(const_cast<gl_context&>(*this), nml_off, &N.front(), N.size());
			tex_off += N.size() * sizeof(vec2);
		}
		if (!T.empty())
			vbo_ptr->replace(const_cast<gl_context&>(*this), tex_off, &T.front(), T.size());

		type_descriptor td3 = element_descriptor_traits<vec3>::get_type_descriptor(P.front());
		aab_ptr->set_attribute_array(*this, prog_ptr->get_position_index(), td3, *vbo_ptr, 0, P.size());
		aab_ptr->enable_array(*this, prog_ptr->get_position_index());
		if (prog_ptr->get_normal_index() != -1) {
			if (normals && normal_indices) {
				aab_ptr->set_attribute_array(*this, prog_ptr->get_normal_index(), td3, *vbo_ptr, nml_off, N.size());
				aab_ptr->enable_array(*this, prog_ptr->get_normal_index());
			}
			else
				aab_ptr->disable_array(*this, prog_ptr->get_normal_index());
		}
		if (prog_ptr->get_texcoord_index() != -1) {
			if (tex_coords && tex_coord_indices) {
				type_descriptor td2 = element_descriptor_traits<vec2>::get_type_descriptor(T.front());
				aab_ptr->set_attribute_array(*this, prog_ptr->get_texcoord_index(), td2, *vbo_ptr, tex_off, T.size());
				aab_ptr->enable_array(*this, prog_ptr->get_texcoord_index());
			}
			else
				aab_ptr->disable_array(*this, prog_ptr->get_texcoord_index());
		}
		aab_ptr->enable(const_cast<gl_context&>(*this));
	}
	return true;
}

/// pass geometry of given faces to current shader program and generate draw calls to render lines for the edges
void gl_context::draw_edges_of_faces(
	const float* vertices, const float* normals, const float* tex_coords,
	const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices,
	int nr_faces, int face_degree, bool flip_normals) const
{
	if (draw_in_compatibility_mode) {
		int k = 0;
		for (int i = 0; i < nr_faces; ++i) {
			glBegin(GL_LINE_LOOP);
			for (int j = 0; j < face_degree; ++j, ++k)
				render_vertex(k, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals);
			glEnd();
		}
		return;
	}
	unsigned nr_vertices = face_degree * nr_faces;
	std::vector<vec3> P, N;
	std::vector<vec2> T;
	if (!prepare_attributes(P, N, T, nr_vertices, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals))
		return;
	for (int i = 0; i < nr_faces; ++i)
		glDrawArrays(GL_LINE_LOOP, i*face_degree, face_degree);
	release_attributes(normals, tex_coords, normal_indices, tex_coord_indices);
}


void gl_context::draw_elements_void(GLenum mode, size_t total_count, GLenum type, size_t type_size, const void* indices) const
{
	ensure_configured();
	size_t drawn = 0;
	const cgv::type::uint8_type* index_ptr = static_cast<const cgv::type::uint8_type*>(indices);
	while (drawn < total_count) {
		size_t count = total_count - drawn;
		if (count > max_nr_indices)
			count = size_t(max_nr_indices);
		glDrawElements(mode, GLsizei(count), type, index_ptr + drawn * type_size);
		drawn += count;
	}
}

size_t max_nr_indices, max_nr_vertices;
void gl_context::ensure_configured() const
{
	if (max_nr_indices != 0)
		return;
	glGetInteger64v(GL_MAX_ELEMENTS_INDICES, reinterpret_cast<GLint64*>(&max_nr_indices));
	glGetInteger64v(GL_MAX_ELEMENTS_VERTICES, reinterpret_cast<GLint64*>(&max_nr_vertices));
}

/// pass geometry of given strip or fan to current shader program and generate draw calls to render lines for the edges
void gl_context::draw_edges_of_strip_or_fan(
	const float* vertices, const float* normals, const float* tex_coords,
	const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices,
	int nr_faces, int face_degree, bool is_fan, bool flip_normals) const
{
	int s = face_degree - 2;
	int i, k = 2;
	std::vector<GLuint> I;
	I.push_back(0);	I.push_back(1);
	for (i = 0; i < nr_faces; ++i) {
		if (is_fan) {
			I.push_back(k - 1);	I.push_back(k);
			I.push_back(0);	I.push_back(k);
			continue;
		}
		if (s == 1) {
			I.push_back(k - 1);	I.push_back(k);
			I.push_back(k - 2);	I.push_back(k);
		}
		else {
			I.push_back(k - 1);	I.push_back(k + 1);
			I.push_back(k - 2);	I.push_back(k);
			I.push_back(k);	I.push_back(k + 1);
		}
		k += 2;
	}

	if (draw_in_compatibility_mode) {
		glBegin(GL_LINES);
		for (GLuint j:I)
			render_vertex(j, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals);
		glEnd();
		return;
	}
	unsigned nr_vertices = 2 + (face_degree - 2) * nr_faces;
	std::vector<vec3> P, N;
	std::vector<vec2> T;
	if (!prepare_attributes(P, N, T, nr_vertices, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals))
		return;
	draw_elements(GL_LINES, I.size(), &I[0]);
	release_attributes(normals, tex_coords, normal_indices, tex_coord_indices);
}

void gl_context::draw_faces(
	const float* vertices, const float* normals, const float* tex_coords, 
	const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
	int nr_faces, int face_degree, bool flip_normals) const
{
	if (draw_in_compatibility_mode) {
		int k = 0;
		if (face_degree < 5)
			glBegin(face_degree == 3 ? GL_TRIANGLES : GL_QUADS);
		for (int i = 0; i < nr_faces; ++i) {
			if (face_degree >= 5)
				glBegin(GL_POLYGON);
			for (int j = 0; j < face_degree; ++j, ++k)
				render_vertex(k, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals);
			if (face_degree >= 5)
				glEnd();
		}
		if (face_degree < 5)
			glEnd();
		return;
	}
	unsigned nr_vertices = face_degree * nr_faces;
	std::vector<vec3> P, N;
	std::vector<vec2> T;
	if (!prepare_attributes(P, N, T, nr_vertices, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals))
		return;
	/*
	for (unsigned x = 0; x < nr_vertices; ++x) {
		std::cout << x << ": [" << P[x] << "]";
		if (N.size() > 0)
			std::cout << ", <" << N[x] << ">";
		if (T.size() > 0) 
			std::cout << ", {" << T[x] << "}";
		std::cout << std::endl;
	}
	*/
	if (face_degree == 3)
		glDrawArrays(GL_TRIANGLES, 0, nr_vertices);
	else {
		for (int i = 0; i < nr_faces; ++i) {
			glDrawArrays(GL_TRIANGLE_FAN, i*face_degree, face_degree);
		}
	}
	release_attributes(normals, tex_coords, normal_indices, tex_coord_indices);
}

void gl_context::draw_strip_or_fan(
		const float* vertices, const float* normals, const float* tex_coords, 
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool is_fan, bool flip_normals) const
{
	int s = face_degree - 2;
	int k = 2;
	if (draw_in_compatibility_mode) {
		glBegin(face_degree == 3 ? (is_fan ? GL_TRIANGLE_FAN : GL_TRIANGLE_STRIP) : GL_QUAD_STRIP);
		render_vertex(0, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals);
		render_vertex(1, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals);
		for (int i = 0; i < nr_faces; ++i)
			for (int j = 0; j < s; ++j, ++k)
				render_vertex(k, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals);
		glEnd();
		return;
	}
	unsigned nr_vertices = 2 + (face_degree-2) * nr_faces;
	std::vector<vec3> P, N;
	std::vector<vec2> T;
	if (!prepare_attributes(P, N, T, nr_vertices, vertices, normals, tex_coords, vertex_indices, normal_indices, tex_coord_indices, flip_normals))
		return;
	glDrawArrays(is_fan ? GL_TRIANGLE_FAN : GL_TRIANGLE_STRIP, 0, nr_vertices);
	release_attributes(normals, tex_coords, normal_indices, tex_coord_indices);
}

void gl_context::set_depth_test_state(DepthTestState state) {
	if(state.enabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	glDepthFunc(map_to_gl(state.test_func));
	context::set_depth_test_state(state);
}

void gl_context::set_depth_func(CompareFunction func) {
	glDepthFunc(map_to_gl(func));
	context::set_depth_func(func);
}

void gl_context::enable_depth_test() {
	glEnable(GL_DEPTH_TEST);
	context::enable_depth_test();
}

void gl_context::disable_depth_test() {
	glDisable(GL_DEPTH_TEST);
	context::disable_depth_test();
}

void gl_context::set_cull_state(CullingMode culling_mode) {
	switch(culling_mode) {
	case CM_OFF:
		glDisable(GL_CULL_FACE);
		break;
	case CM_BACKFACE:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	case CM_FRONTFACE:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	default:
		break;
	}
	context::set_cull_state(culling_mode);
}

void gl_context::set_blend_state(BlendState state) {
	if(state.enabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	if(GLEW_EXT_blend_func_separate) {
		glBlendFuncSeparate(
			map_to_gl(state.src_color),
			map_to_gl(state.dst_color),
			map_to_gl(state.src_alpha),
			map_to_gl(state.dst_alpha)
		);
	} else {
		glBlendFunc(map_to_gl(state.src_color), map_to_gl(state.dst_color));
	}
	context::set_blend_state(state);
}

void gl_context::set_blend_func(BlendFunction src_factor, BlendFunction dst_factor) {
	glBlendFunc(map_to_gl(src_factor), map_to_gl(dst_factor));
	context::set_blend_func(src_factor, dst_factor);
}

void gl_context::set_blend_func_separate(BlendFunction src_color_factor, BlendFunction dst_color_factor, BlendFunction src_alpha_factor, BlendFunction dst_alpha_factor) {
	glBlendFuncSeparate(
		map_to_gl(src_color_factor),
		map_to_gl(dst_color_factor),
		map_to_gl(src_alpha_factor),
		map_to_gl(dst_alpha_factor)
	);
	context::set_blend_func_separate(src_color_factor, dst_color_factor, src_alpha_factor, dst_alpha_factor);
}

void gl_context::enable_blending() {
	glEnable(GL_BLEND);
	context::enable_blending();
}

void gl_context::disable_blending() {
	glDisable(GL_BLEND);
	context::disable_blending();
}

void gl_context::set_buffer_mask(BufferMask mask) {
	glDepthMask(map_to_gl(mask.depth_flag));
	glColorMask(
		map_to_gl(mask.red_flag),
		map_to_gl(mask.green_flag),
		map_to_gl(mask.blue_flag),
		map_to_gl(mask.alpha_flag)
	);
	context::set_buffer_mask(mask);
}

void gl_context::set_depth_mask(bool flag) {
	glDepthMask(map_to_gl(flag));
	context::set_depth_mask(flag);
}

void gl_context::set_color_mask(bvec4 flags) {
	glColorMask(
		map_to_gl(flags[0]),
		map_to_gl(flags[1]),
		map_to_gl(flags[2]),
		map_to_gl(flags[3])
	);
	context::set_color_mask(flags);
}

/// return homogeneous 4x4 viewing matrix, which transforms from world to eye space
dmat4 gl_context::get_modelview_matrix() const
{
	if (support_compatibility_mode && !core_profile) {
		GLdouble V[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, V);
		return dmat4(4,4,V);
	}
	if (modelview_matrix_stack.empty())
		return identity4<double>();
	return modelview_matrix_stack.top();
}

/// return homogeneous 4x4 projection matrix, which transforms from eye to clip space
dmat4 gl_context::get_projection_matrix() const
{
	if (support_compatibility_mode && !core_profile) {
		GLdouble P[16];
		glGetDoublev(GL_PROJECTION_MATRIX, P);
		return dmat4(4,4,P);
	}
	if (projection_matrix_stack.empty())
		return identity4<double>();
	return projection_matrix_stack.top();
}
/// restore previous viewport and depth range arrays defining the window transformations
void gl_context::pop_window_transformation_array()
{
	context::pop_window_transformation_array();
	update_window_transformation_array();
}

void gl_context::update_window_transformation_array()
{
	const std::vector<window_transformation>& wta = window_transformation_stack.top();
	if (wta.size() == 1) {
		const ivec4& viewport = wta.front().viewport;
		const dvec2& depth_range = wta.front().depth_range;
		glViewport(viewport[0], viewport[1], (GLsizei)viewport[2], (GLsizei)viewport[3]);
		glScissor(viewport[0], viewport[1], (GLsizei)viewport[2], (GLsizei)viewport[3]);
		glDepthRange(depth_range[0], depth_range[1]);
	}
	else {
		for (GLuint array_index = 0; array_index < (GLuint)wta.size(); ++array_index) {
			const ivec4& viewport    = wta[array_index].viewport;
			const dvec2& depth_range = wta[array_index].depth_range;
			glViewportIndexedf(array_index, (GLfloat)viewport[0], (GLfloat)viewport[1], (GLfloat)viewport[2], (GLfloat)viewport[3]);
			glScissorIndexed(array_index, viewport[0], viewport[1], (GLsizei)viewport[2], (GLsizei)viewport[3]);
			glDepthRangeIndexed(array_index, (GLclampd)depth_range[0], (GLclampd)depth_range[1]);
		}
	}
}

unsigned gl_context::get_max_window_transformation_array_size() const
{
	GLint max_value = 1;
	if (GLEW_VERSION_4_1)
		glGetIntegerv(GL_MAX_VIEWPORTS, &max_value);
	return max_value;
}

/// set the current viewport or one of the viewports in the window transformation array
void gl_context::set_viewport(const ivec4& viewport, int array_index)
{
	size_t nr = window_transformation_stack.top().size();
	context::set_viewport(viewport, array_index);
	if (nr != window_transformation_stack.top().size())
		update_window_transformation_array();
	else {
		if (array_index < 1) {
			glViewport(viewport[0], viewport[1], (GLsizei)viewport[2], (GLsizei)viewport[3]);
			glScissor(viewport[0], viewport[1], (GLsizei)viewport[2], (GLsizei)viewport[3]);
		}
		else {
			glViewportIndexedf(array_index, (GLfloat)viewport[0], (GLfloat)viewport[1], (GLfloat)viewport[2], (GLfloat)viewport[3]);
			glScissorIndexed(array_index, viewport[0], viewport[1], (GLsizei)viewport[2], (GLsizei)viewport[3]);
		}
	}
}

/// set the current depth range or one of the depth ranges in the window transformation array
void gl_context::set_depth_range(const dvec2& depth_range, int array_index)
{
	size_t nr = window_transformation_stack.top().size();
	context::set_depth_range(depth_range, array_index);
	if (nr != window_transformation_stack.top().size())
		update_window_transformation_array();
	else {
		if (array_index < 1)
			glDepthRange(depth_range[0], depth_range[1]);
		else
			glDepthRangeIndexed(array_index, (GLclampd)depth_range[0], (GLclampd)depth_range[1]);
	}
}
/*
/// return homogeneous 4x4 projection matrix, which transforms from clip to device space
gl_context::dmat4 gl_context::get_device_matrix() const
{
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	dmat4 D; D.zeros();
	D(0, 0) = 0.5*vp[2];
	D(0, 3) = 0.5*vp[2] + vp[0];
	D(1, 1) = -0.5*vp[3]; // flip y-coordinate
	D(1, 3) = get_height() - 0.5*vp[3] - vp[1];
	D(2, 2) = 0.5;
	D(2, 3) = 0.5;
	D(3, 3) = 1.0;
	return D;
}
*/
void gl_context::set_modelview_matrix(const dmat4& V)
{
	if (support_compatibility_mode && !core_profile) {
		GLint mm;
		glGetIntegerv(GL_MATRIX_MODE, &mm);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixd(V);
		glMatrixMode(mm);
	}
	context::set_modelview_matrix(V);
}

void gl_context::set_projection_matrix(const dmat4& P)
{
	if (support_compatibility_mode && !core_profile) {
		GLint mm;
		glGetIntegerv(GL_MATRIX_MODE, &mm);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(P);
		glMatrixMode(mm);
	}
	context::set_projection_matrix(P);
}

/// read the device z-coordinate from the z-buffer for the given device x- and y-coordinates
double gl_context::get_window_z(int x_window, int y_window) const
{
	GLfloat z_window;

	if (!in_render_process() && !is_current())
		make_current();

	glReadPixels(x_window, y_window, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z_window);
	/*
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "gl error:";
		switch (err) {
			case GL_INVALID_ENUM :      std::cout << "invalid enum"; break;
			case GL_INVALID_VALUE :     std::cout << "invalid value"; break;
			case GL_INVALID_OPERATION : std::cout << "invalid operation"; break;
			case GL_STACK_OVERFLOW :    std::cout << "stack overflow"; break;
			case GL_STACK_UNDERFLOW :   std::cout << "stack underflow"; break;
			case GL_OUT_OF_MEMORY :     std::cout << "out of memory"; break;
			default:                    std::cout << "unknown error"; break;
		}
		std::cout << std::endl;

	}
	*/
	return z_window;
}

cgv::data::component_format gl_context::texture_find_best_format(
				const cgv::data::component_format& cf, 
				render_component& rc, const std::vector<cgv::data::data_view>* palettes) const
{
	GLuint& gl_format = (GLuint&)rc.internal_format;
	cgv::data::component_format best_cf;
	gl_format = find_best_texture_format(cf, &best_cf, palettes);
	return best_cf;
}

std::string gl_error_to_string(GLenum eid) {
	switch (eid) {
	case GL_NO_ERROR: return "";
	case GL_INVALID_ENUM: return "invalid enum";
	case GL_INVALID_VALUE: return "invalid value";
	case GL_INVALID_OPERATION: return "invalid operation";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffe";
	case GL_OUT_OF_MEMORY: return "out of memory";
	case GL_STACK_UNDERFLOW: return "stack underflow";
	case GL_STACK_OVERFLOW: return "stack overflow";
	default: 
		return "undefined error (id: " + std::to_string(eid) + ")";
	}
	//return std::string((const char*)gluErrorString(eid));
}

std::string gl_error() {
	GLenum eid = glGetError();
	return gl_error_to_string(eid);
}

bool gl_context::check_gl_error(const std::string& where, const cgv::render::render_component* rc) const
{
	GLenum eid = glGetError();
	if (eid == GL_NO_ERROR)
		return false;
	std::string error_string = where + ": " + gl_error_to_string(eid);
	error(error_string, rc);
	return true;
}

bool gl_context::check_texture_support(TextureType tt, const std::string& where, const cgv::render::render_component* rc) const
{
	switch (tt) {
	case TT_3D:
		if (!GLEW_VERSION_1_2) {
			error(where + ": 3D texture not supported", rc);
			return false;
		}
		break;
	case TT_CUBEMAP:
		if (!GLEW_VERSION_1_3) {
			error(where + ": cubemap texture not supported", rc);
			return false;
		}
	default:
		break;
	}
	return true;
}

bool gl_context::check_shader_support(ShaderType st, const std::string& where, const cgv::render::render_component* rc) const
{
	switch (st) {
	case ST_COMPUTE:
		if (GLEW_VERSION_4_3)
			return true;
		else {
			error(where+": compute shader need not supported OpenGL version 4.3", rc);
			return false;
		}
	case ST_TESS_CONTROL:
	case ST_TESS_EVALUATION:
		if (GLEW_VERSION_4_0)
			return true;
		else {
			error(where+": tessellation shader need not supported OpenGL version 4.0", rc);
			return false;
		}
	case ST_GEOMETRY:
		if (GLEW_VERSION_3_2)
			return true;
		else {
			error(where + ": geometry shader need not supported OpenGL version 3.2", rc);
			return false;
		}
	default:
		if (GLEW_VERSION_2_0)
			return true;
		else {
			error(where + ": shaders need not supported OpenGL version 2.0", rc);
			return false;
		}
	}
}

bool gl_context::check_fbo_support(const std::string& where, const cgv::render::render_component* rc) const
{
	if (!GLEW_VERSION_3_0) {
		error(where + ": framebuffer objects not supported", rc);
		return false;
	}
	return true;
}

GLuint gl_context::texture_generate(texture_base& tb) const
{
	if (!check_texture_support(tb.tt, "gl_context::texture_generate", &tb))
		return get_gl_id(0);
	GLuint tex_id = get_gl_id(0);
	glGenTextures(1, &tex_id);
	if (glGetError() == GL_INVALID_OPERATION)
		error("gl_context::texture_generate: attempt to create texture inside glBegin-glEnd-block", &tb);
	return tex_id;
}

int gl_context::query_integer_constant(ContextIntegerConstant cic) const
{
	GLint gl_const;
	switch (cic) {
		case MAX_NR_GEOMETRY_SHADER_OUTPUT_VERTICES :
			gl_const = GL_MAX_GEOMETRY_OUTPUT_VERTICES;
			break;
	}
	GLint value;
	glGetIntegerv(gl_const, &value);
	return value;
}

GLuint gl_context::texture_bind(TextureType tt, GLuint tex_id) const
{
	GLint tmp_id;
	glGetIntegerv(get_tex_bind(tt), &tmp_id);
	glBindTexture(get_tex_dim(tt), tex_id);
	return tmp_id;
}

void gl_context::texture_unbind(TextureType tt, GLuint tmp_id) const
{
	glBindTexture(get_tex_dim(tt), tmp_id);
}

bool gl_context::texture_create(texture_base& tb, cgv::data::data_format& df) const
{
	GLuint gl_format = (const GLuint&) tb.internal_format;
	
	if (tb.tt == TT_UNDEF)
		tb.tt = (TextureType)df.get_nr_dimensions();
	GLuint tex_id = texture_generate(tb);
	if (tex_id == get_gl_id(0))
		return false;
	GLuint tmp_id = texture_bind(tb.tt, tex_id);

	// extract component type
	unsigned int transfer_format = map_to_gl(df.get_standard_component_format(), df.get_integer_interpretation());
	if (transfer_format == -1) {
		error("could not determine transfer format", &tb);
		return false;
	}
	switch (tb.tt) {
	case TT_1D :
		glTexImage1D(GL_TEXTURE_1D, 0, 
			gl_format, GLsizei(df.get_width()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		break;
	case TT_1D_ARRAY :
		glTexImage2D(GL_TEXTURE_1D_ARRAY, 0,
			gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		break;
	case TT_2D :
		glTexImage2D(GL_TEXTURE_2D, 0, gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		break;
	case TT_MULTISAMPLE_2D:
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, tb.nr_multi_samples, gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), map_to_gl(tb.fixed_sample_locations));
		break;
	case TT_2D_ARRAY :
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), GLsizei(df.get_depth()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		break;
	case TT_MULTISAMPLE_2D_ARRAY:
		glTexStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, tb.nr_multi_samples, gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), GLsizei(df.get_depth()), map_to_gl(tb.fixed_sample_locations));
		break;
	case TT_3D :
		glTexImage3D(GL_TEXTURE_3D, 0,
			gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), GLsizei(df.get_depth()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		break;
	case TT_CUBEMAP :
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,
			gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0,
			gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0,
			gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0,
			gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0,
			gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0,
			gl_format, GLsizei(df.get_width()), GLsizei(df.get_height()), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
	default:
		break;
	}
	if (check_gl_error("gl_context::texture_create", &tb)) {
		glDeleteTextures(1, &tex_id);
		texture_unbind(tb.tt, tmp_id);
		return false;
	}

	texture_unbind(tb.tt, tmp_id);
	tb.have_mipmaps = false;
	tb.handle = get_handle(tex_id);
	return true;
}

bool gl_context::texture_create(
							texture_base& tb,
							cgv::data::data_format& target_format,
							const cgv::data::const_data_view& data,
							int level, int cube_side, int num_array_layers, const std::vector<cgv::data::data_view>* palettes) const
{
	// query the format to be used for the texture
	GLuint gl_tex_format = (const GLuint&) tb.internal_format;

	// define the texture type from the data format and the cube_side parameter
	tb.tt = (TextureType)data.get_format()->get_nr_dimensions();
	if(cube_side > -1) {
		if(tb.tt == TT_2D)
			tb.tt = TT_CUBEMAP;
	} else if(num_array_layers != 0) {
		if(num_array_layers < 0) {
			// automatic inference of layers from texture dimensions
			unsigned n_dims = data.get_format()->get_nr_dimensions();
			if(n_dims == 2)
				tb.tt = TT_1D_ARRAY;
			if(n_dims == 3)
				tb.tt = TT_2D_ARRAY;
		} else {
			switch(tb.tt) {
			case TT_1D: tb.tt = TT_1D_ARRAY; break;
			case TT_2D: tb.tt = TT_2D_ARRAY; break;
			case TT_3D: tb.tt = TT_2D_ARRAY; break;
			default: 
				break;
			}
		}
	}
	// create texture is not yet done
	GLuint tex_id;
	if (tb.is_created()) 
		tex_id = get_gl_id(tb.handle);
	else {
		tex_id = texture_generate(tb);
		if (tex_id == get_gl_id(0))
			return false;
		tb.handle = get_handle(tex_id);
	}

	// bind texture
	GLuint tmp_id = texture_bind(tb.tt, tex_id);

	// load data to texture
	tb.have_mipmaps = load_texture(data, gl_tex_format, level, cube_side, num_array_layers, palettes);
	bool result = !check_gl_error("gl_context::texture_create", &tb);
	// restore old texture
	texture_unbind(tb.tt, tmp_id);
	return result;
}

bool gl_context::texture_create_from_buffer(
						texture_base& tb, 
						cgv::data::data_format& df, 
						int x, int y, int level) const
{
	GLuint gl_format = (const GLuint&) tb.internal_format;

	tb.tt = (TextureType)df.get_nr_dimensions();
	if (tb.tt != TT_2D) {
		tb.last_error = "texture creation from buffer only possible for 2d textures";
		return false;
	}
	GLuint tex_id;
	if (tb.is_created()) 
		tex_id = get_gl_id(tb.handle);
	else {
		tex_id = texture_generate(tb);
		if (tex_id == get_gl_id(0))
			return false;
		tb.handle = get_handle(tex_id);
	}
	GLuint tmp_id = texture_bind(tb.tt, tex_id);

	// check mipmap type
	bool gen_mipmap = level == -1;
	if (gen_mipmap)
		level = 0;

	glCopyTexImage2D(GL_TEXTURE_2D, level, gl_format, x, y, GLsizei(df.get_width()), GLsizei(df.get_height()), 0);
	bool result = false;
	std::string error_string("gl_context::texture_create_from_buffer: ");
	switch (glGetError()) {
	case GL_NO_ERROR :
		result = true;
		break;
	case GL_INVALID_ENUM : 
		error_string += "target was not an accepted value."; 
		break;
	case GL_INVALID_VALUE : 
		error_string += "level was less than zero or greater than log sub 2(max), where max is the returned value of GL_MAX_TEXTURE_SIZE.\n"
							 "or border was not zero or 1.\n"
							 "or width was less than zero, greater than 2 + GL_MAX_TEXTURE_SIZE; or width cannot be represented as 2n + 2 * border for some integer n.";
		break;
	case GL_INVALID_OPERATION : 
		error_string += "glCopyTexImage2D was called between a call to glBegin and the corresponding call to glEnd.";
		break;
	default:
		error_string += gl_error_to_string(glGetError());
		break;
	}
	texture_unbind(tb.tt, tmp_id);
	if (!result)
		error(error_string, &tb);
	else
		if (gen_mipmap)
			result = texture_generate_mipmaps(tb, tb.tt == TT_CUBEMAP ? 2 : (int)tb.tt);

	return result;
}

bool gl_context::texture_replace(
						texture_base& tb, 
						int x, int y, int z, 
						const cgv::data::const_data_view& data, 
						int level, const std::vector<cgv::data::data_view>* palettes) const
{
	if (!tb.is_created()) {
		error("gl_context::texture_replace: attempt to replace in not created texture", &tb);
		return false;
	}
	// determine dimension from location arguments
	unsigned int dim = 1;
	if (y != -1) {
		++dim;
		if (z != -1)
			++dim;
	}
	// check consistency
	if (tb.tt == TT_CUBEMAP) {
		if (dim != 3) {
			error("gl_context::texture_replace: replace on cubemap without the side defined", &tb);
			return false;
		}
		if (z < 0 || z > 5) {
			error("gl_context::texture_replace: replace on cubemap with invalid side specification", &tb);
			return false;
		}
	}
	else {
		if (tb.tt != dim) {
			error("gl_context::texture_replace: replace on texture with invalid position specification", &tb);
			return false;
		}
	}

	// bind texture
	GLuint tmp_id = texture_bind(tb.tt, get_gl_id(tb.handle));
	tb.have_mipmaps = replace_texture(data, level, x, y, z, palettes) || tb.have_mipmaps;
	bool result = !check_gl_error("gl_context::texture_replace", &tb);
	texture_unbind(tb.tt, tmp_id);
	return result;
}

bool gl_context::texture_replace_from_buffer(
							texture_base& tb, 
							int x, int y, int z, 
							int x_buffer, int y_buffer, 
							unsigned int width, unsigned int height, 
							int level) const
{
	if (!tb.is_created()) {
		error("gl_context::texture_replace_from_buffer: attempt to replace in not created texture", &tb);
		return false;
	}
	// determine dimension from location arguments
	unsigned int dim = 2;
	if (z != -1)
		++dim;

	// consistency checks
	if (tb.tt == TT_CUBEMAP) {
		if (dim != 3) {
			error("gl_context::texture_replace_from_buffer: replace on cubemap without the side defined", &tb);
			return false;
		}
		if (z < 0 || z > 5) {
			error("gl_context::texture_replace_from_buffer: replace on cubemap without invalid side specification", &tb);
			return false;
		}
	}
	else {
		if (tb.tt != dim) {
			tb.last_error = "replace on texture with invalid position specification";
			return false;
		}
	}
	// check mipmap type
	bool gen_mipmap = level == -1;
	if (gen_mipmap)
		level = 0;

	// bind texture
	GLuint tmp_id = texture_bind(tb.tt, get_gl_id(tb.handle));
	switch (tb.tt) {
	case TT_2D :      glCopyTexSubImage2D(GL_TEXTURE_2D, level, x, y, x_buffer, y_buffer, width, height); break;
	case TT_3D :      glCopyTexSubImage3D(GL_TEXTURE_3D, level, x, y, z, x_buffer, y_buffer, width, height); break;
	case TT_CUBEMAP : glCopyTexSubImage2D(get_gl_cube_map_target(z), level, x, y, x_buffer, y_buffer, width, height); 
	default: break;
	}
	bool result = !check_gl_error("gl_context::texture_replace_from_buffer", &tb);
	texture_unbind(tb.tt, tmp_id);

	if (result && gen_mipmap)
		result = texture_generate_mipmaps(tb, tb.tt == TT_CUBEMAP ? 2 : (int)tb.tt);

	return result;
}

bool gl_context::texture_create_mipmaps(texture_base& tb, cgv::data::data_format& df) const
{
	GLuint gl_format = (const GLuint&)tb.internal_format;
		
	// extract component type
	unsigned int transfer_format = map_to_gl(df.get_standard_component_format(), df.get_integer_interpretation());

	if(transfer_format == -1) {
		error("could not determine transfer format", &tb);
		return false;
	}

	// extract texture size and compute number of mip-levels
	uvec3 size(unsigned(df.get_width()), unsigned(df.get_height()), unsigned(df.get_depth()));

	unsigned max_size = cgv::math::max_value(size);
	unsigned num_levels = 1 + static_cast<unsigned>(log2(static_cast<float>(max_size)));

	// compute mip-level sizes
	std::vector<uvec3> level_sizes(num_levels);
	level_sizes[0] = size;

	for(unsigned level = 1; level < num_levels; ++level) {
		uvec3 level_size = level_sizes[level - 1];
		level_size = level_size / 2u;
		level_size = cgv::math::max(level_size, uvec3(1u));
		level_sizes[level] = level_size;
	}

	GLuint tmp_id = texture_bind(tb.tt, get_gl_id(tb.handle));

	bool result = true;

	switch(tb.tt) {
	case TT_1D:
		for(unsigned level = 1; level < num_levels; ++level) {
			uvec3 level_size = level_sizes[level];
			glTexImage1D(GL_TEXTURE_1D, level, gl_format, level_size.x(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		}
		break;
	case TT_1D_ARRAY:
		//glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, gl_format, df.get_width(), df.get_height(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		error("create mipmaps not implemented for 1D array textures", &tb);
		result = false;
		break;
	case TT_2D:
		for(unsigned level = 1; level < num_levels; ++level) {
			uvec3 level_size = level_sizes[level];
			glTexImage2D(GL_TEXTURE_2D, level, gl_format, level_size.x(), level_size.y(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		}
		break;
	case TT_MULTISAMPLE_2D:
		//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, tb.nr_multi_samples, gl_format, df.get_width(), df.get_height(), map_to_gl(tb.fixed_sample_locations));
		error("create mipmaps not implemented for 2D multisample textures", &tb);
		result = false;
		break;
	case TT_2D_ARRAY:
		//glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, gl_format, df.get_width(), df.get_height(), df.get_depth(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		error("create mipmaps not implemented for 2D array textures", &tb);
		result = false;
		break;
	case TT_MULTISAMPLE_2D_ARRAY:
		//glTexStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, tb.nr_multi_samples, gl_format, df.get_width(), df.get_height(), df.get_depth(), map_to_gl(tb.fixed_sample_locations));
		error("create mipmaps not implemented for 2D multisample array textures", &tb);
		result = false;
		break;
	case TT_3D:
		for(unsigned level = 1; level < num_levels; ++level) {
			uvec3 level_size = level_sizes[level];
			glTexImage3D(GL_TEXTURE_3D, level, gl_format, level_size.x(), level_size.y(), level_size.z(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		}
		break;
	case TT_CUBEMAP:
		/*glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,
						gl_format, df.get_width(), df.get_height(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0,
						gl_format, df.get_width(), df.get_height(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0,
						gl_format, df.get_width(), df.get_height(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0,
						gl_format, df.get_width(), df.get_height(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0,
						gl_format, df.get_width(), df.get_height(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0,
						gl_format, df.get_width(), df.get_height(), 0, transfer_format, GL_UNSIGNED_BYTE, 0);
		*/
		error("create mipmaps not implemented for cubemap textures", &tb);
		result = false;
	default:
		break;
	}

	if(check_gl_error("gl_context::texture_create_mipmaps", &tb))
		result = false;

	if(result)
		tb.have_mipmaps = true;
	
	texture_unbind(tb.tt, tmp_id);
	return result;
}

bool gl_context::texture_generate_mipmaps(texture_base& tb, unsigned int dim) const
{
	GLuint tmp_id = texture_bind(tb.tt,get_gl_id(tb.handle));

	bool is_array = tb.tt == TT_1D_ARRAY || tb.tt == TT_2D_ARRAY;
	bool is_cubemap = tb.tt == TT_CUBEMAP;
	std::string error_string;
	bool result = generate_mipmaps(dim, is_cubemap, is_array, &error_string);
	if (result)
		tb.have_mipmaps = true;
	else
		error(std::string("gl_context::texture_generate_mipmaps: ") + error_string);

	texture_unbind(tb.tt, tmp_id);
	return result;
}

bool gl_context::texture_destruct(texture_base& tb) const
{
	if (!tb.is_created()) {
		error("gl_context::texture_destruct: attempt to destruct not created texture", &tb);
		return false;
	}
	GLuint tex_id = get_gl_id(tb.handle);
	glDeleteTextures(1, &tex_id);
	bool result = !check_gl_error("gl_context::texture_destruct", &tb);
	tb.handle = 0;
	return result;
}

bool gl_context::texture_set_state(const texture_base& tb) const
{
	if (tb.tt == TT_UNDEF) {
		error("gl_context::texture_set_state: attempt to set state on texture without type", &tb);
		return false;
	}
	GLuint tex_id = (GLuint&) tb.handle - 1;
	if (tex_id == -1) {
		error("gl_context::texture_set_state: attempt of setting texture state of not created texture", &tb);
		return false;
	}
	GLint tmp_id = texture_bind(tb.tt, tex_id);

	if (tb.tt != TT_MULTISAMPLE_2D && tb.tt != TT_MULTISAMPLE_2D_ARRAY) {
		glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_MIN_FILTER, map_to_gl(tb.min_filter));
		glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_MAG_FILTER, map_to_gl(tb.mag_filter));
		if (tb.min_filter == TF_ANISOTROP)
			glTexParameterf(get_tex_dim(tb.tt), GL_TEXTURE_MAX_ANISOTROPY_EXT, tb.anisotropy);
		else
			glTexParameterf(get_tex_dim(tb.tt), GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
		glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_COMPARE_FUNC, map_to_gl(tb.compare_function));
		glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_COMPARE_MODE, (tb.use_compare_function ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE));
		if (!core_profile)
			glTexParameterf(get_tex_dim(tb.tt), GL_TEXTURE_PRIORITY, tb.priority);
		glTexParameterfv(get_tex_dim(tb.tt), GL_TEXTURE_BORDER_COLOR, tb.border_color);
		//	if (tb.border_color[0] >= 0.0f)
		glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_WRAP_S, map_to_gl(tb.wrap_s));
		if (tb.tt > TT_1D)
			glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_WRAP_T, map_to_gl(tb.wrap_t));
		if (tb.tt == TT_3D)
			glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_WRAP_R, map_to_gl(tb.wrap_r));
	}

	bool result = !check_gl_error("gl_context::texture_set_state", &tb);
	texture_unbind(tb.tt, tmp_id);
	return result;
}

bool gl_context::texture_enable(texture_base& tb, int tex_unit, unsigned int dim) const
{
	if (dim < 1 || dim > 3) {
		error("gl_context::texture_enable: invalid texture dimension", &tb);
		return false;
	}
	GLuint tex_id = (GLuint&) tb.handle - 1;
	if (tex_id == -1) {
		error("gl_context::texture_enable: texture not created", &tb);
		return false;
	}
	if (tex_unit >= 0) {
		if (!GLEW_VERSION_1_3) {
			error("gl_context::texture_enable: multi texturing not supported", &tb);
			return false;
		}
		glActiveTexture(GL_TEXTURE0+tex_unit);
	}
	GLint& old_binding = (GLint&) tb.user_data;
	glGetIntegerv(get_tex_bind(tb.tt), &old_binding);
	++old_binding;
	glBindTexture(get_tex_dim(tb.tt), tex_id);
	// glEnable is not needed for texture arrays and will throw an invalid enum error
	//if(!(tb.tt == TT_1D_ARRAY || tb.tt == TT_2D_ARRAY))
	//	glEnable(get_tex_dim(tb.tt));
	bool result = !check_gl_error("gl_context::texture_enable", &tb);
	if (tex_unit >= 0)
		glActiveTexture(GL_TEXTURE0);
	return result;
}

bool gl_context::texture_disable(
						texture_base& tb, 
						int tex_unit, unsigned int dim) const
{
	if (dim < 1 || dim > 3) {
		error("gl_context::texture_disable: invalid texture dimension", &tb);
		return false;
	}
	if (tex_unit == -2) {
		error("gl_context::texture_disable: invalid texture unit", &tb);
		return false;
	}
	GLuint old_binding = (const GLuint&) tb.user_data;
	--old_binding;
	if (tex_unit >= 0)
		glActiveTexture(GL_TEXTURE0+tex_unit);
	// glDisable is not needed for texture arrays and will throw an invalid enum error
	//if(!(tb.tt == TT_1D_ARRAY || tb.tt == TT_2D_ARRAY))
	//	glDisable(get_tex_dim(tb.tt));
	bool result = !check_gl_error("gl_context::texture_disable", &tb);
	glBindTexture(get_tex_dim(tb.tt), old_binding);
	if (tex_unit >= 0)
		glActiveTexture(GL_TEXTURE0);
	return result;
}

bool gl_context::texture_bind_as_image(texture_base& tb, int tex_unit, int level, bool bind_array, int layer, AccessType access) const
{	
	GLuint tex_id = (GLuint&)tb.handle - 1;
	if(tex_id == -1) {
		error("gl_context::texture_enable: texture not created", &tb);
		return false;
	}

	if(!GLEW_VERSION_4_2) {
		error("gl_context::texture_bind_as_image: image textures not supported", &tb);
		return false;
	}

	GLuint gl_format = (const GLuint&)tb.internal_format;
	glBindImageTexture(tex_unit, tex_id, level, map_to_gl(bind_array), layer, map_to_gl(access), gl_format);
	
	bool result = !check_gl_error("gl_context::texture_bind_as_image", &tb);
	return result;
}

bool gl_context::render_buffer_create(render_buffer_base& rb, cgv::data::component_format& cf, int& _width, int& _height) const
{
	if (!GLEW_VERSION_3_0) {
		error("gl_context::render_buffer_create: frame buffer objects not supported", &rb);
		return false;
	}
	if (_width == -1)
		_width = get_width();
	if (_height == -1)
		_height = get_height();

	GLuint rb_id;
	glGenRenderbuffers(1, &rb_id);
	glBindRenderbuffer(GL_RENDERBUFFER, rb_id);

	GLuint& gl_format = (GLuint&)rb.internal_format;
	unsigned i = find_best_match(cf, color_buffer_formats);
	cgv::data::component_format best_cf(color_buffer_formats[i]);
	gl_format = gl_color_buffer_format_ids[i];

	i = find_best_match(cf, depth_formats, &best_cf);
	if (i != -1) {
		best_cf = cgv::data::component_format(depth_formats[i]);
		gl_format = gl_depth_format_ids[i];
	}

	cf = best_cf;
	if (rb.nr_multi_samples == 0)
		glRenderbufferStorage(GL_RENDERBUFFER, gl_format, _width, _height);
	else
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, rb.nr_multi_samples, gl_format, _width, _height);

	if (check_gl_error("gl_context::render_buffer_create", &rb))
		return false;
	rb.handle = get_handle(rb_id);
	return true;
}

bool gl_context::render_buffer_destruct(render_buffer_base& rc) const
{
	if (!GLEW_VERSION_3_0) {
		error("gl_context::render_buffer_destruct: frame buffer objects not supported", &rc);
		return false;
	}
	GLuint rb_id = ((GLuint&) rc.handle)+1;
	glDeleteRenderbuffers(1, &rb_id);
	if (check_gl_error("gl_context::render_buffer_destruct", &rc))
		return false;
	rc.handle = 0;
	return true;
}

bool gl_context::frame_buffer_create(frame_buffer_base& fbb) const
{
	if (!check_fbo_support("gl_context::frame_buffer_create", &fbb))
		return false;
	
	if (!context::frame_buffer_create(fbb))
		return false;

	// allocate framebuffer object
	GLuint fbo_id = 0;
	glGenFramebuffers(1, &fbo_id);
	if (fbo_id == 0) {
		error("gl_context::frame_buffer_create: could not allocate framebuffer object", &fbb);
		return false;
	}
	fbb.handle = get_handle(fbo_id);
	return true;
}

bool gl_context::frame_buffer_enable(frame_buffer_base& fbb)
{
	if (!context::frame_buffer_enable(fbb))
		return false;
	std::vector<int> buffers;
	bool depth_buffer = false;
	get_buffer_list(fbb, depth_buffer, buffers, GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, get_gl_id(fbb.handle));

	if (buffers.size() == 1)
		glDrawBuffer(buffers[0]);
	else if (buffers.size() > 1) {
		glDrawBuffers(GLsizei(buffers.size()), reinterpret_cast<GLenum*>(&buffers[0]));
	}
	else if(depth_buffer) {
		glDrawBuffer(GL_NONE);
		//glReadBuffer(GL_NONE);
	} else {
		error("gl_context::frame_buffer_enable: no attached draw buffer selected!!", &fbb);
		return false;
	}
	return true;
}

/// disable the framebuffer object
bool gl_context::frame_buffer_disable(frame_buffer_base& fbb)
{
	if (!context::frame_buffer_disable(fbb))
		return false;
	if (frame_buffer_stack.empty()) {
		error("gl_context::frame_buffer_disable called with empty frame buffer stack!!", &fbb);
		return false;
	}
	else
		glBindFramebuffer(GL_FRAMEBUFFER, get_gl_id(frame_buffer_stack.top()->handle));
	return true;
}

bool gl_context::frame_buffer_destruct(frame_buffer_base& fbb) const
{
	if (!context::frame_buffer_destruct(fbb))
		return false;
	GLuint fbo_id = get_gl_id(fbb.handle);
	glDeleteFramebuffers(1, &fbo_id);
	fbb.handle = 0;
	return true;
}

void complete_rect_from_vp(ivec4& D, GLint vp[4])
{
	if (D(0) == -1)
		D(0) = vp[0];
	if (D(1) == -1)
		D(1) = vp[1];
	if (D(2) == -1)
		D(2) = vp[0] + vp[2];
	if (D(3) == -1)
		D(3) = vp[1] + vp[3];
}

void gl_context::frame_buffer_blit(
	const frame_buffer_base* src_fbb_ptr, const ivec4& _S,
	frame_buffer_base* dst_fbb_ptr, const ivec4& _D, BufferTypeBits btbs, bool interpolate) const
{
	static const GLenum masks[8]{
		0,
		GL_COLOR_BUFFER_BIT,
		GL_DEPTH_BUFFER_BIT,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
		GL_STENCIL_BUFFER_BIT,
		GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
		GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
		GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT
	};
	ivec4 S = _S;
	ivec4 D = _D;
	if ((src_fbb_ptr == 0 && (S(0) == -1 || S(1) == -1 || S(2) == -1 || S(3) == -1)) ||
		(dst_fbb_ptr == 0 && (D(0) == -1 || D(1) == -1 || D(2) == -1 || D(3) == -1))) {
		GLint vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		if (src_fbb_ptr == 0)
			complete_rect_from_vp(S, vp);
		if (dst_fbb_ptr == 0)
			complete_rect_from_vp(D, vp);
	}
	GLint old_draw_fbo, old_read_fbo;
	if (src_fbb_ptr) {
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &old_read_fbo);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, get_gl_id(src_fbb_ptr->handle));
	}
	if (dst_fbb_ptr) {
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_draw_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, get_gl_id(dst_fbb_ptr->handle));
	}
	glBlitFramebuffer(S(0), S(1), S(2), S(3), D(0), D(1), D(2), D(3), masks[btbs], interpolate ? GL_LINEAR : GL_NEAREST);
	if (src_fbb_ptr) 
		glBindFramebuffer(GL_READ_FRAMEBUFFER, old_read_fbo);
	if (dst_fbb_ptr)
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, old_draw_fbo);
}

bool gl_context::frame_buffer_attach(frame_buffer_base& fbb, const render_buffer_base& rb, bool is_depth, int i) const
{
	if (!context::frame_buffer_attach(fbb, rb, is_depth, i))
		return false;
	GLint old_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_binding);
	glBindFramebuffer(GL_FRAMEBUFFER, get_gl_id(fbb.handle));
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
			is_depth ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0 + i,
			GL_RENDERBUFFER, 
			get_gl_id(rb.handle));
	glBindFramebuffer(GL_FRAMEBUFFER, old_binding);
	return true;
}

/// attach 2d texture to depth buffer if it is a depth texture, to stencil if it is a stencil texture or to the i-th color attachment if it is a color texture
bool gl_context::frame_buffer_attach(frame_buffer_base& fbb, 
												 const texture_base& t, bool is_depth,
												 int level, int i, int z_or_cube_side) const
{
	if (!context::frame_buffer_attach(fbb, t, is_depth, level, i, z_or_cube_side))
		return false;
	GLint old_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_binding);
	glBindFramebuffer(GL_FRAMEBUFFER, get_gl_id(fbb.handle));

	if (z_or_cube_side == -1) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, 
			is_depth ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0+i,
			t.tt == TT_2D ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, get_gl_id(t.handle), level);
	}
	else {
		if (t.tt == TT_CUBEMAP) {
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0+i,
				get_gl_cube_map_target(z_or_cube_side), get_gl_id(t.handle), level);
		}
		else {
			glFramebufferTexture3D(GL_FRAMEBUFFER, 
				GL_COLOR_ATTACHMENT0+i, 
				t.tt == TT_3D ? GL_TEXTURE_3D : GL_TEXTURE_2D_MULTISAMPLE_ARRAY , get_gl_id(t.handle), level, z_or_cube_side);
		}
	}
	bool result = !check_gl_error("gl_context::frame_buffer_attach", &fbb);
	glBindFramebuffer(GL_FRAMEBUFFER, old_binding);
	return result;
}

/// check for completeness, if not complete, get the reason in last_error
bool gl_context::frame_buffer_is_complete(const frame_buffer_base& fbb) const
{
	if (fbb.handle == 0) {
		error("gl_context::frame_buffer_is_complete: attempt to check completeness on frame buffer that is not created", &fbb);
		return false;
	}
	GLint old_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_binding);
	glBindFramebuffer(GL_FRAMEBUFFER, get_gl_id(fbb.handle));
	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, old_binding);
	switch (error) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
	case GL_FRAMEBUFFER_UNDEFINED:
		fbb.last_error = "undefined framebuffer";
		return false;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		fbb.last_error = "incomplete attachment";
		return false;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      fbb.last_error = "incomplete or missing attachment";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      fbb.last_error = "incomplete multisample";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      fbb.last_error = "incomplete layer targets";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      fbb.last_error = "incomplete draw buffer";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      fbb.last_error = "incomplete read buffer";
		return false;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      fbb.last_error = "framebuffer objects unsupported";
		return false;
	}
	fbb.last_error = "unknown error";
	return false;
}

int gl_context::frame_buffer_get_max_nr_color_attachments() const
{
	if (!check_fbo_support("gl_context::frame_buffer_get_max_nr_color_attachments"))
		return 0;

	GLint nr;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &nr);
	return nr;
}

int gl_context::frame_buffer_get_max_nr_draw_buffers() const
{
	if (!check_fbo_support("gl_context::frame_buffer_get_max_nr_draw_buffers"))
		return 0;

	GLint nr;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &nr);
	return nr;
}

GLuint gl_shader_type[] = 
{
	0, GL_COMPUTE_SHADER, GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER
};

void gl_context::shader_code_destruct(render_component& sc) const
{
	if (sc.handle == 0) {
		error("gl_context::shader_code_destruct: shader not created", &sc);
		return;
	}
	glDeleteShader(get_gl_id(sc.handle));
	check_gl_error("gl_context::shader_code_destruct", &sc);
}

bool gl_context::shader_code_create(render_component& sc, ShaderType st, const std::string& source) const
{
	if (!check_shader_support(st, "gl_context::shader_code_create", &sc))
		return false;

	GLuint s_id = glCreateShader(gl_shader_type[st]);
	if (s_id == -1) {
		error(std::string("gl_context::shader_code_create: ")+gl_error(), &sc);
		return false;
	}
	sc.handle = get_handle(s_id);

	const char* s = source.c_str();
	glShaderSource(s_id, 1, &s,NULL);
	if (check_gl_error("gl_context::shader_code_create", &sc))
		return false;

	return true;
}

bool gl_context::shader_code_compile(render_component& sc) const
{
	if (sc.handle == 0) {
		error("gl_context::shader_code_compile: shader not created", &sc);
		return false;
	}
	GLuint s_id = get_gl_id(sc.handle);
	glCompileShader(s_id);
	int result;
	glGetShaderiv(s_id, GL_COMPILE_STATUS, &result); 
	if (result == 1)
		return true;
	sc.last_error = std::string();
	GLint infologLength = 0;
	glGetShaderiv(s_id, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		int charsWritten = 0;
		char *infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(s_id, infologLength, &charsWritten, infoLog);
		sc.last_error = infoLog;
		free(infoLog);
	}
	return false;
}

bool gl_context::shader_program_create(shader_program_base& spb) const
{
	if (!check_shader_support(ST_VERTEX, "gl_context::shader_program_create", &spb))
		return false;
	spb.handle = get_handle(glCreateProgram());
	return true;
}

void gl_context::shader_program_attach(shader_program_base& spb, const render_component& sc) const
{
	if (spb.handle == 0) {
		error("gl_context::shader_program_attach: shader program not created", &spb);
		return;
	}
	glAttachShader(get_gl_id(spb.handle), get_gl_id(sc.handle));
}

void gl_context::shader_program_detach(shader_program_base& spb, const render_component& sc) const
{
	if (spb.handle == 0) {
		error("gl_context::shader_program_detach: shader program not created", &spb);
		return;
	}
	glDetachShader(get_gl_id(spb.handle), get_gl_id(sc.handle));
}

bool gl_context::shader_program_link(shader_program_base& spb) const
{
	if (spb.handle == 0) {
		error("gl_context::shader_program_link: shader program not created", &spb);
		return false;
	}
	GLuint p_id = get_gl_id(spb.handle);
	glLinkProgram(p_id); 
	int result;
	glGetProgramiv(p_id, GL_LINK_STATUS, &result); 
	if (result == 1)
		return context::shader_program_link(spb);
	GLint infologLength = 0;
	glGetProgramiv(p_id, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		GLsizei charsWritten = 0;
		char *infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(p_id, infologLength, &charsWritten, infoLog);
		spb.last_error = infoLog;
		error(std::string("gl_context::shader_program_link\n")+infoLog, &spb);
		free(infoLog);
	}
	return false;
}

bool gl_context::shader_program_set_state(shader_program_base& spb) const
{
	if (spb.handle == 0) {
		error("gl_context::shader_program_set_state: shader program not created", &spb);
		return false;
	}
	GLuint p_id = get_gl_id(spb.handle);
	glProgramParameteriARB(p_id, GL_GEOMETRY_VERTICES_OUT_ARB, spb.geometry_shader_output_count);
	glProgramParameteriARB(p_id, GL_GEOMETRY_INPUT_TYPE_ARB, map_to_gl(spb.geometry_shader_input_type));
	glProgramParameteriARB(p_id, GL_GEOMETRY_OUTPUT_TYPE_ARB, map_to_gl(spb.geometry_shader_output_type));
	return true;
}

bool gl_context::shader_program_enable(shader_program_base& spb)
{
	if (!context::shader_program_enable(spb))
		return false;
	glUseProgram(get_gl_id(spb.handle));	
	shader_program& prog = static_cast<shader_program&>(spb);
	if (auto_set_lights_in_current_shader_program && spb.does_use_lights())
		set_current_lights(prog);
	if (auto_set_material_in_current_shader_program && spb.does_use_material())
		set_current_material(prog);
	if (auto_set_view_in_current_shader_program && spb.does_use_view())
		set_current_view(prog);
	if (auto_set_gamma_in_current_shader_program && spb.does_use_gamma())
		set_current_gamma(prog);
	if (prog.does_context_set_color() && prog.get_color_index() >= 0)
		prog.set_attribute(*this, prog.get_color_index(), current_color);
	return true;
}

bool gl_context::shader_program_disable(shader_program_base& spb)
{
	if (!context::shader_program_disable(spb)) 
		return false;
	if (shader_program_stack.empty())
		glUseProgram(0);
	else
		glUseProgram(get_gl_id(shader_program_stack.top()->handle));
	return true;
}

bool gl_context::shader_program_destruct(shader_program_base& spb) const
{
	if (!context::shader_program_destruct(spb))
		return false;
	glDeleteProgram(get_gl_id(spb.handle));
	return true;
}

int  gl_context::get_uniform_location(const shader_program_base& spb, const std::string& name) const
{
	return glGetUniformLocation(get_gl_id(spb.handle), name.c_str());
}

std::string value_type_index_to_string(type_descriptor td)
{
	std::string res = cgv::type::info::get_type_name(td.coordinate_type);
	switch (td.element_type) {
	case ET_VECTOR:
		res = std::string("vector<") + res + "," + cgv::utils::to_string(td.nr_rows) + ">";
		break;
	case ET_MATRIX:
		res = std::string("matrix<") + res + "," + cgv::utils::to_string(td.nr_rows) + "," + cgv::utils::to_string(td.nr_columns) + ">";
		if (td.is_row_major)
			res += "^T";
	default:
		break;
	}
	if (td.is_array)
		res += "[]";
	return res;
}

bool gl_context::set_uniform_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr) const
{
	if (value_type.is_array) {
		error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") array type not supported, please use set_uniform_array instead.", &spb);
		return false;
	}
	if (!spb.handle) {
		error("gl_context::set_uniform_void() called on not created program", &spb);
		return false;
	}
	bool not_current = shader_program_stack.empty() || shader_program_stack.top() != &spb;
	if (not_current)
		glUseProgram(get_gl_id(spb.handle));
	bool res = true;
	switch (value_type.element_type) {
	case ET_VALUE:
		switch (value_type.coordinate_type) {
		case TI_BOOL: glUniform1i(loc, *reinterpret_cast<const bool*>(value_ptr) ? 1 : 0); break;
		case TI_UINT8: glUniform1ui(loc, *reinterpret_cast<const uint8_type*>(value_ptr)); break;
		case TI_UINT16: glUniform1ui(loc, *reinterpret_cast<const uint16_type*>(value_ptr)); break;
		case TI_UINT32: glUniform1ui(loc, *reinterpret_cast<const uint32_type*>(value_ptr)); break;
		case TI_INT8: glUniform1i(loc, *reinterpret_cast<const int8_type*>(value_ptr)); break;
		case TI_INT16: glUniform1i(loc, *reinterpret_cast<const int16_type*>(value_ptr)); break;
		case TI_INT32: glUniform1i(loc, *reinterpret_cast<const int32_type*>(value_ptr)); break;
		case TI_FLT32: glUniform1f(loc, *reinterpret_cast<const flt32_type*>(value_ptr)); break;
		case TI_FLT64: glUniform1d(loc, *reinterpret_cast<const flt64_type*>(value_ptr)); break;
		default:
			error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") unsupported coordinate type.", &spb);
			res = false; break;
		}
		break;
	case ET_VECTOR:
		switch (value_type.nr_rows) {
		case 2:
			switch (value_type.coordinate_type) {
			case TI_BOOL:   glUniform2i(loc, reinterpret_cast<const bool*>(value_ptr)[0] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[1] ? 1 : 0); break;
			case TI_UINT8:  glUniform2ui(loc, reinterpret_cast<const uint8_type*> (value_ptr)[0], reinterpret_cast<const uint8_type*> (value_ptr)[1]); break;
			case TI_UINT16: glUniform2ui(loc, reinterpret_cast<const uint16_type*>(value_ptr)[0], reinterpret_cast<const uint16_type*>(value_ptr)[1]); break;
			case TI_UINT32: glUniform2ui(loc, reinterpret_cast<const uint32_type*>(value_ptr)[0], reinterpret_cast<const uint32_type*>(value_ptr)[1]); break;
			case TI_INT8:   glUniform2i(loc, reinterpret_cast<const int8_type*>  (value_ptr)[0], reinterpret_cast<const int8_type*>  (value_ptr)[1]); break;
			case TI_INT16:  glUniform2i(loc, reinterpret_cast<const int16_type*> (value_ptr)[0], reinterpret_cast<const int16_type*> (value_ptr)[1]); break;
			case TI_INT32:  glUniform2i(loc, reinterpret_cast<const int32_type*> (value_ptr)[0], reinterpret_cast<const int32_type*> (value_ptr)[1]); break;
			case TI_FLT32:  glUniform2f(loc, reinterpret_cast<const flt32_type*> (value_ptr)[0], reinterpret_cast<const flt32_type*> (value_ptr)[1]); break;
			case TI_FLT64:  glUniform2d(loc, reinterpret_cast<const flt64_type*> (value_ptr)[0], reinterpret_cast<const flt64_type*> (value_ptr)[1]); break;
			default:
				error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") unsupported coordinate type.", &spb);
				res = false; break;
			}
			break;
		case 3:
			switch (value_type.coordinate_type) {
			case TI_BOOL:   glUniform3i(loc, reinterpret_cast<const bool*>(value_ptr)[0] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[1] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[2] ? 1 : 0); break;
			case TI_UINT8:  glUniform3ui(loc, reinterpret_cast<const uint8_type*> (value_ptr)[0], reinterpret_cast<const uint8_type*> (value_ptr)[1], reinterpret_cast<const uint8_type*> (value_ptr)[2]); break;
			case TI_UINT16: glUniform3ui(loc, reinterpret_cast<const uint16_type*>(value_ptr)[0], reinterpret_cast<const uint16_type*>(value_ptr)[1], reinterpret_cast<const uint16_type*>(value_ptr)[2]); break;
			case TI_UINT32: glUniform3ui(loc, reinterpret_cast<const uint32_type*>(value_ptr)[0], reinterpret_cast<const uint32_type*>(value_ptr)[1], reinterpret_cast<const uint32_type*>(value_ptr)[2]); break;
			case TI_INT8:   glUniform3i(loc, reinterpret_cast<const int8_type*>  (value_ptr)[0], reinterpret_cast<const int8_type*>  (value_ptr)[1], reinterpret_cast<const int8_type*>  (value_ptr)[2]); break;
			case TI_INT16:  glUniform3i(loc, reinterpret_cast<const int16_type*> (value_ptr)[0], reinterpret_cast<const int16_type*> (value_ptr)[1], reinterpret_cast<const int16_type*> (value_ptr)[2]); break;
			case TI_INT32:  glUniform3i(loc, reinterpret_cast<const int32_type*> (value_ptr)[0], reinterpret_cast<const int32_type*> (value_ptr)[1], reinterpret_cast<const int32_type*> (value_ptr)[2]); break;
			case TI_FLT32:  glUniform3f(loc, reinterpret_cast<const flt32_type*> (value_ptr)[0], reinterpret_cast<const flt32_type*> (value_ptr)[1], reinterpret_cast<const flt32_type*> (value_ptr)[2]); break;
			case TI_FLT64:  glUniform3d(loc, reinterpret_cast<const flt64_type*> (value_ptr)[0], reinterpret_cast<const flt64_type*> (value_ptr)[1], reinterpret_cast<const flt64_type*> (value_ptr)[2]); break;
			default:
				error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") unsupported coordinate type.", &spb);
				res = false; break;
			}
			break;
		case 4:
			switch (value_type.coordinate_type) {
			case TI_BOOL:   glUniform4i(loc, reinterpret_cast<const bool*>(value_ptr)[0] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[1] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[2] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[3] ? 1 : 0); break;
			case TI_UINT8:  glUniform4ui(loc, reinterpret_cast<const uint8_type*> (value_ptr)[0], reinterpret_cast<const uint8_type*> (value_ptr)[1], reinterpret_cast<const uint8_type*> (value_ptr)[2], reinterpret_cast<const uint8_type*> (value_ptr)[3]); break;
			case TI_UINT16: glUniform4ui(loc, reinterpret_cast<const uint16_type*>(value_ptr)[0], reinterpret_cast<const uint16_type*>(value_ptr)[1], reinterpret_cast<const uint16_type*>(value_ptr)[2], reinterpret_cast<const uint16_type*>(value_ptr)[3]); break;
			case TI_UINT32: glUniform4ui(loc, reinterpret_cast<const uint32_type*>(value_ptr)[0], reinterpret_cast<const uint32_type*>(value_ptr)[1], reinterpret_cast<const uint32_type*>(value_ptr)[2], reinterpret_cast<const uint32_type*>(value_ptr)[3]); break;
			case TI_INT8:   glUniform4i(loc, reinterpret_cast<const int8_type*>  (value_ptr)[0], reinterpret_cast<const int8_type*>  (value_ptr)[1], reinterpret_cast<const int8_type*>  (value_ptr)[2], reinterpret_cast<const int8_type*>  (value_ptr)[3]); break;
			case TI_INT16:  glUniform4i(loc, reinterpret_cast<const int16_type*> (value_ptr)[0], reinterpret_cast<const int16_type*> (value_ptr)[1], reinterpret_cast<const int16_type*> (value_ptr)[2], reinterpret_cast<const int16_type*> (value_ptr)[3]); break;
			case TI_INT32:  glUniform4i(loc, reinterpret_cast<const int32_type*> (value_ptr)[0], reinterpret_cast<const int32_type*> (value_ptr)[1], reinterpret_cast<const int32_type*> (value_ptr)[2], reinterpret_cast<const int32_type*> (value_ptr)[3]); break;
			case TI_FLT32:  glUniform4f(loc, reinterpret_cast<const flt32_type*> (value_ptr)[0], reinterpret_cast<const flt32_type*> (value_ptr)[1], reinterpret_cast<const flt32_type*> (value_ptr)[2], reinterpret_cast<const flt32_type*> (value_ptr)[3]); break;
			case TI_FLT64:  glUniform4d(loc, reinterpret_cast<const flt64_type*> (value_ptr)[0], reinterpret_cast<const flt64_type*> (value_ptr)[1], reinterpret_cast<const flt64_type*> (value_ptr)[2], reinterpret_cast<const flt64_type*> (value_ptr)[3]); break;
			default:
				error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") unsupported coordinate type.", &spb);
				res = false; break;
			}
			break;
		}
		break;
	case ET_MATRIX:
		switch (value_type.coordinate_type) {
		case TI_FLT32:
			switch (value_type.nr_rows) {
			case 2:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix2fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				case 3: glUniformMatrix2x3fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				case 4: glUniformMatrix2x4fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				default:
					error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..,4].", &spb);
					res = false; break;
				}
				break;
			case 3:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix3x2fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				case 3: glUniformMatrix3fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				case 4: glUniformMatrix3x4fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				default:
					error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..,4].", &spb);
					res = false; break;
				}
				break;
			case 4:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix4x2fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				case 3: glUniformMatrix4x3fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				case 4: glUniformMatrix4fv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt32_type*> (value_ptr)); break;
				default:
					error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..,4].", &spb);
					res = false; break;
				}
				break;
			default:
				error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") matrix number of rows outside [2,..,4].", &spb);
				res = false; break;
			}
			break;
		case TI_FLT64:
			switch (value_type.nr_rows) {
			case 2:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix2dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				case 3: glUniformMatrix2x3dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				case 4: glUniformMatrix2x4dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				default:
					error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..,4].", &spb);
					res = false; break;
				}
				break;
			case 3:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix3x2dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				case 3: glUniformMatrix3dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				case 4: glUniformMatrix3x4dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				default:
					error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..,4].", &spb);
					res = false; break;
				}
				break;
			case 4:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix4x2dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				case 3: glUniformMatrix4x3dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				case 4: glUniformMatrix4dv(loc, 1, !value_type.is_row_major, reinterpret_cast<const flt64_type*> (value_ptr)); break;
				default:
					error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..,4].", &spb);
					res = false; break;
				}
				break;
			default:
				error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") matrix number of rows outside [2,..,4].", &spb);
				res = false; break;
			}
			break;
		default:
			error(std::string("gl_context::set_uniform_void(") + value_type_index_to_string(value_type) + ") non float coordinate type not supported.", &spb);
			res = false; break;
		}
		break;
	}
	if (not_current)
		glUseProgram(shader_program_stack.empty() ? 0 : get_gl_id(shader_program_stack.top()->handle));

	if (check_gl_error("gl_context::set_uniform_void()", &spb))
		res = false;
	return res;
}

bool gl_context::set_uniform_array_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr, size_t nr_elements) const
{
	if (!value_type.is_array) {
		error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") non array type not allowed.", &spb);
		return false;
	}
	if (!spb.handle) {
		error("gl_context::set_uniform_array_void() called on not created program", &spb);
		return false;
	}
	bool not_current = shader_program_stack.empty() || shader_program_stack.top() != &spb;
	if (not_current)
		glUseProgram(get_gl_id(spb.handle));
	bool res = true;
	switch (value_type.coordinate_type) {
	case TI_INT32:
		switch (value_type.element_type) {
		case ET_VALUE:
			glUniform1iv(loc, GLsizei(nr_elements), reinterpret_cast<const int32_type*>(value_ptr));
			break;
		case ET_VECTOR:
			switch (value_type.nr_rows) {
			case 2: glUniform2iv(loc, GLsizei(nr_elements), reinterpret_cast<const int32_type*>(value_ptr)); break;
			case 3: glUniform3iv(loc, GLsizei(nr_elements), reinterpret_cast<const int32_type*>(value_ptr)); break;
			case 4: glUniform4iv(loc, GLsizei(nr_elements), reinterpret_cast<const int32_type*>(value_ptr)); break;
			default:
				error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") vector dimension outside [2,..4].", &spb);
				res = false;
				break;
			}
			break;
		case ET_MATRIX:
			error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") type not supported.", &spb);
			res = false;
			break;
		}
		break;
	case TI_UINT32:
		switch (value_type.element_type) {
		case ET_VALUE:
			glUniform1uiv(loc, GLsizei(nr_elements), reinterpret_cast<const uint32_type*>(value_ptr));
			break;
		case ET_VECTOR:
			switch (value_type.nr_rows) {
			case 2: glUniform2uiv(loc, GLsizei(nr_elements), reinterpret_cast<const uint32_type*>(value_ptr)); break;
			case 3:	glUniform3uiv(loc, GLsizei(nr_elements), reinterpret_cast<const uint32_type*>(value_ptr)); break;
			case 4:	glUniform4uiv(loc, GLsizei(nr_elements), reinterpret_cast<const uint32_type*>(value_ptr)); break;
			default:
				error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") vector dimension outside [2,..4].", &spb);
				res = false;
				break;
			}
			break;
		case ET_MATRIX:
			error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") type not supported.", &spb);
			res = false;
			break;
		}
		break;
	case TI_FLT32:
		switch (value_type.element_type) {
		case ET_VALUE:
			glUniform1fv(loc, GLsizei(nr_elements), reinterpret_cast<const flt32_type*>(value_ptr));
			break;
		case ET_VECTOR:
			switch (value_type.nr_rows) {
			case 2: glUniform2fv(loc, GLsizei(nr_elements), reinterpret_cast<const flt32_type*>(value_ptr)); break;
			case 3:	glUniform3fv(loc, GLsizei(nr_elements), reinterpret_cast<const flt32_type*>(value_ptr)); break;
			case 4:	glUniform4fv(loc, GLsizei(nr_elements), reinterpret_cast<const flt32_type*>(value_ptr)); break;
			default:
				error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") vector dimension outside [2,..4].", &spb);
				res = false;
				break;
			}
			break;
		case ET_MATRIX:
			switch (value_type.nr_rows) {
			case 2:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix2fv(loc, GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr)); break;
				case 3: glUniformMatrix2x3fv(loc, GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr)); break;
				case 4: glUniformMatrix2x4fv(loc, GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr)); break;
				default:
					error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..4].", &spb);
					res = false;
					break;
				}
				break;
			case 3:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix3x2fv(loc, GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr)); break;
				case 3:	glUniformMatrix3fv(loc, GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr));	break;
				case 4:	glUniformMatrix3x4fv(loc, GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr)); break;
				default:
					error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..4].", &spb);
					res = false;
					break;
				}
				break;
			case 4:
				switch (value_type.nr_columns) {
				case 2: glUniformMatrix4x2fv(loc, GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr)); break;
				case 3:	glUniformMatrix4x3fv(loc, GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr)); break;
				case 4:	glUniformMatrix4fv(loc,   GLsizei(nr_elements), value_type.is_row_major, reinterpret_cast<const flt32_type*>(value_ptr));	break;
				default:
					error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") matrix number of columns outside [2,..4].", &spb);
					res = false;
					break;
				}
				break;
			default:
				error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") matrix number of rows outside [2,..4].", &spb);
				res = false;
				break;
			}
			break;
		}
		break;
	default:
		error(std::string("gl_context::set_uniform_array_void(") + value_type_index_to_string(value_type) + ") unsupported coordinate type (only int32, uint32, and flt32 supported).", &spb);
		res = false;
		break;
	}
	if (check_gl_error("gl_context::set_uniform_array_void()", &spb))
		res = false;

	if (not_current)
		glUseProgram(shader_program_stack.empty() ? 0 : get_gl_id(shader_program_stack.top()->handle));

	return res;
}

int  gl_context::get_attribute_location(const shader_program_base& spb, const std::string& name) const
{
	GLint loc = glGetAttribLocation(get_gl_id(spb.handle), name.c_str());
	return loc;
}

bool gl_context::set_attribute_void(shader_program_base& spb, int loc, type_descriptor value_type, const void* value_ptr) const
{
	if (!spb.handle) {
		error("gl_context::set_attribute_void() called on not created program", &spb);
		return false;
	}
	bool not_current = shader_program_stack.empty() || shader_program_stack.top() != &spb;
	if (not_current)
		glUseProgram(get_gl_id(spb.handle));
	bool res = true;
	switch (value_type.element_type) {
	case ET_VALUE:
		switch (value_type.coordinate_type) {
		case TI_BOOL:   glVertexAttrib1s(loc, *reinterpret_cast<const bool*>(value_ptr) ? 1 : 0); break;
		case TI_INT8:   glVertexAttrib1s(loc, *reinterpret_cast<const int8_type*>(value_ptr)); break;
		case TI_INT16:  glVertexAttrib1s(loc, *reinterpret_cast<const int16_type*>(value_ptr)); break;
		case TI_INT32:  glVertexAttribI1i(loc, *reinterpret_cast<const int32_type*>(value_ptr)); break;
		case TI_UINT8:  glVertexAttrib1s(loc, *reinterpret_cast<const uint8_type*>(value_ptr)); break;
		case TI_UINT16: glVertexAttribI1ui(loc, *reinterpret_cast<const uint16_type*>(value_ptr)); break;
		case TI_UINT32: glVertexAttribI1ui(loc, *reinterpret_cast<const uint32_type*>(value_ptr)); break;
		case TI_FLT32:  glVertexAttrib1f(loc, *reinterpret_cast<const flt32_type*>(value_ptr)); break;
		case TI_FLT64:  glVertexAttrib1d(loc, *reinterpret_cast<const flt64_type*>(value_ptr)); break;
		default:
			error(std::string("gl_context::set_attribute_void(") + value_type_index_to_string(value_type) + ") type not supported!", &spb);
			res = false;
			break;
		}
		break;
	case ET_VECTOR:
		switch (value_type.nr_rows) {
		case 2:
			switch (value_type.coordinate_type) {
			case TI_BOOL:    glVertexAttrib2s(loc, reinterpret_cast<const bool*>(value_ptr)[0] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[1] ? 1 : 0); break;
			case TI_UINT8:   glVertexAttrib2s(loc, reinterpret_cast<const uint8_type*> (value_ptr)[0], reinterpret_cast<const uint8_type*> (value_ptr)[1]); break;
			case TI_UINT16: glVertexAttribI2ui(loc, reinterpret_cast<const uint16_type*>(value_ptr)[0], reinterpret_cast<const uint16_type*>(value_ptr)[1]); break;
			case TI_UINT32: glVertexAttribI2ui(loc, reinterpret_cast<const uint32_type*>(value_ptr)[0], reinterpret_cast<const uint32_type*>(value_ptr)[1]); break;
			case TI_INT8:    glVertexAttrib2s(loc, reinterpret_cast<const int8_type*>  (value_ptr)[0], reinterpret_cast<const int8_type*>  (value_ptr)[1]); break;
			case TI_INT16:   glVertexAttrib2s(loc, reinterpret_cast<const int16_type*> (value_ptr)[0], reinterpret_cast<const int16_type*> (value_ptr)[1]); break;
			case TI_INT32:  glVertexAttribI2i(loc, reinterpret_cast<const int32_type*> (value_ptr)[0], reinterpret_cast<const int32_type*> (value_ptr)[1]); break;
			case TI_FLT32:   glVertexAttrib2f(loc, reinterpret_cast<const flt32_type*> (value_ptr)[0], reinterpret_cast<const flt32_type*> (value_ptr)[1]); break;
			case TI_FLT64:   glVertexAttrib2d(loc, reinterpret_cast<const flt64_type*> (value_ptr)[0], reinterpret_cast<const flt64_type*> (value_ptr)[1]); break;
			default:
				error(std::string("gl_context::set_attribute_void(") + value_type_index_to_string(value_type) + ") unsupported coordinate type.", &spb);
				res = false;
				break;
			}
			break;
		case 3:
			switch (value_type.coordinate_type) {
			case TI_BOOL:    glVertexAttrib3s (loc, reinterpret_cast<const bool*>(value_ptr)[0] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[1] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[2] ? 1 : 0); break;
			case TI_UINT8:   glVertexAttrib3s (loc, reinterpret_cast<const uint8_type*> (value_ptr)[0], reinterpret_cast<const uint8_type*> (value_ptr)[1], reinterpret_cast<const uint8_type*> (value_ptr)[2]); break;
			case TI_UINT16: glVertexAttribI3ui(loc, reinterpret_cast<const uint16_type*>(value_ptr)[0], reinterpret_cast<const uint16_type*>(value_ptr)[1], reinterpret_cast<const uint16_type*>(value_ptr)[2]); break;
			case TI_UINT32: glVertexAttribI3ui(loc, reinterpret_cast<const uint32_type*>(value_ptr)[0], reinterpret_cast<const uint32_type*>(value_ptr)[1], reinterpret_cast<const uint32_type*>(value_ptr)[2]); break;
			case TI_INT8:    glVertexAttrib3s (loc, reinterpret_cast<const int8_type*>  (value_ptr)[0], reinterpret_cast<const int8_type*>  (value_ptr)[1], reinterpret_cast<const int8_type*>  (value_ptr)[2]); break;
			case TI_INT16:   glVertexAttrib3s (loc, reinterpret_cast<const int16_type*> (value_ptr)[0], reinterpret_cast<const int16_type*> (value_ptr)[1], reinterpret_cast<const int16_type*> (value_ptr)[2]); break;
			case TI_INT32:  glVertexAttribI3i (loc, reinterpret_cast<const int32_type*> (value_ptr)[0], reinterpret_cast<const int32_type*> (value_ptr)[1], reinterpret_cast<const int32_type*> (value_ptr)[2]); break;
			case TI_FLT32:   glVertexAttrib3f (loc, reinterpret_cast<const flt32_type*> (value_ptr)[0], reinterpret_cast<const flt32_type*> (value_ptr)[1], reinterpret_cast<const flt32_type*> (value_ptr)[2]); break;
			case TI_FLT64:   glVertexAttrib3d (loc, reinterpret_cast<const flt64_type*> (value_ptr)[0], reinterpret_cast<const flt64_type*> (value_ptr)[1], reinterpret_cast<const flt64_type*> (value_ptr)[2]); break;
			default:
				error(std::string("gl_context::set_attribute_void(") + value_type_index_to_string(value_type) + ") unsupported coordinate type.", &spb);
				res = false; break;
			}
			break;
		case 4:
			switch (value_type.coordinate_type) {
			case TI_BOOL:    glVertexAttrib4s (loc, reinterpret_cast<const bool*>(value_ptr)[0] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[1] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[2] ? 1 : 0, reinterpret_cast<const bool*>(value_ptr)[3] ? 1 : 0); break;
			case TI_UINT8:   glVertexAttrib4s (loc, reinterpret_cast<const uint8_type*> (value_ptr)[0], reinterpret_cast<const uint8_type*> (value_ptr)[1], reinterpret_cast<const uint8_type*> (value_ptr)[2], reinterpret_cast<const uint8_type*> (value_ptr)[3]); break;
			case TI_UINT16: glVertexAttribI4ui(loc, reinterpret_cast<const uint16_type*>(value_ptr)[0], reinterpret_cast<const uint16_type*>(value_ptr)[1], reinterpret_cast<const uint16_type*>(value_ptr)[2], reinterpret_cast<const uint16_type*>(value_ptr)[3]); break;
			case TI_UINT32: glVertexAttribI4ui(loc, reinterpret_cast<const uint32_type*>(value_ptr)[0], reinterpret_cast<const uint32_type*>(value_ptr)[1], reinterpret_cast<const uint32_type*>(value_ptr)[2], reinterpret_cast<const uint32_type*>(value_ptr)[3]); break;
			case TI_INT8:    glVertexAttrib4s (loc, reinterpret_cast<const int8_type*>  (value_ptr)[0], reinterpret_cast<const int8_type*>  (value_ptr)[1], reinterpret_cast<const int8_type*>  (value_ptr)[2], reinterpret_cast<const int8_type*>  (value_ptr)[3]); break;
			case TI_INT16:   glVertexAttrib4s (loc, reinterpret_cast<const int16_type*> (value_ptr)[0], reinterpret_cast<const int16_type*> (value_ptr)[1], reinterpret_cast<const int16_type*> (value_ptr)[2], reinterpret_cast<const int16_type*> (value_ptr)[3]); break;
			case TI_INT32:  glVertexAttribI4i (loc, reinterpret_cast<const int32_type*> (value_ptr)[0], reinterpret_cast<const int32_type*> (value_ptr)[1], reinterpret_cast<const int32_type*> (value_ptr)[2], reinterpret_cast<const int32_type*> (value_ptr)[3]); break;
			case TI_FLT32:   glVertexAttrib4f (loc, reinterpret_cast<const flt32_type*> (value_ptr)[0], reinterpret_cast<const flt32_type*> (value_ptr)[1], reinterpret_cast<const flt32_type*> (value_ptr)[2], reinterpret_cast<const flt32_type*> (value_ptr)[3]); break;
			case TI_FLT64:   glVertexAttrib4d (loc, reinterpret_cast<const flt64_type*> (value_ptr)[0], reinterpret_cast<const flt64_type*> (value_ptr)[1], reinterpret_cast<const flt64_type*> (value_ptr)[2], reinterpret_cast<const flt64_type*> (value_ptr)[3]); break;
			default:
				error(std::string("gl_context::set_attribute_void(") + value_type_index_to_string(value_type) + ") unsupported coordinate type.", &spb);
				res = false;
				break;
			}
			break;
		default:
			error(std::string("gl_context::set_attribute_void(") + value_type_index_to_string(value_type) + ") vector dimension outside [2..4]", &spb);
			res = false;
			break;
		}
		break;
	case ET_MATRIX:
		error(std::string("gl_context::set_attribute_void(") + value_type_index_to_string(value_type) + ") matrix type not supported!", &spb);
		res = false;
		break;
	}
	if (not_current)
		glUseProgram(shader_program_stack.empty() ? 0 : get_gl_id(shader_program_stack.top()->handle));

	if (check_gl_error("gl_context::set_uniform_array_void()", &spb))
		res = false;
	return res;
}

bool gl_context::attribute_array_binding_create(attribute_array_binding_base& aab) const
{
	if (!GLEW_VERSION_3_0) {
		error("gl_context::attribute_array_binding_create() array attribute bindings not supported", &aab);
		return false;
	}
	GLuint a_id;
	glGenVertexArrays(1, &a_id);
	if (a_id == -1) {
		error(std::string("gl_context::attribute_array_binding_create(): ") + gl_error(), &aab);
		return false;
	}
	aab.ctx_ptr = this;
	aab.handle = get_handle(a_id);
	return true;
}

bool gl_context::attribute_array_binding_destruct(attribute_array_binding_base& aab)
{
	if (&aab == attribute_array_binding_stack.top())
		glBindVertexArray(0);
	if (!context::attribute_array_binding_destruct(aab))
		return false;
	if (!aab.handle) {
		error("gl_context::attribute_array_binding_destruct(): called on not created attribute array binding", &aab);
		return false;
	}
	GLuint a_id = get_gl_id(aab.handle);
	glDeleteVertexArrays(1, &a_id);
	return !check_gl_error("gl_context::attribute_array_binding_destruct");
}

bool gl_context::attribute_array_binding_enable(attribute_array_binding_base& aab)
{
	if (!context::attribute_array_binding_enable(aab))
		return false;
	glBindVertexArray(get_gl_id(aab.handle));
	return !check_gl_error("gl_context::attribute_array_binding_enable");
}

bool gl_context::attribute_array_binding_disable(attribute_array_binding_base& aab)
{
	if (!context::attribute_array_binding_disable(aab))
		return false;
	if (attribute_array_binding_stack.empty())
		glBindVertexArray(0);
	else
		glBindVertexArray(get_gl_id(attribute_array_binding_stack.top()->handle));
	return true;
}

bool gl_context::set_element_array(attribute_array_binding_base* aab, const vertex_buffer_base* vbb) const
{
	if (!vbb) {
		error("gl_context::set_element_array(): called without a vertex buffer object", aab);
		return false;
	}
	if (!vbb->handle) {
		error("gl_context::set_element_array(): called with not created vertex buffer object", vbb);
		return false;
	}
	if (vbb->type != VBT_INDICES) {
		std::cout << "gl_context::set_element_array() : called on vertex buffer object that is not of type VBT_INDICES" << std::endl;
//		error("gl_context::set_element_array(): called on vertex buffer object that is not of type VBT_INDICES", vbb);
//		return false;
	}
	if (aab) {
		if (!aab->handle) {
			error("gl_context::set_element_array(): called on not created attribute array binding", aab);
			return false;
		}
	}
	// enable vertex array
	bool not_current = attribute_array_binding_stack.empty() || attribute_array_binding_stack.top() != aab;
	if (aab && not_current)
		glBindVertexArray(get_gl_id(aab->handle));

	// bind buffer to element array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get_gl_id(vbb->handle));

	if (aab && not_current)
		glBindVertexArray(attribute_array_binding_stack.empty() ? 0 : get_gl_id(attribute_array_binding_stack.top()->handle));

	return !check_gl_error("gl_context::set_element_array_void()", aab);
}


bool gl_context::set_attribute_array_void(attribute_array_binding_base* aab, int loc, type_descriptor value_type, const vertex_buffer_base* vbb, const void* ptr, size_t nr_elements, unsigned stride) const
{
	if (value_type == ET_MATRIX) {
		error("gl_context::set_attribute_array_void(): called with matrix elements not supported", aab);
		return false;
	}
	if (vbb) {
		if (!vbb->handle) {
			error("gl_context::set_attribute_array_void(): called with not created vertex buffer object", vbb);
			return false;
		}
	}
	if (aab) {
		if (!aab->handle) {
			error("gl_context::set_attribute_array_void(): called on not created attribute array binding", aab);
			return false;
		}
	}

	bool not_current = attribute_array_binding_stack.empty() || attribute_array_binding_stack.top() != aab;
	if (aab && not_current)
		glBindVertexArray(get_gl_id(aab->handle));

	if (vbb)
		glBindBuffer(GL_ARRAY_BUFFER, get_gl_id(vbb->handle));

	bool res = true;
	unsigned n = value_type.element_type == ET_VALUE ? 1 : value_type.nr_rows;
	switch (value_type.coordinate_type) {
	case TI_INT8: value_type.normalize ? glVertexAttribPointer(loc, n, GL_BYTE, value_type.normalize, stride, ptr) : glVertexAttribIPointer(loc, n, GL_BYTE, stride, ptr); break;
	case TI_INT16:  value_type.normalize ? glVertexAttribPointer(loc, n, GL_SHORT, value_type.normalize, stride, ptr) : glVertexAttribIPointer(loc, n, GL_SHORT, stride, ptr); break;
	case TI_INT32:  value_type.normalize ? glVertexAttribPointer(loc, n, GL_INT, value_type.normalize, stride, ptr) : glVertexAttribIPointer(loc, n, GL_INT, stride, ptr); break;
	case TI_UINT8:  value_type.normalize ? glVertexAttribPointer(loc, n, GL_UNSIGNED_BYTE, value_type.normalize, stride, ptr) : glVertexAttribIPointer(loc, n, GL_UNSIGNED_BYTE, stride, ptr); break;
	case TI_UINT16: value_type.normalize ? glVertexAttribPointer(loc, n, GL_UNSIGNED_SHORT, value_type.normalize, stride, ptr) : glVertexAttribIPointer(loc, n, GL_UNSIGNED_SHORT, stride, ptr); break;
	case TI_UINT32: value_type.normalize ? glVertexAttribPointer(loc, n, GL_UNSIGNED_INT, value_type.normalize, stride, ptr) : glVertexAttribIPointer(loc, n, GL_UNSIGNED_INT, stride, ptr); break;
	case TI_FLT32: glVertexAttribPointer(loc, n, GL_FLOAT, value_type.normalize, stride, ptr); break;
	case TI_FLT64:
		if (GLEW_VERSION_4_1)
			glVertexAttribLPointer(loc, n, GL_DOUBLE, stride, ptr);
		else {
			error("gl_context::set_attribute_array_void(): called with coordinates of type double only supported starting with OpenGL 4.1", aab);
			res = false;
		}
		break;
	default:
		error("gl_context::set_attribute_array_void(): called with unsupported coordinate type", aab);
		res = false;
	}

	if (res)
		glEnableVertexAttribArray(loc);

	if (vbb)
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (aab && not_current)
		glBindVertexArray(attribute_array_binding_stack.empty() ? 0 : get_gl_id(attribute_array_binding_stack.top()->handle));


	return res && !check_gl_error("gl_context::set_attribute_array_void()", aab);
}

bool gl_context::enable_attribute_array(attribute_array_binding_base* aab, int loc, bool do_enable) const
{
	bool not_current = attribute_array_binding_stack.empty() || attribute_array_binding_stack.top() != aab;
	if (aab) {
		if (!aab->handle) {
			error("gl_context::enable_attribute_array(): called on not created attribute array binding", aab);
			return false;
		}
		if (not_current)
			glBindVertexArray(get_gl_id(aab->handle));
	}

	if (do_enable)
		glEnableVertexAttribArray(loc);
	else
		glDisableVertexAttribArray(loc);

	if (aab && not_current)
		glBindVertexArray(attribute_array_binding_stack.empty() ? 0 : get_gl_id(attribute_array_binding_stack.top()->handle));

	return !check_gl_error("gl_context::enable_attribute_array()");
}

bool gl_context::is_attribute_array_enabled(const attribute_array_binding_base* aab, int loc) const
{
	bool not_current = attribute_array_binding_stack.empty() || attribute_array_binding_stack.top() != aab;
	if (aab) {
		if (!aab->handle) {
			error("gl_context::is_attribute_array_enabled(): called on not created attribute array binding", aab);
			return false;
		}
		if (not_current)
			glBindVertexArray(get_gl_id(aab->handle));
	}

	GLint res;
	glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &res);

	if (aab && not_current)
		glBindVertexArray(attribute_array_binding_stack.empty() ? 0 : get_gl_id(attribute_array_binding_stack.top()->handle));

	return res == GL_TRUE;
}

GLenum buffer_target(VertexBufferType vbt)
{
	static GLenum buffer_targets[] = {
		GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_TEXTURE_BUFFER, GL_UNIFORM_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_SHADER_STORAGE_BUFFER, GL_ATOMIC_COUNTER_BUFFER
	};
	return buffer_targets[vbt];
}

GLenum buffer_usage(VertexBufferUsage vbu)
{
	static GLenum buffer_usages[] = { GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY };
	return buffer_usages[vbu];
}

bool gl_context::vertex_buffer_bind(const vertex_buffer_base& vbb, VertexBufferType _type, unsigned _idx) const
{
	if (_idx == unsigned(-1))
		glBindBuffer(buffer_target(_type), get_gl_id(vbb.handle));
	else
		glBindBufferBase(buffer_target(_type), _idx, get_gl_id(vbb.handle));
	return !check_gl_error("gl_context::vertex_buffer_bind", &vbb);
}

bool gl_context::vertex_buffer_unbind(const vertex_buffer_base& vbb, VertexBufferType _type, unsigned _idx) const {
	if(_idx == unsigned(-1))
		glBindBuffer(buffer_target(_type), 0);
	else
		glBindBufferBase(buffer_target(_type), _idx, 0);
	return !check_gl_error("gl_context::vertex_buffer_unbind", &vbb);
}

bool gl_context::vertex_buffer_create(vertex_buffer_base& vbb, const void* array_ptr, size_t size_in_bytes) const
{
	if (!GLEW_VERSION_2_0) {
		error("gl_context::vertex_buffer_create() vertex buffer objects not supported", &vbb);
		return false;
	}
	GLuint b_id;
	glGenBuffers(1, &b_id);
	if (b_id == -1) {
		error(std::string("gl_context::vertex_buffer_create(): ") + gl_error(), &vbb);
		return false;
	}
	vbb.handle = get_handle(b_id);
	glBindBuffer(buffer_target(vbb.type), b_id);
	glBufferData(buffer_target(vbb.type), size_in_bytes, array_ptr, buffer_usage(vbb.usage));
	glBindBuffer(buffer_target(vbb.type), 0);
	return !check_gl_error("gl_context::vertex_buffer_create", &vbb);
}

bool gl_context::vertex_buffer_resize(vertex_buffer_base& vbb, const void* array_ptr, size_t size_in_bytes) const {
	if(!vbb.handle) {
		error("gl_context::vertex_buffer_resize() vertex buffer object must be created before", &vbb);
		return false;
	}
	GLuint b_id = get_gl_id(vbb.handle);
	glBindBuffer(buffer_target(vbb.type), b_id);
	glBufferData(buffer_target(vbb.type), size_in_bytes, array_ptr, buffer_usage(vbb.usage));
	glBindBuffer(buffer_target(vbb.type), 0);
	return !check_gl_error("gl_context::vertex_buffer_resize", &vbb);
}

bool gl_context::vertex_buffer_replace(vertex_buffer_base& vbb, size_t offset, size_t size_in_bytes, const void* array_ptr) const
{
	if (!vbb.handle) {
		error("gl_context::vertex_buffer_replace() vertex buffer object must be created before", &vbb);
		return false;
	}
	GLuint b_id = get_gl_id(vbb.handle);
	glBindBuffer(buffer_target(vbb.type), b_id);
	glBufferSubData(buffer_target(vbb.type), offset, size_in_bytes, array_ptr);
	glBindBuffer(buffer_target(vbb.type), 0);
	return !check_gl_error("gl_context::vertex_buffer_replace", &vbb);
}

bool gl_context::vertex_buffer_copy(const vertex_buffer_base& src, size_t src_offset, vertex_buffer_base& target, size_t target_offset, size_t size_in_bytes) const
{
	if (!src.handle || !target.handle) {
		error("gl_context::vertex_buffer_copy() source and destination vertex buffer objects must have been created before", &src);
		return false;
	}
	GLuint b_id = get_gl_id(src.handle);
	glBindBuffer(GL_COPY_READ_BUFFER, get_gl_id(src.handle));
	glBindBuffer(GL_COPY_WRITE_BUFFER, get_gl_id(target.handle));
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, src_offset, target_offset, size_in_bytes);
	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	return !check_gl_error("gl_context::vertex_buffer_copy", &src);

}

bool gl_context::vertex_buffer_copy_back(vertex_buffer_base& vbb, size_t offset, size_t size_in_bytes, void* array_ptr) const
{
	if (!vbb.handle) {
		error("gl_context::vertex_buffer_copy_back() vertex buffer object must be created", &vbb);
		return false;
	}
	GLuint b_id = get_gl_id(vbb.handle);
	glBindBuffer(GL_COPY_READ_BUFFER, b_id);
	glGetBufferSubData(GL_COPY_READ_BUFFER, offset, size_in_bytes, array_ptr);
	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	return !check_gl_error("gl_context::vertex_buffer_copy_back", &vbb);
}

bool gl_context::vertex_buffer_destruct(vertex_buffer_base& vbb) const
{
	if (vbb.handle) {
		GLuint b_id = get_gl_id(vbb.handle);
		glDeleteBuffers(1, &b_id);
		return !check_gl_error("gl_context::vertex_buffer_destruct");
	}
	else {
		error("gl_context::vertex_buffer_destruct(): called on not created vertex buffer", &vbb);
		return false;
	}
}


		}
	}
}