#include <cgv/base/group.h>
#include "context.h"
#include <cgv/media/image/image_writer.h>
#include <cgv/math/ftransform.h>
#include <cgv/base/traverser.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>

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
	modelview_matrix_stack.push(cgv::math::identity4<double>());
	projection_matrix_stack.push(cgv::math::identity4<double>());
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
	light_source_handle = 1;

	auto_set_view_in_current_shader_program = true;
	auto_set_lights_in_current_shader_program = true;
	auto_set_material_in_current_shader_program = true;
	support_compatibility_mode = true;
	draw_in_compatibility_mode = false;

	default_light_source[0].set_local_to_eye(true);
	default_light_source[0].set_position(vec3(-0.4f, 0.3f, 0.8f));
	default_light_source[1].set_local_to_eye(true);
	default_light_source[1].set_position(vec3(0.0f, 1.0f, 0.0f));
	default_light_source_handles[0] = 0;
	default_light_source_handles[1] = 0;

	current_material_ptr = 0;
	current_material_is_textured = false;
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

/// check for current program, prepare it for rendering and return pointer to it
shader_program_base* context::get_current_program() const
{
	if (shader_program_stack.empty()) {
		error("context::get_current_program() called in core profile without current shader program");
		return 0;
	}
	shader_program_base& prog = *shader_program_stack.top();
	return &prog;
}

/// return the number of light sources
size_t context::get_nr_light_sources() const 
{
	return light_sources.size(); 
}

/// helper function to place lights 
context::vec3 context::get_light_eye_position(const cgv::media::illum::light_source& light, bool place_now) const
{
	vec3 Le = light.get_position();
	if (place_now && !light.is_local_to_eye()) {
		dvec4 hL(Le(0), Le(1), Le(2), light.get_type() == cgv::media::illum::LT_DIRECTIONAL ? 0.0f : 1.0f);
		hL = get_modelview_matrix()*hL;
		Le = (const dvec3&)hL;
		if (light.get_type() != cgv::media::illum::LT_DIRECTIONAL)
			Le /= float(hL(3));
	}
	return Le;
}

/// add a new light source, enable it if \c enable is true and place it relative to current model view transformation if \c place_now is true; return handle to light source
void* context::add_light_source(const cgv::media::illum::light_source& light, bool enabled, bool place_now)
{
	// construct new light source handle
	++light_source_handle;
	void* handle = reinterpret_cast<void*>(light_source_handle);
	// determine light source position
	vec3 Le = get_light_eye_position(light, place_now); 
	//
	int idx = -1;
	if (enabled) {
		idx = enabled_light_source_handles.size();
		enabled_light_source_handles.push_back(handle);
	}
	// store new light source in map
	light_sources[handle] = std::pair<cgv::media::illum::light_source, light_source_status>(light, { enabled, Le, idx });
	// set light sources in shader code if necessary
	if (enabled)
		on_lights_changed();
	// return handle of new light source
	return handle;
}
/// remove a light source by handle and whether it existed
bool context::remove_light_source(void* handle)
{
	// find handle in map
	auto iter = light_sources.find(handle);
	if (iter == light_sources.end())
		return false;
	// check if light source was enabled
	if (iter->second.second.enabled) {
		// then remove from list of enabled light sources
		enabled_light_source_handles.erase(enabled_light_source_handles.begin() + iter->second.second.light_source_index);
		// and correct indices of moved light sources
		for (int i = iter->second.second.light_source_index; i < (int)enabled_light_source_handles.size(); ++i)
			light_sources[enabled_light_source_handles[i]].second.light_source_index = i;
		on_lights_changed();
	}
	// remove from map
	light_sources.erase(iter);
	return true;
}
/// read access to light source
const cgv::media::illum::light_source& context::get_light_source(void* handle) const
{
	const auto iter = light_sources.find(handle);
	return iter->second.first;
}

/// read access to light source status
const context::light_source_status& context::get_light_source_status(void* handle) const
{
	const auto iter = light_sources.find(handle);
	return iter->second.second;
}


/// set light source newly
void context::set_light_source(void* handle, const cgv::media::illum::light_source& light, bool place_now)
{
	auto iter = light_sources.find(handle);
	iter->second.first = light;
	if (iter->second.second.enabled)
		on_lights_changed();
}

/// set the shader program view matrices to the currently enabled view matrices
void context::set_current_view(shader_program& prog, bool modelview_deps, bool projection_deps) const
{
	if (modelview_deps) {
		cgv::math::fmat<float, 4, 4> V(modelview_matrix_stack.top());
		prog.set_uniform(*this, "modelview_matrix", V);
		cgv::math::fmat<float, 3, 3> NM;
		NM(0, 0) = V(0, 0);
		NM(0, 1) = V(0, 1);
		NM(0, 2) = V(0, 2);
		NM(1, 0) = V(1, 0);
		NM(1, 1) = V(1, 1);
		NM(1, 2) = V(1, 2);
		NM(2, 0) = V(2, 0);
		NM(2, 1) = V(2, 1);
		NM(2, 2) = V(2, 2);
		NM.transpose();
		NM = inv(NM);
		prog.set_uniform(*this, "normal_matrix", NM);
	}
	if (projection_deps) {
		cgv::math::fmat<float, 4, 4> P(projection_matrix_stack.top());
		prog.set_uniform(*this, "projection_matrix", P);
	}
}

/// set the shader program material to the currently enabled material
void context::set_current_material(shader_program& prog) const
{
	if (!current_material_ptr)
		return;

	prog.set_material_uniform(*this, "material", *current_material_ptr);
	if (current_material_is_textured) {

	}
}

/// set the shader program lights to the currently enabled lights
void context::set_current_lights(shader_program& prog) const
{
	size_t nr_lights = get_nr_enabled_light_sources();
	for (size_t i = 0; i < nr_lights; ++i) {
		std::string prefix = std::string("light_sources[") + cgv::utils::to_string(i) + "]";
		void* light_source_handle = get_enabled_light_source_handle(i);
		const auto iter = light_sources.find(light_source_handle);
		if (prog.set_light_uniform(*this, prefix, iter->second.first))
			prog.set_uniform(*this, prefix + ".position", iter->second.second.eye_position);
	}
	prog.set_uniform(*this, "nr_light_sources", (int)nr_lights);
}

void context::on_lights_changed()
{
	if (!auto_set_lights_in_current_shader_program)
		return;

	if (shader_program_stack.empty())
		return;

	cgv::render::shader_program& prog = *static_cast<cgv::render::shader_program*>(shader_program_stack.top());
	if (!prog.does_use_lights())
		return;

	set_current_lights(prog);
}

/// place the given light source relative to current model viel transformation
void context::place_light_source(void* handle)
{
	auto iter = light_sources.find(handle);
	// determine light source position
	iter->second.second.eye_position = get_light_eye_position(iter->second.first, true);
	if (iter->second.second.enabled)
		on_lights_changed();
}

/// return maximum number of light sources, that can be enabled in parallel 
unsigned context::get_max_nr_enabled_light_sources() const
{
	return 8;
}

/// return the number of light sources
size_t context::get_nr_enabled_light_sources() const
{
	return enabled_light_source_handles.size();
}

/// access to handle of i-th light source
void* context::get_enabled_light_source_handle(size_t i) const
{
	return enabled_light_source_handles[i];
}

/// enable a given light source and return whether there existed a light source with given handle
bool context::enable_light_source(void* handle)
{
	auto iter = light_sources.find(handle);
	if (iter == light_sources.end())
		return false;
	if (iter->second.second.enabled)
		return true;
	iter->second.second.enabled = true;
	iter->second.second.light_source_index = enabled_light_source_handles.size();
	enabled_light_source_handles.push_back(handle);
	on_lights_changed();
	return true;
}

/// disable a given light source and return whether there existed a light source with given handle
bool context::disable_light_source(void* handle)
{
	auto iter = light_sources.find(handle);
	if (iter == light_sources.end())
		return false;
	if (!iter->second.second.enabled)
		return true;
	iter->second.second.enabled = false;
	enabled_light_source_handles.erase(enabled_light_source_handles.begin()+iter->second.second.light_source_index);
	for (int i= iter->second.second.light_source_index; i < (int)enabled_light_source_handles.size(); ++i)
		light_sources[enabled_light_source_handles[i]].second.light_source_index = i;
	iter->second.second.light_source_index = -1;
	on_lights_changed();
	return true;

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
	// ensure that default light sources are created
	if (default_light_source_handles[0] == 0) {
		for (unsigned i=0; i<nr_default_light_sources; ++i)
			default_light_source_handles[i] = add_light_source(default_light_source[i], false, false);
	}
	render_info ri;
	ri.pass  = rp;
	ri.flags = rpf;
	ri.user_data = user_data;

	render_pass_stack.push(ri);

	init_render_pass();

	if (get_render_pass_flags()&RPF_SET_LIGHTS) {
		for (unsigned i = 0; i < nr_default_light_sources; ++i)
			place_light_source(default_light_source_handles[i]);
	}

	group* grp = dynamic_cast<group*>(this);
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

void compute_face_normals(const float* vertices, float* normals, const int* vertex_indices, int* normal_indices, int nr_faces, int face_degree)
{
	for (int i = 0; i < nr_faces; ++i) {
		context::vec3& normal = reinterpret_cast<context::vec3&>(normals[3 * i]);
		normal.zeros();
		context::vec3 reference_pnt = *reinterpret_cast<const context::vec3*>(vertices + 3 * vertex_indices[face_degree*i + face_degree - 1]);
		context::vec3 last_difference;
		last_difference.zeros();
		for (int j = 0; j < face_degree; ++j) {
			context::vec3 new_difference = *reinterpret_cast<const context::vec3*>(vertices + 3 * vertex_indices[face_degree*i + j]) - reference_pnt;
			normal += cross(last_difference, new_difference);
			last_difference = new_difference;
		}
		normal.normalize();
		for (int j = 0; j<face_degree; ++j)
			normal_indices[face_degree*i+j] = i;
	}
}

/// tesselate a unit cube with extent from [-1,-1,-1] to [1,1,1] with face normals that can be flipped
void context::tesselate_unit_cube(bool flip_normals, bool edges)
{
	static float V[8*3] = {
		-1,-1,+1,
		+1,-1,+1,
		-1,+1,+1,
		+1,+1,+1,
		-1,-1,-1,
		+1,-1,-1,
		-1,+1,-1,
		+1,+1,-1
	};
	static float N[6*3] = {
		-1,0,0, +1,0,0, 
		0,-1,0, 0,+1,0,
		0,0,-1, 0,0,+1
	};
	static const float ot = float(1.0 / 3);
	static const float tt = float(2.0 / 3);
	static float T[14*2] = {
		 0,ot , 0,tt ,
		 0.25f,0 , 0.25f,ot ,
		 0.25f,tt , 0.25f,1 ,
		 0.5f,0 , 0.5f,ot ,
		 0.5f,tt , 0.5f,1 ,
		 0.75f,ot , 0.75f,tt ,
		 1,ot , 1,tt 
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
	if (edges)
		draw_edges_of_faces(V, N, T, F, FN, FT, 6, 4, flip_normals);
	else
		draw_faces(V,N,T,F,FN,FT,6,4, flip_normals);
}

/// tesselate an axis aligned box in single precision
void context::tesselate_box(const cgv::media::axis_aligned_box<double, 3>& B, bool flip_normals, bool edges) const
{
	static float N[6 * 3] = {
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
	float V[8 * 3];

	for (unsigned i = 0; i < 8; ++i) {
		V[3 * i]	 = float((i & 1) == 0 ? B.get_min_pnt()(0) : B.get_max_pnt()(0));
		V[3 * i + 1] = float((i & 2) == 0 ? B.get_min_pnt()(1) : B.get_max_pnt()(1));
		V[3 * i + 2] = float((i & 4) != 0 ? B.get_min_pnt()(2) : B.get_max_pnt()(2));
	}
	if (edges)
		draw_edges_of_faces(V, N, 0, F, FN, 0, 6, 4, flip_normals);
	else
		draw_faces(V, N, 0, F, FN, 0, 6, 4, flip_normals);
}

/// tesselate a prism 
void context::tesselate_unit_prism(bool flip_normals, bool edges)
{
	static const float V[6*3] = {
		-1, -1, -1,
		 1, -1, -1,
		 0, -1,  1,
		-1,  1, -1,
		 1,  1, -1,
		 0,  1,  1
	};
	static float a = 1.0f/sqrt(5.0f);
	static float b = 2*a;
	static const float N[5*3] = {
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

	if (edges) {
		draw_edges_of_faces(V, N, 0, FT, FTN, 0, 2, 3, flip_normals);
		draw_edges_of_strip_or_fan(V, N, 0, FQ, FQN, 0, 3, 4, flip_normals);
	}
	else {
		draw_faces(V, N, 0, FT, FTN, 0, 2, 3, flip_normals);
		draw_strip_or_fan(V, N, 0, FQ, FQN, 0, 3, 4, flip_normals);
	}
}

/// tesselate a circular disk of radius 1
void context::tesselate_unit_disk(int resolution, bool flip_normals, bool edges)
{
	std::vector<float> V; V.reserve(3*(resolution+1));
	std::vector<float> N; N.reserve(3*(resolution+1));
	std::vector<float> T; T.reserve(2*(resolution+1));

	std::vector<int> F; F.resize(resolution+1);
	int i;
	for (i = 0; i <= resolution; ++i)
		F[i] = i;

	float step = float(2*M_PI/resolution);
	float phi   = 0;
	for (i = 0; i <= resolution; ++i, phi += step) {
		float cp = cos(phi);
		float sp = sin(phi);
		N.push_back(0);
		N.push_back(0);
		N.push_back(1);
		T.push_back((float)i/resolution);
		T.push_back(1);
		V.push_back(cp);
		V.push_back(sp);
		V.push_back(0);
	}
	if (edges)
		draw_edges_of_faces(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], 1, resolution + 1, flip_normals);
	else
		draw_faces(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], 1, resolution + 1, flip_normals);
}

/// tesselate a cone of radius 1
void context::tesselate_unit_cone(int resolution, bool flip_normals, bool edges)
{
	std::vector<float> V; V.reserve(6*(resolution+1));
	std::vector<float> N; N.reserve(6*(resolution+1));
	std::vector<float> T; T.reserve(4*(resolution+1));

	std::vector<int> F; F.resize(2*resolution+2);
	int i;
	for (i = 0; i <= 2*resolution+1; ++i)
		F[i] = i;

	static float a = 1.0f/sqrt(5.0f);
	static float b = 2*a;
	float step = float(2*M_PI/resolution);
	float phi   = 0;
	float u = 0;
	float duv = float(1.0/resolution);
	for (int i = 0; i <= resolution; ++i, u += duv, phi += step) {
		float cp = cos(phi); 
		float sp = sin(phi);
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
	if (edges)
		draw_edges_of_strip_or_fan(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], resolution, 4, false, flip_normals);
	else
		draw_strip_or_fan(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], resolution, 4, false, flip_normals);
}

/// tesselate a cylinder of radius 1
void context::tesselate_unit_cylinder(int resolution, bool flip_normals, bool edges)
{
	std::vector<float> V; V.reserve(6*(resolution+1));
	std::vector<float> N; N.reserve(6*(resolution+1));
	std::vector<float> T; T.reserve(4*(resolution+1));

	std::vector<int> F; F.resize(2*(resolution+1));
	int i;
	for (i = 0; i <= 2*resolution+1; ++i)
		F[i] = i;

	float step = float(2*M_PI/resolution);
	float phi   = 0;
	float u = 0;
	float duv = float(1.0/resolution);
	for (int i = 0; i <= resolution; ++i, u += duv, phi += step) {
		float cp = cos(phi);
		float sp = sin(phi);
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
	if (edges)
		draw_edges_of_strip_or_fan(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], resolution, 4, false, flip_normals);
	else
		draw_strip_or_fan(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], resolution, 4, false, flip_normals);
}

/// tesselate a torus with major radius of one and given minor radius
void context::tesselate_unit_torus(float minor_radius, int resolution, bool flip_normals, bool edges)
{
	std::vector<float> V; V.resize(6*(resolution+1));
	std::vector<float> N; N.resize(6*(resolution+1));
	std::vector<float> T; T.resize(4*(resolution+1));
	std::vector<int> F; F.resize(2*(resolution+1));
	int i;
	for (int i = 0; i <= resolution; ++i) {
		F[2*i] = 2*i;
		F[2*i+1] = 2*i+1;
	}
	float step  = float(2*M_PI/resolution);
	float phi   = 0;
	float cp1   = 1, sp1 = 0;
	float u = 0;
	float duv = float(1.0/resolution);
	for (i = 0; i < resolution; ++i, u += duv) {
		float cp0 = cp1, sp0 = sp1;
		phi += step;
		cp1 = cos(phi); 
		sp1 = sin(phi);
		float theta = 0;
		float v = 0;
		int kv=0, kn=0, kt=0;
		for (int j = 0; j <= resolution; ++j, theta += step, v += duv) {
			float ct = cos(theta), st = sin(theta);
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
		if (edges)
			draw_edges_of_strip_or_fan(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], resolution, 4, false, flip_normals);
		else
			draw_strip_or_fan(&V[0],&N[0],&T[0],&F[0],&F[0],&F[0],resolution,4,false, flip_normals);
	}
}
/// tesselate a sphere of radius 1
void context::tesselate_unit_sphere(int resolution, bool flip_normals, bool edges)
{
	std::vector<float> V; V.resize(6*(resolution+1));
	std::vector<float> N; N.resize(6*(resolution+1));
	std::vector<float> T; T.resize(4*(resolution+1));
	std::vector<int> F; F.resize(2*(resolution+1));
	int i;
	for (int i = 0; i <= resolution; ++i) {
		F[2*i] = 2*i;
		F[2*i+1] = 2*i+1;
	}
	float step = float(M_PI/resolution);
	float phi   = 0;
	float cp1   = 1, sp1 = 0;
	float u = 0;
	float duv = float(1.0/resolution);
	for (i = 0; i < resolution; ++i, u += duv) {
		float cp0 = cp1, sp0 = sp1;
		phi += 2*step;
		cp1 = cos(phi); 
		sp1 = sin(phi);
		float theta = float(-0.5*M_PI);
		float v = 0;
		int kv=0, kn=0, kt=0;
		for (int j = 0; j <= resolution; ++j, theta += step, v += duv) {
			float ct = cos(theta), st = sin(theta);
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
		if (edges)
			draw_edges_of_strip_or_fan(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], resolution, 4, false, flip_normals);
		else
			draw_strip_or_fan(&V[0], &N[0], &T[0], &F[0], &F[0], &F[0], resolution, 4, false, flip_normals);
	}

}
/// tesselate a tetrahedron
void context::tesselate_unit_tetrahedron(bool flip_normals, bool edges)
{
	static const float a = float(1.0/(2*sqrt(3.0)));
	static const float b = float(1.0/(3*sqrt(3.0/2)));
	static const float V[4*3] = {
		 -0.5, -a, -b,
		  0.5, -a, -b,
		    0,2*a, -b,
		    0,  0,2*b
	};
	static const int F[4*3] = {
		0,2,1,3,2,0,3,0,1,3,1,2
	};
	static int FN[4*3];
	static float N[4*3];
	static bool computed = false;
	if (!computed) {
		compute_face_normals(V, N, F, FN, 4, 3);
		computed = true;
	}
	if (edges)
		draw_edges_of_faces(V, N, 0, F, FN, 0, 4, 3, flip_normals);
	else
		draw_faces(V, N, 0, F, FN, 0, 4, 3, flip_normals);
}


/// tesselate a unit square 
void context::tesselate_unit_square(bool flip_normals, bool edges)
{
	static float N[1*3] = {
		0,0,+1
	};
	static float V[4*3] = {
		-1,-1,0, +1,-1,0,
		+1,+1,0, -1,+1,0
	};
	static float T[4*2] = {
		0,0, 1,0,
		1,1, 0,1
	};
	static int FN[1*4] = {
		0,0,0,0
	};
	static int F[1*4] = {
		0,1,2,3
	};
	if (edges)
		draw_edges_of_faces(V, N, T, F, FN, F, 1, 4, flip_normals);
	else
		draw_faces(V, N, T, F, FN, F, 1, 4, flip_normals);
}


/// tesselate a octahedron
void context::tesselate_unit_octahedron(bool flip_normals, bool edges)
{
	static float N[8*3] = {
		-1,-1,+1, +1,-1,+1, -1,+1,+1, +1,+1,+1,
		-1,-1,-1, +1,-1,-1, -1,+1,-1, +1,+1,-1
	};
	static float V[6*3] = {
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
	if (edges)
		draw_edges_of_faces(V, N, 0, F, FN, 0, 8, 3, flip_normals);
	else
		draw_faces(V, N, 0, F, FN, 0, 8, 3, flip_normals);
}

/// render an icosahedron at a given center.
void tesselate_unit_dodecahedron_or_icosahedron(context& c, bool dual, bool flip_normals, bool edges)
{
	static const float h = 0.4472135956f;
	static const float r = 0.8944271912f;
	static const float s = float(M_PI/2.5);
	static const float o = float(M_PI/5);
	static const float V[13*3] = {
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
	static float N[20*3];
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
	if (!dual) {
		if (edges)
			c.draw_edges_of_faces(V, N, 0, F, FN, 0, 20, 3, flip_normals);
		else
			c.draw_faces(V, N, 0, F, FN, 0, 20, 3, flip_normals);
	}
	else {
		if (edges)
			c.draw_edges_of_faces(N, V, 0, DF, DFN, 0, 12, 5, flip_normals);
		else
			c.draw_faces(N, V, 0, DF, DFN, 0, 12, 5, flip_normals);
	}
}

/// tesselate a dodecahedron
void context::tesselate_unit_dodecahedron(bool flip_normals, bool edges)
{
	tesselate_unit_dodecahedron_or_icosahedron(*this, true, flip_normals, edges);
}
/// tesselate an icosahedron
void context::tesselate_unit_icosahedron(bool flip_normals, bool edges)
{
	tesselate_unit_dodecahedron_or_icosahedron(*this, false, flip_normals, edges);
}

/// set the current material 
void context::set_material(const cgv::media::illum::surface_material& material)
{
	current_material_ptr = &material;
	current_material_is_textured = false;

	if (!auto_set_material_in_current_shader_program)
		return;

	if (shader_program_stack.empty())
		return;

	cgv::render::shader_program& prog = *static_cast<cgv::render::shader_program*>(shader_program_stack.top());
	if (!prog.does_use_material())
		return;

	prog.set_material_uniform(*this, "material", material);
}

void context::push_modelview_matrix()
{
	modelview_matrix_stack.push(get_modelview_matrix());
}

/// multiply given matrix from right to current modelview matrix
void context::mul_modelview_matrix(const dmat4& V)
{
	set_modelview_matrix(get_modelview_matrix()*V);
}

/// see push_V for an explanation
void context::pop_modelview_matrix()
{
	if (modelview_matrix_stack.size() == 1) {
		std::cerr << "attempt to completely empty modelview stack avoided." << std::endl;
		return;
	}
	modelview_matrix_stack.pop();
	set_modelview_matrix(modelview_matrix_stack.top());
}
/// same as push_V but for the projection matrix - a different matrix stack is used.
void context::push_projection_matrix() 
{
	projection_matrix_stack.push(get_projection_matrix());
}
/// see push_P for an explanation
void context::pop_projection_matrix()
{
	if (projection_matrix_stack.size() == 1) {
		std::cerr << "attempt to completely empty projection stack avoided." << std::endl;
		return;
	}
	projection_matrix_stack.pop();
	set_projection_matrix(projection_matrix_stack.top());
}
/// multiply given matrix from right to current projection matrix
void context::mul_projection_matrix(const dmat4& P)
{
	set_projection_matrix(get_projection_matrix()*P);
}

void context::set_modelview_matrix(const dmat4& V)
{
	// set new modelview matrix on matrix stack
	modelview_matrix_stack.top() = V;

	// update in current shader
	if (!auto_set_view_in_current_shader_program)
		return;

	if (shader_program_stack.empty())
		return;
	cgv::render::shader_program& prog = *static_cast<cgv::render::shader_program*>(shader_program_stack.top());
	if (!prog.does_use_view())
		return;
	set_current_view(prog, true, false);
}

void context::set_projection_matrix(const dmat4& P)
{
	// set new projection matrix on matrix stack
	projection_matrix_stack.top() = P;

	// update in current shader
	if (!auto_set_view_in_current_shader_program)
		return;

	if (shader_program_stack.empty())
		return;
	cgv::render::shader_program& prog = *static_cast<cgv::render::shader_program*>(shader_program_stack.top());
	if (!prog.does_use_view())
		return;
	set_current_view(prog, false, true);
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

/// transform point p into cursor coordinates and put x and y coordinates into the passed variables
void context::put_cursor_coords(const vec_type& p, int& x, int& y) const
{
	dvec4 p4(0, 0, 0, 1);
	for (unsigned int c = 0; c < p.size(); ++c)
		p4(c) = p(c);
	p4 = get_modelview_projection_device_matrix()*p4;
	x = (int)(p4(0) / p4(3));
	y = (int)(p4(1) / p4(3));
}

/// sets the current text ouput position
void context::set_cursor(const vec_type& pos, 
		const std::string& text, TextAlignment ta,
		int x_offset, int y_offset)
{
	int x,y;
	put_cursor_coords(pos, x, y);
	if (!text.empty() && get_current_font_face()) {
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
context::dmat4 context::get_modelview_projection_device_matrix() const
{
	return get_device_matrix()*get_projection_matrix()*get_modelview_matrix();
}

/// compute a the location in world space of a device x/y-location. For this the device point is extended with the device z-coordinate currently stored in depth buffer.
context::vec3 context::get_point_W(int x_D, int y_D) const
{
	return get_point_W(x_D, y_D, get_z_D(x_D,y_D));
}

context::vec3 context::get_point_W(int x_D, int y_D, const dmat4& MVPD) const
{
	return get_point_W(x_D, y_D, get_z_D(x_D,y_D), MVPD);
}
/// compute a the location in world space of a device point.
context::vec3 context::get_point_W(int x_D, int y_D, double z_D) const
{
	return get_point_W(x_D, y_D, z_D, get_modelview_projection_device_matrix());
}

/// compute a the location in world space of a device point.
context::vec3 context::get_point_W(const vec3& p_D) const
{
	return get_point_W(p_D, get_modelview_projection_device_matrix());
}

/// compute a the location in world space of a device point.
context::vec3 context::get_point_W(const vec3& p_D, const dmat4& MVPD) const
{
	dmat_type A(4, 4, &MVPD(0, 0));
	dvec_type x;
	dvec_type b(p_D(0),p_D(1),p_D(2),1.0);
	svd_solve(A,b,x);
	return vec3(float(x(0)/x(3)), float(x(1)/x(3)), float(x(2)/x(3)));
}

context::vec3 context::get_point_W(int x_D, int y_D, double z_D, const dmat4& MVPD) const
{
	return get_point_W(vec3(x_D+0.5f,y_D+0.5f,(float)z_D),MVPD);
}

void context::tesselate_arrow(double length, double aspect, double rel_tip_radius, double tip_aspect, int res, bool edges)
{
	std::cout << "tesselate_arrow not implemented in cgv::render::context" << std::endl;
}

void context::tesselate_arrow(const cgv::math::fvec<double, 3>& start, const cgv::math::fvec<double, 3>& end, double aspect, double rel_tip_radius, double tip_aspect, int res, bool edges)
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
	compare_function = CF_LEQUAL;
	use_compare_function = false;
	have_mipmaps = false;
}

shader_program_base::shader_program_base()
{
	is_enabled = false;
	geometry_shader_input_type = PT_POINTS;
	geometry_shader_output_type = PT_POINTS;
	geometry_shader_output_count = 1;

	auto_detect_uniforms = true;
	auto_detect_vertex_attributes = true;

	uses_view = false;
	uses_material = false;
	uses_lights = false;

	position_index = -1;
	normal_index = -1;
	color_index = -1;
	texcoord_index = -1;
}

// configure program
void shader_program_base::specify_standard_uniforms(bool view, bool material, bool lights)
{
	auto_detect_uniforms = false;
	uses_view = view;
	uses_material = material;
	uses_lights = lights;
}

void shader_program_base::specify_standard_vertex_attribute_names(context& ctx, bool color, bool normal, bool texcoord)
{
	auto_detect_vertex_attributes = false;
	position_index = ctx.get_attribute_location(*this, "position");
	color_index = color ? ctx.get_attribute_location(*this, "color") : -1;
	normal_index = normal ? ctx.get_attribute_location(*this, "normal") : -1;
	texcoord_index = texcoord ? ctx.get_attribute_location(*this, "texcoord") : -1;
}

void shader_program_base::specify_vertex_attribute_names(context& ctx, const std::string& position, const std::string& color, const std::string& normal, const std::string& texcoord)
{
	auto_detect_vertex_attributes = false;
	position_index = position.empty() ? -1 : ctx.get_attribute_location(*this, position);
	color_index = color.empty() ? -1 : ctx.get_attribute_location(*this, color);
	normal_index = normal.empty() ? -1 : ctx.get_attribute_location(*this, normal);
	texcoord_index = texcoord.empty() ? -1 : ctx.get_attribute_location(*this, texcoord);
}

bool context::shader_program_enable(shader_program_base& spb)
{
	if (spb.is_enabled) {
		if (shader_program_stack.top() == &spb) {
			error("context::shader_program_enable() called with program that is currently active", &spb);
			return false;
		}
		error("context::shader_program_enable() called with program that is recursively reactivated", &spb);
		return false;
	}
	shader_program_stack.push(&spb);
	spb.is_enabled = true;

	if (spb.auto_detect_vertex_attributes) {
		spb.position_index = get_attribute_location(spb, "position");
		spb.color_index = get_attribute_location(spb, "color");
		spb.normal_index = get_attribute_location(spb, "normal");
		spb.texcoord_index = get_attribute_location(spb, "texcoord");
		spb.auto_detect_vertex_attributes = false;
	}
	if (spb.auto_detect_uniforms) {
		spb.uses_lights = get_uniform_location(spb, "nr_light_sources") != -1;
		spb.uses_material = get_uniform_location(spb, "material.brdf_type") != -1;
		spb.uses_view = get_uniform_location(spb, "modelview_matrix") != -1;
		spb.auto_detect_uniforms = false;
	}
	return true;
}

bool context::shader_program_disable(shader_program_base& spb)
{
	if (shader_program_stack.empty()) {
		error("context::shader_program_disable() called with empty stack", &spb);
		return false;
	}
	if (!spb.is_enabled) {
		error("context::shader_program_disable() called with disabled program", &spb);
		return false;
	}
	if (shader_program_stack.top() != &spb) {
		error("context::shader_program_disable() called with program that was not on top of shader program stack", &spb);
		return false;
	}
	shader_program_stack.pop();
	spb.is_enabled = false;
	return true;
}

bool context::shader_program_destruct(shader_program_base& spb) const
{
	if (spb.is_enabled) {
		error("context::shader_program_destruct() on shader program that was still enabled", &spb);
/*		if (shader_program_stack.top() == &spb)
			shader_program_disable(spb);
		else {
			error("context::shader_program_destruct() on shader program that was still enabled", &spb);
			// remove destructed program from stack
			std::vector<shader_program_base*> tmp;
			while (!shader_program_stack.empty()) {
				shader_program_base* t = shader_program_stack.top();
				shader_program_stack.pop();
				if (t == &spb)
					break;
				tmp.push_back(t);
			}
			while (!tmp.empty()) {
				shader_program_stack.push(tmp.back());
				tmp.pop_back();
			}
		}*/
		return false;
	}
	return true;
}

attribute_array_binding_base::attribute_array_binding_base()
{
	is_enabled = false;
}


bool context::attribute_array_binding_destruct(attribute_array_binding_base& aab) const
{
	if (aab.is_enabled) {
		error("context::attribute_array_binding_destruct() on array binding that was still enabled", &aab);
		/*
		if (attribute_array_binding_stack.top() == &aab)
			attribute_array_binding_disable(aab);
		else {
			// remove destructed binding from stack
			std::vector<attribute_array_binding_base*> tmp;
			while (!attribute_array_binding_stack.empty()) {
				attribute_array_binding_base* t = attribute_array_binding_stack.top();
				attribute_array_binding_stack.pop();
				if (t == &aab)
					break;
				tmp.push_back(t);
			}
			while (!tmp.empty()) {
				attribute_array_binding_stack.push(tmp.back());
				tmp.pop_back();
			}
		}
		*/
		return false;
	}
	return true;
}

bool context::attribute_array_binding_enable(attribute_array_binding_base& aab)
{
	if (!aab.handle) {
		error("context::attribute_array_binding_enable() called in not created attribute array binding.", &aab);
		return false;
	}
	if (aab.is_enabled) {
		if (attribute_array_binding_stack.top() == &aab) {
			error("context::attribute_array_binding_enable() called with array binding that is currently active", &aab);
			return false;
		}
		error("context::attribute_array_binding_enable() called with array binding that is recursively reactivated", &aab);
		return false;
	}
	attribute_array_binding_stack.push(&aab);
	aab.is_enabled = true;
	return true;
}

bool context::attribute_array_binding_disable(attribute_array_binding_base& aab)
{
	if (attribute_array_binding_stack.empty()) {
		error("context::attribute_array_binding_disable() called with empty stack", &aab);
		return false;
	}
	if (!aab.is_enabled) {
		error("context::attribute_array_binding_disable() called with disabled array binding", &aab);
		return false;
	}
	if (attribute_array_binding_stack.top() != &aab) {
		error("context::attribute_array_binding_disable() called with array binding that was not on top of array binding stack", &aab);
		return false;
	}
	attribute_array_binding_stack.pop();
	aab.is_enabled = false;
	return true;
}

vertex_buffer_base::vertex_buffer_base()
{
	type = VBT_VERTICES;
	usage = VBU_STATIC_DRAW;
}


/// initialize members
frame_buffer_base::frame_buffer_base()
{
	is_enabled = false;
	width = -1;
	height = -1;
	std::fill(attached, attached+16,false);
}

void context::get_buffer_list(frame_buffer_base& fbb, std::vector<int>& buffers, int offset)
{
	if (fbb.enabled_color_attachments.size() == 0) {
		for (int i = 0; i < 16; ++i)
			if (fbb.attached[i])
				buffers.push_back(i + offset);
	}
	else {
		for (int i = 0; i < (int)fbb.enabled_color_attachments.size(); ++i)
			if (fbb.attached[fbb.enabled_color_attachments[i]])
				buffers.push_back(fbb.enabled_color_attachments[i]+offset);
	}
}

bool context::frame_buffer_create(frame_buffer_base& fbb) const
{
	if (fbb.width == -1)
		fbb.width = get_width();
	if (fbb.height == -1)
		fbb.height = get_height();
	return true;
}

bool context::frame_buffer_attach(frame_buffer_base& fbb, const render_component& rb, bool is_depth, int i) const
{
	if (fbb.handle == 0) {
		error("gl_context::frame_buffer_attach: attempt to attach to frame buffer that is not created", &fbb);
		return false;
	}
	if (rb.handle == 0) {
		error("gl_context::frame_buffer_attach: attempt to attach empty render buffer", &fbb);
		return false;
	}
	if (!is_depth)
		fbb.attached[i] = true;

	return true;
}

bool context::frame_buffer_attach(frame_buffer_base& fbb, const texture_base& t, bool is_depth, int level, int i, int z_or_cube_side) const
{
	if (fbb.handle == 0) {
		error("context::frame_buffer_attach: attempt to attach to frame buffer that is not created", &fbb);
		return false;
	}
	if (t.handle == 0) {
		error("context::frame_buffer_attach: attempt to attach texture that is not created", &fbb);
		return false;
	}
	if (!is_depth)
		fbb.attached[i] = true;
	
	return true;
}

bool context::frame_buffer_enable(frame_buffer_base& fbb)
{
	if (fbb.handle == 0) {
		error("context::frame_buffer_enable: attempt to enable not created frame buffer", &fbb);
		return false;
	}
	if (fbb.is_enabled) {
		if (frame_buffer_stack.top() == &fbb) {
			error("context::frame_buffer_enable() called with frame buffer that is currently active", &fbb);
			return false;
		}
		error("context::frame_buffer_enable() called with frame buffer that is recursively reactivated", &fbb);
		return false;
	}
	frame_buffer_stack.push(&fbb);
	fbb.is_enabled = true;
	return true;
}

bool context::frame_buffer_disable(frame_buffer_base& fbb)
{
	if (frame_buffer_stack.empty()) {
		error("gl_context::frame_buffer_disable called with empty stack", &fbb);
		return false;
	}
	if (frame_buffer_stack.top() != &fbb) {
		error("gl_context::frame_buffer_disable called with different frame buffer enabled", &fbb);
		return false;
	}
	frame_buffer_stack.pop();
	return true;
}

bool context::frame_buffer_destruct(frame_buffer_base& fbb) const
{
	if (fbb.handle == 0) {
		error("context::frame_buffer_destruct: attempt to destruct not created frame buffer", &fbb);
		return false;
	}
	if (fbb.is_enabled) {
		error("context::frame_buffer_destruct() on frame buffer that was still enabled", &fbb);
/*
		if (frame_buffer_stack.top() == &fbb)
			frame_buffer_disable(fbb);
		else {
			// remove destructed program from stack
			std::vector<frame_buffer_base*> tmp;
			while (!frame_buffer_stack.empty()) {
				frame_buffer_base* t = frame_buffer_stack.top();
				frame_buffer_stack.pop();
				if (t == &fbb)
					break;
				tmp.push_back(t);
			}
			while (!tmp.empty()) {
				frame_buffer_stack.push(tmp.back());
				tmp.pop_back();
			}
		}
		*/
		return false;
	}
	return true;
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