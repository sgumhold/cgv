#include "gl_context.h"
#include "gl_tools.h"
#include <cgv/base/base.h>
#include <cgv/base/action.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/textured_material.h>
#include <cgv/utils/scan.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/gui/event_handler.h>
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
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
			
GLuint map_to_gl(PrimitiveType pt)
{
	static GLuint pt_to_gl[] = {
		-1,
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
		GL_POLYGON
	};
	return pt_to_gl[pt];
}

GLuint map_to_gl(MaterialSide ms)
{
	static GLuint ms_to_gl[] = {
		0,
		GL_FRONT,
		GL_BACK,
		GL_FRONT_AND_BACK
	};
	return ms_to_gl[ms];
}

GLuint get_gl_id(void* handle)
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

/// construct gl_context and attach signals
gl_context::gl_context()
{
	info_font_size = 14;
	// check if a new context has been created or if the size of the viewport has been changed
	font_ptr info_font = find_font("Courier New");
	if (info_font.empty()) {
		info_font = find_font("Courier");
		if (info_font.empty()) {
			info_font = find_font("system");
		}
	}
	if (!info_font.empty())
//		std::cerr << "could not find a font" << std::endl;
//	else {
		info_font_face = info_font->get_font_face(FFA_REGULAR);
//	}

	show_help = false;
	show_stats = true;
}

/// return the used rendering API
RenderAPI gl_context::get_render_api() const
{
	return RA_OPENGL;
}

/// define lighting mode, viewing pyramid and the rendering mode
bool gl_context::configure_gl()
{
	if (!ensure_glew_initialized()) {
		error("gl_context::configure_gl could not initialize glew");
		return false;
	}

	if (check_gl_error("gl_context::configure_gl before on enter"))
		return false;

	enable_font_face(info_font_face, info_font_size);
	/*
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	*/
	// use the eye location to compute the specular lighting
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
	// this makes opengl normalize all surface normals before lighting calculations,
	// which is essential when using scaling to deform tesselated primities
	glEnable(GL_NORMALIZE);
	glViewport(0, 0, get_width(), get_height());
	if (check_gl_error("gl_context::configure_gl before init of children"))
		return false;
	
	group_ptr grp(get_group_interface());
	single_method_action<cgv::render::drawable, bool, cgv::render::context&> sma(*this, &drawable::init, false, false);
	for (unsigned i = 0; i<grp->get_nr_children(); ++i)
		traverser(sma, "nc").traverse(grp->get_child(i));

	if (check_gl_error("gl_context::configure_gl after init of children."))
		return false;

	return true;
}

void gl_context::resize_gl()
{
	group_ptr grp(get_group_interface());
	glViewport(0, 0, get_width(), get_height());
	if (grp) {
		single_method_action_2<drawable, void, unsigned int, unsigned int> sma(get_width(), get_height(), &drawable::resize);
		traverser(sma).traverse(grp);
	}
}


void gl_context::init_render_pass()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glEnable(GL_NORMALIZE);

	if (get_render_pass_flags()&RPF_SET_LIGHTS) {
		// define diffuse and specular components of 4 light sources to gray and white
		// if the diffuse components where also white the 4 light contributions would
		// generate colors larger than one
		glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, white);
		glLightfv(GL_LIGHT3, GL_DIFFUSE, white);
		glLightfv(GL_LIGHT0, GL_SPECULAR, white);
		glLightfv(GL_LIGHT1, GL_SPECULAR, white);
		glLightfv(GL_LIGHT2, GL_SPECULAR, white);
		glLightfv(GL_LIGHT3, GL_SPECULAR, white);
	}
	if (get_render_pass_flags()&RPF_SET_LIGHTS_ON) {
		// enable lighting calculations
		glEnable(GL_LIGHTING);
		// turn the 4 lights on
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
		glDisable(GL_LIGHT3);
	}
	if (get_render_pass_flags()&RPF_SET_MATERIAL) {
		// set the surface material colors and the specular exponent,
		// which is between 0 and 128 and generates sharpest highlights for 128 and no
		// distinguished highlight for 0
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 80);
	}
	if (get_render_pass_flags()&RPF_ENABLE_MATERIAL) {
		// this mode allows to define the ambient and diffuse color of the surface material
		// via the glColor commands
		glEnable(GL_COLOR_MATERIAL);
	}
	if (get_render_pass_flags()&RPF_SET_STATE_FLAGS) {
		// enable the depth test in order to show only the front most elements of the scene in 
		// each pixel
		glEnable(GL_DEPTH_TEST);
		// cull away the faces whose orientation points away from the observer
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
	}
	if ((get_render_pass_flags()&RPF_SET_PROJECTION) != 0) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45,(double)get_width()/get_height(),0.001,1000.0);
	}
	glMatrixMode(GL_MODELVIEW);
	if ((get_render_pass_flags()&RPF_SET_MODELVIEW) != 0) {
		glLoadIdentity();
		gluLookAt(0,0,10, 0,0,0, 0,1,0);
	}
	if (get_render_pass_flags()&RPF_SET_LIGHTS) {
		float lps[] = { 0,1,1,0, 1,1,0,0, 0,0,1,0, 0,1,0,0 };
		glLightfv(GL_LIGHT0, GL_POSITION, lps);
		glLightfv(GL_LIGHT1, GL_POSITION, lps+4);
		glLightfv(GL_LIGHT2, GL_POSITION, lps+8);
		glLightfv(GL_LIGHT3, GL_POSITION, lps+12);
	}
	if (check_gl_error("gl_context::init_render_pass before init_frame"))
		return;

	group* grp = get_group_interface();
	if (grp && (get_render_pass_flags()&RPF_DRAWABLES_INIT_FRAME)) {
		single_method_action<drawable,void,cgv::render::context&> sma(*this, &drawable::init_frame, true, true);
		traverser(sma).traverse(group_ptr(grp));
	}

	if (check_gl_error("gl_context::init_render_pass after init_frame"))
		return;
	// this defines the background color to which the frame buffer is set by glClear
	if (get_render_pass_flags()&RPF_SET_CLEAR_COLOR)
		glClearColor(bg_r,bg_g,bg_b,bg_a);
	// this defines the background color to which the accum buffer is set by glClear
	if (get_render_pass_flags()&RPF_SET_CLEAR_ACCUM)
		glClearAccum(bg_accum_r,bg_accum_g,bg_accum_b,bg_accum_a);
	// this defines the background depth buffer value set by glClear
	if (get_render_pass_flags()&RPF_SET_CLEAR_DEPTH)
		glClearDepth(bg_d);
	// this defines the background depth buffer value set by glClear
	if (get_render_pass_flags()&RPF_SET_CLEAR_STENCIL)
		glClearStencil(bg_s);
	// clear necessary buffers
	GLenum bits = 0;
	if (get_render_pass_flags()&RPF_CLEAR_COLOR)
		bits |= GL_COLOR_BUFFER_BIT;
	if (get_render_pass_flags()&RPF_CLEAR_DEPTH)
		bits |= GL_DEPTH_BUFFER_BIT;
	if (get_render_pass_flags()&RPF_CLEAR_STENCIL)
		bits |= GL_STENCIL_BUFFER_BIT;
	if (get_render_pass_flags()&RPF_CLEAR_ACCUM)
		bits |= GL_ACCUM_BUFFER_BIT;
	if (bits)
		glClear(bits);
}

///
void gl_context::finish_render_pass()
{
	glPopAttrib();
/*	if (get_render_pass_flags()&RPF_SET_LIGHTS) {
		// undo change in lights
		glLightfv(GL_LIGHT0, GL_DIFFUSE, black);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, black);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, black);
		glLightfv(GL_LIGHT3, GL_DIFFUSE, black);
		glLightfv(GL_LIGHT0, GL_SPECULAR, black);
		glLightfv(GL_LIGHT1, GL_SPECULAR, black);
		glLightfv(GL_LIGHT2, GL_SPECULAR, black);
		glLightfv(GL_LIGHT3, GL_SPECULAR, black);
	}
	if (get_render_pass_flags()&RPF_SET_LIGHTS_ON) {
		// enable lighting calculations
		glDisable(GL_LIGHTING);
		// turn the 4 lights on
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
		glDisable(GL_LIGHT3);
	}
	if (get_render_pass_flags()&RPF_SET_MATERIAL) {
		// set the surface material colors and the specular exponent,
		// which is between 0 and 128 and generates sharpest highlights for 128 and no
		// distinguished highlight for 0
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
	}
	if (get_render_pass_flags()&RPF_ENABLE_MATERIAL) {
		// this mode allows to define the ambient and diffuse color of the surface material
		// via the glColor commands
		glDisable(GL_COLOR_MATERIAL);
	}
	if (get_render_pass_flags()&RPF_SET_STATE_FLAGS) {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}
	if ((get_render_pass_flags()&RPF_SET_PROJECTION) != 0) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
	}
	glMatrixMode(GL_MODELVIEW);
	if ((get_render_pass_flags()&RPF_SET_MODELVIEW) != 0) {
		glLoadIdentity();
	}
	// this defines the background color to which the frame buffer is set by glClear
	if (get_render_pass_flags()&RPF_SET_CLEAR_COLOR)
		glClearColor(0,0,0,0);
	// this defines the background color to which the accum buffer is set by glClear
	if (get_render_pass_flags()&RPF_SET_CLEAR_ACCUM)
		glClearAccum(0,0,0,0);
	// this defines the background depth buffer value set by glClear
	if (get_render_pass_flags()&RPF_SET_CLEAR_DEPTH)
		glClearDepth(1);
	// this defines the background depth buffer value set by glClear
	if (get_render_pass_flags()&RPF_SET_CLEAR_STENCIL)
		glClearStencil(0);
		*/
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
		glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		   if (bg_r+bg_g+bg_b < 1.5f)
				glColor4f(1,1,1,1);
			else
				glColor4f(0,0,0,1);

			push_pixel_coords();
			enable_font_face(info_font_face, info_font_size);
			format_callback_handler fch(output_stream());
			set_cursor(20,20);
			group_ptr grp(get_group_interface());
			if (grp && show_stats) {
				single_method_action<cgv::base::base,void,std::ostream&> sma(output_stream(), &cgv::base::base::stream_stats, false, false);
				traverser(sma,"nc").traverse(grp,&fch);
//				traverser(make_action<std::ostream&>(output_stream(), &base::stream_stats, false),"nc").traverse(base_ptr(this),&fch);
				output_stream() << std::endl;
			}
		    if (bg_r+bg_g+bg_b < 1.5f)
				glColor4f(1,1,0,1);
			else
				glColor4f(0.4f,0.3f,0,1);
			if (grp && show_help) {
				// collect help from myself and all children
				single_method_action<event_handler,void,std::ostream&> sma(output_stream(), &event_handler::stream_help, false, false);
				traverser(sma,"nc").traverse(grp,&fch);
//				traverser(make_action<std::ostream&>(output_stream(), &event_handler::stream_help, false),"nc").traverse(base_ptr(this),&fch);
				output_stream().flush();
			}
			pop_pixel_coords();
		// turn lighting back on for the next frame
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glPopAttrib();
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

void enable_material_color(GLenum side, const textured_material::color_type& c, float alpha, GLenum type)
{
	GLfloat v[4] = {c[0],c[1],c[2],c[3]*alpha};
	glMaterialfv(side, type, v);
}


/// enable a material without textures
void gl_context::enable_material(const cgv::media::illum::phong_material& mat, MaterialSide ms, float alpha)
{
	if (ms == MS_NONE)
		return;
	if (ms != MS_BACK) {
		glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);
		glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
	}
	unsigned side = map_to_gl(ms);
	enable_material_color(side, mat.get_ambient(),alpha,GL_AMBIENT);
	enable_material_color(side, mat.get_diffuse(),alpha,GL_DIFFUSE);
	enable_material_color(side, mat.get_specular(),alpha,GL_SPECULAR);
	enable_material_color(side, mat.get_emission(),alpha,GL_EMISSION);
	glMaterialf(side, GL_SHININESS, mat.get_shininess());
	if (phong_shading && (ms != MS_BACK)) {
		shader_program& prog = ref_textured_material_prog(*this);
		prog.enable(*this);
		prog.set_uniform(*this, "use_bump_map", false);
		prog.set_uniform(*this, "use_diffuse_map", false);
		set_lighting_parameters(*this, prog);
	}
}

/// disable phong material
void gl_context::disable_material(const cgv::media::illum::phong_material& mat)
{
	if (phong_shading)
		ref_textured_material_prog(*this).disable(*this);
	glPopAttrib();
}

/// enable a material with textures
void gl_context::enable_material(const textured_material& mat, MaterialSide ms, float alpha)
{
	if (ms == MS_NONE)
		return;
	bool do_alpha = (mat.get_diffuse_texture() != 0) && mat.get_diffuse_texture()->get_component_name(mat.get_diffuse_texture()->get_nr_components() - 1)[0] == 'A';
	if (ms != MS_BACK) {
		GLuint flags = do_alpha ? GL_COLOR_BUFFER_BIT : GL_CURRENT_BIT;
		if (mat.get_bump_texture() != 0 || mat.get_diffuse_texture() != 0)
			flags |= GL_TEXTURE_BIT;
		flags |= GL_LIGHTING_BIT | GL_ENABLE_BIT;
		glPushAttrib(flags);
		glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
	}

	unsigned side = map_to_gl(ms);
	enable_material_color(side, mat.get_ambient(), alpha, GL_AMBIENT);
	enable_material_color(side, mat.get_diffuse(),alpha,GL_DIFFUSE);
	enable_material_color(side, mat.get_specular(),alpha,GL_SPECULAR);
	enable_material_color(side, mat.get_emission(),alpha,GL_EMISSION);
	glMaterialf(side, GL_SHININESS, mat.get_shininess());

	if (ms != MS_BACK) {
		if ((mat.get_bump_texture() || phong_shading) && ref_textured_material_prog(*this).is_linked()) {
			shader_program& prog = ref_textured_material_prog(*this);
			bool use_bump_map = mat.get_bump_texture() != 0;
			if (use_bump_map)
				mat.get_bump_texture()->enable(*this, 0);

			bool use_diffuse_map = mat.get_diffuse_texture() != 0;
			if (use_diffuse_map)
				mat.get_diffuse_texture()->enable(*this, 1);

			prog.enable(*this);
			prog.set_uniform(*this, "use_bump_map", use_bump_map);
			if (use_bump_map) {
				prog.set_uniform(*this, "bump_map", 0);
				prog.set_uniform(*this, "bump_map_res", (int)(mat.get_bump_texture()->get_width()));
				prog.set_uniform(*this, "bump_scale", 400 * mat.get_bump_scale());
			}
			prog.set_uniform(*this, "use_diffuse_map", use_diffuse_map);
			if (use_diffuse_map)
				prog.set_uniform(*this, "diffuse_map", 1);
			set_lighting_parameters(*this, prog);
		}
		else if (mat.get_diffuse_texture()) {
			enable_material_color(side, textured_material::color_type(1, 1, 1, 1), alpha, GL_DIFFUSE);
			mat.get_diffuse_texture()->enable(*this);
			glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV, GL_MODULATE);
		}
		if (do_alpha) {
			glEnable(GL_ALPHA_TEST);
			switch (mat.get_alpha_test_func()) {
			case textured_material::AT_ALWAYS: glAlphaFunc(GL_ALWAYS, mat.get_alpha_threshold()); break;
			case textured_material::AT_LESS: glAlphaFunc(GL_LESS, mat.get_alpha_threshold()); break;
			case textured_material::AT_EQUAL: glAlphaFunc(GL_EQUAL, mat.get_alpha_threshold()); break;
			case textured_material::AT_GREATER: glAlphaFunc(GL_GREATER, mat.get_alpha_threshold()); break;
			}
		}
		else
			glColor4f(1, 1, 1, alpha);
	}
}
/// disable phong material
void gl_context::disable_material(const textured_material& mat)
{
	if ((mat.get_bump_texture() || phong_shading) && ref_textured_material_prog(*this).is_linked()) {
		shader_program& prog = ref_textured_material_prog(*this);
		prog.disable(*this);
		if (mat.get_diffuse_texture())
			mat.get_diffuse_texture()->disable(*this);
		if (mat.get_bump_texture())
			mat.get_bump_texture()->disable(*this);
	}
	else if (mat.get_diffuse_texture()) {
		mat.get_diffuse_texture()->disable(*this);
	}
	glPopAttrib();
}


/// return maximum number of supported light sources
unsigned gl_context::get_max_nr_lights() const
{
	GLint max_nr_lights;
	glGetIntegerv(GL_MAX_LIGHTS, &max_nr_lights);
	return (unsigned) max_nr_lights;
}

/// enable a light source and return a handled to be used for disabling, if no more light source unit available 0 is returned
void* gl_context::enable_light(const cgv::media::illum::light_source& light)
{
	unsigned max_nr_lights = get_max_nr_lights();
	unsigned light_idx = 0;
	GLboolean flag;
	do {
		if (light_idx >= max_nr_lights)
			return 0;
		glGetBooleanv(GL_LIGHT0+light_idx, &flag);
		if (flag)
			++light_idx;
	} while (flag);
	
	GLfloat col[4] = {1,1,1,1};
	(cgv::media::illum::light_source::color_type&)(col[0]) = light.get_ambient();
	glLightfv(GL_LIGHT0+light_idx, GL_AMBIENT, col);
	(cgv::media::illum::light_source::color_type&)(col[0]) = light.get_diffuse();
	glLightfv(GL_LIGHT0+light_idx, GL_DIFFUSE, col);
	(cgv::media::illum::light_source::color_type&)(col[0]) = light.get_specular();
	glLightfv(GL_LIGHT0+light_idx, GL_SPECULAR, col);
	glLightfv(GL_LIGHT0+light_idx, GL_POSITION, light.get_location());
	if (light.get_type() != cgv::media::illum::LT_DIRECTIONAL) {
		glLightf(GL_LIGHT0+light_idx, GL_CONSTANT_ATTENUATION, light.get_attenuation()(0));
		glLightf(GL_LIGHT0+light_idx, GL_LINEAR_ATTENUATION, light.get_attenuation()(1));
		glLightf(GL_LIGHT0+light_idx, GL_QUADRATIC_ATTENUATION, light.get_attenuation()(2));
	}
	else {
		glLightf(GL_LIGHT0+light_idx, GL_CONSTANT_ATTENUATION, 1);
		glLightf(GL_LIGHT0+light_idx, GL_LINEAR_ATTENUATION, 0);
		glLightf(GL_LIGHT0+light_idx, GL_QUADRATIC_ATTENUATION, 0);
	}
	if (light.get_type() == cgv::media::illum::LT_SPOT) {
		glLightf(GL_LIGHT0+light_idx, GL_SPOT_CUTOFF, light.get_spot_cutoff());
		glLightf(GL_LIGHT0+light_idx, GL_SPOT_EXPONENT, light.get_spot_exponent());
		glLightfv(GL_LIGHT0+light_idx, GL_SPOT_DIRECTION, light.get_spot_direction());
	}
	else {
		glLightf(GL_LIGHT0+light_idx, GL_SPOT_CUTOFF, 180.0f);
		glLightf(GL_LIGHT0+light_idx, GL_SPOT_EXPONENT, 0.0f);
		static float dir[3] = {0,0,1};
		glLightfv(GL_LIGHT0+light_idx, GL_SPOT_DIRECTION, dir);
	}

	glEnable(GL_LIGHT0+light_idx);

	void* handle = 0;
	(unsigned&) handle = light_idx+1;
	return handle;
}

/// disable a previously enabled light
void gl_context::disable_light(void* handle)
{
	if (handle == 0)
		return;
	unsigned light_idx = (unsigned&)handle - 1;
	glDisable(GL_LIGHT0+light_idx);
}

template <typename T>
void gl_rotate(const T& a, const cgv::math::fvec<T,3>& axis);

template <> void gl_rotate<float>(const float& a, const cgv::math::fvec<float,3>& axis)
{
	glRotatef(a, axis(0), axis(1), axis(2));
}

template <> void gl_rotate<double>(const double& a, const cgv::math::fvec<double,3>& axis)
{
	glRotated(a, axis(0), axis(1), axis(2));
}


template <typename T>
void rotate(const cgv::math::fvec<T,3>& src, const cgv::math::fvec<T,3>& dest)
{
	T c = dot(src,dest);
	cgv::math::fvec<T,3> axis = cross(src,dest);
	T s = axis.length();
	T a = atan2(s,c)*180/(float)M_PI;
	gl_rotate(a, axis);
}

void gl_context::tesselate_arrow(double length, double aspect, double rel_tip_radius, double tip_aspect, int res)
{
	double cyl_radius = length*aspect;
	double cone_radius = rel_tip_radius*cyl_radius;
	double cone_length = cone_radius/tip_aspect;
	double cyl_length = length - cone_length;
	push_V();
	glScaled(cyl_radius,cyl_radius,0.5*cyl_length);
		push_V();
			glRotatef(180,1,0,0);
			tesselate_unit_disk(res);
		pop_V();

	glTranslated(0,0,1);
		tesselate_unit_cylinder(res);

	glTranslated(0,0,1);
	glScaled(rel_tip_radius,rel_tip_radius,cone_length/cyl_length);
		push_V();
			glRotatef(180,1,0,0);
				tesselate_unit_disk(res);
		pop_V();
	glTranslated(0,0,1);
		tesselate_unit_cone(res);
	pop_V();
}

///
void gl_context::tesselate_arrow(const cgv::math::fvec<double, 3>& start, const cgv::math::fvec<double, 3>& end, double aspect, double rel_tip_radius, double tip_aspect, int res)
{
	push_V();
	glTranslated(start(0),start(1),start(2));
	rotate(cgv::math::fvec<double,3>(0,0,1), end-start);
	tesselate_arrow((end-start).length(), aspect, rel_tip_radius, tip_aspect, res);
	pop_V();
}

void gl_context::draw_light_source(const light_source& l, float i, float light_scale)
{
	glPushAttrib(GL_LIGHTING_BIT|GL_CURRENT_BIT);

	GLfloat e[] = { l.get_diffuse()[0]*i,l.get_diffuse()[1]*i,l.get_diffuse()[2]*i, 1 };
	GLfloat s[] = { 0.5f,0.5f,0.5f,1 };
	glColor3f(0,0,0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, e);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 40);

	push_V();
	switch (l.get_type()) {
	case LT_DIRECTIONAL : 
		glScalef(light_scale, light_scale, light_scale);
		rotate(light_source::vec_type(0,0,-1),(const light_source::vec_type&)l.get_location());
		tesselate_arrow(2,0.1f,2,0.5);
		break;
	case LT_POINT :
		glTranslatef(l.get_location()(0)/l.get_location()(3), l.get_location()(1)/l.get_location()(3), l.get_location()(2)/l.get_location()(3));
		glScalef(0.3f*light_scale, 0.3f*light_scale, 0.3f*light_scale);
		tesselate_unit_sphere();
		break;
	case LT_SPOT :
		glTranslatef(l.get_location()(0)/l.get_location()(3), l.get_location()(1)/l.get_location()(3), l.get_location()(2)/l.get_location()(3));
		glScalef(light_scale, light_scale, light_scale);
		rotate(light_source::vec_type(0,0,-1),l.get_spot_direction());
		{
			float t = tan(l.get_spot_cutoff()*(float)M_PI/180);
			if (l.get_spot_cutoff() > 45.0f)
				glScalef(1,1,0.5f/t);
			else
				glScalef(t,t,0.5);
			glTranslatef(0,0,-1);
			GLboolean cull;
			glGetBooleanv(GL_CULL_FACE, &cull);
			glDisable(GL_CULL_FACE);
			tesselate_unit_cone();
			if (cull)
				glEnable(GL_CULL_FACE);
		}
	}
	pop_V();

	glPopAttrib();
}


/// use this to push transformation matrices on the stack such that x and y coordinates correspond to window coordinates
void gl_context::push_pixel_coords()
{
	push_P();
	push_V();
	// push projection matrix
	glMatrixMode(GL_PROJECTION);
	// set orthogonal projection
	glLoadIdentity();
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	gluOrtho2D(0,vp[2],vp[3],0);
	// push modelview matrix
	glMatrixMode(GL_MODELVIEW);
	// use identity for modelview
	glLoadIdentity();
}
/// transform point p into cursor coordinates and put x and y coordinates into the passed variables
void gl_context::put_cursor_coords(const vec_type& p, int& x, int& y) const
{
	vec_type p4(0,0,0,1);
	for (unsigned int c=0; c<p.size(); ++c)
		p4(c) = p(c);
	p4 = get_DPV()*p4;
	x = (int)(p4(0)/p4(3));
	y = (int)(p4(1)/p4(3));
}


/// pop previously changed transformation matrices 
void gl_context::pop_pixel_coords()
{
	pop_V();
	pop_P();
}

/// implement according to specification in context class
bool gl_context::read_frame_buffer(data::data_view& dv, 
											  unsigned int x, unsigned int y, FrameBufferType buffer_type, 
											  TypeId type, data::ComponentFormat cf, int w, int h)
{
	const cgv::data::data_format* df = dv.get_format();
	if (df) {
		w = df->get_width();
		h = df->get_height();
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
		gl_buffer = GL_AUX0+buffer_type;
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


void render_vertex(int k, const double* vertices, const double* normals, const double* tex_coords, 
						  const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, bool flip_normals)
{
	if (normals && normal_indices) {
		if (flip_normals) {
			double n[3] = { -normals[3*normal_indices[k]],-normals[3*normal_indices[k]+1],-normals[3*normal_indices[k]+2] };
			glNormal3dv(n);
		}
		else
			glNormal3dv(normals+3*normal_indices[k]);
	}
	if (tex_coords && tex_coord_indices)
		glTexCoord2dv(tex_coords+2*tex_coord_indices[k]);
	glVertex3dv(vertices+3*vertex_indices[k]);
}


void gl_context::draw_faces(
	const double* vertices, const double* normals, const double* tex_coords, 
	const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
	int nr_faces, int face_degree, bool flip_normals) const
{
	int k = 0;
	if (face_degree < 5)
		glBegin( face_degree == 3 ? GL_TRIANGLES : GL_QUADS);
	for (int i = 0; i<nr_faces; ++i) {
		if (face_degree >= 5)
			glBegin(GL_POLYGON);
		for (int j=0; j<face_degree; ++j, ++k)
			render_vertex(k, vertices, normals, tex_coords, vertex_indices, normal_indices,tex_coord_indices,flip_normals);
		if (face_degree >= 5)
			glEnd();		
	}
	if (face_degree < 5)
		glEnd();
}

void gl_context::draw_strip_or_fan(
		const double* vertices, const double* normals, const double* tex_coords, 
		const int* vertex_indices, const int* normal_indices, const int* tex_coord_indices, 
		int nr_faces, int face_degree, bool is_fan, bool flip_normals) const
{
	glBegin( face_degree == 3 ? (is_fan ? GL_TRIANGLE_FAN : GL_TRIANGLE_STRIP) : GL_QUAD_STRIP);
	render_vertex(0, vertices, normals, tex_coords, vertex_indices, normal_indices,tex_coord_indices,flip_normals);
	render_vertex(1, vertices, normals, tex_coords, vertex_indices, normal_indices,tex_coord_indices,flip_normals);
	int s = face_degree-2;
	int k = 2;
	for (int i = 0; i<nr_faces; ++i)
		for (int j=0; j<s; ++j, ++k)
			render_vertex(k, vertices, normals, tex_coords, vertex_indices, normal_indices,tex_coord_indices,flip_normals);
	glEnd();
}	

/// return homogeneous 4x4 viewing matrix, which transforms from world to eye space
gl_context::mat_type gl_context::get_V() const
{
	GLdouble V[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, V);
	return mat_type(4,4,V);
}

void gl_context::set_V(const mat_type& V) const
{
	GLint mm;
	glGetIntegerv(GL_MATRIX_MODE, &mm);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(V);
	glMatrixMode(mm);
}

/// return homogeneous 4x4 projection matrix, which transforms from eye to clip space
gl_context::mat_type gl_context::get_P() const
{
	GLdouble P[16];
	glGetDoublev(GL_PROJECTION_MATRIX, P);
	return mat_type(4,4,P);
}

void gl_context::set_P(const mat_type& P) const
{
	GLint mm;
	glGetIntegerv(GL_MATRIX_MODE, &mm);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(P);
	glMatrixMode(mm);
}


/// return homogeneous 4x4 projection matrix, which transforms from clip to device space
gl_context::mat_type gl_context::get_D() const
{
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	mat_type D(4,4,0.0);
	D(0,0) =  0.5*vp[2];
	D(0, 3) = 0.5*vp[2] + vp[0];
	D(1,1) = -0.5*vp[3]; // flip y-coordinate
	D(1, 3) = get_height() - 0.5*vp[3] - vp[1];
	D(2,2) =  0.5;
	D(2,3) =  0.5;
	D(3,3) =  1.0;
	return D;
}

/// read the device z-coordinate from the z-buffer for the given device x- and y-coordinates
double gl_context::get_z_D(int x_D, int y_D) const
{
	GLfloat z_D;

	if (!in_render_process() && !is_current())
		make_current();

	glReadPixels(x_D, get_height()-y_D-1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z_D);
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
	return z_D;
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


GLuint get_tex_dim(TextureType tt) {
	static GLuint tex_dim[] = { 0, GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };
	return tex_dim[tt];
}

GLuint get_tex_bind(TextureType tt) {
	static GLuint tex_bind[] = { 0, GL_TEXTURE_BINDING_1D, GL_TEXTURE_BINDING_2D, GL_TEXTURE_BINDING_3D, GL_TEXTURE_BINDING_CUBE_MAP, GL_TEXTURE_BUFFER };
	return tex_bind[tt];
}


unsigned int map_to_gl(TextureWrap wrap)
{
	static const GLenum gl_texture_wrap[] = { 
		GL_REPEAT, 
		GL_CLAMP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, 
		GL_MIRROR_CLAMP_EXT, GL_MIRROR_CLAMP_TO_EDGE_EXT, GL_MIRROR_CLAMP_TO_BORDER_EXT, GL_MIRRORED_REPEAT
	};
	return gl_texture_wrap[wrap];
}

unsigned int map_to_gl(TextureFilter filter_type)
{
	static const GLenum gl_texture_filter[] = {
		GL_NEAREST, GL_LINEAR, 
		GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, 
		GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
		GL_TEXTURE_MAX_ANISOTROPY_EXT
	};
	return gl_texture_filter[filter_type];
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

std::string gl_error() {
	GLenum eid = glGetError();
	return std::string((const char*)gluErrorString(eid));
}

bool gl_context::check_gl_error(const std::string& where, const cgv::render::render_component* rc)
{
	GLenum eid = glGetError();
	if (eid == GL_NO_ERROR)
		return false;
	std::string error_string = where + ": " + std::string((const char*)gluErrorString(eid));
	error(error_string, rc);
	return true;
}

bool gl_context::check_texture_support(TextureType tt, const std::string& where, const cgv::render::render_component* rc)
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
		break;
	}
	return true;
}

bool gl_context::check_shader_support(ShaderType st, const std::string& where, const cgv::render::render_component* rc)
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
	case ST_TESS_EVALUTION:
		if (GLEW_VERSION_4_0)
			return true;
		else {
			error(where+": tesselation shader need not supported OpenGL version 4.0", rc);
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

bool gl_context::check_fbo_support(const std::string& where, const cgv::render::render_component* rc)
{
	if (!GLEW_VERSION_3_0) {
		error(where + ": framebuffer objects not supported", rc);
		return false;
	}
	return true;
}

GLuint gl_context::texture_generate(texture_base& tb)
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

GLuint gl_context::texture_bind(TextureType tt, GLuint tex_id)
{
	GLint tmp_id;
	glGetIntegerv(get_tex_bind(tt), &tmp_id);
	glBindTexture(get_tex_dim(tt), tex_id);
	return tmp_id;
}

void gl_context::texture_unbind(TextureType tt, GLuint tmp_id)
{
	glBindTexture(get_tex_dim(tt), tmp_id);
}

bool gl_context::texture_create(texture_base& tb, cgv::data::data_format& df)
{
	GLuint gl_format = (const GLuint&) tb.internal_format;
	
	if (tb.tt == TT_UNDEF)
		tb.tt = (TextureType)df.get_nr_dimensions();
	GLuint tex_id = texture_generate(tb);
	if (tex_id == get_gl_id(0))
		return false;
	GLuint tmp_id = texture_bind(tb.tt, tex_id);

	switch (tb.tt) {
	case TT_1D :
		glTexImage1D(GL_TEXTURE_1D, 0, 
			gl_format, df.get_width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		break;
	case TT_2D :
		{
			unsigned int comp_type = GL_RGBA;
			if (std::string(df.get_component_name(0)) == "D")
				comp_type = GL_DEPTH_COMPONENT;
			glTexImage2D(GL_TEXTURE_2D, 0, 
				gl_format, df.get_width(), df.get_height(), 0, comp_type, GL_UNSIGNED_BYTE, 0);
			break;
		}
	case TT_3D :
		glTexImage3D(GL_TEXTURE_3D, 0, 
			gl_format, df.get_width(), df.get_height(), df.get_depth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		break;
	case TT_CUBEMAP :
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
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
							int level, int cube_side, const std::vector<cgv::data::data_view>* palettes)
{
	// query the format to be used for the texture
	GLuint gl_tex_format = (const GLuint&) tb.internal_format;

	// define the texture type from the data format and the cube_side parameter
	tb.tt = (TextureType)data.get_format()->get_nr_dimensions();
	if (tb.tt == TT_2D && cube_side != -1)
		tb.tt = TT_CUBEMAP;
	
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
	tb.have_mipmaps = load_texture(data, gl_tex_format, level, cube_side, palettes);
	bool result = !check_gl_error("gl_context::texture_create", &tb);
	// restore old texture
	texture_unbind(tb.tt, tmp_id);
	return result;
}

bool gl_context::texture_create_from_buffer(
						texture_base& tb, 
						cgv::data::data_format& df, 
						int x, int y, int level)
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

	glCopyTexImage2D(GL_TEXTURE_2D, level, gl_format, x, y, df.get_width(), df.get_height(), 0);
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
		error_string += (const char*)gluErrorString(glGetError());
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
						int level, const std::vector<cgv::data::data_view>* palettes)
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
			error("gl_context::texture_replace: replace on cubemap without invalid side specification", &tb);
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
							int level)
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
	case TT_CUBEMAP : glCopyTexSubImage2D(get_gl_cube_map_target(z), level, x, y, x_buffer, y_buffer, width, height); break;
	}
	bool result = !check_gl_error("gl_context::texture_replace_from_buffer", &tb);
	texture_unbind(tb.tt, tmp_id);

	if (result && gen_mipmap)
		result = texture_generate_mipmaps(tb, tb.tt == TT_CUBEMAP ? 2 : (int)tb.tt);

	return result;
}

bool gl_context::texture_generate_mipmaps(texture_base& tb, unsigned int dim)
{
	GLuint tmp_id = texture_bind(tb.tt,get_gl_id(tb.handle));

	std::string error_string;
	bool result = generate_mipmaps(dim, &error_string);
	if (result)
		tb.have_mipmaps = true;
	else
		error(std::string("gl_context::texture_generate_mipmaps: ") + error_string);

	texture_unbind(tb.tt, tmp_id);
	return result;
}

bool gl_context::texture_destruct(texture_base& tb)
{
	if (!is_created()) {
		error("gl_context::texture_destruct: attempt to destruct not created texture", &tb);
		return false;
	}
	GLuint tex_id = get_gl_id(tb.handle);
	glDeleteTextures(1, &tex_id);
	bool result = !check_gl_error("gl_context::texture_destruct", &tb);
	tb.handle = 0;
	return result;
}

bool gl_context::texture_set_state(const texture_base& tb)
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

	glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_MIN_FILTER, map_to_gl(tb.min_filter));
	glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_MAG_FILTER, map_to_gl(tb.mag_filter));
	glTexParameterf(get_tex_dim(tb.tt), GL_TEXTURE_PRIORITY, tb.priority);
	if (tb.min_filter == TF_ANISOTROP)
		glTexParameterf(get_tex_dim(tb.tt), GL_TEXTURE_MAX_ANISOTROPY_EXT, tb.anisotropy);
	else
		glTexParameterf(get_tex_dim(tb.tt), GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
	if (tb.border_color[0] >= 0.0f)
		glTexParameterfv(get_tex_dim(tb.tt), GL_TEXTURE_BORDER_COLOR, tb.border_color);
	glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_WRAP_S, map_to_gl(tb.wrap_s));
	if (tb.tt > TT_1D)
		glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_WRAP_T, map_to_gl(tb.wrap_t));
	if (tb.tt == TT_3D)
		glTexParameteri(get_tex_dim(tb.tt), GL_TEXTURE_WRAP_R, map_to_gl(tb.wrap_r));

	bool result = !check_gl_error("gl_context::texture_set_state", &tb);
	texture_unbind(tb.tt, tmp_id);
	return result;
}

bool gl_context::texture_enable(
						texture_base& tb, 
						int tex_unit, unsigned int dim)
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
	glEnable(get_tex_dim(tb.tt));
	bool result = !check_gl_error("gl_context::texture_enable", &tb);
	if (tex_unit >= 0)
		glActiveTexture(GL_TEXTURE0);
	return result;
}

bool gl_context::texture_disable(
						texture_base& tb, 
						int tex_unit, unsigned int dim)
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
	glDisable(get_tex_dim(tb.tt));
	bool result = !check_gl_error("gl_context::texture_disable", &tb);
	glBindTexture(get_tex_dim(tb.tt), old_binding);
	if (tex_unit >= 0)
		glActiveTexture(GL_TEXTURE0);
	return result;
}

bool gl_context::render_buffer_create(
	render_component& rc,
	cgv::data::component_format& cf,
	int& _width, int& _height)
{
	if (!GLEW_VERSION_3_0) {
		error("gl_context::render_buffer_create: frame buffer objects not supported", &rc);
		return false;
	}
	if (_width == -1)
		_width = get_width();
	if (_height == -1)
		_height = get_height();

	GLuint rb_id;
	glGenRenderbuffers(1, &rb_id);
	glBindRenderbuffer(GL_RENDERBUFFER, rb_id);

	GLuint& gl_format = (GLuint&)rc.internal_format;
	unsigned i = find_best_match(cf, color_buffer_formats);
	cgv::data::component_format best_cf(color_buffer_formats[i]);
	gl_format = gl_color_buffer_format_ids[i];

	i = find_best_match(cf, depth_formats, &best_cf);
	if (i != -1) {
		best_cf = cgv::data::component_format(depth_formats[i]);
		gl_format = gl_depth_format_ids[i];
	}

	cf = best_cf;

	glRenderbufferStorage(GL_RENDERBUFFER, gl_format, _width, _height);

	if (check_gl_error("gl_context::render_buffer_create", &rc))
		return false;
	rc.handle = get_handle(rb_id);
	return true;
}

bool gl_context::render_buffer_destruct(render_component& rc)
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

bool gl_context::frame_buffer_create(frame_buffer_base& fbb)
{
	if (!check_fbo_support("gl_context::frame_buffer_create", &fbb))
		return false;

	if (fbb.width == -1)
		fbb.width = get_width();
	if (fbb.height == -1)
		fbb.height = get_height();

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

void gl_context::frame_buffer_bind(const frame_buffer_base& fbb, void*& user_data) const
{
	if (fbb.handle == 0)
		return;
	GLuint fbo_id = get_gl_id(fbb.handle);
	GLint old_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_binding);
	user_data = get_handle(old_binding);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
}

void gl_context::frame_buffer_unbind(const frame_buffer_base& fbb, void*& user_data) const
{
	GLuint old_binding = get_gl_id(user_data);
	glBindFramebuffer(GL_FRAMEBUFFER, old_binding);
	user_data = 0;
}

void gl_context::frame_buffer_bind(frame_buffer_base& fbb) const
{
	frame_buffer_bind(fbb, fbb.user_data);
}

void gl_context::frame_buffer_unbind(frame_buffer_base& fbb) const
{
	frame_buffer_unbind(fbb, fbb.user_data);
}

bool gl_context::frame_buffer_enable(frame_buffer_base& fbb)
{
	if (fbb.handle == 0) {
		error("gl_context::frame_buffer_enable: attempt to enable not created frame buffer", &fbb);
		return false;
	}
	GLuint fbo_id = get_gl_id(fbb.handle);
	int i;
	int n = 0;
	GLenum draw_buffers[16];
	if (fbb.enabled_color_attachments.size() == 0) {
		for (i = 0; i < 16; ++i)
			if (fbb.attached[i]) {
				draw_buffers[n] = GL_COLOR_ATTACHMENT0+i;
				++n;
			}
	}
	else {
		for (i = 0; i < (int)fbb.enabled_color_attachments.size(); ++i) {
			if (fbb.attached[fbb.enabled_color_attachments[i]]) {
				draw_buffers[n] = GL_COLOR_ATTACHMENT0+fbb.enabled_color_attachments[i];
				++n;
			}
		}
	}
	frame_buffer_stack.push(0);
	frame_buffer_bind(fbb, frame_buffer_stack.top());
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_VIEWPORT_BIT);
	if (n == 1)
		glDrawBuffer(draw_buffers[0]);
	else if (n > 1) {
		glDrawBuffers(n, draw_buffers);
	}
	else {
		error("gl_context::frame_buffer_enable: no attached draw buffer selected!!", &fbb);
		return false;
	}
	glViewport(0,0,fbb.width, fbb.height);
	return true;
}

/// disable the framebuffer object
bool gl_context::frame_buffer_disable(frame_buffer_base& fbb)
{
	glPopAttrib();
	frame_buffer_unbind(fbb, frame_buffer_stack.top());
	frame_buffer_stack.pop();
	return true;
}

bool gl_context::frame_buffer_destruct(frame_buffer_base& fbb)
{
	if (fbb.handle == 0) {
		error("gl_context::frame_buffer_destruct: attempt to destruct not created frame buffer", &fbb);
		return false;
	}
	GLuint fbo_id = get_gl_id(fbb.handle);
	glDeleteFramebuffers(1, &fbo_id);
	fbb.handle = 0;
	fbb.user_data = 0;
	return true;
}

bool gl_context::frame_buffer_attach(frame_buffer_base& fbb, const render_component& rb, bool is_depth, int i)
{
	if (fbb.handle == 0) {
		error("gl_context::frame_buffer_attach: attempt to attach to frame buffer that is not created", &fbb);
		return false;
	}
	if (rb.handle == 0) {
		error("gl_context::frame_buffer_attach: attempt to attach empty render buffer", &fbb);
		return false;
	}
	GLuint rb_id = get_gl_id(rb.handle);
	void* user_data;
	frame_buffer_bind(fbb, user_data);
	if (is_depth) {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER, 
			rb_id);
	}
	else {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
			GL_COLOR_ATTACHMENT0+i,
			GL_RENDERBUFFER, 
			rb_id);
		fbb.attached[i] = true;
	}
	frame_buffer_unbind(fbb, user_data);
	return true;
}

/// attach 2d texture to depth buffer if it is a depth texture, to stencil if it is a stencil texture or to the i-th color attachment if it is a color texture
bool gl_context::frame_buffer_attach(frame_buffer_base& fbb, 
												 const texture_base& t, bool is_depth,
												 int level, int i, int z_or_cube_side)
{
	if (fbb.handle == 0) {
		error("gl_context::frame_buffer_attach: attempt to attach to frame buffer that is not created", &fbb);
		return false;
	}
	void* user_data;
	frame_buffer_bind(fbb, user_data);
	GLuint tex_id = get_gl_id(t.handle);
	if (z_or_cube_side == -1) {
		if (is_depth) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, 
				GL_DEPTH_ATTACHMENT,
				GL_TEXTURE_2D, tex_id, level);
		}
		else {
			glFramebufferTexture2D(GL_FRAMEBUFFER, 
				GL_COLOR_ATTACHMENT0+i, 
				GL_TEXTURE_2D, tex_id, level);
			fbb.attached[i] = true;
		}
	}
	else {
		if (t.tt == TT_CUBEMAP) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, 
				GL_COLOR_ATTACHMENT0+i, 
				get_gl_cube_map_target(z_or_cube_side), tex_id, level);
		}
		else {
			glFramebufferTexture3D(GL_FRAMEBUFFER, 
				GL_COLOR_ATTACHMENT0+i, 
				GL_TEXTURE_3D, tex_id, level, z_or_cube_side);
		}
		fbb.attached[i] = true;
	}
	bool result = !check_gl_error("gl_context::frame_buffer_attach", &fbb);
	frame_buffer_unbind(fbb, user_data);
	return result;
}

/// check for completeness, if not complete, get the reason in last_error
bool gl_context::frame_buffer_is_complete(const frame_buffer_base& fbb) const
{
	if (fbb.handle == 0) {
		error("gl_context::frame_buffer_is_complete: attempt to check completeness on frame buffer that is not created", &fbb);
		return false;
	}
	void* user_data;
	frame_buffer_bind(fbb,user_data);
	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	frame_buffer_unbind(fbb,user_data);
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

int gl_context::frame_buffer_get_max_nr_color_attachments()
{
	if (!check_fbo_support("gl_context::frame_buffer_get_max_nr_color_attachments"))
		return 0;

	GLint nr;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &nr);
	return nr;
}

int gl_context::frame_buffer_get_max_nr_draw_buffers()
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

void gl_context::shader_code_destruct(render_component& sc)
{
	if (sc.handle == 0) {
		error("gl_context::shader_code_destruct: shader not created", &sc);
		return;
	}
	glDeleteShader(get_gl_id(sc.handle));
	check_gl_error("gl_context::shader_code_destruct", &sc);
}

bool gl_context::shader_code_create(render_component& sc, ShaderType st, const std::string& source)
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

bool gl_context::shader_code_compile(render_component& sc)
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

bool gl_context::shader_program_create(shader_program_base& spb)
{
	if (!check_shader_support(ST_VERTEX, "gl_context::shader_program_create", &spb))
		return false;
	spb.handle = get_handle(glCreateProgram());
	return true;
}

void gl_context::shader_program_attach(shader_program_base& spb, const render_component& sc)
{
	if (spb.handle == 0) {
		error("gl_context::shader_program_attach: shader program not created", &spb);
		return;
	}
	glAttachShader(get_gl_id(spb.handle), get_gl_id(sc.handle));
}

void gl_context::shader_program_detach(shader_program_base& spb, const render_component& sc)
{
	if (spb.handle == 0) {
		error("gl_context::shader_program_detach: shader program not created", &spb);
		return;
	}
	glDetachShader(get_gl_id(spb.handle), get_gl_id(sc.handle));
}

bool gl_context::shader_program_link(shader_program_base& spb)
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
		return true;
	GLint infologLength = 0;
	glGetProgramiv(p_id, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0) {
		GLsizei charsWritten = 0;
		char *infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(p_id, infologLength, &charsWritten, infoLog);
		error(std::string("gl_context::shader_program_link\n")+infoLog, &spb);
		free(infoLog);
	}
	return false;
}

bool gl_context::shader_program_set_state(shader_program_base& spb)
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
	GLint old_p_id;
	glGetIntegerv(GL_CURRENT_PROGRAM, &old_p_id);
	GLuint p_id = get_gl_id(spb.handle);
	(GLint&)(spb.user_data) = old_p_id;
	glUseProgram(p_id); 
	return true;
}

bool gl_context::shader_program_disable(shader_program_base& spb)
{
	GLuint p_id = get_gl_id(spb.handle);
	GLint old_p_id = (GLint&)spb.user_data;
	glUseProgram(old_p_id); 
	return true;
}
void gl_context::shader_program_destruct(shader_program_base& spb)
{
	glDeleteProgram(get_gl_id(spb.handle));
}

std::string value_type_index_to_string(int value_type)
{
	std::string res, post_fix;
	if ((value_type & TO_ARRAY_MASK) > 0) {
		switch (value_type & TO_ARRAY_MASK) {
		case TO_VECTOR:
			res = "vector<";
			post_fix = ">";
			break;
		case TO_VEC_OF:
			res = "cgv::math::vec<";
			post_fix = ">";
			break;
		case TO_POINTER:
			post_fix = "*";
			break;
		}
	}
	value_type = value_type & ~TO_ARRAY_MASK;
	if ((value_type & TO_BASE_MASK) > 0) {
		switch (value_type & TO_BASE_MASK) {
		case TO_VEC:
			res += "cgv::math::vec<";
			post_fix = std::string(">") + post_fix;
			break;
		case TO_FVEC2:
			res += "cgv::math::fvec<";
			post_fix = std::string(",2>") + post_fix;
			break;
		case TO_FVEC3:
			res += "cgv::math::fvec<";
			post_fix = std::string(",3>") + post_fix;
			break;
		case TO_FVEC4:
			res += "cgv::math::fvec<";
			post_fix = std::string(",4>") + post_fix;
			break;
		case TO_MAT:
			res += "cgv::math::mat<";
			post_fix = std::string(">") + post_fix;
			break;
		case TO_FMAT2:
			res += "cgv::math::fmat<";
			post_fix = std::string(",2>") + post_fix;
			break;
		case TO_FMAT3:
			res += "cgv::math::fmat<";
			post_fix = std::string(",3>") + post_fix;
			break;
		case TO_FMAT4:
			res += "cgv::math::fmat<";
			post_fix = std::string(",4>") + post_fix;
			break;
		}
	}
	value_type = value_type & ~TO_BASE_MASK;
	return res + cgv::type::info::get_type_name(cgv::type::info::TypeId(value_type)) + post_fix;
}

bool gl_context::set_uniform_void(shader_program_base& spb, const std::string& name, int value_type, bool force_array, const void* value_ptr)
{
	GLuint p_id = get_gl_id(spb.handle);
	GLint loc = glGetUniformLocation(p_id, name.c_str());
	if (loc == -1) {
		error(std::string("gl_context::set_uniform_void(): Can not find uniform location ") + name, &spb);
		return false;
	}
	GLint old_p_id;
	glGetIntegerv(GL_CURRENT_PROGRAM, &old_p_id);
	glUseProgram(p_id); 
	switch (value_type) {
	case TI_BOOL : glUniform1i(loc, *reinterpret_cast<const bool*>(value_ptr) ? 1 : 0); break;
	case TI_UINT8 : glUniform1ui(loc, *reinterpret_cast<const uint8_type*>(value_ptr)); break;
	case TI_UINT16 : glUniform1ui(loc, *reinterpret_cast<const uint16_type*>(value_ptr)); break;
	case TI_UINT32 : glUniform1ui(loc, *reinterpret_cast<const uint32_type*>(value_ptr)); break;
	case TI_INT8 : glUniform1i(loc, *reinterpret_cast<const int8_type*>(value_ptr)); break;
	case TI_INT16 : glUniform1i(loc, *reinterpret_cast<const int16_type*>(value_ptr)); break;
	case TI_INT32 : glUniform1i(loc, *reinterpret_cast<const int32_type*>(value_ptr)); break;
	case TI_FLT32 : glUniform1f(loc, *reinterpret_cast<const flt32_type*>(value_ptr)); break;

#include "gl_context_switch.h"

	case TI_FLT32 + TO_VECTOR + TO_VEC:
	{
		unsigned i;
		const std::vector<vec<flt32_type> >& vm = *reinterpret_cast<const std::vector<vec<flt32_type> >*>(value_ptr);
		for (i = 0; i<vm.size(); ++i) {
			GLint loc = glGetUniformLocation(p_id, (name + "[" + cgv::utils::to_string(i) + "]").c_str());
			if (loc == -1) {
				spb.last_error = std::string("Can not find uniform location ") + name + "[" + cgv::utils::to_string(i) + "]";
				return false;
			}
			switch (vm[i].size()) {
			case  4: glUniform2fv(loc, 1, vm[i]); break;
			case  9: glUniform3fv(loc, 1, vm[i]); break;
			case 16: glUniform4fv(loc, 1, vm[i]); break;
			}
		}
		break;
	}
	case TI_FLT32 + TO_VECTOR + TO_FVEC2:
	{
		const std::vector<fvec<flt32_type, 2> >& vm = *reinterpret_cast<const std::vector<fvec<flt32_type, 2> >*>(value_ptr);
		glUniform2fv(loc, (GLsizei)vm.size(), &vm[0](0));
		break;
	}
	case TI_FLT32 + TO_VECTOR + TO_FVEC3:
	{
		const std::vector<fvec<flt32_type, 3> >& vm = *reinterpret_cast<const std::vector<fvec<flt32_type, 3> >*>(value_ptr);
		glUniform3fv(loc, (GLsizei)vm.size(), &vm[0](0));
		break;
	}
	case TI_FLT32 + TO_VECTOR + TO_FVEC4:
	{
		const std::vector<fvec<flt32_type, 4> >& vm = *reinterpret_cast<const std::vector<fvec<flt32_type, 4> >*>(value_ptr);
		glUniform4fv(loc, (GLsizei)vm.size(), &vm[0](0));
		break;
	}

	case TI_FLT32 + TO_VECTOR + TO_MAT:
		{
			unsigned i;
			const std::vector<mat<flt32_type> >& vm = *reinterpret_cast<const std::vector<mat<flt32_type> >*>(value_ptr);
			for (i=0; i<vm.size(); ++i) {
				GLint loc = glGetUniformLocation(p_id, (name + "[" + cgv::utils::to_string(i) + "]").c_str());
				if (loc == -1) {
					spb.last_error = std::string("Can not find uniform location ") + name + "[" + cgv::utils::to_string(i) + "]";
					return false;
				}
				switch (vm[i].size()) {
				case  4 : glUniformMatrix2fv(loc, 1, 0, vm[i]); break;
				case  9 : glUniformMatrix3fv(loc, 1, 0, vm[i]); break;
				case 16 : glUniformMatrix4fv(loc, 1, 0, vm[i]); break;
				}
			}
			break;
		}
	case TI_FLT32 + TO_VECTOR + TO_FMAT2:
		{
			const std::vector<fmat<flt32_type,2,2> >& vm = *reinterpret_cast<const std::vector<fmat<flt32_type,2,2> >*>(value_ptr);
			glUniformMatrix2fv(loc, (GLsizei)vm.size(), 0, &vm[0](0,0)); 
			break;
		}
	case TI_FLT32 + TO_VECTOR + TO_FMAT3:
		{
			const std::vector<fmat<flt32_type,3,3> >& vm = *reinterpret_cast<const std::vector<fmat<flt32_type,3,3> >*>(value_ptr);
			glUniformMatrix3fv(loc, (GLsizei)vm.size(), 0, &vm[0](0, 0));
			break;
		}
	case TI_FLT32 + TO_VECTOR + TO_FMAT4:
		{
			const std::vector<fmat<flt32_type,4,4> >& vm = *reinterpret_cast<const std::vector<fmat<flt32_type,4,4> >*>(value_ptr);
			glUniformMatrix4fv(loc, (GLsizei)vm.size(), 0, &vm[0](0, 0));
			break;
		}
	default: 
		error(std::string("gl_context::set_uniform_void() uniform of type ") + value_type_index_to_string(value_type) + " not supported!", &spb);
		glUseProgram(old_p_id); 
		return false;
	}
	glUseProgram(old_p_id); 
	check_gl_error("gl_context::set_uniform_void()", &spb);
	return true;
}
void gl_context::enable_attribute_array(int loc, bool do_enable)
{
	if (do_enable)
		glEnableVertexAttribArray(loc);
	else
		glDisableVertexAttribArray(loc);
	check_gl_error("gl_context::enable_attribute_array()");
}

int  gl_context::get_attribute_location(shader_program_base& spb, const std::string& name) const
{
	GLuint p_id = get_gl_id(spb.handle);
	return glGetAttribLocation(p_id, name.c_str());
}

bool gl_context::set_attribute_void(shader_program_base& spb, int loc, int value_type, bool force_array, const void* value_ptr, unsigned stride, unsigned size)
{
	if ((value_type & TO_ARRAY_MASK) == 0) {
		if ((value_type & TO_BASE_MASK) == 0) {
			switch (value_type) {
			case TI_INT8:   glVertexAttrib1s(loc, *reinterpret_cast<const int8_type*>(value_ptr)); break;
			case TI_INT16:  glVertexAttrib1s(loc, *reinterpret_cast<const int16_type*>(value_ptr)); break;
			case TI_INT32:  glVertexAttribI1i(loc, *reinterpret_cast<const int32_type*>(value_ptr)); break;
			case TI_UINT8:  glVertexAttrib1s(loc, *reinterpret_cast<const uint8_type*>(value_ptr)); break;
			case TI_UINT16: glVertexAttrib1s(loc, *reinterpret_cast<const uint16_type*>(value_ptr)); break;
			case TI_UINT32: glVertexAttribI1ui(loc, *reinterpret_cast<const uint32_type*>(value_ptr)); break;
			case TI_FLT32:  glVertexAttrib1f(loc, *reinterpret_cast<const flt32_type*>(value_ptr)); break;
			default:
				error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb);
				return false;
			}
		}
		else {
			switch (value_type & TO_BASE_MASK) {
			case TO_FVEC2:
				switch (value_type & ~TO_BASE_MASK) {
				case TI_INT16:  glVertexAttrib2sv(loc, &(*reinterpret_cast<const cgv::math::fvec<int16_type, 2>*>(value_ptr))(0)); break;
				case TI_INT32:  glVertexAttribI2iv(loc, &(*reinterpret_cast<const cgv::math::fvec<int32_type, 2>*>(value_ptr))(0)); break;
				case TI_UINT32: glVertexAttribI2uiv(loc, &(*reinterpret_cast<const cgv::math::fvec<uint32_type, 2>*>(value_ptr))(0)); break;
				case TI_FLT32:  glVertexAttrib2fv(loc, &(*reinterpret_cast<const cgv::math::fvec<flt32_type, 2>*>(value_ptr))(0)); break;
				default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
				}
				break;
			case TO_FVEC3:
				switch (value_type & ~TO_BASE_MASK) {
				case TI_INT16:  glVertexAttrib3sv(loc, &(*reinterpret_cast<const cgv::math::fvec<int16_type, 3>*>(value_ptr))(0)); break;
				case TI_INT32:  glVertexAttribI3iv(loc, &(*reinterpret_cast<const cgv::math::fvec<int32_type, 3>*>(value_ptr))(0)); break;
				case TI_UINT32: glVertexAttribI3uiv(loc, &(*reinterpret_cast<const cgv::math::fvec<uint32_type, 3>*>(value_ptr))(0)); break;
				case TI_FLT32:  glVertexAttrib3fv(loc, &(*reinterpret_cast<const cgv::math::fvec<flt32_type, 3>*>(value_ptr))(0)); break;
				default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
				}
				break;
			case TO_FVEC4:
				switch (value_type & ~TO_BASE_MASK) {
				case TI_INT16:  glVertexAttrib4sv(loc, &(*reinterpret_cast<const cgv::math::fvec<int16_type, 4>*>(value_ptr))(0)); break;
				case TI_INT32:  glVertexAttribI4iv(loc, &(*reinterpret_cast<const cgv::math::fvec<int32_type, 4>*>(value_ptr))(0)); break;
				case TI_UINT32: glVertexAttribI4uiv(loc, &(*reinterpret_cast<const cgv::math::fvec<uint32_type, 4>*>(value_ptr))(0)); break;
				case TI_FLT32:  glVertexAttrib4fv(loc, &(*reinterpret_cast<const cgv::math::fvec<flt32_type, 4>*>(value_ptr))(0)); break;
				default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
				}
				break;
			case TO_VEC: // consider force_array flag here
			case TO_MAT:
			case TO_FMAT2:
			case TO_FMAT3:
			case TO_FMAT4:
			default:
				error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb);
				return false;
			}
		}
	}
	else {
		switch (value_type & TO_ARRAY_MASK) {
		case TO_VECTOR :
			if ((value_type & TO_BASE_MASK) == 0) {
				switch (value_type & ~(TO_BASE_MASK + TO_ARRAY_MASK)) {
				case TI_INT8:   glVertexAttribIPointer(loc, 1, GL_BYTE, 0, &reinterpret_cast<const std::vector<int8_type>*>(value_ptr)->front()); break;
				case TI_INT16:  glVertexAttribIPointer(loc, 1, GL_SHORT, 0, &reinterpret_cast<const std::vector<int16_type>*>(value_ptr)->front()); break;
				case TI_INT32:  glVertexAttribIPointer(loc, 1, GL_INT, 0, &reinterpret_cast<const std::vector<int32_type>*>(value_ptr)->front()); break;
				case TI_UINT8:  glVertexAttribIPointer(loc, 1, GL_UNSIGNED_BYTE, 0, &reinterpret_cast<const std::vector<uint8_type>*>(value_ptr)->front()); break;
				case TI_UINT16: glVertexAttribIPointer(loc, 1, GL_UNSIGNED_SHORT, 0, &reinterpret_cast<const std::vector<uint16_type>*>(value_ptr)->front()); break;
				case TI_UINT32: glVertexAttribIPointer(loc, 1, GL_UNSIGNED_INT, 0, &reinterpret_cast<const std::vector<uint32_type>*>(value_ptr)->front()); break;
				case TI_FLT32:  glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, 0, &reinterpret_cast<const std::vector<flt32_type>*>(value_ptr)->front()); break;
				default:
					error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb);
					return false;
				}
			}
			else {
				switch (value_type & TO_BASE_MASK) {
				case TO_FVEC2:
					switch (value_type & ~(TO_BASE_MASK + TO_ARRAY_MASK)) {
					case TI_INT16:  glVertexAttribIPointer(loc, 2, GL_SHORT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<int16_type, 2> >*>(value_ptr)->front()); break;
					case TI_INT32:  glVertexAttribIPointer(loc, 2, GL_INT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<int32_type,2> >*>(value_ptr)->front()); break;
					case TI_UINT32: glVertexAttribIPointer(loc, 2, GL_UNSIGNED_INT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<uint32_type, 2> >*>(value_ptr)->front()); break;
					case TI_FLT32:  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<flt32_type,2> >*>(value_ptr)->front()); break;
					default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
					}
					break;
				case TO_FVEC3:
					switch (value_type & ~(TO_BASE_MASK+TO_ARRAY_MASK)) {
					case TI_INT16:  glVertexAttribIPointer(loc, 3, GL_SHORT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<int16_type, 3> >*>(value_ptr)->front()); break;
					case TI_INT32:  glVertexAttribIPointer(loc, 3, GL_INT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<int32_type, 3> >*>(value_ptr)->front()); break;
					case TI_UINT32: glVertexAttribIPointer(loc, 3, GL_UNSIGNED_INT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<uint32_type, 3> >*>(value_ptr)->front()); break;
					case TI_FLT32:  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<flt32_type, 3> >*>(value_ptr)->front()); break;
					default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
					}
					break;
				case TO_FVEC4:
					switch (value_type & ~(TO_BASE_MASK + TO_ARRAY_MASK)) {
					case TI_INT16:  glVertexAttribIPointer(loc, 4, GL_SHORT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<int16_type, 4> >*>(value_ptr)->front()); break;
					case TI_INT32:  glVertexAttribIPointer(loc, 4, GL_INT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<int32_type, 4> >*>(value_ptr)->front()); break;
					case TI_UINT32: glVertexAttribIPointer(loc, 4, GL_UNSIGNED_INT, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<uint32_type, 4> >*>(value_ptr)->front()); break;
					case TI_FLT32:  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, &reinterpret_cast<const std::vector<cgv::math::fvec<flt32_type, 4> >*>(value_ptr)->front()); break;
					default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
					}
					break;
				case TO_VEC:
				case TO_MAT:
				case TO_FMAT2:
				case TO_FMAT3:
				case TO_FMAT4:
				default:
					error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb);
					return false;
				}
			}
			break;
		case TO_POINTER:
			if ((value_type & TO_BASE_MASK) == 0) {
				switch (value_type & ~(TO_BASE_MASK + TO_ARRAY_MASK)) {
				case TI_INT8:   glVertexAttribIPointer(loc, 1, GL_BYTE, stride, reinterpret_cast<const int8_type*>(value_ptr)); break;
				case TI_INT16:  glVertexAttribIPointer(loc, 1, GL_SHORT, stride, reinterpret_cast<const int16_type*>(value_ptr)); break;
				case TI_INT32:  glVertexAttribIPointer(loc, 1, GL_INT, stride, reinterpret_cast<const int32_type*>(value_ptr)); break;
				case TI_UINT8:  glVertexAttribIPointer(loc, 1, GL_UNSIGNED_BYTE, stride, reinterpret_cast<const uint8_type*>(value_ptr)); break;
				case TI_UINT16: glVertexAttribIPointer(loc, 1, GL_UNSIGNED_SHORT, stride, reinterpret_cast<const uint16_type*>(value_ptr)); break;
				case TI_UINT32: glVertexAttribIPointer(loc, 1, GL_UNSIGNED_INT, stride, reinterpret_cast<const uint32_type*>(value_ptr)); break;
				case TI_FLT32:  glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const flt32_type*>(value_ptr)); break;
				default:
					error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb);
					return false;
				}
			}
			else {
				switch (value_type & TO_BASE_MASK) {
				case TO_FVEC2:
					switch (value_type & ~(TO_BASE_MASK + TO_ARRAY_MASK)) {
					case TI_INT16:  glVertexAttribIPointer(loc, 2, GL_SHORT, stride, reinterpret_cast<const int16_type*>(value_ptr)); break;
					case TI_INT32:  glVertexAttribIPointer(loc, 2, GL_INT, stride, reinterpret_cast<const int32_type*>(value_ptr)); break;
					case TI_UINT32: glVertexAttribIPointer(loc, 2, GL_UNSIGNED_INT, stride, reinterpret_cast<const uint32_type*>(value_ptr)); break;
					case TI_FLT32:  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const flt32_type*>(value_ptr)); break;
					default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
					}
					break;
				case TO_FVEC3:
					switch (value_type & ~(TO_BASE_MASK + TO_ARRAY_MASK)) {
					case TI_INT16:  glVertexAttribIPointer(loc, 3, GL_SHORT, stride, reinterpret_cast<const int16_type*>(value_ptr)); break;
					case TI_INT32:  glVertexAttribIPointer(loc, 3, GL_INT, stride, reinterpret_cast<const int32_type*>(value_ptr)); break;
					case TI_UINT32: glVertexAttribIPointer(loc, 3, GL_UNSIGNED_INT, stride, reinterpret_cast<const uint32_type*>(value_ptr)); break;
					case TI_FLT32:  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const flt32_type*>(value_ptr)); break;
					default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
					}
					break;
				case TO_FVEC4:
					switch (value_type & ~(TO_BASE_MASK + TO_ARRAY_MASK)) {
					case TI_INT16:  glVertexAttribIPointer(loc, 4, GL_SHORT, stride, reinterpret_cast<const int16_type*>(value_ptr)); break;
					case TI_INT32:  glVertexAttribIPointer(loc, 4, GL_INT, stride, reinterpret_cast<const int32_type*>(value_ptr)); break;
					case TI_UINT32: glVertexAttribIPointer(loc, 4, GL_UNSIGNED_INT, stride, reinterpret_cast<const uint32_type*>(value_ptr)); break;
					case TI_FLT32:  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const flt32_type*>(value_ptr)); break;
					default: error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb); return false;
					}
					break;
				case TO_VEC:
				case TO_MAT:
				case TO_FMAT2:
				case TO_FMAT3:
				case TO_FMAT4:
				default:
					error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb);
					return false;
				}
			}
			break;
		default:
			error(std::string("gl_context::set_attribute_void() attribute of type ") + value_type_index_to_string(value_type) + " not supported!", &spb);
			return false;
		}
	}
	check_gl_error("gl_context::set_attribute_void()", &spb);
	return true;
}

		}
	}
}