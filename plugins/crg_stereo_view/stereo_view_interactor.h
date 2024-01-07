#pragma once

#include <cgv/base/base.h>
#include <cgv/render/drawable.h>
#include <cgv/render/stereo_view.h>
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

extern CGV_API cgv::reflect::enum_reflection_traits<StereoMousePointer> get_reflection_traits(const StereoMousePointer&);

/// class that manages a stereoscopic view
class CGV_API stereo_view_interactor : 
	public cgv::base::node, 
	public cgv::gui::event_handler, 
	public cgv::render::multi_pass_drawable,
	public cgv::gui::provider,
	public cgv::render::stereo_view
{
public:
	typedef cgv::math::fvec<float, 3>      vec3;
	typedef cgv::math::fvec<double, 3>    dvec3;
	typedef cgv::math::fmat<double, 3, 3> dmat3;
	typedef cgv::math::fmat<double, 4, 4> dmat4;
	typedef cgv::media::axis_aligned_box<double, 3> dbox3;
protected:
	GlsuStereoMode stereo_mode;
	GlsuEye mono_mode;
	GlsuAnaglyphConfiguration anaglyph_config;
	bool stereo_enabled;
	bool stereo_translate_in_model_view;
	bool two_d_enabled;
	bool fix_view_up_dir;
	bool adapt_aspect_ratio_to_stereo_mode;
public:
	void set_default_values();
	GlsuStereoMode get_stereo_mode() const { return stereo_mode; }
	virtual void set_stereo_mode(GlsuStereoMode sm) { stereo_mode = sm; on_set(&stereo_mode); }
	GlsuEye get_mono_mode() const { return mono_mode; }
	virtual void set_mono_mode(GlsuEye mm) { mono_mode = mm; on_set(&mono_mode); }
	virtual void set_anaglyph_config(GlsuAnaglyphConfiguration _ac) { anaglyph_config = _ac; on_set(&anaglyph_config); }
	bool is_stereo_enabled() const { return stereo_enabled; }
	virtual void enable_stereo(bool e = true) { stereo_enabled = e; on_set(&stereo_enabled); }
protected:
	/// whether messages should be shown to user in case something fails
	bool enable_messages;
	double z_near_derived, z_far_derived;
	float depth_offset, depth_scale;
	bool auto_view_images;
	bool show_focus;
	bool clip_relative_to_extent;
	double pan_sensitivity, zoom_sensitivity, rotate_sensitivity;

	// members for screen shots
	bool write_images;
	bool write_depth;
	bool write_color;
	bool write_stereo;
	int  write_width, write_height;
	std::string image_file_name_prefix;
	void write_images_to_file();

	void on_stereo_change();

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
	void dir_gui_cb(dvec3& dir, int i);
	void add_dir_control(const std::string& name, dvec3& dir);
	void check_write_image(cgv::render::context& ctx, const char* post_fix = "", bool done = true);

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
	std::string get_property_declarations();
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	void on_set(void* m);
	void on_rotation_change();

	void set_view_orientation(const std::string& axes);
	/// set the current projection matrix
	void gl_set_projection_matrix(cgv::render::context& ctx, GlsuEye e, double aspect);
	void gl_set_modelview_matrix(cgv::render::context& ctx, GlsuEye e, double aspect, const cgv::render::view& view);
	/// ensure sufficient number of viewport views
	unsigned get_viewport_index(unsigned col_index, unsigned row_index) const;
	void ensure_viewport_view_number(unsigned nr);

	/**@name gamepad control*/
	//@{
	bool use_gamepad;
	bool gamepad_attached;
	float deadzone;
	int left_mode, right_mode;
	cgv::math::fvec<float,2> left_stick, right_stick, trigger;

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
	cgv::ivec4 split_viewport(const cgv::ivec4 vp, int col_idx, int row_idx) const;
	//@}
public:
	///
	stereo_view_interactor(const char* name);
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

	GlsuEye current_e;
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

#include <cgv/config/lib_end.h>
