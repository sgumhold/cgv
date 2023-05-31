#include "camera_animator.h"

//#include <filesystem>
//#include <random>
//
//#include <omp.h>
//
//#include <cgv/base/find_action.h>
//#include <cgv/defines/quote.h>
//#include <cgv/gui/dialog.h>
//#include <cgv/gui/key_event.h>
//#include <cgv/gui/mouse_event.h>
//#include <cgv/gui/theme_info.h>
#include <cgv/gui/theme_info.h>
#include <cgv/gui/trigger.h>
//#include <cgv/math/ftransform.h>
//#include <cgv/media/image/image.h>
//#include <cgv/media/image/image_reader.h>
//#include <cgv/utils/advanced_scan.h>
//#include <cgv/utils/file.h>
//#include <cgv/utils/stopwatch.h>
//#include <cgv_app/color_map_reader.h>
//#include <cgv_app/color_map_writer.h>



using namespace cgv::render;

camera_animator::camera_animator() : application_plugin("Camera Animator") {

	keyframes_rd.style.measure_point_size_in_pixel = true;
	keyframes_rd.style.percentual_halo_width = 33.3f;

	paths_rd.style.measure_line_width_in_pixel = true;
	paths_rd.style.default_line_width = 2.0f;
	paths_rd.style.halo_color = rgb(1.0f);
	paths_rd.style.halo_color_strength = 0.5f;
	paths_rd.style.percentual_halo_width = -100.0f;
	paths_rd.style.blend_width_in_pixel = 1.0f;
	paths_rd.style.blend_lines = true;
	
	keyframe_editor_ptr = register_overlay<keyframe_editor_overlay>("Keyframe Editor");
	keyframe_editor_ptr->set_on_change_callback(std::bind(&camera_animator::handle_editor_change, this));
	keyframe_editor_ptr->set_visibility(show);

	connect(cgv::gui::get_animation_trigger().shoot, this, &camera_animator::handle_timer_event);

	animation = std::make_shared<animation_data>();
}

void camera_animator::clear(context& ctx) {

	keyframe_renderer.clear(ctx);
	path_renderer.clear(ctx);

	keyframes_rd.destruct(ctx);
	paths_rd.destruct(ctx);
}

bool camera_animator::self_reflect(cgv::reflect::reflection_handler& rh) {

	return false;
		//rh.reflect_member("input_path", input_path);
}

bool camera_animator::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	/*unsigned et = e.get_kind();

	if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&)e;
		cgv::gui::KeyAction ka = ke.get_action();

		/* ka is one of:
			cgv::gui::[
				KA_PRESS,
				KA_RELEASE,
				KA_REPEAT
			]
		*
		if(ka == cgv::gui::KA_PRESS) {
			unsigned short key = ke.get_key();

			bool handled = false;

			switch(ke.get_key()) {
			case 'T':
				if(transfer_function_mode == TransferFunctionMode::k4Channel) {
					if(tf_editor_ptr) {
						tf_editor_ptr->toggle_visibility();
						handled = true;
					}
				} else {
					if(m_editor_lines_ptr) {
						m_editor_lines_ptr->toggle_visibility();

						set_tree_node_visibility(m_editor_lines_ptr, m_editor_lines_ptr->is_visible());
						handled = true;
					}
				}
				break;
			case 'S':
				if(m_editor_scatterplot_ptr) {
					m_editor_scatterplot_ptr->toggle_visibility();

					set_tree_node_visibility(m_editor_scatterplot_ptr, m_editor_scatterplot_ptr->is_visible());
					handled = true;
				}
				break;
			case 'P':
				add_primitive();
				if(!is_tree_node_visible(m_shared_data_ptr)) {
					set_tree_node_visibility(m_shared_data_ptr, true);
				}
				break;
			case 'R':
				if(m_shared_data_ptr->is_primitive_selected) {
					remove_primitive(m_shared_data_ptr->selected_primitive_id);
					handled = true;
				}
				break;
			case cgv::gui::KEY_Space:
				is_peak_mode = !is_peak_mode;
				handled = true;
				break;
			default: break;
			}

			if(handled) {
				post_redraw();
				return true;
			}
		}

		return false;
	} else if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&)e;
		cgv::gui::MouseAction ma = me.get_action();

		/* ma is one of:
			cgv::gui::[
				MA_DRAG,
				MA_ENTER,
				MA_LEAVE,
				MA_MOVE,
				MA_PRESS,
				MA_RELEASE,
				MA_WHEEL
			]
		*

		return false;
	} else {
		return false;
	}*/

	return false;
}

void camera_animator::handle_timer_event(double t, double dt) {

	if(animate && !record) {
		if(set_animation_state(true)) {
			animation->time += dt;
			animation->frame = static_cast<size_t>(static_cast<float>(animation->timecode) * animation->time);
		} else {
			animate = false;
		}
	}
}

void camera_animator::on_set(const cgv::app::on_set_evaluator& m) {

	if(m.is(record)) {
		get_context()->set_gamma(record ? 1.0f : 2.2f);
		animation->use_continuous_time = !record;
	}

	if(m.is(animation->frame))
		set_animation_state(false);

	if(m.is(animation->time))
		set_animation_state(true);

	if(m.is(apply))
		set_animation_state(false);

	if(m.is(show)) {
		if(keyframe_editor_ptr)
			keyframe_editor_ptr->set_visibility(show);
	}
}

bool camera_animator::on_exit_request() {
	/*// TODO: does not seem to fire when window is maximized?
#ifndef _DEBUG
	if(fh.has_unsaved_changes) {
		return cgv::gui::question("The transfer function has unsaved changes. Are you sure you want to quit?");
	}
#endif

	save_data_set_meta_file(dataset.meta_fn);
	*/
	return true;
}

bool camera_animator::init(context& ctx) {

	bool success = true;

	success &= keyframe_renderer.init(ctx);
	success &= path_renderer.init(ctx);

	success &= keyframes_rd.init(ctx);
	success &= paths_rd.init(ctx);

	auto& theme = cgv::gui::theme_info::instance();

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
	
	if(keyframe_editor_ptr)
		keyframe_editor_ptr->set_data(animation);

	return success;
}

void camera_animator::init_frame(context& ctx) {

	if(!view_ptr && (view_ptr = find_view_as_node())) {}
}

void camera_animator::finish_frame(context& ctx) {

	if(show) {
		keyframes_rd.render(ctx, keyframe_renderer);
		paths_rd.render(ctx, path_renderer);
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
			animate = false;
	}
}

void camera_animator::create_gui() {

	add_decorator("Camera Animator", "heading", "level=2");

	auto limits = get_max_frame_and_time();

	//size_t max_frame = 0;
	//float max_time = 0.0f;
	//if(animation) {
	//	max_frame = animation->frame_count();
	//	max_time = static_cast<float>(max_frame) / static_cast<float>(animation->timecode);
	//}

	add_member_control(this, "Frame", animation->frame, "value_slider", "min=0;max=" + std::to_string(limits.first) + ";step=1");
	add_member_control(this, "Time", animation->time, "value_slider", "min=0;max=" + std::to_string(limits.second) + ";step=0.01");
	add_member_control(this, "", animation->time, "wheel", "min=0;max=4" + std::to_string(limits.second) + ";step=0.005");

	add_member_control(this, "Record", record, "check");
	add_member_control(this, "Apply", apply, "check");
	add_member_control(this, "Show", show, "check");
	connect_copy(add_button("Play Animation")->click, rebind(this, &camera_animator::play_animation));
	connect_copy(add_button("Pause Animation")->click, rebind(this, &camera_animator::pause_animation));
	connect_copy(add_button("Reset Animation")->click, rebind(this, &camera_animator::reset_animation));
	connect_copy(add_button("Print Info")->click, rebind(this, &camera_animator::print_view_information));
	
	connect_copy(add_button("Save Image")->click, rebind(this, &camera_animator::write_single_image));

	inline_object_gui(keyframe_editor_ptr);
}

#include <cgv/base/register.h>
cgv::base::object_registration<camera_animator> camera_animator_reg("");
