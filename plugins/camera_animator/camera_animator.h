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

	cgv::render::point_renderer keyframe_renderer;
	cgv::render::line_renderer path_renderer;

	cgv::render::point_render_data<> keyframes_rd;
	cgv::render::line_render_data<> paths_rd;

	std::shared_ptr<animation_data> animation;

	bool animate = false;
	bool record = false;
	bool apply = false;
	bool show = false;

	void reset_animation() {

		animation->reset();
		animate = false;

		set_animation_state(false);

		post_redraw();
	}

	void play_animation() {

		animate = true;
		post_redraw();
	}

	void pause_animation() {

		animate = false;
		post_redraw();
	}

	void print_view_information() {
		
		std::cout << "View:" << std::endl;
		std::cout << "Eye    = " << view_ptr->get_eye() << std::endl;
		std::cout << "Focus  = " << view_ptr->get_focus() << std::endl;
		std::cout << "Up Dir = " << view_ptr->get_view_up_dir() << std::endl;
	}

	bool set_animation_state(bool use_continuous_time) {

		animation->use_continuous_time = use_continuous_time;
		// TODO: replace true with get valid frame from animation
		bool run = apply ? animation->apply(view_ptr) : true;

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

		set_animation_state(false);
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
