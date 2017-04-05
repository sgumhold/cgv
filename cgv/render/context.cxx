#include "context.h"
#include <cgv/media/image/image_writer.h>
#include <cgv/base/traverser.h>
#include <cgv/render/drawable.h>

using namespace cgv::base;
using namespace cgv::media::image;

namespace cgv {
	namespace render {

		
const int nr_backgrounds = 5;
float background_colors[] = {
	0,0,0,0,
	1.0f, 0.4f, 0.5f,0,
	0.4f, 1, 0.7f,0,
	0.5f, 0.5f, 1,0,
	1,1,1,0,
};

/// construct config with default parameters
context_creation_config::context_creation_config()
{
	/// default: false
	stereo_mode = false;
	/// default: true
	double_buffer = true;
	/// default: false
	alpha_buffer = true;
	/// default: 0
	stencil_bits = 0;
	/// default: false
	forward_compatible = false;
	/// default: false in release and true in debug version
#ifdef _DEBUG
	debug = true;
#else
	debug = false;
#endif
	/// default: false
	core_profile = false;
	/// default: 0
	accumulation_bits = 0;
	/// default: -1 ... major version of maximum supported OpenGL version
	version_major = -1;
	/// default: -1 ... minor version of maximum supported OpenGL version
	version_minor = -1;
	/// default: 0
	nr_multi_samples = 0;
}

/// return "render_config"
std::string context_creation_config::get_type_name() const
{
	return "context_creation_config";
}

/// reflect the shader_path member
bool context_creation_config::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return
		srh.reflect_member("stereo_mode", stereo_mode) &&
		srh.reflect_member("double_buffer", double_buffer) &&
		srh.reflect_member("alpha_buffer", alpha_buffer) &&
		srh.reflect_member("stencil_bits", stencil_bits) &&
		srh.reflect_member("forward_compatible", forward_compatible) &&
		srh.reflect_member("accumulation_bits", accumulation_bits) &&
		srh.reflect_member("version_major", version_major) &&
		srh.reflect_member("version_minor", version_minor) &&
		srh.reflect_member("nr_multi_samples", nr_multi_samples);
}

/// construct config with default parameters
render_config::render_config()
{
	/// default: -1 ... no fullscreen
	fullscreen_monitor = -1;
	/// default: 640
	window_width = 640;
	/// default: 480
	window_height = 480;
	/// default: false
	abort_on_error = false;;
	/// default: true (only in case a gui_driver, which supports this, is loaded)
	dialog_on_error = true;
	/// default: true
	show_error_on_console = true;
}

/// return "render_config"
std::string render_config::get_type_name() const
{
	return "render_config";
}

/// reflect the shader_path member
bool render_config::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return
		context_creation_config::self_reflect(srh) &&
		srh.reflect_member("stereo_mode", stereo_mode) &&
		srh.reflect_member("double_buffer", double_buffer) &&
		srh.reflect_member("alpha_buffer", alpha_buffer) &&
		srh.reflect_member("stencil_bits", stencil_bits) &&
		srh.reflect_member("forward_compatible", forward_compatible) &&
		srh.reflect_member("accumulation_bits", accumulation_bits) &&
		srh.reflect_member("version_major", version_major) &&
		srh.reflect_member("version_minor", version_minor) &&
		srh.reflect_member("nr_multi_samples", nr_multi_samples) &&
		srh.reflect_member("fullscreen_monitor", fullscreen_monitor) &&
		srh.reflect_member("window_width", window_width) &&
		srh.reflect_member("window_height", window_height) &&
		srh.reflect_member("abort_on_error", abort_on_error) &&
		srh.reflect_member("dialog_on_error", dialog_on_error) &&
		srh.reflect_member("show_error_on_console", show_error_on_console);
}

/// return a pointer to the current shader configuration
render_config_ptr get_render_config()
{
	static render_config_ptr rcp = new render_config();
	return rcp;
}

context::context()
{
	x_offset = 10;
	y_offset = 20;
	tab_size = 5;
	cursor_x = x_offset;
	cursor_y = y_offset;
	nr_identations = 0;
	at_line_begin = true;

	default_render_flags = RenderPassFlags(RPF_DEFAULT);
	current_background = 0;
	bg_r = bg_g = bg_b = bg_a = 0.0f;
	bg_accum_r = bg_accum_g = bg_accum_b = bg_accum_a = 0.0f;
	bg_d = 1.0f;
	bg_s = 0;
	current_font_size = 14;
	
	phong_shading = true;

	do_screen_shot = false;
}

/// error handling
void context::error(const std::string& message, const render_component* rc) const
{
	if (rc)
		rc->last_error = message;
	if (get_render_config()->show_error_on_console)
		std::cerr << message << std::endl;
	if (get_render_config()->abort_on_error)
		abort();
}


/// virtual destructor
context::~context()
{

}

void context::init_render_pass()
{
}

///
group* context::get_group_interface()
{
	return dynamic_cast<group*>(this);
}

/// 
void context::draw_textual_info()
{
}

///
void context::perform_screen_shot()
{
}

///
void context::finish_render_pass()
{
}


/// helper method to integrate a new child
void context::configure_new_child(base_ptr child)
{
	if (is_created()) {
		make_current();

		single_method_action<cgv::render::drawable,void,cgv::render::context*> sma(this, &drawable::set_context);
		traverser(sma, "nc").traverse(child);

		single_method_action<cgv::render::drawable,bool,cgv::render::context&> sma1(*this, &drawable::init);
		traverser(sma1, "nc").traverse(child);

		post_redraw();
	}
}

/// set a user defined background color
void context::set_bg_color(float r, float g, float b, float a)
{
	bg_r = r;
	bg_g = g;
	bg_b = b;
	bg_a = a;
	if (!in_render_process())
		post_redraw();
}

/// set a user defined background alpha value
void context::set_bg_alpha(float a)
{
	bg_a = a;
	if (!in_render_process())
		post_redraw();
}

/// set a user defined background color
void context::set_bg_accum_color(float r, float g, float b, float a)
{
	bg_accum_r = r;
	bg_accum_g = g;
	bg_accum_b = b;
	bg_accum_a = a;
	if (!in_render_process())
		post_redraw();
}

/// set a user defined background alpha value
void context::set_bg_accum_alpha(float a)
{
	bg_accum_a = a;
	if (!in_render_process())
		post_redraw();
}

/// set a user defined background depth
void context::set_bg_depth(float d)
{
	bg_d = d;
	if (!in_render_process())
		post_redraw();
}

/// set a user defined background color
void context::set_bg_stencil(int s)
{
	bg_s = s;
	if (!in_render_process())
		post_redraw();
}

/// copy the current back ground rgba color into the given float array
void context::put_bg_color(float* rgba) const
{
	rgba[0] = bg_r;
	rgba[1] = bg_g;
	rgba[2] = bg_b;
	rgba[3] = bg_a;
}

/// return the current alpha value for clearing the background
float context::get_bg_alpha() const
{
	return bg_a;
}

void context::put_bg_accum_color(float* rgba) const
{
	rgba[0] = bg_accum_r;
	rgba[1] = bg_accum_g;
	rgba[2] = bg_accum_b;
	rgba[3] = bg_accum_a;
}

float context::get_bg_accum_alpha() const
{
	return bg_accum_a;
}

/// return the current depth value for clearing the background
float context::get_bg_depth() const
{
	return bg_d;
}

/// return the current stencil value for clearing the background
int context::get_bg_stencil() const
{
	return bg_s;
}


/// set a user defined background color
void context::set_bg_clr_idx(unsigned int idx)
{
	current_background = idx;
	if (idx == -1)
		current_background = nr_backgrounds-1;
	else if (current_background >= nr_backgrounds)
		current_background = 0;
	set_bg_color(background_colors[4*current_background],background_colors[4*current_background+1],background_colors[4*current_background+2],background_colors[4*current_background+3]);
}

/// return the current index of the background color
unsigned int context::get_bg_clr_idx() const
{
	return current_background;
}

/// enable phong shading with the help of a shader (enabled by default)
void context::enable_phong_shading()
{
	phong_shading = true;
}

void context::disable_phong_shading()
{
	phong_shading = false;
}

/// return the current render pass
RenderPass context::get_render_pass() const
{
	if (render_pass_stack.empty())
		return RP_NONE;
	return render_pass_stack.top().pass;
}
/// return the current render pass flags
RenderPassFlags context::get_render_pass_flags() const
{
	if (render_pass_stack.empty())
		return RPF_NONE;
	return render_pass_stack.top().flags;
}

/// return the default render pass flags
RenderPassFlags context::get_default_render_pass_flags() const
{
	return default_render_flags;
}
/// return the default render pass flags
void context::set_default_render_pass_flags(RenderPassFlags rpf)
{
	default_render_flags = rpf;
	if (!in_render_process())
		post_redraw();
}

/// return the current render pass user data
void* context::get_render_pass_user_data() const
{
	return render_pass_stack.top().user_data;
}


/// perform the given render task
void context::render_pass(RenderPass rp, RenderPassFlags rpf, void* user_data)
{
	render_info ri;
	ri.pass  = rp;
	ri.flags = rpf;
	ri.user_data = user_data;

	render_pass_stack.push(ri);

	init_render_pass();

	group* grp = get_group_interface();
	if (grp && (rpf&RPF_DRAWABLES_DRAW)) {
		matched_method_action<drawable,void,void,context&> 
			mma(*this, &drawable::draw, &drawable::finish_draw, true, true);
		traverser(mma).traverse(group_ptr(grp));
	}
	if (rpf&RPF_DRAW_TEXTUAL_INFO)
		draw_textual_info();
	if (grp && (rpf&RPF_DRAWABLES_FINISH_FRAME)) {
		single_method_action<drawable,void,context&> 
			sma(*this, &drawable::finish_frame, true, true);
		traverser(sma).traverse(group_ptr(grp));
	}
	if (grp && (rpf&RPF_DRAWABLES_AFTER_FINISH)) {
		single_method_action<drawable,void,context&> 
			sma(*this, &drawable::after_finish, true, true);
		traverser(sma).traverse(group_ptr(grp));
	}
	if ((rpf&RPF_HANDLE_SCREEN_SHOT) && do_screen_shot) {
		perform_screen_shot();
		do_screen_shot = false;
	}

	finish_render_pass();

	render_pass_stack.pop();
}

/// callback method for processing of text from the output stream
void context::process_text(const std::string& text)
{
	push_pixel_coords();
	unsigned int i, j = 0;
	for (i = 0; i<text.size(); ++i) {
		int n = i-j;
		switch (text[i]) {
		case '\a' :
			draw_text(text.substr(j,n));
			++nr_identations;
			if (at_line_begin)
				cursor_x += (int)(tab_size*current_font_size);
			j = i+1;
			break;
		case '\b' :
			draw_text(text.substr(j,n));
			if (nr_identations > 0) {
				--nr_identations;
				if (at_line_begin)
					cursor_x -= (int)(tab_size*current_font_size);
			}
			j = i+1;
			break;
		case '\t' :
			draw_text(text.substr(j,n));
			cursor_x = (((cursor_x-x_offset)/(int)(tab_size*current_font_size))+1)*(int)(tab_size*current_font_size)+x_offset;
			at_line_begin = false;
			j = i+1;
			break;
		case '\n' :
			draw_text(text.substr(j,n));
			cursor_x = x_offset+(int)(nr_identations*tab_size*current_font_size);
			cursor_y += (int)(1.2f*current_font_size);
			at_line_begin = true;
			j = i+1;
			break;
		default:
			at_line_begin = false;
		}
	}	
	draw_text(text.substr(j,i-j));
	pop_pixel_coords();
}

/// draw some text at cursor position and update cursor position
void context::draw_text(const std::string& text)
{
}


/// returns an output stream whose output is printed at the current cursor location
std::ostream& context::output_stream()
{
	return out_stream;
}

/// return the size in pixels of the currently enabled font face
float context::get_current_font_size() const
{
	return current_font_size;
}

/// return the currently enabled font face
cgv::media::font::font_face_ptr context::get_current_font_face() const
{
	return current_font_face;
}


/// write the content of a buffer to an image file
bool context::write_frame_buffer_to_image(const std::string& file_name, data::ComponentFormat cf,
														FrameBufferType buffer_type, unsigned int x, unsigned int y, int w, int h,
														float depth_offset, float depth_scale)
{
	data::data_view dv;
	if (cf == CF_D) {
		if (read_frame_buffer(dv, x, y, buffer_type, type::info::TI_FLT32, cf, w, h)) {
			data::data_format df("uint8[L]");
			df.set_width(dv.get_format()->get_width());
			df.set_height(dv.get_format()->get_height());
			data::data_view dv1(&df);
			unsigned int n = df.get_width()*df.get_height();
			const float* src = dv.get_ptr<float>();
			unsigned char* dst = dv1.get_ptr<unsigned char>();
			for (unsigned int i=0; i<n; ++i, ++dst, ++src)
				*dst = (unsigned char)((*src - depth_offset)*depth_scale*255);
			image_writer w(file_name);
			if (w.write_image(dv1)) {
				return true;
			}
		}
	}
	else if (read_frame_buffer(dv, x, y, buffer_type, type::info::TI_UINT8, cf, w, h)) {
		if (cf == CF_S) {
			const_cast<data::data_format*>(dv.get_format())->set_component_names("L");
			unsigned int n = dv.get_format()->get_width()*dv.get_format()->get_height();
			unsigned char* dst = dv.get_ptr<unsigned char>();
			unsigned char s = (int)depth_scale;
			for (unsigned int i=0; i<n; ++i, ++dst)
				*dst *= s;
		}
		image_writer w(file_name);
		if (w.write_image(dv)) {
			return true;
		}
	}
	return false;
}

std::string to_string(TextureWrap wrap)
{
	const char* wrap_str[] = {
		"repeat", "clamp", "clamp_to_edge", "clamp_to_border", "mirror_clamp",
		"mirror_clamp_to_edge", "mirror_clamp_to_border"
	};
	return wrap_str[wrap];
}


/// convert texture type to string
std::string to_string(TextureType tt)
{
	const char* tt_str[] = {
		"undefined", "Texture1D", "Texture2D", "Texture3D", "CubeTexture"
	};
	return tt_str[tt];
}

/// convert texture cube side to string
std::string to_string(TextureCubeSides tcs)
{
	const char* tcs_str[] = {
		"x+", "x-", "y+", "y-", "z+", "z-"
	};
	return tcs_str[tcs];
}

/// convert primitive type to string
std::string to_string(PrimitiveType pt)
{
	const char* pt_str[] = {
		"undef", "points", "lines", "lines_adjacency", "line_strip", "line_strip_adjacency", "line_loop", 
		"triangles", "triangles_adjacency", "triangle_strip", "triangle_strip_adjacency", "triangle_fan", 
		"quads", "quad_strip", "polygon"
	};
	return pt_str[pt];
}


std::string to_string(TextureFilter filter_type)
{
	const char* filter_str[] = {
		"nearest", 
		"linear", 
		"nearest_mipmap_nearest", 
		"linear_mipmap_nearest", 
		"nearest_mipmap_linear",
		"linear_mipmap_linear",
		"anisotrop"
	};
	return filter_str[filter_type];
}

// declare some colors by name
float black[4]     = { 0, 0, 0, 1 };
float white[4]     = { 1, 1, 1, 1 };
float gray[4]      = { 0.25f, 0.25f, 0.25f, 1 };
float green[4]     = { 0, 1, 0, 1 };
float brown[4]     = { 0.3f, 0.1f, 0, 1 };
float dark_red[4]  = { 0.4f, 0, 0, 1 };
float cyan[4]      = { 0, 1, 1, 1 };
float yellow[4]    = { 1, 1, 0, 1 };
float red[4]       = { 1, 0, 0, 1 };
float blue[4]      = { 0, 0, 1, 1 };

void normalize(double* n)
{
	double in = 1.0/sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
	n[0] *= in;
	n[1] *= in;
	n[2] *= in;
}

void compute_normal(double* n, const double* v0, const double* v1, const double* v2)
{
	n[0] = (v1[1]-v0[1])*(v2[2]-v0[2])-(v1[2]-v0[2])*(v2[1]-v0[1]);
	n[1] = (v1[2]-v0[2])*(v2[0]-v0[0])-(v1[0]-v0[0])*(v2[2]-v0[2]);
	n[2] = (v1[0]-v0[0])*(v2[1]-v0[1])-(v1[1]-v0[1])*(v2[0]-v0[0]);
	normalize(n);
}

void compute_face_normals(const double* vertices, double* normals, const int* vertex_indices, int* normal_indices, int nr_faces, int face_degree)
{
	for (int i = 0; i < nr_faces; ++i) {
		compute_normal(normals+3*i, 
					   vertices+3*vertex_indices[face_degree*i],
					   vertices+3*vertex_indices[face_degree*i+1],
					   vertices+3*vertex_indices[face_degree*i+2]);
		for (int j = 0; j<face_degree; ++j)
			normal_indices[face_degree*i+j] = i;
	}
}

/// tesselate a unit cube with extent from [-1,-1,-1] to [1,1,1] with face normals that can be flipped
void context::tesselate_unit_cube(bool flip_normals)
{
	static double V[8*3] = {
		-1,-1,+1,
		+1,-1,+1,
		-1,+1,+1,
		+1,+1,+1,
		-1,-1,-1,
		+1,-1,-1,
		-1,+1,-1,
		+1,+1,-1
	};
	static double N[6*3] = {
		-1,0,0, +1,0,0, 
		0,-1,0, 0,+1,0,
		0,0,-1, 0,0,+1
	};
	static double T[14*2] = { 
		 0,1.0/3 , 0,2.0/3 ,
		 0.25,0 , 0.25,1.0/3 ,
		 0.25,2.0/3 , 0.25,1 ,
		 0.5,0 , 0.5,1.0/3 ,
		 0.5,2.0/3 , 0.5,1 ,
		 0.75,1.0/3 , 0.75,2.0/3 ,
		 1,1.0/3 , 1,2.0/3 
	};
	static int F[6*4] = {
		0,2,6,4,
		1,5,7,3,
		0,4,5,1,
		2,3,7,6,
		4,6,7,5,
		0,1,3,2 
	};
	static int FN[6*4] = {
		0,0,0,0, 1,1,1,1,
		2,2,2,2, 3,3,3,3,
		4,4,4,4, 5,5,5,5
	};
	static int FT[6*4] = {
		3,4,1,0 ,7,10,11,8 ,
		3,2,6,7 ,4,8,9,5 ,
		12,13,11,10 ,3,7,8,4 
	};
	draw_faces(V,N,T,F,FN,FT,6,4, flip_normals);
}

/// tesselate an axis aligned box in single precision
void context::tesselate_box(const cgv::media::axis_aligned_box<double, 3>& B, bool flip_normals) const
{
	static double N[6 * 3] = {
		-1, 0, 0, +1, 0, 0,
		0, -1, 0, 0, +1, 0,
		0, 0, -1, 0, 0, +1
	};
	static int F[6 * 4] = {
		0, 2, 6, 4,
		1, 5, 7, 3,
		0, 4, 5, 1,
		2, 3, 7, 6,
		4, 6, 7, 5,
		0, 1, 3, 2
	};
	static int FN[6 * 4] = {
		0, 0, 0, 0, 1, 1, 1, 1,
		2, 2, 2, 2, 3, 3, 3, 3,
		4, 4, 4, 4, 5, 5, 5, 5
	};
	double V[8 * 3];

	for (unsigned i = 0; i < 8; ++i) {
		V[3 * i] = (i & 1) == 0 ? B.get_min_pnt()(0) : B.get_max_pnt()(0);
		V[3 * i + 1] = (i & 2) == 0 ? B.get_min_pnt()(1) : B.get_max_pnt()(1);
		V[3 * i + 2] = (i & 4) != 0 ? B.get_min_pnt()(2) : B.get_max_pnt()(2);
	}
	draw_faces(V, N, 0, F, FN, 0, 6, 4, flip_normals);
}

/// tesselate a prism 
void context::tesselate_unit_prism(bool flip_normals)
{
	static const double V[6*3] = {
		-1, -1, -1,
		 1, -1, -1,
		 0, -1,  1,
		-1,  1, -1,
		 1,  1, -1,
		 0,  1,  1
	};
	static double a = 1.0/sqrt(5.0);
	static double b = 2*a;
	static const double N[5*3] = {
		 0,-1, 0,
		 0, 1, 0,
		 0, 0,-1,
		-b, 0, a, 
		 b, 0, a
	};
	static const int FT[2*3] = { 0,1,2,	5,4,3 };
	static const int FQ[8] = { 4,1, 3,0, 5,2, 4,1};
	static const int FTN[2*3] = { 0,0,0, 1,1,1 };
	static const int FQN[8] = { 2,2, 2,2, 3,3, 4,4, };

	draw_faces(V, N, 0, FT, FTN, 0, 2, 3, flip_normals);
	draw_strip_or_fan(V, N, 0, FQ, FQN, 0, 3, 4, flip_normals);
}

/// tesselate a circular disk of radius 1
void context::tesselate_unit_disk(int resolution, bool flip_normals)
{
	std::vector<double> V; V.reserve(3*(resolution+1));
	std::vector<double> N; N.reserve(3*(resolution+1));
	std::vector<double> T; T.reserve(2*(resolution+1));

	std::vector<int> F; F.resize(resolution+1);
	int i;
	for (i = 0; i <= resolution; ++i)
		F[i] = i;

	double step = 2*M_PI/resolution;
	double phi   = 0;
	for (i = 0; i <= resolution; ++i, phi += step) {
		double cp = cos(phi); 
		double sp = sin(phi);
		N.push_back(0);
		N.push_back(0);
		N.push_back(1);
		T.push_back((double)i/resolution);
		T.push_back(1);
		V.push_back(cp);
		V.push_back(sp);
		V.push_back(0);
	}
	draw_faces(&V[0],&N[0],&T[0],&F[0],&F[0],&F[0],1,resolution+1,flip_normals);
}

/// tesselate a cone of radius 1
void context::tesselate_unit_cone(int resolution, bool flip_normals)
{
	std::vector<double> V; V.reserve(6*(resolution+1));
	std::vector<double> N; N.reserve(6*(resolution+1));
	std::vector<double> T; T.reserve(4*(resolution+1));

	std::vector<int> F; F.resize(2*resolution+2);
	int i;
	for (i = 0; i <= 2*resolution+1; ++i)
		F[i] = i;

	static double a = 1.0/sqrt(5.0);
	static double b = 2*a;
	double step = 2*M_PI/resolution;
	double phi   = 0;
	double u = 0;
	double duv = 1.0/resolution;
	for (int i = 0; i <= resolution; ++i, u += duv, phi += step) {
		double cp = cos(phi); 
		double sp = sin(phi);
		N.push_back(b*cp);
		N.push_back(b*sp);
		N.push_back(a);
		T.push_back(u);
		T.push_back(1);
		V.push_back(0);
		V.push_back(0);
		V.push_back(1);
		N.push_back(b*cp);
		N.push_back(b*sp);
		N.push_back(a);
		T.push_back(u);
		T.push_back(0);
		V.push_back(cp);
		V.push_back(sp);
		V.push_back(-1);
	}
	draw_strip_or_fan(&V[0],&N[0],&T[0],&F[0],&F[0],&F[0],resolution,4,false,flip_normals);
}

/// tesselate a cylinder of radius 1
void context::tesselate_unit_cylinder(int resolution, bool flip_normals)
{
	std::vector<double> V; V.reserve(6*(resolution+1));
	std::vector<double> N; N.reserve(6*(resolution+1));
	std::vector<double> T; T.reserve(4*(resolution+1));

	std::vector<int> F; F.resize(2*(resolution+1));
	int i;
	for (i = 0; i <= 2*resolution+1; ++i)
		F[i] = i;

	double step = 2*M_PI/resolution;
	double phi   = 0;
	double u = 0;
	double duv = 1.0/resolution;
	for (int i = 0; i <= resolution; ++i, u += duv, phi += step) {
		double cp = cos(phi); 
		double sp = sin(phi);
		N.push_back(cp);
		N.push_back(sp);
		N.push_back(0);
		T.push_back(u);
		T.push_back(1);
		V.push_back(cp);
		V.push_back(sp);
		V.push_back(1);
		N.push_back(cp);
		N.push_back(sp);
		N.push_back(0);
		T.push_back(u);
		T.push_back(0);
		V.push_back(cp);
		V.push_back(sp);
		V.push_back(-1);
	}
	draw_strip_or_fan(&V[0],&N[0],&T[0],&F[0],&F[0],&F[0],resolution,4,false,flip_normals);
}

/// tesselate a torus with major radius of one and given minor radius
void context::tesselate_unit_torus(float minor_radius, int resolution, bool flip_normals)
{
	std::vector<double> V; V.resize(6*(resolution+1));
	std::vector<double> N; N.resize(6*(resolution+1));
	std::vector<double> T; T.resize(4*(resolution+1));
	std::vector<int> F; F.resize(2*(resolution+1));
	int i;
	for (int i = 0; i <= resolution; ++i) {
		F[2*i] = 2*i;
		F[2*i+1] = 2*i+1;
	}
	double step  = 2*M_PI/resolution;
	double phi   = 0;
	double cp1   = 1, sp1 = 0;
	double u = 0;
	double duv = 1.0/resolution;
	for (i = 0; i < resolution; ++i, u += duv) {
		double cp0 = cp1, sp0 = sp1;
		phi += step;
		cp1 = cos(phi); 
		sp1 = sin(phi);
		double theta = 0;
		double v = 0;
		int kv=0, kn=0, kt=0;
		for (int j = 0; j <= resolution; ++j, theta += step, v += duv) {
			double ct = cos(theta), st = sin(theta);
			N[kn++] = ct*cp0;
			N[kn++] = ct*sp0;
			N[kn++] = st;
			T[kt++] = u;
			T[kt++] = v;
			V[kv++] = cp0+minor_radius*cp0*ct;
			V[kv++] = sp0+minor_radius*sp0*ct;
			V[kv++] = minor_radius*st;
			N[kn++] = ct*cp1;
			N[kn++] = ct*sp1;
			N[kn++] = st;
			T[kt++] = u+duv;
			T[kt++] = v;
			V[kv++] = cp1+minor_radius*cp1*ct;
			V[kv++] = sp1+minor_radius*sp1*ct;
			V[kv++] = minor_radius*st;
		}
		draw_strip_or_fan(&V[0],&N[0],&T[0],&F[0],&F[0],&F[0],resolution,4,false, flip_normals);
	}
}
/// tesselate a sphere of radius 1
void context::tesselate_unit_sphere(int resolution, bool flip_normals)
{
	std::vector<double> V; V.resize(6*(resolution+1));
	std::vector<double> N; N.resize(6*(resolution+1));
	std::vector<double> T; T.resize(4*(resolution+1));
	std::vector<int> F; F.resize(2*(resolution+1));
	int i;
	for (int i = 0; i <= resolution; ++i) {
		F[2*i] = 2*i;
		F[2*i+1] = 2*i+1;
	}
	double step = M_PI/resolution;
	double phi   = 0;
	double cp1   = 1, sp1 = 0;
	double u = 0;
	double duv = 1.0/resolution;
	for (i = 0; i < resolution; ++i, u += duv) {
		double cp0 = cp1, sp0 = sp1;
		phi += 2*step;
		cp1 = cos(phi); 
		sp1 = sin(phi);
		double theta = -0.5*M_PI;
		double v = 0;
		int kv=0, kn=0, kt=0;
		for (int j = 0; j <= resolution; ++j, theta += step, v += duv) {
			double ct = cos(theta), st = sin(theta);
			N[kn++] = ct*cp0;
			N[kn++] = ct*sp0;
			N[kn++] = st;
			T[kt++] = u;
			T[kt++] = v;
			V[kv++] = ct*cp0;
			V[kv++] = ct*sp0;
			V[kv++] = st;
			N[kn++] = ct*cp1;
			N[kn++] = ct*sp1;
			N[kn++] = st;
			T[kt++] = u+duv;
			T[kt++] = v;
			V[kv++] = ct*cp1;
			V[kv++] = ct*sp1;
			V[kv++] = st;
		}
		draw_strip_or_fan(&V[0],&N[0],&T[0],&F[0],&F[0],&F[0],resolution,4,false, flip_normals);
	}

}
/// tesselate a tetrahedron
void context::tesselate_unit_tetrahedron(bool flip_normals)
{
	static const double a = 1.0/(2*sqrt(3.0));
	static const double b = 1.0/(3*sqrt(3.0/2));
	static const double V[4*3] = {
		 -0.5, -a, -b,
		  0.5, -a, -b,
		    0,2*a, -b,
		    0,  0,2*b
	};
	static const int F[4*3] = {
		0,2,1,3,2,0,3,0,1,3,1,2
	};
	static int FN[4*3];
	static double N[4*3];
	static bool computed = false;
	if (!computed) {
		compute_face_normals(V, N, F, FN, 4, 3);
		computed = true;
	}
	draw_faces(V, N, 0, F, FN, 0, 4, 3, flip_normals);
}


/// tesselate a unit square 
void context::tesselate_unit_square(bool flip_normals)
{
	static double N[1*3] = {
		0,0,+1
	};
	static double V[4*3] = {
		-1,-1,0, +1,-1,0,
		+1,+1,0, -1,+1,0
	};
	static double T[4*2] = {
		0,0, 1,0,
		1,1, 0,1
	};
	static int FN[1*4] = {
		0,0,0,0
	};
	static int F[1*4] = {
		0,1,2,3
	};
	draw_faces(V, N, T, F, FN, F, 1, 4, flip_normals);
}


/// tesselate a octahedron
void context::tesselate_unit_octahedron(bool flip_normals)
{
	static double N[8*3] = {
		-1,-1,+1, +1,-1,+1, -1,+1,+1, +1,+1,+1,
		-1,-1,-1, +1,-1,-1, -1,+1,-1, +1,+1,-1
	};
	static double V[6*3] = {
		-1,0,0, +1,0,0,
		0,-1,0, 0,+1,0,
		0,0,-1, 0,0,+1
	};
	static int FN[8*3] = {
		0,0,0,
		1,1,1,
		2,2,2,
		3,3,3,
		4,4,4,
		5,5,5,
		6,6,6,
		7,7,7
	};
	static int F[8*3] = {
		0,2,5,
		1,5,2,
		5,3,0,
		3,5,1,
		4,2,0,
		4,1,2,
		3,4,0,
		1,4,3
	};
	draw_faces(V, N, 0, F, FN, 0, 8, 3, flip_normals);
}

/// render an icosahedron at a given center.
void tesselate_unit_dodecahedron_or_icosahedron(context& c, bool dual, bool flip_normals)
{
	static const double h = 0.4472135956;
	static const double r = 0.8944271912;
	static const double s = M_PI/2.5;
	static const double o = M_PI/5;
	static const double V[13*3] = {
		0,0,-1,
		r*sin(1*s),r*cos(1*s),-h,
		r*sin(2*s),r*cos(2*s),-h,
		r*sin(3*s),r*cos(3*s),-h,
		r*sin(4*s),r*cos(4*s),-h,
		r*sin(5*s),r*cos(5*s),-h,
		r*sin(1*s+o),r*cos(1*s+o),h,
		r*sin(2*s+o),r*cos(2*s+o),h,
		r*sin(3*s+o),r*cos(3*s+o),h,
		r*sin(4*s+o),r*cos(4*s+o),h,
		r*sin(5*s+o),r*cos(5*s+o),h,
		0,0,1,
		0,0,0
	};
	static int F[20*3] = {
		0,1,2, 0,2,3, 0,3,4, 0,4,5, 0,5,1,
		6,2,1, 2,6,7, 7,3,2, 3,7,8, 8,4,3, 4,8,9, 9,5,4, 5,9,10, 10,1,5, 6,1,10,
		11,6,10, 11,10,9, 11,9,8, 11,8,7, 11,7,6
	};
	static int DF[12*5] = {
		0,1,2,3,4,
		5,6,7,1,0,
		7,8,9,2,1,
		9,10,11,3,2,
		11,12,13,4,3,
		13,14,5,0,4,
		16,15,14,13,12,
		17,16,12,11,10,
		18,17,10,9,8,
		19,18,8,7,6,
		15,19,6,5,14,
		15,16,17,18,19
	};
	static double N[20*3];
	static int FN[20*3];
	static int DFN[12*5] = {
		0,0,0,0,0,
		2,2,2,2,2,
		3,3,3,3,3,
		4,4,4,4,4,
		5,5,5,5,5,
		1,1,1,1,1,
		10,10,10,10,10,
		9,9,9,9,9,
		8,8,8,8,8,
		7,7,7,7,7,
		6,6,6,6,6,
		11,11,11,11,11
	};

	static bool computed = false;
	if (!computed) {
		compute_face_normals(V, N, F, FN, 20, 3);
		computed = true;
	}
	if (!dual)
		c.draw_faces(V, N, 0, F, FN, 0, 20, 3, flip_normals);
	else
		c.draw_faces(N, V, 0, DF, DFN, 0, 12, 5, flip_normals);
}

/// tesselate a dodecahedron
void context::tesselate_unit_dodecahedron(bool flip_normals)
{
	tesselate_unit_dodecahedron_or_icosahedron(*this, true, flip_normals);
}
/// tesselate an icosahedron
void context::tesselate_unit_icosahedron(bool flip_normals)
{
	tesselate_unit_dodecahedron_or_icosahedron(*this, false, flip_normals);
}

void context::push_V()
{
	V_stack.push(get_V());
}

/// see push_V for an explanation
void context::pop_V()
{
	set_V(V_stack.top());
	V_stack.pop();
}
/// same as push_V but for the projection matrix - a different matrix stack is used.
void context::push_P() 
{
	P_stack.push(get_P());
}
/// see push_P for an explanation
void context::pop_P()
{
	set_P(P_stack.top());
	P_stack.pop();
}


/// set a new cursor position, which is only valid between calls of push_pixel_coords and pop_pixel_coords
void context::set_cursor(int x, int y)
{
	output_stream().flush();
	cursor_x = x;
	cursor_y = y;
	x_offset = x;
	y_offset = y;
	nr_identations = 0;
	at_line_begin = true;
}

/// sets the current text ouput position
void context::set_cursor(const cgv::math::vec<float>& pos, 
		const std::string& text, TextAlignment ta,
		int x_offset, int y_offset)
{
	vec_type p = pos;
	set_cursor(p,text,ta,x_offset,y_offset);
}

/// sets the current text ouput position
void context::set_cursor(const cgv::math::vec<double>& pos, 
		const std::string& text, TextAlignment ta,
		int x_offset, int y_offset)
{
	int x,y;
	put_cursor_coords(pos, x, y);
	if (!text.empty()) {
		float h = get_current_font_size();
		float w = get_current_font_face()->measure_text_width(text, h);
		switch (ta&3) {
		case 0 : x -= (int)(floor(w)*0.5f);break;
		case 2 : x -= (int)floor(w);break;
		default: break;
		}
		switch (ta&12) {
		case 0 : y += (int)(floor(h)*0.5f);break;
		case 4 : y += (int)floor(h);break;
		default: break;
		}
	}
	x += x_offset;
	y += y_offset;
	set_cursor(x,y);
}

/// store the current cursor location in the passed references to x and y coordinate
void context::get_cursor(int& x, int& y) const
{
	x = cursor_x;
	y = cursor_y;
}

/// return homogeneous 4x4 matrix, which transforms from world to device space
context::mat_type context::get_DPV() const
{
	return get_D()*get_P()*get_V();
}

/// compute a the location in world space of a device x/y-location. For this the device point is extended with the device z-coordinate currently stored in depth buffer.
context::vec_type context::get_point_W(int x_D, int y_D) const
{
	return get_point_W(x_D, y_D, get_z_D(x_D,y_D));
}

context::vec_type context::get_point_W(int x_D, int y_D, const mat_type& DPV) const
{
	return get_point_W(x_D, y_D, get_z_D(x_D,y_D), DPV);
}
/// compute a the location in world space of a device point.
context::vec_type context::get_point_W(int x_D, int y_D, double z_D) const
{
	return get_point_W(x_D, y_D, z_D, get_DPV());
}

/// compute a the location in world space of a device point.
context::vec_type context::get_point_W(const vec_type& p_D) const
{
	return get_point_W(p_D, get_DPV());
}

/// compute a the location in world space of a device point.
context::vec_type context::get_point_W(const vec_type& p_D, const mat_type& DPV) const
{
	vec_type x(4);
	vec_type b(4);
	b(0) = p_D(0);
	b(1) = p_D(1);
	b(2) = p_D(2);
	b(3) = 1;
	svd_solve(DPV,b,x);
	vec_type p(3);
	p(0) = x(0)/x(3);
	p(1) = x(1)/x(3);
	p(2) = x(2)/x(3);
	return p;
}

context::vec_type context::get_point_W(int x_D, int y_D, double z_D, const mat_type& MPD) const
{
	return get_point_W(vec_type(x_D+0.5f,y_D+0.5f,z_D),MPD);
}

void context::tesselate_arrow(double length, double aspect, double rel_tip_radius, double tip_aspect, int res)
{
	std::cout << "tesselate_arrow not implemented in cgv::render::context" << std::endl;
}

void context::tesselate_arrow(const cgv::math::fvec<double, 3>& start, const cgv::math::fvec<double, 3>& end, double aspect, double rel_tip_radius, double tip_aspect, int res)
{
	std::cout << "tesselate_arrow not implemented in cgv::render::context" << std::endl;
}

void context::draw_light_source(const cgv::media::illum::light_source& l, float intensity_scale, float light_scale)
{
	std::cout << "draw_light_source not implemented in cgv::render::context" << std::endl;
}

render_component::render_component()
{
	handle = 0;
	internal_format = 0;
	user_data = 0;
	ctx_ptr = 0;
}

/// return whether component has been created
bool render_component::is_created() const
{
	return handle != 0;
}


void render_component::put_id_void(void* ptr) const
{
	if (!ctx_ptr) {
		std::cerr << "no context set when render_component::put_id_void was called" << std::endl;
		return;
	}
	ctx_ptr->put_id(handle, ptr);
}

/// initialize members
texture_base::texture_base(TextureType _tt)
{
	mag_filter = TF_LINEAR;
	min_filter = TF_LINEAR_MIPMAP_LINEAR;
	wrap_s = TW_CLAMP_TO_EDGE;
	wrap_t = TW_CLAMP_TO_EDGE;
	wrap_r = TW_CLAMP_TO_EDGE;
	anisotropy = 1;
	priority = 0.5f;
	border_color[0] = 1;
	border_color[1] = 1;
	border_color[2] = 1;
	border_color[3] = 1;
	tt = _tt;
	have_mipmaps = false;
}

shader_program_base::shader_program_base()
{
	is_enabled = false;
	geometry_shader_input_type = PT_POINTS;
	geometry_shader_output_type = PT_POINTS;
	geometry_shader_output_count = 1;
}

attribute_array_binding_base::attribute_array_binding_base()
{
}

vertex_buffer_base::vertex_buffer_base()
{
	type = VBT_VERTICES;
	usage = VBU_STATIC_DRAW;
}


/// initialize members
frame_buffer_base::frame_buffer_base()
{
	width = -1;
	height = -1;
	std::fill(attached, attached+16,false);
}

std::vector<context_creation_function_type>& ref_context_creation_functions()
{
	static std::vector<context_creation_function_type> ccfs;
	return ccfs;
}

/// registration context creation functions
void register_context_factory(context_creation_function_type fp)
{
	ref_context_creation_functions().push_back(fp);
}

context_factory_registration::context_factory_registration(context_creation_function_type fp)
{
	register_context_factory(fp);
};

/** construct a context of the given size. This is primarily used to create
    a context without a window for console applications that render into a frame
    buffer object only. After usage you need to delete the context by hand. */
context* create_context(RenderAPI api, 
		unsigned int w, unsigned int h, 
		const std::string& title, bool show)
{
	std::vector<context_creation_function_type>& ccfs = ref_context_creation_functions();
	for (unsigned i=0; i<ccfs.size(); ++i) {
		context* ctx = ccfs[i](api,w,h,title,show);
		if (ctx) {
			ctx->make_current();
			return ctx;
		}
	}
	std::cerr << "could not create context for given parameters" << std::endl;
	return 0;
}


	}
}

#include <cgv/base/register.h>

struct render_config_registration
{
	render_config_registration()
	{
		cgv::base::register_object(cgv::render::get_render_config(), "register global render config");
	}
};

render_config_registration render_config_registration_instance;