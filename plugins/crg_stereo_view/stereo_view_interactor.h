#pragma once

#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl_view.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/reflect/reflect_enum.h>
#include <cgv_gl/gl/gl.h>
#include <glsu/GL/glsu.h>

#include "lib_begin.h"

class CGV_API ext_view : public cgv::render::gl::gl_view
{
protected:
	double eye_distance;
	double parallax_zero_scale;
	GlsuStereoMode stereo_mode;
	GlsuEye mono_mode;
	GlsuAnaglyphConfiguration anaglyph_config;
	bool stereo_enabled;
	bool stereo_translate_in_model_view;
	bool two_d_enabled;
	bool fix_view_up_dir;
public:
	ext_view();
	void set_default_values();
	GlsuStereoMode get_stereo_mode() const { return stereo_mode; }
	virtual void set_stereo_mode(GlsuStereoMode sm) { stereo_mode = sm; }
	GlsuEye get_mono_mode() const { return mono_mode; }
	virtual void set_mono_mode(GlsuEye mm) { mono_mode = mm; }
	virtual void set_anaglyph_config(GlsuAnaglyphConfiguration _ac) { anaglyph_config = _ac; }
	bool is_stereo_enabled() const { return stereo_enabled; }
	virtual void enable_stereo(bool e = true) { stereo_enabled = e; }
	double get_eye_distance() const { return eye_distance; }
	virtual void set_eye_distance(double e) { eye_distance = e; }
	double get_parallax_zero_scale() const { return parallax_zero_scale; }
	double get_parallax_zero_z() const;
	virtual void set_parallax_zero_scale(double pzs) { parallax_zero_scale = pzs; }
};

enum StereoMousePointer {
	SMP_BITMAP,
	SMP_PIXELS,
	SMP_ARROW
};

extern CGV_API cgv::reflect::enum_reflection_traits<StereoMousePointer> get_reflection_traits(const StereoMousePointer&);

class CGV_API stereo_view_interactor : 
	public cgv::base::node, 
	public cgv::gui::event_handler, 
	public cgv::render::drawable,
	public cgv::gui::provider,
	public ext_view
{
protected:
	double z_near_derived, z_far_derived;
	float depth_offset, depth_scale;
	bool auto_view_images;
	bool show_focus;
	bool clip_relative_to_extent;
	double zoom_sensitivity, rotate_sensitivity;


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
	void dir_gui_cb(vec_type& dir, int i);
	void add_dir_control(const std::string& name, vec_type& dir);
	void check_write_image(cgv::render::context& ctx, const char* post_fix = "", bool done = true);


	/// overload to set local lights before modelview matrix is set
	virtual void on_set_local_lights();

	///
	StereoMousePointer stereo_mouse_pointer;
	///
	void draw_mouse_pointer_as_bitmap(cgv::render::context& ctx, int x, int y, int center_x, int center_y, int vp_width, int vp_height, bool visible, cgv::render::context::mat_type &DPV);
	///
	void draw_mouse_pointer_as_pixels(cgv::render::context& ctx, int x, int y, int center_x, int center_y, int vp_width, int vp_height, bool visible, cgv::render::context::mat_type &DPV);
	///
	void draw_mouse_pointer_as_arrow(cgv::render::context& ctx, int x, int y, int center_x, int center_y, int vp_width, int vp_height, bool visible, cgv::render::context::mat_type &DPV);
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


	bool animate_view;
	pnt_type target_view_dir;
	pnt_type target_view_up_dir;
	void set_view_orientation(const std::string& axes);
	int correct_anim_dir_vector(cgv::render::view::pnt_type& dir, const cgv::render::view::pnt_type& v, const cgv::render::view::pnt_type* up) const;
	void timer_event(double t, double dt);
	/// set the current projection matrix
	void gl_set_projection_matrix(GlsuEye e, double aspect);
	void gl_set_modelview_matrix(GlsuEye e, double aspect, const cgv::render::view& view);
	/// ensure sufficient number of viewport views
	unsigned get_viewport_index(unsigned col_index, unsigned row_index) const;
	void ensure_viewport_view_number(unsigned nr);
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
	void deactivate_split_viewport();
	/// make a viewport manage its own view
	void viewport_use_individual_view(unsigned col_index, unsigned row_index);
	/// check whether viewport manage its own view
	bool does_viewport_use_individual_view(unsigned col_index, unsigned row_index) const { 
		unsigned i = get_viewport_index(col_index, row_index);
		return i >= use_individual_view.size() ? false : use_individual_view[i];
	}
	/// access the view of a given viewport
	cgv::render::view& ref_viewport_view(unsigned col_index, unsigned row_index);
	//@}
	//! given a mouse location and the pixel extent of the context, return the DPV matrix for unprojection
	/*! In stereo modes with split viewport, the returned DPV is the one the mouse pointer is on.
	The return value is in this case -1 or 1 and tells if DPV corresponds to the left (-1) or right (1) viewport.
	Furthermore, the DPV of the corresponding mouse location in the other eye is returned through DPV_other_ptr
	and the mouse location in x_other_ptr and y_other_ptr. In anaglyph or quad buffer stereo mode the other
	mouse location is identical to the incoming x and y location and 0 is returned. In mono mode,
	the other DPV and mouse locations are set to values identical to DPV and x,y and also 0 is returned.

	In case the viewport splitting was enabled during the last drawing process, the DPV and
	DPV_other matrices are set to the one valid in the panel that the mouse position x,y is
	in. The panel column and row indices are passed to the vp_col_idx and vp_row_idx pointers.
	In case that viewport splitting was disabled, 0 is passed to the panel location index pointers.

	Finally, the vp_width, vp_height, vp_center_x, and vp_center_y pointers are set to the viewport size 
	and center mouse location of the panel panel that the mouse pointer is in.

	All pointer arguments starting with DPV_other_ptr can be set to the null pointer.*/
	int get_DPVs(int x, int y, int width, int height,
		cgv::math::mat<double>** DPV_pptr,
		cgv::math::mat<double>** DPV_other_pptr = 0, int* x_other_ptr = 0, int* y_other_ptr = 0,
		int* vp_col_idx_ptr = 0, int* vp_row_idx_ptr = 0,
		int* vp_width_ptr = 0, int *vp_height_ptr = 0,
		int* vp_center_x_ptr = 0, int* vp_center_y_ptr = 0,
		int* vp_center_x_other_ptr = 0, int* vp_center_y_other_ptr = 0);
	//! given a pixel location x,y return the z-value from the depth buffer, which ranges from 0.0 at z_near to 1.0 at z_far and a point in world coordinates
	/*! in case of stereo rendering two z-values exist that can be unprojected to two points in world
	    coordinates. In this case the possibility with smaller z value is selected. */
	void get_vp_col_and_row_indices(cgv::render::context& ctx, int x, int y, int& vp_col_idx, int& vp_row_idx);
	double get_z_and_unproject(cgv::render::context& ctx, int x, int y, pnt_type& p);
	void set_focus(const pnt_type& foc) { ext_view::set_focus(foc); update_vec_member(view::focus); }
	void set_view_up_dir(const vec_type& vud) { ext_view::set_view_up_dir(vud); update_vec_member(view_up_dir); }
	void set_view_dir(const vec_type& vd) { ext_view::set_view_dir(vd); update_vec_member(view_dir); }
	void set_y_extent_at_focus(double ext) { ext_view::set_y_extent_at_focus(ext); update_member(&y_extent_at_focus); }
	void set_y_view_angle(double angle) { ext_view::set_y_view_angle(angle); update_member(&y_view_angle); }
	void set_stereo_mode(GlsuStereoMode sm) { ext_view::set_stereo_mode(sm); update_member(&stereo_mode); }
	void enable_stereo(bool e = true) { ext_view::enable_stereo(e); update_member(&stereo_enabled); }
	void set_eye_distance(double e) { ext_view::set_eye_distance(e); update_member(&eye_distance); }
	void set_parallax_zero_scale(double pzs) { ext_view::set_parallax_zero_scale(pzs); update_member(&parallax_zero_scale); }
	void set_anaglyph_config(GlsuAnaglyphConfiguration _ac) { ext_view::set_anaglyph_config(_ac); update_member(&anaglyph_config); }
	/// return the type name 
	std::string get_type_name() const;
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	bool init(cgv::render::context& ctx);
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
	cgv::gui::shortcut get_shortcut() const;
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const;
	/// you must overload this for gui creation
	void create_gui();
	///
	void roll(cgv::render::view& view, double angle);
	///
	void rotate_image_plane(cgv::render::view& view, double ax, double ay);
private:
	double check_for_click;
	
	cgv::render::context::mat_type DPV, DPV_right;
	cgv::render::context::mat_type V, P;

	GlsuEye current_e;
	int current_vp[4], current_sb[4];

	bool do_viewport_splitting;
	unsigned nr_viewport_columns;
	unsigned nr_viewport_rows;
	std::vector<cgv::render::context::mat_type> DPVs, DPVs_right;
	std::vector<cgv::render::view> views;
	std::vector<bool> use_individual_view;

	bool last_do_viewport_splitting;
	unsigned last_nr_viewport_columns;
	unsigned last_nr_viewport_rows;

	int last_x, last_y;
};

#include <cgv/config/lib_end.h>
