#include "camera_animator.h"

#include <cgv/gui/theme_info.h>
#include <cgv/gui/trigger.h>
#include <cgv/utils/advanced_scan.h>

#include <tinyxml2.h>

using namespace cgv::render;

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

	keyframe_editor_ptr = register_overlay<keyframe_editor_overlay>("Keyframe Editor");
	keyframe_editor_ptr->set_on_change_callback(std::bind(&camera_animator::handle_editor_change, this));
	keyframe_editor_ptr->set_visibility(show_editor);
	keyframe_editor_ptr->gui_options.create_default_tree_node = false;
	keyframe_editor_ptr->gui_options.show_layout_options = false;

	connect(cgv::gui::get_animation_trigger().shoot, this, &camera_animator::handle_timer_event);

	animation = std::make_shared<animation_data>();
}

void camera_animator::clear(context& ctx) {

	local_point_renderer.clear(ctx);
	local_line_renderer.clear(ctx);

	eye_rd.destruct(ctx);
	view_rd.destruct(ctx);
	keyframes_rd.destruct(ctx);
	paths_rd.destruct(ctx);
}

bool camera_animator::self_reflect(cgv::reflect::reflection_handler& rh) {

	return false;
		//rh.reflect_member("input_path", input_path);
}

bool camera_animator::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();

	if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&)e;
		cgv::gui::KeyAction ka = ke.get_action();

		if(ka == cgv::gui::KA_PRESS) {
			unsigned short key = ke.get_key();

			switch(ke.get_key()) {
			case 'C':
				show_camera = !show_camera;
				on_set(&show_camera);
				return true;
			case 'T':
				show_editor = !show_editor;
				on_set(&show_editor);
				return true;
			case 'P':
				show_path = !show_path;
				on_set(&show_path);
				return true;
			default: break;
			}
		}

		return false;
	}

	return false;
}

void camera_animator::handle_timer_event(double t, double dt) {

	if(animate && !record) {
		if(set_animation_state(true)) {
			animation->time += static_cast<float>(dt);
			animation->frame = animation->time_to_frame();
		} else {
			set_animate(false);
		}
	}
}

void camera_animator::handle_on_set(const cgv::app::on_set_evaluator& m) {

	if(m.is(input_path)) {
		if(!load_animation(input_path))
			std::cout << "Error: Could not load animation from " << input_path << std::endl;
	}

	if(m.is(record)) {
		get_context()->set_gamma(record ? 1.0f : 2.2f);
		animation->use_continuous_time = !record;
		
		show_path = false;
		update_member(&show_path);
		show_camera = false;
		update_member(&show_camera);
		show_editor = false;
		on_set(&show_editor);
		apply = true;
		on_set(&apply);
	}

	if(m.is(animation->frame))
		set_animation_state(false);

	if(m.is(animation->time))
		set_animation_state(true);

	if(m.is(apply))
		set_animation_state(false);

	if(m.is(show_editor)) {
		if(keyframe_editor_ptr)
			keyframe_editor_ptr->set_visibility(show_editor);
	}
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

bool camera_animator::init(context& ctx) {

	bool success = true;

	success &= local_point_renderer.init(ctx);
	success &= local_line_renderer.init(ctx);

	success &= eye_rd.init(ctx);
	success &= view_rd.init(ctx);
	success &= keyframes_rd.init(ctx);
	success &= paths_rd.init(ctx);

	auto& theme = cgv::gui::theme_info::instance();
	eye_color = 0.5f * theme.highlight();
	focus_color = 0.5f * theme.warning();

	if(keyframe_editor_ptr)
		keyframe_editor_ptr->set_data(animation);

	view_parameters view;
	animation->current_view(view);
	create_camera_render_data(view);
	create_path_render_data();

	return success;
}

void camera_animator::init_frame(context& ctx) {

	if(!view_ptr && (view_ptr = find_view_as_node())) {
		if(keyframe_editor_ptr)
			keyframe_editor_ptr->set_view(view_ptr);
	}
}

void camera_animator::finish_frame(context& ctx) {

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
}

void camera_animator::after_finish(context& ctx) {

	if(view_ptr && animate && record) {
		std::string frame_number = std::to_string(animation->frame);
		if(frame_number.length() == 1)
			frame_number = "00" + frame_number;
		else if(frame_number.length() == 2)
			frame_number = "0" + frame_number;

		write_image("rec/" + frame_number + ".bmp");
		if(set_animation_state(false))
			animation->frame += 1;
		else
			set_animate(false);
	}
}

void camera_animator::create_gui() {

	add_decorator("Camera Animator", "heading", "level=2");

	std::string filter = "Camera Animation (xml):*.xml";
	add_gui("Animation File", input_path, "file_name", "title='Open Camera Animation';filter='" + filter + "';save=false;w=168;small_icon=true");

	add_member_control(this, "Show Camera", show_camera, "check");
	add_member_control(this, "Show Path", show_path, "check");
	add_member_control(this, "Show Timeline", show_editor, "check");
	add_member_control(this, "Apply Animation", apply, "check");

	add_decorator("Playback", "heading", "level=4");

	auto limits = get_max_frame_and_time();

	add_member_control(this, "Frame", animation->frame, "value_slider", "min=0;max=" + std::to_string(limits.first) + ";step=1");
	add_member_control(this, "Time", animation->time, "value_slider", "min=0;max=" + std::to_string(limits.second) + ";step=0.01");
	add_member_control(this, "", animation->time, "wheel", "min=0;max=4" + std::to_string(limits.second) + ";step=0.005");

	std::string options = "w=25;tooltip=";
	constexpr char* align = "%x+=10";

	connect_copy(add_button("@|<", options + "'Rewind to start (keep playing)'", align)->click, rebind(this, &camera_animator::skip_to_start));
	connect_copy(add_button("@square", options + "'Stop playback'", align)->click, rebind(this, &camera_animator::reset_animation));
	connect_copy(add_button("@<|", options + "'Previous frame'", align)->click, rebind(this, &camera_animator::skip_frame, cgv::signal::const_expression<bool>(true)));
	play_pause_btn = add_button(animate ? "@pause" : "@play", options + "'Play/Pause'", align);
	connect_copy(play_pause_btn->click, rebind(this, &camera_animator::toggle_animation));
	connect_copy(add_button("@|>", options + "'Next frame'", align)->click, rebind(this, &camera_animator::skip_frame, cgv::signal::const_expression<bool>(false)));
	connect_copy(add_button("@>|", options + "'Skip to end'")->click, rebind(this, &camera_animator::skip_to_end));
	
	add_decorator("", "separator");

	
	


	// TODO: add field to define output folder
	//add_member_control(this, "Output Path", output_path, "string");
	add_member_control(this, "Record to Disk", record, "check");
	connect_copy(add_button("Save Current View")->click, rebind(this, &camera_animator::write_single_image));

	add_decorator("", "separator");

	inline_object_gui(keyframe_editor_ptr);
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
	bool run = animation->current_view(view);

	if(run && apply)
		view.apply(view_ptr);

	create_camera_render_data(view);

	if(keyframe_editor_ptr)
		keyframe_editor_ptr->update();

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
		eye_rd.ref_pos()[0] = view.eye_position;
		eye_rd.ref_pos()[1] = view.focus_position;
		eye_rd.set_out_of_date();
	} else {
		eye_rd.clear();
		eye_rd.add(view.eye_position, 16.0f, eye_color);
		eye_rd.add(view.focus_position, 12.0f, focus_color);
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
		view_rd.add(rgb(1.0f, 0.0f, 0.0f));
		view_rd.add(rgb(0.0f, 1.0f, 0.0f));
		view_rd.add(rgb(0.0f, 0.0f, 1.0f));

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

		view_rd.fill(rgb(0.5f));
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

void camera_animator::handle_editor_change() {

	create_path_render_data();

	udpate_animation_member_limits();

	update_member(&animation->frame);
	set_animation_state(false);
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
			//std::string timecode_str = root->Attribute("timecode");

			int timecode = 30;
			root->QueryIntAttribute("timecode", &timecode);

			cgv::math::clamp(timecode, 1, 120);

			if(auto keyframe_elements = root->FirstChildElement("Keyframes")) {
				auto keyframe_element = keyframe_elements->FirstChildElement();

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
						} else {
							// TODO: error message
						}
					}

					//std::cout << keyframe_element->Attribute("frame") << std::endl;
					keyframe_element = keyframe_element->NextSiblingElement();
				}
				/*if(auto timecode = properties->FirstChildElement("Timecode")) {
					std::cout << "Timecode: " << timecode->Name() << std::endl;
					std::cout << "Timecode: " << timecode->GetText() << std::endl;
				}*/

				success = true;
			}
		}
	} else {
		//std::cout << "Could not load xml" << std::endl;
	}

	if(success) {
		animation->timecode = timecode;
		animation->keyframes.clear();
		
		for(auto tuple : keyframes) {
			keyframe k;
			k.ease(easing_functions::to_id(std::get<1>(tuple)));
			k.camera_state = std::get<2>(tuple);
			animation->keyframes.insert(std::get<0>(tuple), k);
		}
	} else {
		animation->keyframes.clear();
	}

	udpate_animation_member_limits();

	reset_animation();
	create_path_render_data();

	if(keyframe_editor_ptr)
		keyframe_editor_ptr->update();

	return success;
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

// TODO: use specified output path (plus frame number if animation is defined)
void camera_animator::write_single_image() {

	write_image("output.bmp");
}

#include <cgv/base/register.h>
cgv::base::object_registration<camera_animator> camera_animator_reg("");
