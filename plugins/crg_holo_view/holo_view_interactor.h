#pragma once

#include <cgv/base/base.h>
#include <cgv/render/drawable.h>
#include <cgv/render/stereo_view.h>
#include <cgv/render/texture.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/provider.h>
#include <cgv/reflect/reflect_enum.h>
#include <cgv_gl/gl/gl.h>
#include <glsu/GL/glsu.h>
#if defined(_WINDOWS) || defined(WIN32) || defined(WIN64)
#undef max
#undef min
#endif

#include "lib_begin.h"

enum StereoMousePointer {
	SMP_BITMAP,
	SMP_PIXELS,
	SMP_ARROW
};

namespace cgv {
	namespace math {
		///return a vector containing the component-wise modulo value
		template <typename T, uint32_t N>
		const fvec<T, N> mod(const fvec<T, N>& v, const fvec<T, N>& t) {
			fvec<T, N> c;
			for (unsigned i = 0; i < N; ++i)
				c(i) = v(i) % t(i);
			return c;
		}
		template <uint32_t N>
		const fvec<float, N> mod(const fvec<float, N>& v, const fvec<float, N>& t) {
			fvec<float, N> c;
			for (unsigned i = 0; i < N; ++i)
				c(i) = std::fmod(v(i), t(i));
			return c;
		}
		template <uint32_t N>
		const fvec<double, N> mod(const fvec<double, N>& v, const fvec<double, N>& t) {
			fvec<double, N> c;
			for (unsigned i = 0; i < N; ++i)
				c(i) = std::fmod(v(i), t(i));
			return c;
		}
	}
}

struct holo_display_calibration
{
	using vec3 = cgv::vec3;
	using ivec2 = cgv::ivec2;

	unsigned width = 3840;
	unsigned height = 2160;

	vec3 x_min = vec3(0.01f);
	vec3 x_max = vec3(0.99f);

	vec3 y_min = vec3(0.01f);
	vec3 y_max = vec3(0.99f);

	float length = 42.8996f;
	float step_x = 35.3762f;
	float step_y = 1.0f;
	vec3 offset = vec3(42.4f, 39.8f, 37.2f);

	vec3 compute_subpixel_base_line_coordinates(ivec2 pixel)
	{
		vec3 bl_crd_0 = vec3(step_x * float(pixel.x()) + step_y * float(int(height) - pixel.y() - 1)) + offset;
		vec3 bl_crd_1 = mod(bl_crd_0, vec3(length));
		vec3 bl_crd_2 = bl_crd_1 / length;
		return bl_crd_2;
	}
	vec3 compute_subpixel_x_coordinates(ivec2 pixel)
	{
		return float(pixel.x()) * (x_max-x_min) + x_min;
	}
	vec3 compute_subpixel_y_coordinates(ivec2 pixel)
	{
		return float(pixel.y()) * (y_max - y_min) + y_min;
	}
};

/// class that manages a stereoscopic view
class CGV_API holo_view_interactor : 
	public cgv::base::node, 
	public cgv::gui::event_handler, 
	public cgv::render::multi_pass_drawable,
	public cgv::gui::provider,
	public cgv::render::stereo_view
{
public:
	using rgb = cgv::rgb;
	using vec2 = cgv::vec2;
	using vec3 = cgv::vec3;
	using vec4 = cgv::vec4;
	using ivec4 = cgv::ivec4;
protected:
	// interaction
	bool two_d_enabled;
	bool fix_view_up_dir;

	// stereo
	bool stereo_translate_in_model_view;
	float eye_separation_factor = 10.0f;
	/// multiply inter-eye-location with eye_separation_factor
	float compute_eye_base(int view_index) const;

	// display
	holo_display_calibration display_calib;


	// rendering
public:
	enum HoloMode { HM_SINGLE, HM_QUILT, HM_VOLUME } holo_mode = HM_SINGLE;
protected:
	unsigned view_width = 1638;
	unsigned view_height = 910;
	unsigned nr_views = 45;
	unsigned view_index = 22;
	int blit_offset_x = 0, blit_offset_y = 0;
	bool generate_hologram = true;
	bool display_write_to_file = false;
private:
	cgv::render::texture display_tex;
	cgv::render::frame_buffer display_fbo;
protected:
	// quilt
	rgb quilt_bg_color = rgb(0.5f, 0.5f, 0.5f);
	bool quilt_use_offline_texture = true;
	unsigned quilt_width = 8192;
	unsigned quilt_height = 8192;
	unsigned quilt_nr_cols = 5;
	unsigned quilt_nr_rows = 9;
	bool quilt_interpolate = true;
	bool quilt_write_to_file = false;
private:
	// internal parameters used during multipass rendering
	unsigned vi = 0, quilt_col = 0, quilt_row = 0;
	cgv::render::texture quilt_tex;
	cgv::render::frame_buffer quilt_fbo;
	cgv::render::render_buffer quilt_depth_buffer;
	cgv::render::shader_program quilt_prog;

protected:

	cgv::render::frame_buffer volume_fbo;
	cgv::render::render_buffer volume_depth_buffer;
	cgv::render::shader_program volume_prog;

	cgv::render::texture volume_tex;


public:
	void set_default_values();
protected:
	/// whether messages should be shown to user in case something fails
	bool enable_messages;
	double z_near_derived, z_far_derived;
	bool show_focus;
	bool clip_relative_to_extent;
	double pan_sensitivity, zoom_sensitivity, rotate_sensitivity;

	template <typename T>
	void update_vec_member(cgv::math::vec<T>& v) {
		for (unsigned int i=0; i<v.size(); ++i)
			update_member(&v(i));
	}
	template <typename T, cgv::type::uint32_type N>
	void update_vec_member(cgv::math::fvec<T,N>& v) {
		for (unsigned int i=0; i<N; ++i)
			update_member(&v(i));
	}
	///
	StereoMousePointer stereo_mouse_pointer;
	///
	void draw_mouse_pointer_as_bitmap(cgv::render::context& ctx, int x, int y, int center_x, int center_y, int vp_width, int vp_height, bool visible, const dmat4 &MPW);
	///
	void draw_mouse_pointer_as_pixels(cgv::render::context& ctx, int x, int y, int center_x, int center_y, int vp_width, int vp_height, bool visible, const dmat4 &MPW);
	///
	void draw_mouse_pointer_as_arrow(cgv::render::context& ctx, int x, int y, int center_x, int center_y, int vp_width, int vp_height, bool visible, const dmat4 &MPW);
	///
	void draw_mouse_pointer(cgv::render::context& ctx, bool visible);
	///
	void draw_focus();
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	void on_set(void* m);
	void on_rotation_change();

	void set_view_orientation(const std::string& axes);
	/// set the current projection matrix
	void gl_set_projection_matrix(cgv::render::context& ctx, float e, double aspect);
	void gl_set_modelview_matrix(cgv::render::context& ctx, float e, double aspect, const cgv::render::view& view);
	/// ensure sufficient number of viewport views
	unsigned get_viewport_index(unsigned col_index, unsigned row_index) const;
	void ensure_viewport_view_number(unsigned nr);

	/**@name gamepad control*/
	//@{
	bool use_gamepad;
	bool gamepad_attached;
	float deadzone;
	int left_mode, right_mode;
	vec2 left_stick, right_stick, trigger;

	bool gamepad_emulation;
	bool emulation_active;
	float emulation_axes[6];
	float plus_key_values[6];
	bool plus_key_down[6];
	double plus_key_toggle_time[6];
	float minus_key_values[6];
	double minus_key_toggle_time[6];
	bool minus_key_down[6];
	void check_emulation_active();
	void plus_key_action(int i, cgv::gui::KeyAction action);
	void minus_key_action(int i, cgv::gui::KeyAction action);

	void timer_event(double t, double dt);
	ivec4 split_viewport(const ivec4 vp, int col_idx, int row_idx) const;
	//@}


	/// surface manager stuff
	void enable_surface(cgv::render::context& ctx);
	void disable_surface(cgv::render::context& ctx);
	void post_process_surface(cgv::render::context& ctx);
public:
	///
	holo_view_interactor(const char* name);
	/**@name viewport splitting*/
	//@{
	/// call this function before a drawing process to support viewport splitting inside the draw call via the activate/deactivate functions
	void enable_viewport_splitting(unsigned nr_cols, unsigned nr_rows);
	/// check whether viewport splitting is activated and optionally set the number of columns and rows if corresponding pointers are passed
	bool is_viewport_splitting_enabled(unsigned* nr_cols_ptr = 0, unsigned* nr_rows_ptr = 0) const;
	/// disable viewport splitting
	void disable_viewport_splitting();
	/// inside the drawing process activate the sub-viewport with the given column and row indices, always terminate an activated viewport with deactivate_split_viewport
	void activate_split_viewport(cgv::render::context& ctx, unsigned col_index, unsigned row_index);
	/// deactivate the previously split viewport
	void deactivate_split_viewport(cgv::render::context& ctx);
	/// make a viewport manage its own view
	void enable_viewport_individual_view(unsigned col_index, unsigned row_index, bool enable = true);
	/// check whether viewport manage its own view
	bool does_viewport_use_individual_view(unsigned col_index, unsigned row_index) const { 
		unsigned i = get_viewport_index(col_index, row_index);
		return i >= use_individual_view.size() ? false : use_individual_view[i];
	}
	/// access the view of a given viewport
	cgv::render::view& ref_viewport_view(unsigned col_index, unsigned row_index);
	//@}
	//! given a mouse location and the pixel extent of the context, return the MPW matrix for unprojection
	int get_modelview_projection_window_matrices(int x, int y, int width, int height,
		const dmat4** MPW_pptr,
		const dmat4** MPW_other_pptr = 0, int* x_other_ptr = 0, int* y_other_ptr = 0,
		int* vp_col_idx_ptr = 0, int* vp_row_idx_ptr = 0,
		int* vp_width_ptr = 0, int *vp_height_ptr = 0,
		int* vp_center_x_ptr = 0, int* vp_center_y_ptr = 0,
		int* vp_center_x_other_ptr = 0, int* vp_center_y_other_ptr = 0) const;
	//! given a pixel location x,y return the z-value from the depth buffer, which ranges from 0.0 at z_near to 1.0 at z_far and a point in world coordinates
	/*! in case of stereo rendering two z-values exist that can be unprojected to two points in world
	    coordinates. In this case the possibility with smaller z value is selected. */
	void get_vp_col_and_row_indices(cgv::render::context& ctx, int x, int y, int& vp_col_idx, int& vp_row_idx);
	double get_z_and_unproject(cgv::render::context& ctx, int x, int y, dvec3& p);
	void set_focus(const dvec3& foc) { stereo_view::set_focus(foc); update_vec_member(view::focus); }
	void set_view_up_dir(const dvec3& vud) { stereo_view::set_view_up_dir(vud); update_vec_member(view_up_dir); }
	void set_view_dir(const dvec3& vd) { stereo_view::set_view_dir(vd); update_vec_member(view_dir); }
	void set_y_extent_at_focus(double ext) { stereo_view::set_y_extent_at_focus(ext); on_set(&y_extent_at_focus); }
	void set_y_view_angle(double angle) { stereo_view::set_y_view_angle(angle); on_set(&y_view_angle); }
	void set_eye_distance(double e) { stereo_view::set_eye_distance(e); on_set(&eye_distance); }
	void set_parallax_zero_scale(double pzs) { stereo_view::set_parallax_zero_scale(pzs); on_set(&parallax_zero_scale); }
	/// return the type name 
	std::string get_type_name() const;
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	/// build shader programs
	bool init(cgv::render::context&);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context&);
	/// 
	void draw(cgv::render::context&);
	/// this method is called in one pass over all drawables after drawing
	void finish_frame(cgv::render::context&);
	/// this method is called in one pass over all drawables after finish frame
	void after_finish(cgv::render::context&);
	/// return a shortcut to activate the gui without menu navigation
	//cgv::gui::shortcut get_shortcut() const;
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const;
	/// you must overload this for gui creation
	void create_gui();

	void set_z_near(double z);
	void set_z_far(double z);
	void set_default_view();
private:
	double check_for_click;
	
	dmat4 MPW, MPW_right;
	dmat4 V, P;

	float current_e = 0.0f;
	//int current_vp[4], current_sb[4];

	bool do_viewport_splitting;
	unsigned nr_viewport_columns;
	unsigned nr_viewport_rows;
	unsigned viewport_shrinkage;
	std::vector<dmat4> MPWs, MPWs_right;
	std::vector<cgv::render::view> views;
	std::vector<bool> use_individual_view;

	bool last_do_viewport_splitting;
	unsigned last_nr_viewport_columns;
	unsigned last_nr_viewport_rows;

	int last_x, last_y;
};

extern CGV_API cgv::reflect::enum_reflection_traits<StereoMousePointer> get_reflection_traits(const StereoMousePointer&);
extern CGV_API cgv::reflect::enum_reflection_traits<holo_view_interactor::HoloMode> get_reflection_traits(const holo_view_interactor::HoloMode&);

#include <cgv/config/lib_end.h>
