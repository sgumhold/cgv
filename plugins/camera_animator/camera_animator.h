#pragma once

#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv_app/application_plugin.h>
#include <cgv_gl/line_render_data.h>
#include <cgv_gl/point_render_data.h>

#include "easing_functions.h"
#include "animation_data.h"
#include "keyframe_editor_overlay.h"

#include "lib_begin.h"

class CGV_API camera_animator : public cgv::app::application_plugin {
protected:
	/// store a pointer to the view
	cgv::render::view* view_ptr = nullptr;
	
	keyframe_editor_overlay_ptr keyframe_editor_ptr;

	cgv::render::rgb eye_color;
	cgv::render::rgb focus_color;

	cgv::render::point_renderer local_point_renderer;
	cgv::render::line_renderer local_line_renderer;

	cgv::render::point_render_data<> eye_rd, keyframes_rd;
	cgv::render::line_render_data<> view_rd, paths_rd;

	cgv::render::mat4 view_transformation;

	std::shared_ptr<animation_data> animation;

	std::string input_path;
	std::string output_path;

	bool animate = false;
	bool record = false;
	bool apply = false;
	bool show_camera = true;
	bool show_path = true;
	bool show_editor = true;

	cgv::gui::button_ptr play_pause_btn;

	void set_animate(bool flag);

	void reset_animation();

	void toggle_animation();

	void skip_to_start();

	void skip_to_end();

	void skip_frame(bool reverse);

	bool set_animation_state(bool use_continuous_time);

	std::pair<size_t, float> get_max_frame_and_time();

	void udpate_animation_member_limits();

	void create_camera_render_data(const view_parameters& view);

	void create_path_render_data();

	void handle_editor_change();

	bool load_animation(const std::string& file_name);

	void write_image(const std::string& file_name);

	void write_single_image();

public:
	class camera_animator();
	std::string get_type_name() const { return "camera_animator"; }

	void clear(cgv::render::context& ctx);

	bool self_reflect(cgv::reflect::reflection_handler& rh);
	void stream_help(std::ostream& os) {}
	void stream_stats(std::ostream& os) {}

	bool handle_event(cgv::gui::event& e);
	void handle_timer_event(double t, double dt);
	void handle_on_set(const cgv::app::on_set_evaluator& m);
	bool on_exit_request();

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void finish_frame(cgv::render::context& ctx);
	void after_finish(cgv::render::context& ctx);

	void create_gui();
};

#include <cgv/config/lib_end.h>
