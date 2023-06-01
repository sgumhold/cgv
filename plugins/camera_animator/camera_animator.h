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

	cgv::render::point_renderer local_point_renderer;
	cgv::render::line_renderer local_line_renderer;

	cgv::render::point_render_data<> eye_rd;
	cgv::render::point_render_data<> keyframes_rd;
	cgv::render::line_render_data<> paths_rd;

	cgv::render::rgb eye_color;
	cgv::render::rgb focus_color;

	std::shared_ptr<animation_data> animation;

	bool animate = false;
	bool record = false;
	bool apply = false;
	bool show = true;

	cgv::gui::button_ptr play_pause_btn;

	void set_animate(bool flag) {

		animate = flag;
		play_pause_btn->set_name(animate ? "@pause" : "@play");
		play_pause_btn->update();
	}

	void reset_animation() {

		animation->reset();
		set_animate(false);

		set_animation_state(false);

		post_redraw();
	}

	void play_animation() {

		set_animate(true);
		post_redraw();
	}

	void pause_animation() {

		set_animate(false);
		post_redraw();
	}

	void toggle_animation() {

		set_animate(!animate);
		post_redraw();
	}

	void skip_to_start() {
		
		animation->frame = 0;
		set_animation_state(false);
		post_redraw();
	}

	void skip_to_end() {

		animation->frame = animation->frame_count();
		set_animation_state(false);
		set_animate(false);
		post_redraw();
	}

	void skip_frame(bool reverse) {

		if(reverse) {
			if(animation->frame > 0)
				animation->frame--;
		} else {
			if(animation->frame < animation->frame_count())
				animation->frame++;
		}

		set_animation_state(false);
		post_redraw();
	}

	bool set_animation_state(bool use_continuous_time) {

		animation->use_continuous_time = use_continuous_time;

		if(use_continuous_time)
			animation->frame = static_cast<size_t>(animation->timecode * animation->time);
		else
			animation->time = static_cast<float>(animation->frame) / static_cast<float>(animation->timecode);

		update_member(&animation->frame);
		update_member(&animation->time);

		bool run = apply ? animation->apply(view_ptr) : animation->frame < animation->frame_count();




		view_parameters view = animation->current_view();

		if(eye_rd.render_count() == 2) {
			eye_rd.ref_pos()[0] = view.eye_position;
			eye_rd.ref_pos()[1] = view.focus_position;
			eye_rd.set_out_of_date();
		} else {
			eye_rd.clear();
			eye_rd.add(view.eye_position, 16.0f, eye_color);
			eye_rd.add(view.focus_position, 12.0f, focus_color);
		}
	



		if(keyframe_editor_ptr)
			keyframe_editor_ptr->update();

		post_redraw();
		return run;
	}

	void write_image(const std::string& file_name) {

		std::string folder_path = cgv::utils::file::get_path(file_name);

		if(!cgv::utils::dir::exists(folder_path))
			cgv::utils::dir::mkdir(folder_path);

		if(get_context()->write_frame_buffer_to_image(file_name, cgv::data::CF_RGB, cgv::render::FB_FRONT))
			std::cout << "Successfully wrote: " << file_name << std::endl;
		else
			std::cout << "Error while writing: " << file_name << std::endl;
	}

	void write_single_image() {

		write_image("output.bmp");
	}

	void handle_editor_change() {

		create_render_data();

		update_gui_time_limits(get_max_frame_and_time());

		update_member(&animation->frame);
		set_animation_state(false);
	}

	std::pair<size_t, float> get_max_frame_and_time() {

		std::pair<size_t, float> pair;

		pair.first = 0;
		pair.second = 0.0f;

		//size_t max_frame = 0;
		//float max_time = 0.0f;
		if(animation) {
			pair.first = animation->frame_count();
			pair.second = static_cast<float>(pair.first) / static_cast<float>(animation->timecode);
		}

		return pair;
	}

	template<typename T>
	void set_control_property(T& control_value, const std::string& name, const std::string& value) {

		int idx = 0;
		auto control_ptr = find_control(control_value, &idx);

		while(control_ptr) {
			++idx;
			control_ptr->set(name, value);
			control_ptr = find_control(control_value, &idx);
		}
	}

	void update_gui_time_limits(const std::pair<size_t, float>& limits) {

		set_control_property(animation->frame, "max", std::to_string(limits.first));
		set_control_property(animation->time, "max", std::to_string(limits.second));
	}

	void create_render_data() {

		auto& theme = cgv::gui::theme_info::instance();

		keyframes_rd.clear();
		paths_rd.clear();

		if(!animation)
			return;

		for(const auto& keyframe : animation->keyframes)
			keyframes_rd.add(keyframe.second.camera_state.eye_position, 12.0f, theme.highlight());

		for(const auto& keyframe : animation->keyframes)
			keyframes_rd.add(keyframe.second.camera_state.focus_position, 8.0f, theme.warning());

		for(size_t i = 1; i < keyframes_rd.ref_pos().size() / 2; ++i) {
			const auto& position0 = keyframes_rd.ref_pos()[i - 1];
			const auto& position1 = keyframes_rd.ref_pos()[i];

			paths_rd.add(position0, position1);
			paths_rd.add(theme.highlight());
		}

		for(size_t i = 1 + keyframes_rd.ref_pos().size() / 2; i < keyframes_rd.ref_pos().size(); ++i) {
			const auto& position0 = keyframes_rd.ref_pos()[i - 1];
			const auto& position1 = keyframes_rd.ref_pos()[i];

			paths_rd.add(position0, position1);
			paths_rd.add(theme.warning());
		}
	}

public:
	class camera_animator();
	std::string get_type_name() const { return "camera_animator"; }

	void clear(cgv::render::context& ctx);

	bool self_reflect(cgv::reflect::reflection_handler& rh);
	void stream_help(std::ostream& os) {}
	void stream_stats(std::ostream& os) {}

	bool handle_event(cgv::gui::event& e);
	void handle_timer_event(double t, double dt);
	
	void on_set(const cgv::app::on_set_evaluator& m);
	bool on_exit_request();

	bool init(cgv::render::context& ctx);
	void init_frame(cgv::render::context& ctx);
	void finish_frame(cgv::render::context& ctx);
	void after_finish(cgv::render::context& ctx);

	void create_gui();
};

#include <cgv/config/lib_end.h>
