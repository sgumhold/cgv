#pragma once

#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl_view.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
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
	GlsuAnaglyphConfiguration ac;
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
	virtual void set_anaglyph_config(GlsuAnaglyphConfiguration _ac) { ac = _ac; }
	bool is_stereo_enabled() const { return stereo_enabled; }
	virtual void enable_stereo(bool e = true) { stereo_enabled = e; }
	double get_eye_distance() const { return eye_distance; }
	virtual void set_eye_distance(double e) { eye_distance = e; }
	double get_parallax_zero_scale() const { return parallax_zero_scale; }
	double get_parallax_zero_z() const;
	virtual void set_parallax_zero_scale(double pzs) { parallax_zero_scale = pzs; }
	void put_coordinate_system(vec_type& x, vec_type& y, vec_type& z) const;
};

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
	void draw_focus();
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	std::string get_property_declarations();
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	void on_set(void* m);
	void on_rotation_change();
	double get_z_and_unproject(cgv::render::context* ctx, int x, int y, pnt_type& p);


	bool animate_view;
	pnt_type target_view_dir;
	pnt_type target_view_up_dir;
	void set_view_orientation(const std::string& axes);
	int correct_anim_dir_vector(cgv::render::view::pnt_type& dir, const cgv::render::view::pnt_type& v, const cgv::render::view::pnt_type* up) const;
	void timer_event(double t, double dt);
public:
	///
	stereo_view_interactor(const char* name);

	void set_focus(const pnt_type& foc) { ext_view::set_focus(foc); update_vec_member(view::focus); }
	void set_view_up_dir(const vec_type& vud) { ext_view::set_view_up_dir(vud); update_vec_member(view_up_dir); }
	void set_view_dir(const vec_type& vd) { ext_view::set_view_dir(vd); update_vec_member(view_dir); }
	void set_y_extent_at_focus(double ext) { ext_view::set_y_extent_at_focus(ext); update_member(&y_extent_at_focus); }
	void set_y_view_angle(double angle) { ext_view::set_y_view_angle(angle); update_member(&y_view_angle); }
	void set_stereo_mode(GlsuStereoMode sm) { ext_view::set_stereo_mode(sm); update_member(&stereo_mode); }
	void enable_stereo(bool e = true) { ext_view::enable_stereo(e); update_member(&stereo_enabled); }
	void set_eye_distance(double e) { ext_view::set_eye_distance(e); update_member(&eye_distance); }
	void set_parallax_zero_scale(double pzs) { ext_view::set_parallax_zero_scale(pzs); update_member(&parallax_zero_scale); }
	void set_anaglyph_config(GlsuAnaglyphConfiguration _ac) { ext_view::set_anaglyph_config(_ac); update_member(&ac); }
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
	void roll(double angle);
	///
	void rotate_image_plane(double ax, double ay);
private:
	double check_for_click;
	cgv::render::context::mat_type DPV;
	cgv::render::context::mat_type V,P;
};

#include <cgv/config/lib_end.h>
