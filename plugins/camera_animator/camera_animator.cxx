#include "camera_animator.h"

#include <cgv/gui/theme_info.h>
#include <cgv/gui/trigger.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/os/cmdline_tools.h>
#include <tinyxml2/tinyxml2.h>

#include <stdio.h>

using namespace cgv::render;

bool camera_animator::open_ffmpeg_pipe(const std::string& file_name)
{
	auto* ctx_ptr = get_context();
	if (ctx_ptr == 0)
		return false;
	int w = ctx_ptr->get_width(), h = ctx_ptr->get_height();

	std::string input_str = "-";
	std::string cmd_begin = "ffmpeg -y -f rawvideo -s " + 
		cgv::utils::to_string(w) + "x" + cgv::utils::to_string(h) +
		" -framerate " + cgv::utils::to_string(fps) + " -pix_fmt rgb24 -i ";
	std::string cmd_end   = " -c:v libx264 -shortest " + file_name;

	if (use_named_pipe) {
		named_thread_ptr = new cgv::os::named_pipe_output_thread("video_pipe");
		std::string cmd = cmd_begin + named_thread_ptr->get_pipe_path() + cmd_end;
		std::cout << "Cmd: " << cmd << std::endl;
		named_thread_ptr->start();
		fp = cgv::os::open_system_input(cmd);
		return fp != 0;
	}
	else {
		std::string cmd = cmd_begin + "-" + cmd_end;
		std::cout << "Cmd: " << cmd << std::endl;
		thread_ptr = new cgv::os::pipe_output_thread(cmd);
		thread_ptr->start();
		return true;
	}
}

bool camera_animator::write_image_to_ffmpeg_pipe()
{
	auto* ctx_ptr = get_context();
	if (ctx_ptr == 0)
		return false;
	
	cgv::data::data_view dv;
	ctx_ptr->read_frame_buffer(dv);
	auto* fmt_ptr = dv.get_format();
	cgv::os::queued_output_thread* queued_thread_ptr;
	if (use_named_pipe)
		queued_thread_ptr = named_thread_ptr;
	else
		queued_thread_ptr = thread_ptr;

	if (queued_thread_ptr->has_connection()) {
		queued_thread_ptr->send_block(dv.get_ptr<char>(), fmt_ptr->get_width() * fmt_ptr->get_height() * 3);
		nr_blocks = queued_thread_ptr->get_nr_blocks();
	}
	else {
		std::cout << "no connection" << std::endl;
		nr_blocks = 0;
	}
	update_member(&nr_blocks);
	return true;
}

bool camera_animator::close_ffmpeg_pipe()
{
	cgv::os::queued_output_thread* queued_thread_ptr;
	if (use_named_pipe)
		queued_thread_ptr = named_thread_ptr;
	else
		queued_thread_ptr = thread_ptr;

	if (queued_thread_ptr) {
		queued_thread_ptr->done();
		std::cout << "wait_for_completion" << std::endl;
		queued_thread_ptr->wait_for_completion();
	}
	int result;
	if (use_named_pipe) {
		std::cout << "wait_for_pipe_closure" << std::endl;
		result = cgv::os::close_system_input(fp);
		delete named_thread_ptr;
		named_thread_ptr = 0;
	}
	else {
		result = thread_ptr->get_result();
		delete thread_ptr;
		thread_ptr = 0;
	}
	nr_blocks = 0;
	update_member(&nr_blocks);
	std::cout << "ffmpeg returned " << result << std::endl;
	return true;
}

camera_animator::camera_animator() : application_plugin("Camera Animator") {

	eye_rd.style.measure_point_size_in_pixel = true;
	eye_rd.style.percentual_halo_width = 33.3f;

	keyframes_rd.style = eye_rd.style;
	
	paths_rd.style.measure_line_width_in_pixel = true;
	paths_rd.style.default_line_width = 2.0f;
	paths_rd.style.halo_color = rgb(1.0f);
	paths_rd.style.halo_color_strength = 0.5f;
	paths_rd.style.percentual_halo_width = -100.0f;
	paths_rd.style.blend_width_in_pixel = 1.0f;
	paths_rd.style.blend_lines = true;

	view_rd.style = paths_rd.style;
	view_rd.style.default_line_width = 1.0f;
	view_rd.style.percentual_halo_width = 0.0f;
	
	view_transformation.identity();

	eye_gizmo.set_move_callback(std::bind(&camera_animator::handle_eye_gizmo_move, this));
	eye_gizmo.set_scale(0.5f);

	focus_gizmo.set_move_callback(std::bind(&camera_animator::handle_focus_gizmo_move, this));
	focus_gizmo.set_scale(0.5f);

	timeline_ptr = register_overlay<keyframe_editor_overlay>("Keyframe Editor");
	timeline_ptr->set_on_change_callback(std::bind(&camera_animator::handle_editor_change, this, std::placeholders::_1));
	timeline_ptr->gui_options.create_default_tree_node = false;
	timeline_ptr->gui_options.show_layout_options = false;

	connect(cgv::gui::get_animation_trigger().shoot, this, &camera_animator::handle_timer_event);

	animation = std::make_shared<animation_data>();

	input_file_helper = cgv::gui::file_helper(this, "Open/Save Camera Animation", cgv::gui::file_helper::Mode::kOpenAndSave);
	input_file_helper.add_filter("Camera Animation", "xml");

	output_directory_helper = cgv::gui::directory_helper(this, "Select Output Folder", cgv::gui::directory_helper::Mode::kOpen);
	output_directory_helper.directory_name = "./output";
	
	help.add_line("Keybindings:");
	help.add_bullet_point("A : Toggle apply animation to camera");
	help.add_bullet_point("C : Toggle camera visibility");
	help.add_bullet_point("P : Toggle camera path visibility");
	help.add_bullet_point("R : Toggle record mode");
	help.add_bullet_point("Spacebar : Play/pause animation");
}

void camera_animator::clear(context& ctx) {

	eye_gizmo.destruct(ctx);
	focus_gizmo.destruct(ctx);

	local_point_renderer.clear(ctx);
	local_line_renderer.clear(ctx);

	eye_rd.destruct(ctx);
	view_rd.destruct(ctx);
	keyframes_rd.destruct(ctx);
	paths_rd.destruct(ctx);
}

bool camera_animator::self_reflect(cgv::reflect::reflection_handler& rh) {

	return 
		rh.reflect_member("input_path", input_file_helper.file_name) &&
		rh.reflect_member("video_file", video_file_name);
}

bool camera_animator::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();

	auto& ctx = *get_context();

	if(selected_keyframe) {
		if(eye_gizmo.handle(e, ctx) || focus_gizmo.handle(e, ctx)) {
			post_redraw();
			return true;
		}
	}

	if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		if(me.get_button() == cgv::gui::MB_LEFT_BUTTON && ma == cgv::gui::MA_PRESS) {
			if(view_ptr) {
				ivec2 viewport_size(ctx.get_width(), ctx.get_height());
				ivec2 mpos(
					static_cast<int>(me.get_x()),
					viewport_size.y() - static_cast<int>(me.get_y()) - 1
				);

				cgv::math::ray3 ray(
					static_cast<vec2>(mpos),
					static_cast<vec2>(viewport_size),
					view_ptr->get_eye(),
					ctx.get_projection_matrix() * ctx.get_modelview_matrix()
				);

				// TODO: implement picking
			}
		}

	} else if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&)e;
		cgv::gui::KeyAction ka = ke.get_action();

		if(ka == cgv::gui::KA_PRESS) {
			unsigned short key = ke.get_key();
			if (ke.get_modifiers() == 0) {
				switch (ke.get_key()) {
				case 'A':
					if(!record) {
						apply = !apply;
						on_set(&apply);
						return true;
					}
					break;
				case 'C':
					if(!record) {
						show_camera = !show_camera;
						on_set(&show_camera);
						return true;
					}
					break;
				case 'P':
					if(!record) {
						show_path = !show_path;
						on_set(&show_path);
						return true;
					}
					break;
				case 'R':
					record = !record;
					on_set(&record);
					return true;
				case 'V':
					video_open = !video_open;
					on_set(&video_open);
					return true;
				case cgv::gui::KEY_Space:
					toggle_animation();
					return true;
				default: break;
				}
			}
		}
		return false;
	}
	return false;
}

void camera_animator::handle_timer_event(double t, double dt) {

	if (thread_ptr) {
		size_t new_nr_blocks = thread_ptr->get_nr_blocks();
		if (new_nr_blocks != nr_blocks) {
			nr_blocks = new_nr_blocks;
			update_member(&nr_blocks);
		}
	}

	if(animate && !record) {
		if(set_animation_state(true)) {
			animation->time += static_cast<float>(dt);
			animation->frame = animation->time_to_frame(animation->time);
		} else {
			set_animate(false);
		}
	}
}

void camera_animator::handle_member_change(const cgv::utils::pointer_test& m) {
	if (m.is(video_open)) {
		if (video_open) {
			if (!video_file_name.empty())
				open_ffmpeg_pipe(video_file_name);
			else {
				video_open = false;
				update_member(&video_open);
				cgv::gui::message("Choose video file before opening.");
			}
		}
		else {
			close_ffmpeg_pipe();
		}
	}
	if(m.is(input_file_helper.file_name)) {
		const std::string& file_name = input_file_helper.file_name;
		if(input_file_helper.save()) {
			// force the file name to have a xml extension if not already present
			input_file_helper.ensure_extension("xml", true);

			if(save_animation(file_name)) {
				input_file_helper.update();
				// TODO: implement note on unsaved changes
				//has_unsaved_changes = false;
				//on_set(&has_unsaved_changes);
			} else {
				std::cout << "Error: Could not write animation to " << file_name << std::endl;
			}
		} else {
			if(!load_animation(file_name))
				std::cout << "Error: Could not load animation from " << file_name << std::endl;
		}
	}

	if(m.is(record)) {
		get_context()->set_gamma(record ? 1.0f : 2.2f);
		animation->use_continuous_time = !record;
		
		if(timeline_ptr)
			timeline_ptr->set_visibility(!record);

		std::string active = record ? "false" : "true";
		set_control_property(show_camera, "active", active);
		set_control_property(show_path, "active", active);
		set_control_property(apply, "active", active);
		
		set_animation_state(false);
	}

	if(m.is(animation->frame))
		set_animation_state(false);

	if(m.is(animation->time))
		set_animation_state(true);

	if(m.is(apply))
		set_animation_state(false);
}

bool camera_animator::on_exit_request() {
	/*
	if(fh.has_unsaved_changes) {
		return cgv::gui::question("The transfer function has unsaved changes. Are you sure you want to quit?");
	}

	save_data_set_meta_file(dataset.meta_fn);
	*/
	return true;
}

void camera_animator::on_select() {

	show();
}

void camera_animator::on_deselect() {

	hide();
}

bool camera_animator::init(context& ctx) {

	bool success = true;

	success &= eye_gizmo.init(ctx);
	success &= focus_gizmo.init(ctx);

	success &= local_point_renderer.init(ctx);
	success &= local_line_renderer.init(ctx);

	success &= eye_rd.init(ctx);
	success &= view_rd.init(ctx);
	success &= keyframes_rd.init(ctx);
	success &= paths_rd.init(ctx);

	auto& theme = cgv::gui::theme_info::instance();
	eye_color = 0.5f * theme.highlight();
	focus_color = 0.5f * theme.warning();

	if(timeline_ptr)
		timeline_ptr->set_data(animation);

	create_path_render_data();

	return success;
}

void camera_animator::init_frame(context& ctx) {

	if(!view_ptr && (view_ptr = find_view_as_node())) {
		eye_gizmo.set_view_ptr(view_ptr);
		focus_gizmo.set_view_ptr(view_ptr);

		if(timeline_ptr)
			timeline_ptr->set_view_ptr(view_ptr);

		view_parameters view;
		animation->current_view(view);
		create_camera_render_data(view);
	}
}

void camera_animator::finish_frame(context& ctx) {

	if(record)
		return;

	if(show_camera) {
		eye_rd.render(ctx, local_point_renderer);

		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(view_transformation);
		view_rd.render(ctx, local_line_renderer);
		ctx.pop_modelview_matrix();
	}

	if(show_path) {
		keyframes_rd.render(ctx, local_point_renderer);
		paths_rd.render(ctx, local_line_renderer);
	}

	if(selected_keyframe) {
		eye_gizmo.draw(ctx);
		focus_gizmo.draw(ctx);
	}
}

void camera_animator::after_finish(context& ctx) {

	if(view_ptr && animate && record) {
		std::string frame_number = std::to_string(animation->frame);
		if(frame_number.length() == 1)
			frame_number = "00" + frame_number;
		else if(frame_number.length() == 2)
			frame_number = "0" + frame_number;
		if (fp)
			write_image_to_ffmpeg_pipe();
		else
			write_image(output_directory_helper.directory_name + "/" + frame_number + ".bmp");
		if(set_animation_state(false))
			animation->frame += 1;
		else
			set_animate(false);
	}
}

void camera_animator::create_gui() {

	add_decorator("Camera Animator", "heading", "level=2");
	help.create_gui(this);

	input_file_helper.create_gui("Animation File");
	add_member_control(this, "Show Camera", show_camera, "check");
	add_member_control(this, "Show Path", show_path, "check");
	add_member_control(this, "Apply Animation", apply, "check");

	add_decorator("Playback", "heading", "level=4");

	auto limits = get_max_frame_and_time();

	add_member_control(this, "Frame", animation->frame, "value_slider", "min=0;max=" + std::to_string(limits.first) + ";step=1");
	add_member_control(this, "Time", animation->time, "value_slider", "min=0;max=" + std::to_string(limits.second) + ";step=0.01");
	add_member_control(this, "", animation->time, "wheel", "min=0;max=4" + std::to_string(limits.second) + ";step=0.005");

	std::string options = "w=25;tooltip=";
	constexpr auto align = "%x+=10";

	connect_copy(add_button("@|<", options + "'Rewind to start (keep playing)'", align)->click, rebind(this, &camera_animator::skip_to_start));
	connect_copy(add_button("@square", options + "'Stop playback'", align)->click, rebind(this, &camera_animator::reset_animation));
	connect_copy(add_button("@<|", options + "'Previous frame'", align)->click, rebind(this, &camera_animator::skip_frame, cgv::signal::const_expression<bool>(true)));
	play_pause_btn = add_button(animate ? "@pause" : "@play", options + "'Play/Pause'", align);
	connect_copy(play_pause_btn->click, rebind(this, &camera_animator::toggle_animation));
	connect_copy(add_button("@|>", options + "'Next frame'", align)->click, rebind(this, &camera_animator::skip_frame, cgv::signal::const_expression<bool>(false)));
	connect_copy(add_button("@>|", options + "'Skip to end'")->click, rebind(this, &camera_animator::skip_to_end));
	
	add_decorator("", "separator");
	add_decorator("Recording", "heading", "level=4");

	output_directory_helper.create_gui("Output Folder");
	add_gui("Video File", video_file_name, "file_name", "save=true;open=false;title='Save video file';filter='video (mp4):*.mp4|all files:*.*';w=170");
	add_member_control(this, "Use Named Pipe", use_named_pipe, "toggle");
	add_member_control(this, "Video Open", video_open, "toggle");
	add_view("Nr Queued Frames", nr_blocks);
	add_member_control(this, "Record to Disk", record, "check");
	connect_copy(add_button("Save Current View")->click, rebind(this, &camera_animator::write_single_image));

	add_decorator("", "separator");

	inline_object_gui(timeline_ptr);
}

void camera_animator::set_animate(bool flag) {

	animate = flag;
	play_pause_btn->set_name(animate ? "@pause" : "@play");
	play_pause_btn->update();
	post_redraw();
}

void camera_animator::reset_animation() {

	animation->reset();
	set_animate(false);
	set_animation_state(false);
}

void camera_animator::toggle_animation() {

	set_animate(!animate);
}

void camera_animator::skip_to_start() {

	animation->frame = 0;
	set_animation_state(false);
}

void camera_animator::skip_to_end() {

	animation->frame = animation->frame_count();
	set_animation_state(false);
	set_animate(false);
}

void camera_animator::skip_frame(bool reverse) {

	if(reverse) {
		if(animation->frame > 0)
			animation->frame--;
	} else {
		if(animation->frame < animation->frame_count())
			animation->frame++;
	}

	set_animation_state(false);
}

bool camera_animator::set_animation_state(bool use_continuous_time) {

	animation->use_continuous_time = use_continuous_time;

	if(use_continuous_time)
		animation->frame = static_cast<size_t>(animation->timecode * animation->time);
	else
		animation->time = static_cast<float>(animation->frame) / static_cast<float>(animation->timecode);

	update_member(&animation->frame);
	update_member(&animation->time);

	view_parameters view;
	bool run = animation->current_view(view) && animation->frame < animation->frame_count();

	if(run && apply || record)
		view.apply(view_ptr);

	create_camera_render_data(view);

	if(timeline_ptr)
		timeline_ptr->update();

	post_redraw();
	return run;
}

std::pair<size_t, float> camera_animator::get_max_frame_and_time() {

	if(animation)
		return { animation->frame_count(), animation->duration() };
	return { 0ull, 0.0f };
}

void camera_animator::udpate_animation_member_limits() {

	auto limits = get_max_frame_and_time();

	set_control_property(animation->frame, "max", std::to_string(limits.first));
	set_control_property(animation->time, "max", std::to_string(limits.second));
}

void camera_animator::create_camera_render_data(const view_parameters& view) {

	if(!view_ptr)
		return;

	if(eye_rd.render_count() == 2) {
		eye_rd.positions[0] = view.eye_position;
		eye_rd.positions[1] = view.focus_position;
		eye_rd.set_out_of_date();
	} else {
		eye_rd.clear();
		eye_rd.add(view.eye_position, eye_color, 16.0f);
		eye_rd.add(view.focus_position, focus_color, 12.0f);
	}

	if(!view_rd.render_count()) {
		view_rd.clear();
		const vec3 org(0.0f);
		const vec3 x_axis(1.0f, 0.0f, 0.0f);
		const vec3 y_axis(0.0f, 1.0f, 0.0f);
		const vec3 z_axis(0.0f, 0.0f, 1.0f);

		view_rd.add(org, x_axis);
		view_rd.add(org, y_axis);
		view_rd.add(org, z_axis);
		view_rd.add_segment_color(rgb(1.0f, 0.0f, 0.0f));
		view_rd.add_segment_color(rgb(0.0f, 1.0f, 0.0f));
		view_rd.add_segment_color(rgb(0.0f, 0.0f, 1.0f));

		float a = static_cast<float>(view_ptr->get_tan_of_half_of_fovy(true));

		vec3 corner[4];
		corner[0] = z_axis - a * x_axis - a * y_axis;
		corner[1] = z_axis - a * x_axis + a * y_axis;
		corner[2] = z_axis + a * x_axis - a * y_axis;
		corner[3] = z_axis + a * x_axis + a * y_axis;

		view_rd.add(org, corner[0]);
		view_rd.add(org, corner[1]);
		view_rd.add(org, corner[2]);
		view_rd.add(org, corner[3]);

		view_rd.add(corner[0], corner[1]);
		view_rd.add(corner[1], corner[3]);
		view_rd.add(corner[3], corner[2]);
		view_rd.add(corner[2], corner[0]);

		view_rd.fill_colors(rgb(0.5f));
	}

	const float scale = 0.1f;

	view_transformation.identity();
	view_transformation.set_col(0, scale * vec4(view.side_direction(), 0.0f));
	view_transformation.set_col(1, scale * vec4(view.up_direction, 0.0f));
	view_transformation.set_col(2, scale * vec4(view.view_direction(), 0.0f));
	view_transformation.set_col(3, vec4(view.eye_position, 1.0f));
}

void camera_animator::create_path_render_data() {

	auto& theme = cgv::gui::theme_info::instance();

	keyframes_rd.clear();
	paths_rd.clear();

	if(!animation)
		return;

	for(const auto& pair : animation->ref_keyframes())
		keyframes_rd.add(pair.second.camera_state.eye_position, theme.highlight(), 12.0f);
	
	for(const auto& pair : animation->ref_keyframes())
		keyframes_rd.add(pair.second.camera_state.focus_position, theme.warning(), 8.0f);
	
	for(size_t i = 1; i < keyframes_rd.size() / 2; ++i) {
		const auto& position0 = keyframes_rd.positions[i - 1];
		const auto& position1 = keyframes_rd.positions[i];

		paths_rd.add(position0, position1);
		paths_rd.add_color(theme.highlight());
	}

	for(size_t i = 1 + keyframes_rd.size() / 2; i < keyframes_rd.size(); ++i) {
		const auto& position0 = keyframes_rd.positions[i - 1];
		const auto& position1 = keyframes_rd.positions[i];

		paths_rd.add(position0, position1);
		paths_rd.add_color(theme.warning());
	}
}

void camera_animator::handle_eye_gizmo_move() {

	if(selected_keyframe) {
		selected_keyframe->camera_state.eye_position = eye_gizmo.get_position();
		create_path_render_data();
		set_animation_state(false);
	}
}

void camera_animator::handle_focus_gizmo_move() {

	if(selected_keyframe) {
		selected_keyframe->camera_state.focus_position = focus_gizmo.get_position();
		create_path_render_data();
		set_animation_state(false);
	}
}

void camera_animator::handle_editor_change(keyframe_editor_overlay::Event e) {

	switch(e) {
	case keyframe_editor_overlay::Event::kTimeChange:
		set_animation_state(false);
		break;
	case keyframe_editor_overlay::Event::kKeyMove:
		udpate_animation_member_limits(); // no break
	case keyframe_editor_overlay::Event::kKeyCreate:
	case keyframe_editor_overlay::Event::kKeyDelete:
	case keyframe_editor_overlay::Event::kKeyChange:
		create_path_render_data();
		set_animation_state(false);
		break;
	case keyframe_editor_overlay::Event::kKeySelect:
		if(timeline_ptr) {
			if(/**/(selected_keyframe = animation->keyframe_at(timeline_ptr->get_selected_frame()))/**/) {
				eye_gizmo.set_position(selected_keyframe->camera_state.eye_position);
				focus_gizmo.set_position(selected_keyframe->camera_state.focus_position);
			}
		}
		break;
	case keyframe_editor_overlay::Event::kKeyDeselect:
		selected_keyframe = nullptr;
		break;
	default: break;
	}
}

bool camera_animator::load_animation(const std::string& file_name) {

	auto str2vec3 = [](const std::string& str, vec3& val) {
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(str, tokens, "", true, "", "", ",");

		if(tokens.size() != 3)
			return false;

		vec3 v(0.0f);

		if(!cgv::utils::from_string(v[0], to_string(tokens[0])))
			return false;
		if(!cgv::utils::from_string(v[1], to_string(tokens[1])))
			return false;
		if(!cgv::utils::from_string(v[2], to_string(tokens[2])))
			return false;

		val = v;
		return true;
	};

	bool success = false;
	int timecode = 30;
	std::vector<std::tuple<int, std::string, view_parameters>> keyframes;

	tinyxml2::XMLDocument doc;
	if(doc.LoadFile(file_name.c_str()) == tinyxml2::XML_SUCCESS) {
		auto root = doc.RootElement();
		if(root && strcmp(root->Name(), "CameraAnimation") == 0) {
			int timecode = 30;
			root->QueryIntAttribute("timecode", &timecode);

			cgv::math::clamp(timecode, 1, 120);

			if(auto keyframes_element = root->FirstChildElement("Keyframes")) {
				auto keyframe_element = keyframes_element->FirstChildElement();

				while(keyframe_element) {
					int frame = -1;
					keyframe_element->QueryIntAttribute("frame", &frame);

					if(frame > -1) {
						std::string ease = keyframe_element->Attribute("ease");
						std::string eye_str = keyframe_element->Attribute("eye");
						std::string focus_str = keyframe_element->Attribute("focus");
						std::string up_str = keyframe_element->Attribute("up");

						bool success = true;
						view_parameters view;
						success &= str2vec3(eye_str, view.eye_position);
						success &= str2vec3(focus_str, view.focus_position);
						success &= str2vec3(up_str, view.up_direction);

						if(success) {
							keyframes.push_back({ frame, ease, view });
						}
					}

					keyframe_element = keyframe_element->NextSiblingElement();
				}
				
				success = true;
			}
		}
	}

	animation->clear();

	if(success) {
		animation->timecode = timecode;
		
		for(auto tuple : keyframes) {
			keyframe k;
			k.ease(easing_functions::to_id(std::get<1>(tuple)));
			k.camera_state = std::get<2>(tuple);
			animation->insert_keyframe(std::get<0>(tuple), k);
		}
	}

	udpate_animation_member_limits();

	reset_animation();
	create_path_render_data();

	if(timeline_ptr)
		timeline_ptr->update();

	return success;
}

bool camera_animator::save_animation(const std::string& file_name) {

	auto vec32str = [](vec3& val) {
		return std::to_string(val[0]) + ", " + std::to_string(val[1]) + ", " + std::to_string(val[2]);
	};

	tinyxml2::XMLDocument doc;

	auto root = doc.NewElement("CameraAnimation");
	root->SetAttribute("timecode", animation->timecode);
	doc.InsertFirstChild(root);

	auto keyframes_element = doc.NewElement("Keyframes");
	root->InsertEndChild(keyframes_element);

	for(const auto& pair : animation->ref_keyframes()) {
		keyframe k = pair.second;

		auto keyframe_element = doc.NewElement("Keyframe");
		keyframe_element->SetAttribute("frame", pair.first);
		keyframe_element->SetAttribute("ease", easing_functions::to_string(k.easing_id()).c_str());
		keyframe_element->SetAttribute("eye", vec32str(k.camera_state.eye_position).c_str());
		keyframe_element->SetAttribute("focus", vec32str(k.camera_state.focus_position).c_str());
		keyframe_element->SetAttribute("up", vec32str(k.camera_state.up_direction).c_str());

		keyframes_element->InsertEndChild(keyframe_element);
	}

	return doc.SaveFile(file_name.c_str()) == tinyxml2::XML_SUCCESS;
}

void camera_animator::write_image(const std::string& file_name) {

	std::string folder_path = cgv::utils::file::get_path(file_name);

	if(!cgv::utils::dir::exists(folder_path))
		cgv::utils::dir::mkdir(folder_path);

	std::cout << "Writing " << file_name << " ...";
	if(get_context()->write_frame_buffer_to_image(file_name, cgv::data::CF_RGB, cgv::render::FB_FRONT))
		std::cout << "OK" << std::endl;
	else
		std::cout << "Error" << std::endl;
}

void camera_animator::write_single_image() {

	write_image(output_directory_helper.directory_name + "/frame" + std::to_string(animation->frame) + ".bmp");
}

#include <cgv/base/register.h>
cgv::base::object_registration<camera_animator> camera_animator_reg("");
