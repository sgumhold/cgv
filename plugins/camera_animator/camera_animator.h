#pragma once

#include <cgv/gui/file_helper.h>
#include <cgv/gui/help_message.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv_app/application_plugin.h>
#include <cgv_app/gizmo.h>
#include <cgv_gl/line_render_data.h>
#include <cgv_gl/point_render_data.h>
#include <cgv/os/pipe_thread.h>

#include "easing_functions.h"
#include "animation_data.h"
#include "keyframe_editor_overlay.h"

#include "lib_begin.h"

class CGV_API camera_animator : public cgv::app::application_plugin {
protected:
	using vec2 = cgv::vec2;
	using vec3 = cgv::vec3;
	using vec4 = cgv::vec4;
	using ivec2 = cgv::ivec2;
	using rgb = cgv::rgb;

	FILE* fp = 0;
	int fps = 30;
	std::string video_file_name;
	bool video_open = false;
	bool use_named_pipe = true;
	cgv::os::pipe_output_thread* thread_ptr = 0;
	cgv::os::named_pipe_output_thread* named_thread_ptr = 0;
	size_t nr_blocks = 0;
	bool open_ffmpeg_pipe(const std::string& file_name);
	bool write_image_to_ffmpeg_pipe();
	bool close_ffmpeg_pipe();

	cgv::gui::help_message help;

	/// store a pointer to the view
	cgv::render::view* view_ptr = nullptr;
	cgv::app::gizmo eye_gizmo, focus_gizmo;
	keyframe_editor_overlay_ptr timeline_ptr;
	keyframe* selected_keyframe = nullptr;

	rgb eye_color;
	rgb focus_color;

	cgv::render::point_renderer local_point_renderer;
	cgv::render::line_renderer local_line_renderer;

	cgv::render::point_render_data<> eye_rd, keyframes_rd;
	cgv::render::line_render_data<> view_rd, paths_rd;

	cgv::mat4 view_transformation;

	std::shared_ptr<animation_data> animation;

	cgv::gui::file_helper input_file_helper;
	cgv::gui::directory_helper output_directory_helper;

	bool animate = false;
	bool record = false;
	bool apply = false;
	bool show_camera = true;
	bool show_path = true;

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

	void handle_eye_gizmo_move();

	void handle_focus_gizmo_move();

	void handle_editor_change(keyframe_editor_overlay::Event e);

	bool load_animation(const std::string& file_name);

	bool save_animation(const std::string& file_name);

	void write_image(const std::string& file_name);

	void write_single_image();

public:
	 camera_animator();
	std::string get_type_name() const { return "camera_animator"; }

	void clear(cgv::render::context& ctx);

	bool self_reflect(cgv::reflect::reflection_handler& rh);
	void stream_help(std::ostream& os) {}
	void stream_stats(std::ostream& os) {}

	bool handle_event(cgv::gui::event& e);
	void handle_timer_event(double t, double dt);
	void handle_member_change(const cgv::utils::pointer_test& m);
	bool on_exit_request();

	void on_select();
	void on_deselect();

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void finish_frame(cgv::render::context& ctx);
	void after_finish(cgv::render::context& ctx);

	void create_gui();
};

#include <cgv/config/lib_end.h>
