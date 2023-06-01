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
	
	keyframe_editor_ptr = register_overlay<keyframe_editor_overlay>("Keyframe Editor");
	keyframe_editor_ptr->set_on_change_callback(std::bind(&camera_animator::handle_editor_change, this));
	keyframe_editor_ptr->set_visibility(show);
	keyframe_editor_ptr->gui_options.create_default_tree_node = false;
	keyframe_editor_ptr->gui_options.show_layout_options = false;

	connect(cgv::gui::get_animation_trigger().shoot, this, &camera_animator::handle_timer_event);

	animation = std::make_shared<animation_data>();
}

void camera_animator::clear(context& ctx) {

	local_point_renderer.clear(ctx);
	local_line_renderer.clear(ctx);

	eye_rd.destruct(ctx);
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
			set_animate(false);
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

	success &= local_point_renderer.init(ctx);
	success &= local_line_renderer.init(ctx);

	success &= eye_rd.init(ctx);
	success &= keyframes_rd.init(ctx);
	success &= paths_rd.init(ctx);

	auto& theme = cgv::gui::theme_info::instance();
	eye_color = 0.5f * theme.highlight();
	focus_color = 0.5f * theme.warning();

	if(keyframe_editor_ptr)
		keyframe_editor_ptr->set_data(animation);

	create_render_data();

	return success;
}

void camera_animator::init_frame(context& ctx) {

	if(!view_ptr && (view_ptr = find_view_as_node())) {
		if(keyframe_editor_ptr)
			keyframe_editor_ptr->set_view(view_ptr);
	}
}

void camera_animator::finish_frame(context& ctx) {

	if(show) {
		eye_rd.render(ctx, local_point_renderer);
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

	add_member_control(this, "Show Controls", show, "check");
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

	add_member_control(this, "Record to Disk", record, "check");
	connect_copy(add_button("Save Current View")->click, rebind(this, &camera_animator::write_single_image));

	add_decorator("", "separator");

	inline_object_gui(keyframe_editor_ptr);
}

#include <cgv/base/register.h>
cgv::base::object_registration<camera_animator> camera_animator_reg("");
