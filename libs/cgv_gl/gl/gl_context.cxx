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
		GL_LINES_ADJACENCY_EXT,
		GL_LINE_STRIP,
		GL_LINE_STRIP_ADJACENCY_EXT,
		GL_LINE_LOOP,
		GL_TRIANGLES,
		GL_TRIANGLES_ADJACENCY_EXT,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_STRIP_ADJACENCY_EXT,
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

void gl_context::put_id(void* handle, void* ptr) const
{
	GLuint tex_id = (const GLuint&) handle - 1;
	*static_cast<GLuint*>(ptr) = tex_id;
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
	last_context = (void*)-1;
	last_width = -1;
	last_height = -1;

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
void gl_context::configure_gl(void* new_context)
{
	group_ptr grp(get_group_interface());
	if (new_context != last_context) {
		last_context = new_context;
		enable_font_face(info_font_face,info_font_size);
		// use the eye location to compute the specular lighting
		glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
		// this makes opengl normalize all surface normals before lighting calculations,
		// which is essential when using scaling to deform tesselated primities
		glEnable(GL_NORMALIZE);
		glViewport(0,0,get_width(),get_height());
		last_width = get_width();
		last_height = get_height();

		/*
		if (grp) {
			single_method_action<drawable,void,cgv::render::context*> sma(this, &drawable::set_context);
			traverser(sma).traverse(grp);
			single_method_action<drawable,bool,cgv::render::context&> sma1(*this, &drawable::init);
			traverser(sma1).traverse(grp);
		}
		*/
	}
	else if ((int)last_width != get_width() || (int)last_height != get_height()) {
		last_width  = get_width();
		last_height = get_height();
		glViewport(0,0,get_width(),get_height());
		if (grp) {
			single_method_action_2<drawable,void,unsigned int,unsigned int> sma(last_width, last_height, &drawable::resize);
			traverser(sma).traverse(grp);
		}
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
	group* grp = get_group_interface();
	if (grp && (get_render_pass_flags()&RPF_DRAWABLES_INIT_FRAME)) {
		single_method_action<drawable,void,cgv::render::context&> sma(*this, &drawable::init_frame, true, true);
		traverser(sma).traverse(group_ptr(grp));
	}
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

void enable_material_color(const textured_material::color_type& c, float alpha, GLenum type)
{
	GLfloat v[4] = {c[0],c[1],c[2],c[3]*alpha};
	glMaterialfv(GL_FRONT_AND_BACK, type, v);
}


/// enable a material without textures
void gl_context::enable_material(const cgv::media::illum::phong_material& mat, MaterialSide ms, float alpha)
{
	glPushAttrib(GL_LIGHTING_BIT|GL_ENABLE_BIT);
	glEnable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	enable_material_color(mat.get_ambient(),alpha,GL_AMBIENT);
	enable_material_color(mat.get_diffuse(),alpha,GL_DIFFUSE);
	enable_material_color(mat.get_specular(),alpha,GL_SPECULAR);
	enable_material_color(mat.get_emission(),alpha,GL_EMISSION);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.get_shininess());
	if (phong_shading) {
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
	bool do_alpha = (mat.get_diffuse_texture() != 0) && mat.get_diffuse_texture()->get_component_name(mat.get_diffuse_texture()->get_nr_components()-1)[0] == 'A';
	GLuint flags = do_alpha ? GL_COLOR_BUFFER_BIT : GL_CURRENT_BIT;
	if (mat.get_bump_texture() != 0 || mat.get_diffuse_texture() != 0)
		flags |= GL_TEXTURE_BIT;
	flags |= GL_LIGHTING_BIT|GL_ENABLE_BIT;
	glPushAttrib(flags);

	glEnable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	enable_material_color(mat.get_ambient(),alpha,GL_AMBIENT);
	enable_material_color(mat.get_diffuse(),alpha,GL_DIFFUSE);
	enable_material_color(mat.get_specular(),alpha,GL_SPECULAR);
	enable_material_color(mat.get_emission(),alpha,GL_EMISSION);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.get_shininess());

	if ((mat.get_bump_texture() || phong_shading) && ref_textured_material_prog(*this).is_linked()) {
		shader_program& prog = ref_textured_material_prog(*this);
		bool use_bump_map = mat.get_bump_texture() != 0;
		if (use_bump_map)
			mat.get_bump_texture()->enable(*this,0);

		bool use_diffuse_map = mat.get_diffuse_texture() != 0;
		if (use_diffuse_map)
			mat.get_diffuse_texture()->enable(*this,1);

		prog.enable(*this);
		prog.set_uniform(*this, "use_bump_map", use_bump_map);
		if (use_bump_map) {
			prog.set_uniform(*this, "bump_map", 0);
			prog.set_uniform(*this, "bump_map_res", (int) (mat.get_bump_texture()->get_width()));
			prog.set_uniform(*this, "bump_scale", 400*mat.get_bump_scale());
		}
		prog.set_uniform(*this, "use_diffuse_map", use_diffuse_map);
		if (use_diffuse_map)
			prog.set_uniform(*this, "diffuse_map", 1);
		set_lighting_parameters(*this, prog);
	}
	else if (mat.get_diffuse_texture()) {
		enable_material_color(textured_material::color_type(1,1,1,1),alpha,GL_DIFFUSE);
		mat.get_diffuse_texture()->enable(*this);
		glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV, GL_MODULATE);
	}
	if (do_alpha) {
		glEnable(GL_ALPHA_TEST);
		switch (mat.get_alpha_test_func()) {
		case textured_material::AT_ALWAYS : glAlphaFunc(GL_ALWAYS, mat.get_alpha_threshold()); break;
		case textured_material::AT_LESS   : glAlphaFunc(GL_LESS, mat.get_alpha_threshold()); break;
		case textured_material::AT_EQUAL  : glAlphaFunc(GL_EQUAL, mat.get_alpha_threshold()); break;
		case textured_material::AT_GREATER: glAlphaFunc(GL_GREATER, mat.get_alpha_threshold()); break;
		}
	}
	else
		glColor4f(1,1,1,alpha);
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

void gl_context::tesselate_arrow(double length, double aspect, double rel_tip_radius, double tip_aspect)
{
	double cyl_radius = length*aspect;
	double cone_radius = rel_tip_radius*cyl_radius;
	double cone_length = cone_radius/tip_aspect;
	double cyl_length = length - cone_length;
	push_V();
	glScaled(cyl_radius,cyl_radius,0.5*cyl_length);
		push_V();
			glRotatef(180,1,0,0);
				tesselate_unit_disk();
		pop_V();

	glTranslated(0,0,1);
		tesselate_unit_cylinder();

	glTranslated(0,0,1);
	glScaled(rel_tip_radius,rel_tip_radius,cone_length/cyl_length);
		push_V();
			glRotatef(180,1,0,0);
				tesselate_unit_disk();
		pop_V();
	glTranslated(0,0,1);
		tesselate_unit_cone();
	pop_V();
}

///
void gl_context::tesselate_arrow(const cgv::math::fvec<double,3>& start, const cgv::math::fvec<double,3>& end, double aspect, double rel_tip_radius, double tip_aspect)
{
	push_V();
	glTranslated(start(0),start(1),start(2));
	rotate(cgv::math::fvec<double,3>(0,0,1), end-start);
	tesselate_arrow((end-start).length(), aspect, rel_tip_radius, tip_aspect);
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
			std::cerr << "read_frame_buffer: received invalid dimensions ("
				<< w << "," << h << ")" << std::endl;
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
		std::cerr << "read_frame_buffer: could not make component type " 
			<< cgv::type::info::get_type_name(df->get_component_type())
			<< " to gl type" << std::endl;
		return false;
	}
	GLuint gl_format = GL_DEPTH_COMPONENT;
	if (cf != cgv::data::CF_D) {
		gl_format = GL_STENCIL_INDEX;
		if (cf != cgv::data::CF_S) {
			gl_format = map_to_gl(cf);
			if (gl_format == GL_RGB && cf != cgv::data::CF_RGB) {
				std::cerr << "read_frame_buffer: could not match component format " 
					<< df->get_component_format() << std::endl;
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
			std::cout << "invalid buffer type " << buffer_type << std::endl;
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
	D(1, 3) = 0.5*vp[3] + vp[1];
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

	glReadPixels(x_D, get_height()-y_D, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z_D);
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


GLuint tex_dim[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP_EXT };
GLuint tex_bind[]= { GL_TEXTURE_BINDING_1D, GL_TEXTURE_BINDING_2D, GL_TEXTURE_BINDING_3D, GL_TEXTURE_BINDING_CUBE_MAP_EXT,GL_TEXTURE_BUFFER };

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

GLuint gl_context::texture_generate(texture_base& tb)
{
	if (tb.tt == TT_3D && !(ensure_glew_initialized() && glTexImage3D)) {
		tb.last_error = "attempt to create 3d texture, which is not supported";
		return -1;
	}
	if (tb.tt == TT_CUBEMAP && !(ensure_glew_initialized() && glewIsExtensionSupported("GL_EXT_texture_cube_map"))) {
		tb.last_error = "attempt to create cube map, which is not supported";
		return -1;
	}
	GLuint tex_id = -1;
	glGenTextures(1, &tex_id);
	if (glGetError() == GL_INVALID_OPERATION) {
		tb.last_error = "attempt to create texture inside glBegin-glEnd-block";
		return tex_id;
	}
	return tex_id;
}

int gl_context::query_integer_constant(ContextIntegerConstant cic) const
{
	GLint gl_const;
	switch (cic) {
		case MAX_NR_GEOMETRY_SHADER_OUTPUT_VERTICES :
			gl_const = GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT;
			break;
	}
	GLint value;
	glGetIntegerv(gl_const, &value);
	return value;
}

GLuint gl_context::texture_bind(TextureType tt, GLuint tex_id)
{
	GLint tmp_id;
	glGetIntegerv(tex_bind[tt-1], &tmp_id);
	glBindTexture(tex_dim[tt-1], tex_id);
	return tmp_id;
}

void gl_context::texture_unbind(TextureType tt, GLuint tmp_id)
{
	glBindTexture(tex_dim[tt-1], tmp_id);
}

bool gl_context::texture_create(
						texture_base& tb, 
						cgv::data::data_format& df)
{
	GLuint gl_format = (const GLuint&) tb.internal_format;
	
	if (tb.tt == TT_UNDEF)
		tb.tt = (TextureType)df.get_nr_dimensions();
	GLuint tex_id = texture_generate(tb);
	if (tex_id == -1)
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
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, 0,
			gl_format, df.get_width(), df.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		break;
	}
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		tb.last_error = std::string((const char*)gluErrorString(error));
		glDeleteTextures(1, &tex_id);
		texture_unbind(tb.tt, tmp_id);
		return false;
	}

	texture_unbind(tb.tt, tmp_id);
	tb.have_mipmaps = false;
	reinterpret_cast<GLuint&>(tb.handle) = tex_id+1;
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
		tex_id = (const GLuint&) tb.handle - 1;
	else {
		tex_id = texture_generate(tb);
		if (tex_id == -1)
			return false;
		reinterpret_cast<GLuint&>(tb.handle) = tex_id+1;
	}

	// bind texture
	GLuint tmp_id = texture_bind(tb.tt, tex_id);

	// load data to texture
	tb.have_mipmaps = load_texture(data, gl_tex_format, level, cube_side, palettes);
	
	// restore old texture
	texture_unbind(tb.tt, tmp_id);
	return true;
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
		tex_id = (const GLuint&) tb.handle - 1;
	else {
		tex_id = texture_generate(tb);
		if (tex_id == -1)
			return false;
		reinterpret_cast<GLuint&>(tb.handle) = tex_id+1;
	}
	GLuint tmp_id = texture_bind(tb.tt, tex_id);

	// check mipmap type
	bool gen_mipmap = level == -1;
	if (gen_mipmap)
		level = 0;

	glCopyTexImage2D(GL_TEXTURE_2D, level, gl_format, x, y, df.get_width(), df.get_height(), 0);
	bool error = true;
	switch (glGetError()) {
	case GL_INVALID_ENUM : 
		tb.last_error = "target was not an accepted value."; 
		break;
	case GL_INVALID_VALUE : 
		tb.last_error = "level was less than zero or greater than log sub 2(max), where max is the returned value of GL_MAX_TEXTURE_SIZE.\n"
							 "or border was not zero or 1.\n"
							 "or width was less than zero, greater than 2 + GL_MAX_TEXTURE_SIZE; or width cannot be represented as 2n + 2 * border for some integer n.";
		break;
	case GL_INVALID_OPERATION : 
		tb.last_error = "glCopyTexImage2D was called between a call to glBegin and the corresponding call to glEnd.";
		break;
	default:
		error = false;
	}
	texture_unbind(tb.tt, tmp_id);

	if (gen_mipmap) 
		texture_generate_mipmaps(tb, tb.tt == TT_CUBEMAP ? 2 : (int)tb.tt);

	return error;
}

bool gl_context::texture_replace(
						texture_base& tb, 
						int x, int y, int z, 
						const cgv::data::const_data_view& data, 
						int level, const std::vector<cgv::data::data_view>* palettes)
{
	GLuint tex_id = (const GLuint&) tb.handle - 1;
	if (tex_id == -1) {
	tb.last_error = "attempt to replace in not created texture";
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
			tb.last_error = "replace on cubemap without the side defined";
			return false;
		}
		if (z < 0 || z > 5) {
			tb.last_error = "replace on cubemap without invalid side specification";
			return false;
		}
	}
	else {
		if (tb.tt != dim) {
			tb.last_error = "replace on texture with invalid position specification";
			return false;
		}
	}

	// bind texture
	GLuint tmp_id = texture_bind(tb.tt,tex_id);
	tb.have_mipmaps = replace_texture(data, level, x, y, z, palettes) || tb.have_mipmaps;
	texture_unbind(tb.tt, tmp_id);
	return true;
}

bool gl_context::texture_replace_from_buffer(
							texture_base& tb, 
							int x, int y, int z, 
							int x_buffer, int y_buffer, 
							unsigned int width, unsigned int height, 
							int level)
{
	GLuint tex_id = (const GLuint&) tb.handle - 1;
	if (tex_id == -1) {
		tb.last_error = "attempt to replace in not created texture";
		return false;
	}

	// determine dimension from location arguments
	unsigned int dim = 2;
	if (z != -1)
		++dim;

	// consistency checks
	if (tb.tt == TT_CUBEMAP) {
		if (dim != 3) {
			tb.last_error = "replace on cubemap without the side defined";
			return false;
		}
		if (z < 0 || z > 5) {
			tb.last_error = "replace on cubemap without invalid side specification";
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
	GLuint tmp_id = texture_bind(tb.tt, tex_id);

	switch (tb.tt) {
		case TT_2D :
			glCopyTexSubImage2D(GL_TEXTURE_2D, level, x, y, 
										x_buffer, y_buffer, width, height);
			break;
		case TT_3D :
			glCopyTexSubImage3D(GL_TEXTURE_3D, level, x, y, z, 
										x_buffer, y_buffer, width, height);
			break;
		case TT_CUBEMAP :
			glCopyTexSubImage2D(get_gl_cube_map_target(z), level, x, y, 
										x_buffer, y_buffer, width, height);
			break;
	}
	texture_unbind(tb.tt, tmp_id);

	if (gen_mipmap) 
		texture_generate_mipmaps(tb, tb.tt == TT_CUBEMAP ? 2 : (int)tb.tt);
	return true;
}

bool gl_context::texture_generate_mipmaps(texture_base& tb, unsigned int dim)
{
	GLuint tex_id = ((const GLuint&) tb.handle)-1;
	GLuint tmp_id = texture_bind(tb.tt,tex_id);

	bool res = generate_mipmaps(dim, &tb.last_error);
	if (res) 
		tb.have_mipmaps = true;

	texture_unbind(tb.tt, tmp_id);
	return res;
}

bool gl_context::texture_destruct(render_component& rc)
{
	GLuint tex_id = ((const GLuint&) rc.handle)-1;
	if (tex_id == -1) {
		rc.last_error = "attempt to destruct not created texture";
		return false;
	}
	glDeleteTextures(1, &tex_id);
	rc.handle = 0;
	return true;
}

bool gl_context::texture_set_state(const texture_base& ts)
{
	if (ts.tt == TT_UNDEF) {
		ts.last_error = "attempt to set state on texture without type";
		return false;
	}
	GLuint tex_id = (GLuint&) ts.handle - 1;
	if (tex_id == -1) {
		ts.last_error = "attempt of setting texture state of not created texture";
		return false;
	}
	GLint tmp_id = texture_bind(ts.tt, tex_id);

	glTexParameteri(tex_dim[ts.tt-1], GL_TEXTURE_MIN_FILTER, map_to_gl(ts.min_filter));
	glTexParameteri(tex_dim[ts.tt-1], GL_TEXTURE_MAG_FILTER, map_to_gl(ts.mag_filter));
	glTexParameterf(tex_dim[ts.tt-1], GL_TEXTURE_PRIORITY, ts.priority);
	if (ts.min_filter == TF_ANISOTROP)
		glTexParameterf(tex_dim[ts.tt-1], GL_TEXTURE_MAX_ANISOTROPY_EXT, ts.anisotropy);
	else
		glTexParameterf(tex_dim[ts.tt-1], GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
	if (ts.border_color[0] >= 0.0f)
		glTexParameterfv(tex_dim[ts.tt-1], GL_TEXTURE_BORDER_COLOR, ts.border_color);
	glTexParameteri(tex_dim[ts.tt-1], GL_TEXTURE_WRAP_S, map_to_gl(ts.wrap_s));
	if (ts.tt > TT_1D)
		glTexParameteri(tex_dim[ts.tt-1], GL_TEXTURE_WRAP_T, map_to_gl(ts.wrap_t));
	if (ts.tt == TT_3D)
		glTexParameteri(tex_dim[ts.tt-1], GL_TEXTURE_WRAP_R, map_to_gl(ts.wrap_r));

	texture_unbind(ts.tt, tmp_id);
	return true;
}

bool gl_context::texture_enable(
						texture_base& tb, 
						int tex_unit, unsigned int dim)
{
	if (dim < 1 || dim > 3) {
		tb.last_error = "invalid texture dimension";
		return false;
	}
	GLuint tex_id = (GLuint&) tb.handle - 1;
	if (tex_id == -1) {
		tb.last_error = "texture not created";
		return false;
	}
	if (tex_unit >= 0) {
		if (!ensure_glew_initialized() || !glActiveTextureARB) {
			tb.last_error = "multi texturing not supported";
			return false;
		}
		glActiveTextureARB(GL_TEXTURE0_ARB+tex_unit);
	}
	GLint& old_binding = (GLint&) tb.user_data;
	glGetIntegerv(tex_bind[tb.tt-1], &old_binding);
	++old_binding;
	glBindTexture(tex_dim[tb.tt-1], tex_id);
	glEnable(tex_dim[tb.tt-1]);
	if (tex_unit >= 0)
		glActiveTextureARB(GL_TEXTURE0_ARB);
	return true;
}

bool gl_context::texture_disable(
						const texture_base& tb, 
						int tex_unit, unsigned int dim)
{
	if (dim < 1 || dim > 3) {
		tb.last_error = "invalid texture dimension";
		return false;
	}
	if (tex_unit == -2) {
		tb.last_error = "invalid texture unit";
		return false;
	}
	GLuint old_binding = (const GLuint&) tb.user_data;
	--old_binding;
	if (tex_unit >= 0)
		glActiveTextureARB(GL_TEXTURE0_ARB+tex_unit);
	glDisable(tex_dim[tb.tt-1]);
	glBindTexture(tex_dim[tb.tt-1], old_binding);
	if (tex_unit >= 0)
		glActiveTextureARB(GL_TEXTURE0_ARB);
	return true;
}

bool gl_context::render_buffer_create(
						render_component& rc, 
						cgv::data::component_format& cf, 
						int& _width, int& _height)
{
	if (!(ensure_glew_initialized() && GLEW_EXT_framebuffer_object)) {
		rc.last_error = "frame buffer objects not supported";
		return false;
	}
	if (_width == -1)
		_width = get_width();
	if (_height == -1)
		_height = get_height();

	GLuint rb_id;
	glGenRenderbuffersEXT(1, &rb_id);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb_id);

	GLuint& gl_format = (GLuint&)rc.internal_format;
	unsigned int i = find_best_match(cf,color_buffer_formats);
	cgv::data::component_format best_cf(color_buffer_formats[i]);
	gl_format = gl_color_buffer_format_ids[i];

	if (ensure_glew_initialized() && GLEW_ARB_depth_texture) {
		i = find_best_match(cf,depth_formats,&best_cf);
		if (i != -1) {
			best_cf = cgv::data::component_format(depth_formats[i]);
			gl_format = gl_depth_format_ids[i];
		}
	}
	cf = best_cf;

	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, gl_format,
		_width, _height);

	++rb_id;
	rc.handle = (void*&) rb_id;
	--rb_id;
	return true;
}

bool gl_context::render_buffer_destruct(render_component& rc)
{
	if (!(ensure_glew_initialized() && GLEW_EXT_framebuffer_object)) {
		rc.last_error = "frame buffer objects not supported";
		return false;
	}
	GLuint rb_id = ((GLuint&) rc.handle)+1;
	glDeleteRenderbuffersEXT(1, &rb_id);
	rc.handle = 0;
	return true;
}

bool gl_context::frame_buffer_create(frame_buffer_base& fbb)
{
	if (fbb.width == -1)
		fbb.width = get_width();
	if (fbb.height == -1)
		fbb.height = get_height();

	if (!ensure_glew_initialized()) {
		fbb.last_error = "could not initialize glew";
		return false;
	}
	if (!GLEW_EXT_framebuffer_object) {
		fbb.last_error = "framebuffer objects not supported";
		return false;
	}
	// allocate framebuffer object
	GLuint fbo_id;
	glGenFramebuffersEXT(1, &fbo_id);
	if (fbo_id == 0) {
		fbb.last_error = "could not allocate framebuffer object";
		return false;
	}
	++fbo_id;
	fbb.handle = (void*&) fbo_id;
	--fbo_id;
	return true;
}

void gl_context::frame_buffer_bind(const frame_buffer_base& fbb, void*& user_data) const
{
	if (fbb.handle == 0)
		return;
	GLuint fbo_id = (GLuint&) fbb.handle - 1;
	GLint old_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &old_binding);
	reinterpret_cast<GLuint&>(user_data) = old_binding + 1;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
}

void gl_context::frame_buffer_unbind(const frame_buffer_base& fbb, void*& user_data) const
{
	GLuint old_binding = (const GLuint&) user_data - 1;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, old_binding);
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
		fbb.last_error = "attempt to enable not created frame buffer";
		return false;
	}
	GLuint fbo_id = (GLuint&) fbb.handle-1;
	int i;
	int n = 0;
	GLenum draw_buffers[16];
	if (fbb.enabled_color_attachments.size() == 0) {
		for (i = 0; i < 16; ++i)
			if (fbb.attached[i]) {
				draw_buffers[n] = GL_COLOR_ATTACHMENT0_EXT+i;
				++n;
			}
	}
	else {
		for (i = 0; i < (int)fbb.enabled_color_attachments.size(); ++i) {
			if (fbb.attached[fbb.enabled_color_attachments[i]]) {
				draw_buffers[n] = GL_COLOR_ATTACHMENT0_EXT+fbb.enabled_color_attachments[i];
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
		fbb.last_error = "no attached draw buffer selected!!";
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
	if (!(ensure_glew_initialized() && GLEW_EXT_framebuffer_object)) {
		fbb.last_error = "frame buffer objects not supported";
		return false;
	}
	if (fbb.handle == 0) {
		fbb.last_error = "attempt to destruct not created frame buffer";
		return false;
	}
	GLuint fbo_id = (GLuint&)fbb.handle - 1;
	glDeleteFramebuffersEXT(1, &fbo_id);
	fbb.handle = 0;
	fbb.user_data = 0;
	return true;
}

bool gl_context::frame_buffer_attach(frame_buffer_base& fbb, 
												 const render_component& rb, bool is_depth, int i)
{
	if (rb.handle == 0) {
		fbb.last_error = "attempt to attach empty render buffer";
		return false;
	}
	GLuint rb_id = (const GLuint&) rb.handle - 1;
	void* user_data;
	frame_buffer_bind(fbb, user_data);
	if (is_depth) {
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
			GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, 
			rb_id);
	}
	else {
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, 
			GL_COLOR_ATTACHMENT0_EXT+i,
			GL_RENDERBUFFER_EXT, 
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
	void* user_data;
	frame_buffer_bind(fbb, user_data);
	GLuint tex_id = (const GLuint&) t.handle - 1;
	if (z_or_cube_side == -1) {
		if (is_depth) {
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
				GL_DEPTH_ATTACHMENT_EXT,
				GL_TEXTURE_2D, tex_id, level);
		}
		else {
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
				GL_COLOR_ATTACHMENT0_EXT+i, 
				GL_TEXTURE_2D, tex_id, level);
			fbb.attached[i] = true;
		}
	}
	else {
		if (t.tt == TT_CUBEMAP) {
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
				GL_COLOR_ATTACHMENT0_EXT+i, 
				get_gl_cube_map_target(z_or_cube_side), tex_id, level);
		}
		else {
			glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, 
				GL_COLOR_ATTACHMENT0_EXT+i, 
				GL_TEXTURE_3D, tex_id, level, z_or_cube_side);
		}
		fbb.attached[i] = true;
	}
	frame_buffer_unbind(fbb, user_data);
	return true;
}

/// check for completeness, if not complete, get the reason in last_error
bool gl_context::frame_buffer_is_complete(const frame_buffer_base& fbb) const
{
	void* user_data;
	frame_buffer_bind(fbb,user_data);
	GLenum error = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	frame_buffer_unbind(fbb,user_data);
	switch (error) {
	case GL_FRAMEBUFFER_COMPLETE_EXT:
		return true;
   case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		fbb.last_error = "incomplete attachment";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      fbb.last_error = "incomplete or missing attachment";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      fbb.last_error = "incomplete dimensions";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      fbb.last_error = "incomplete formats";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      fbb.last_error = "incomplete draw buffer";
		return false;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      fbb.last_error = "incomplete read buffer";
		return false;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      fbb.last_error = "framebuffer objects unsupported";
		return false;
	}
   fbb.last_error = "unknown error";
	return false;
}

int gl_context::frame_buffer_get_max_nr_color_attachments()
{
	if (!(ensure_glew_initialized() && GLEW_EXT_framebuffer_object)) {
		std::cerr << "frame buffer objects not supported" << std::endl;
		return 0;
	}
	GLint nr;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &nr);
	return nr;
}

int gl_context::frame_buffer_get_max_nr_draw_buffers()
{
	GLint nr;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &nr);
	return nr;
}

GLuint gl_shader_type[] = 
{
	GL_VERTEX_SHADER_ARB, GL_VERTEX_SHADER_ARB, GL_GEOMETRY_SHADER_EXT, GL_FRAGMENT_SHADER_ARB
};

void gl_context::shader_code_destruct(void* handle)
{
	GLuint s_id = (GLuint&) handle - 1;
	glDeleteObjectARB(s_id);
}

void* gl_context::shader_code_create(const std::string& source, ShaderType st, std::string& last_error)
{
	if (!ensure_glew_initialized()) {
		last_error = "could not initialize glew";
		return 0;
	}
	if (!GLEW_ARB_shader_objects) {
		last_error = "shaders are not supported";
		return 0;
	}
	if (st == ST_GEOMETRY && !(GLEW_EXT_geometry_shader4)) {
		last_error = "geometry shaders not supported";
		return 0;
	}
	GLuint s_id = glCreateShaderObjectARB(gl_shader_type[st]);
	if (s_id == -1)
		return 0;
	const char* s = source.c_str();
	glShaderSourceARB(s_id, 1, &s,NULL);
	void* handle = 0;
	reinterpret_cast<GLuint&>(handle) = s_id+1;
	return handle;
}

bool gl_context::shader_code_compile(void* handle, std::string& last_error)
{
	GLuint s_id = (GLuint&)handle - 1;
	glCompileShaderARB(s_id);
	int result;
	glGetObjectParameterivARB(s_id, GL_OBJECT_COMPILE_STATUS_ARB, &result); 
	if (result == 1)
		return true;
	last_error = std::string();
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	glGetObjectParameterivARB(s_id, GL_OBJECT_INFO_LOG_LENGTH_ARB,
					 &infologLength);
	if (infologLength > 0) {
		infoLog = (char *)malloc(infologLength);
		glGetInfoLogARB(s_id, infologLength, &charsWritten, infoLog);
		last_error = infoLog;
		free(infoLog);
	}
	return false;
}

bool gl_context::shader_program_create(void* &handle, std::string& last_error)
{
	if (!ensure_glew_initialized()) {
		last_error = "could not initialize glew";
		return false;
	}
	if (!GLEW_ARB_shader_objects) {
		last_error = "shaders are not supported";
		return false;
	}
	GLuint p_id = glCreateProgramObjectARB();
	(GLuint&) handle = p_id + 1;
	return true;
}

void gl_context::shader_program_attach(void* handle, void* code_handle)
{
	GLuint p_id = (const GLuint&) handle - 1;
	GLuint c_id = (const GLuint&) code_handle - 1;
	glAttachObjectARB(p_id,c_id);
}

void gl_context::shader_program_detach(void* handle, void* code_handle)
{
	GLuint p_id = (const GLuint&) handle - 1;
	GLuint c_id = (const GLuint&) code_handle - 1;
	glDetachObjectARB(p_id,c_id);
}

bool gl_context::shader_program_link(void* handle, std::string& last_error)
{
	GLuint p_id = (const GLuint&) handle - 1;
	glLinkProgramARB(p_id); 
	int result;
	glGetObjectParameterivARB(p_id, GL_OBJECT_LINK_STATUS_ARB, &result); 
	if (result == 1)
		return true;
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	glGetObjectParameterivARB(p_id, GL_OBJECT_INFO_LOG_LENGTH_ARB,
					 &infologLength);
	if (infologLength > 0) {
		infoLog = (char *)malloc(infologLength);
		glGetInfoLogARB(p_id, infologLength, &charsWritten, infoLog);
		last_error = infoLog;
		free(infoLog);
	}
	return false;
}

bool gl_context::shader_program_set_state(shader_program_base& spb)
{
	GLuint p_id = (const GLuint&) spb.handle - 1;
	glProgramParameteriEXT(p_id, GL_GEOMETRY_VERTICES_OUT_EXT, spb.geometry_shader_output_count);
	glProgramParameteriEXT(p_id, GL_GEOMETRY_INPUT_TYPE_EXT, map_to_gl(spb.geometry_shader_input_type));
	glProgramParameteriEXT(p_id, GL_GEOMETRY_OUTPUT_TYPE_EXT, map_to_gl(spb.geometry_shader_output_type));
	return true;
}

bool gl_context::shader_program_enable(render_component& rc)
{
	GLuint p_id = (const GLuint&) rc.handle - 1;
	GLhandleARB old_p_id = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
	(GLhandleARB&)(rc.user_data) = old_p_id;
	glUseProgramObjectARB(p_id); 
	return true;
}

bool gl_context::shader_program_disable(render_component& rc)
{
	GLuint p_id = (const GLuint&) rc.handle - 1;
	GLhandleARB old_p_id = (GLhandleARB&)rc.user_data;
	glUseProgramObjectARB(old_p_id); 
	glUseProgramObjectARB(0); 
	return true;
}
void gl_context::shader_program_destruct(void* handle)
{
	GLuint p_id = (const GLuint&) handle - 1;
	glDeleteObjectARB(p_id);
}

bool gl_context::set_uniform_void(void* handle, 
		const std::string& name, int value_type, bool dimension_independent, 
		const void* value_ptr, std::string& last_error)
{
	GLuint p_id = (const GLuint&) handle - 1;
	GLint loc = glGetUniformLocation(p_id, name.c_str());
	if (loc == -1) {
		last_error = std::string("Can not find uniform location ")+name;
		return false;
	}
	GLhandleARB old_p_id = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
	glUseProgramObjectARB(p_id); 
	switch (value_type) {
	case TI_BOOL : glUniform1i(loc, *static_cast<const bool*>(value_ptr) ? 1 : 0); break;
	case TI_UINT8 : glUniform1ui(loc, *static_cast<const uint8_type*>(value_ptr)); break;
	case TI_UINT16 : glUniform1ui(loc, *static_cast<const uint16_type*>(value_ptr)); break;
	case TI_UINT32 : glUniform1ui(loc, *static_cast<const uint32_type*>(value_ptr)); break;
	case TI_INT8 : glUniform1i(loc, *static_cast<const int8_type*>(value_ptr)); break;
	case TI_INT16 : glUniform1i(loc, *static_cast<const int16_type*>(value_ptr)); break;
	case TI_INT32 : glUniform1i(loc, *static_cast<const int32_type*>(value_ptr)); break;
	case TI_FLT32 : glUniform1f(loc, *static_cast<const flt32_type*>(value_ptr)); break;

#include "gl_context_switch.h"

	case TI_FLT32 + UTO_VECTOR_VEC:
	{
		unsigned i;
		const std::vector<vec<flt32_type> >& vm = *static_cast<const std::vector<vec<flt32_type> >*>(value_ptr);
		for (i = 0; i<vm.size(); ++i) {
			GLint loc = glGetUniformLocation(p_id, (name + "[" + cgv::utils::to_string(i) + "]").c_str());
			if (loc == -1) {
				last_error = std::string("Can not find uniform location ") + name + "[" + cgv::utils::to_string(i) + "]";
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
	case TI_FLT32 + UTO_VECTOR_FVEC:
	{
		const std::vector<fvec<flt32_type, 2> >& vm = *static_cast<const std::vector<fvec<flt32_type, 2> >*>(value_ptr);
		glUniform2fv(loc, (GLsizei)vm.size(), &vm[0](0));
		break;
	}
	case TI_FLT32 + UTO_VECTOR_FVEC + UTO_DIV:
	{
		const std::vector<fvec<flt32_type, 3> >& vm = *static_cast<const std::vector<fvec<flt32_type, 3> >*>(value_ptr);
		glUniform3fv(loc, (GLsizei)vm.size(), &vm[0](0));
		break;
	}
	case TI_FLT32 + UTO_VECTOR_FVEC + 2 * UTO_DIV:
	{
		const std::vector<fvec<flt32_type, 4> >& vm = *static_cast<const std::vector<fvec<flt32_type, 4> >*>(value_ptr);
		glUniform4fv(loc, (GLsizei)vm.size(), &vm[0](0));
		break;
	}

	case TI_FLT32 + UTO_VECTOR_MAT:
		{
			unsigned i;
			const std::vector<mat<flt32_type> >& vm = *static_cast<const std::vector<mat<flt32_type> >*>(value_ptr);
			for (i=0; i<vm.size(); ++i) {
				GLint loc = glGetUniformLocation(p_id, (name + "[" + cgv::utils::to_string(i) + "]").c_str());
				if (loc == -1) {
					last_error = std::string("Can not find uniform location ") + name + "[" + cgv::utils::to_string(i) + "]";
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
	case TI_FLT32+UTO_VECTOR_FMAT : 
		{
			const std::vector<fmat<flt32_type,2,2> >& vm = *static_cast<const std::vector<fmat<flt32_type,2,2> >*>(value_ptr);
			glUniformMatrix2fv(loc, (GLsizei)vm.size(), 0, &vm[0](0,0)); 
			break;
		}
	case TI_FLT32+UTO_VECTOR_FMAT + UTO_DIV: 
		{
			const std::vector<fmat<flt32_type,3,3> >& vm = *static_cast<const std::vector<fmat<flt32_type,3,3> >*>(value_ptr);
			glUniformMatrix3fv(loc, (GLsizei)vm.size(), 0, &vm[0](0, 0));
			break;
		}
	case TI_FLT32+UTO_VECTOR_FMAT + 2*UTO_DIV: 
		{
			const std::vector<fmat<flt32_type,4,4> >& vm = *static_cast<const std::vector<fmat<flt32_type,4,4> >*>(value_ptr);
			glUniformMatrix4fv(loc, (GLsizei)vm.size(), 0, &vm[0](0, 0));
			break;
		}
	default: 
		std::cerr << "uniform of type " << value_type << " not supported!" << std::endl;
		last_error = "uniform type not supported"; 
		glUseProgramObjectARB(old_p_id); 
		return false;
	}
	glUseProgramObjectARB(old_p_id); 
	return true;
}

		}
	}
}