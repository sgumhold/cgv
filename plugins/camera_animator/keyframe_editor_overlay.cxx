#include "keyframe_editor_overlay.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/convert_string.h>

using namespace cgv::render;

keyframe_editor_overlay::keyframe_editor_overlay() {

	set_name("Keyframe Editor");
	block_events = true;
	gui_options.allow_stretch = false;
	gui_options.allow_margin = false;
	
	set_stretch(SO_HORIZONTAL);
	set_size(ivec2(100, layout.total_height(padding())));

	scrollbar.set_drag_callback(std::bind(&keyframe_editor_overlay::handle_scrollbar_drag, this));
	marker.set_drag_callback(std::bind(&keyframe_editor_overlay::handle_marker_drag, this));
	keyframes.set_drag_callback(std::bind(&keyframe_editor_overlay::handle_keyframe_drag, this));
	keyframes.set_drag_end_callback(std::bind(&keyframe_editor_overlay::handle_keyframe_drag_end, this));
	keyframes.set_selection_change_callback(std::bind(&keyframe_editor_overlay::handle_keyframe_selection_change, this));

	help.add_bullet_point("Click on a keyframe to select it.");
	help.add_bullet_point("Drag a keyframe to change its time. Dragging onto another keyframe will revert the drag.");
	help.add_bullet_point("Click \"Set\" to add a new keyframe or change the existing keyframe at the current time.");
	help.add_bullet_point("Press [Del] or Click \"Remove Selected\" to remove the selected keyframe.");

	line_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::rectangle);
}

void keyframe_editor_overlay::clear(context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);

	themed_canvas_overlay::clear(ctx);

	line_renderer.destruct(ctx);
	labels.destruct(ctx);
}

bool keyframe_editor_overlay::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();

	cgv::g2d::irect container = get_rectangle();
	container.x() -= layout.timeline_offset;

	if(keyframes.handle(e, get_viewport_size(), container))
		return true;

	if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&) e;
		cgv::gui::KeyAction ka = ke.get_action();

		switch(ke.get_key()) {
		case cgv::gui::KEY_Delete:
			if(ka == cgv::gui::KA_PRESS) {
				if(selected_frame != -1) {
					erase_selected_keyframe();
					post_damage();
					return true;
				}
			}
			break;
		default: break;
		}
	} else if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		if(me.get_button() == cgv::gui::MB_LEFT_BUTTON && ma == cgv::gui::MA_PRESS) {
			ivec2 local_mouse_position = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()));

			if(layout.marker_constraint.contains(local_mouse_position))
				set_frame(position_to_frame(local_mouse_position.x() + layout.timeline_offset));
			
			if(!scrollbar.empty()) {
				auto& handle = scrollbar[0];

				if(layout.scrollbar_constraint.contains(local_mouse_position) && !handle.contains(local_mouse_position)) {
					handle.position = local_mouse_position.x() - 0.5f * handle.w();
					handle.apply_constraint(layout.scrollbar_constraint);

					set_timeline_offset();
				}
			}

			post_damage();
		}

		if(ma == cgv::gui::MA_WHEEL) {
			if(!scrollbar.empty()) {
				auto& handle = scrollbar[0];

				float frames_per_pixel = static_cast<float>(layout.timeline_frames) / layout.scrollbar_constraint.w();

				handle.position += 5.0f * me.get_dy() / frames_per_pixel;
				handle.apply_constraint(layout.scrollbar_constraint);

				set_timeline_offset();
				post_damage();
				return true;
			}
		}

		if(scrollbar.handle(e, get_viewport_size(), get_rectangle()))
			return true;
		
		if(marker.handle(e, get_viewport_size(), container))
			return true;
	}
	
	return false;
}

void keyframe_editor_overlay::handle_member_change(const cgv::utils::pointer_test& m) {

	if(m.is(easing_function_id)) {
		if(data) {
			if(keyframe* k = data->keyframe_at(selected_frame)) {
				k->ease(easing_function_id);

				invoke_callback(Event::kKeyChange);
			}
		}
	}

	if(m.is(new_frame_count)) {
		new_frame_count = cgv::math::clamp((size_t)new_frame_count, (size_t)0, (size_t)9000);
		if(data) {
			new_duration = data->frame_to_time(new_frame_count);
			update_member(&new_duration);
		}
	}

	if(m.is(new_duration)) {
		new_duration = cgv::math::clamp(new_duration, 0.0f, 300.0f);
		if(data) {
			new_frame_count = data->time_to_frame(new_duration);
			update_member(&new_frame_count);
		}
	}
}

bool keyframe_editor_overlay::init(context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);

	register_shader("background", cgv::g2d::shaders::grid);
	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("circle", cgv::g2d::shaders::circle);
	register_shader("arrow", cgv::g2d::shaders::arrow);
	
	bool success = themed_canvas_overlay::init(ctx);
	success &= line_renderer.init(ctx);
	success &= labels.init(ctx);
	
	return success;
}

void keyframe_editor_overlay::init_frame(context& ctx) {

	if(ensure_layout(ctx)) {
		layout.update(get_rectangle().size, padding(), data ? data->frame_count() : 0);

		if(scrollbar.empty()) {
			cgv::g2d::draggable handle;
			handle.position = static_cast<vec2>(layout.scrollbar_constraint.position);
			handle.size = vec2(16.0f, static_cast<float>(layout.scrollbar_height));
			scrollbar.add(handle);
		}

		scrollbar[0].w() = std::max(
			layout.scrollbar_constraint.w() * static_cast<float>(layout.scrollbar_constraint.w()) / static_cast<float>(layout.timeline.w()),
			24.0f);

		scrollbar.set_constraint(layout.scrollbar_constraint);

		if(marker.empty()) {
			cgv::g2d::draggable handle;
			handle.position = static_cast<vec2>(layout.marker_constraint.position);
			handle.size = vec2(static_cast<float>(layout.marker_height), static_cast<float>(layout.marker_width));
			marker.add(handle);
		}

		marker.set_constraint(layout.marker_constraint);

		labels.clear();
		labels.add_text("0", vec2(0.0f), TA_BOTTOM);

		for(size_t i = 0; i <= layout.timeline_frames; ++i) {
			if(i % 5 == 0) {
				vec2 position = vec2(
					static_cast<float>(padding() + layout.frame_width * i + layout.frame_width / 2),
					static_cast<float>(layout.total_height(padding()) - 10 - layout.marker_height + 7)
				);
				labels.add_text(std::to_string(i), position, TA_BOTTOM);
			}
		}
		
		create_keyframe_draggables();
		keyframes.set_constraint(layout.timeline);

		if(data) {
			if(data->frame > layout.timeline_frames) {
				data->frame = layout.timeline_frames;

				invoke_callback(Event::kTimeChange);
			}
		}

		lines.clear();
		for(size_t i = 0; i < layout.timeline_frames; ++i) {
			float x = static_cast<float>(padding() + layout.frame_width * static_cast<int>(i + 1));
			lines.add(vec2(x, static_cast<float>(layout.timeline.y())));
		}
	}
}

void keyframe_editor_overlay::draw_content(context& ctx) {

	begin_content(ctx);

	const auto& theme = cgv::gui::theme_info::instance();

	// draw container
	auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");
	
	if(data) {
		// draw scrollbar
		draw_scrollbar(ctx, content_canvas);

		// offset timeline based on scrollbar position
		content_canvas.push_modelview_matrix();
		content_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(vec2(static_cast<float>(-layout.timeline_offset), 0.0f)));

		// draw inner border
		content_canvas.enable_shader(ctx, rect_prog);
		border_style.apply(ctx, rect_prog);
		content_canvas.draw_shape(ctx, layout.timeline);

		content_canvas.disable_current_shader(ctx);

		// draw frame separation lines
		auto& line_prog = line_renderer.enable_prog(ctx);
		line_prog.set_attribute(ctx, "size", vec2(1.0f, static_cast<float>(layout.timeline.h())));
		line_renderer.render(ctx, content_canvas, PT_POINTS, lines, line_style);

		draw_time_marker_and_labels(ctx, content_canvas);

		draw_keyframes(ctx, content_canvas);
		
		// revert offset
		content_canvas.pop_modelview_matrix(ctx);
	}

	content_canvas.disable_current_shader(ctx);

	end_content(ctx);
}

void keyframe_editor_overlay::set_view_ptr(view* view_ptr) {

	this->view_ptr = view_ptr;
}

void keyframe_editor_overlay::set_data(std::shared_ptr<animation_data> data) {

	this->data = data;

	new_frame_count = data->timecode;
	new_duration = data->frame_to_time(data->timecode);
	update_member(&new_frame_count);
	update_member(&new_duration);

	update();
}

void keyframe_editor_overlay::update() {

	create_keyframe_draggables();
	set_marker_position_from_frame();
	post_damage();
}

void keyframe_editor_overlay::add_keyframe() {

	if(view_ptr && data) {
		view_parameters view;
		view.extract(view_ptr);

		if(keyframe* k = data->keyframe_at(data->frame)) {
			k->camera_state = view;
			invoke_callback(Event::kKeyChange);
		} else {
			data->insert_keyframe(data->frame, keyframe(view, easing_functions::Id::kLinear));
			invoke_callback(Event::kKeyCreate);
		}

		set_selected_frame(data->frame);
	}
}

void keyframe_editor_overlay::erase_selected_keyframe() {

	if(data) {
		data->erase_keyframe(selected_frame);
		set_selected_frame(-1);

		invoke_callback(Event::kKeyDelete);
	}
}

size_t keyframe_editor_overlay::position_to_frame(int position) {

	return (position - padding()) / layout.frame_width;
}

int keyframe_editor_overlay::frame_to_position(size_t frame) {

	return static_cast<int>(frame) * layout.frame_width + padding();
}

int keyframe_editor_overlay::frame_to_scrollbar_position(size_t frame) {

	float t = static_cast<float>(frame) / static_cast<float>(layout.timeline_frames);
	return static_cast<int>(layout.scrollbar_constraint.x() + t * layout.scrollbar_constraint.w());
}

void keyframe_editor_overlay::set_marker_position_from_frame() {

	if(data && !marker.empty()) {
		marker[0].x() = static_cast<float>(frame_to_position(data->frame));
		post_damage();
	}
}

void keyframe_editor_overlay::set_timeline_offset() {

	if(!scrollbar.empty()) {
		const auto& handle = scrollbar[0];
		float t = static_cast<float>(handle.x() - layout.scrollbar_constraint.x()) / static_cast<float>(layout.scrollbar_constraint.w() - handle.w());
		layout.timeline_offset = static_cast<int>(t * (layout.timeline.w() - layout.scrollbar_constraint.w()));
	}
}

void keyframe_editor_overlay::set_frame(size_t frame) {

	if(data) {
		data->frame = frame;
		invoke_callback(Event::kTimeChange);

		set_marker_position_from_frame();

		post_damage();
	}
}

void keyframe_editor_overlay::set_selected_frame(size_t frame) {

	selected_frame = frame;

	if(data) {
		if(keyframe* k = data->keyframe_at(selected_frame)) {
			easing_function_id = k->easing_id();

			invoke_callback(Event::kKeySelect);
		}
	}

	if(selected_frame == -1)
		invoke_callback(Event::kKeyDeselect);

	post_recreate_gui();
}

void keyframe_editor_overlay::change_duration(bool before) {

	bool change = true;
	size_t frame = selected_frame;
	size_t new_selected_frame = selected_frame;

	if(before) {
		auto it = data->ref_keyframes().lower_bound(selected_frame);

		if(it != data->ref_keyframes().end()) {
			if(it == data->ref_keyframes().begin()) {
				frame = -1;
				new_selected_frame = new_frame_count;
			} else {
				it = std::prev(it);
				frame = it->first;
				new_selected_frame = frame + new_frame_count;
			}
		} else {
			change = false;
		}
	}

	if(change) {
		data->change_duration_after(frame, new_frame_count);
		invoke_callback(Event::kKeyMove);
		
		if(selected_frame != new_selected_frame)
			set_selected_frame(new_selected_frame);
	}
}

void keyframe_editor_overlay::create_keyframe_draggables() {

	if(data) {
		keyframes.clear();
		for(const auto& pair : data->ref_keyframes()) {
			keyframe_draggable d;

			d.frame = pair.first;
			d.x() = static_cast<float>(frame_to_position(pair.first));
			d.y() = static_cast<float>(layout.timeline.y());
			d.size = vec2(static_cast<float>(layout.frame_width), static_cast<float>(layout.timeline_height));
			keyframes.add(d);
		}
	}
}

void keyframe_editor_overlay::handle_scrollbar_drag() {

	if(scrollbar.get_dragged())
		set_timeline_offset();

	post_damage();
}

void keyframe_editor_overlay::handle_marker_drag() {

	const auto dragged = marker.get_dragged();
	if(dragged)
		set_frame(position_to_frame(static_cast<int>(round(dragged->x()) + padding() / 2)));

	post_damage();
}

void keyframe_editor_overlay::handle_keyframe_drag() {

	const auto dragged = keyframes.get_dragged();

	if(dragged) {
		size_t frame = position_to_frame(static_cast<int>(round(dragged->x() + 0.5f * dragged->size.x())));
		dragged->x() = static_cast<float>(frame_to_position(frame));
	}

	post_damage();
}

void keyframe_editor_overlay::handle_keyframe_drag_end() {

	const auto selected = keyframes.get_selected();

	if(selected) {
		size_t frame = position_to_frame(static_cast<int>(round(selected->x() + 0.5f * selected->size.x())));
		selected->x() = static_cast<float>(frame_to_position(selected->frame));

		bool was_selected = selected->frame == selected_frame;

		if(selected->frame != frame && data) {

			if(data->move_keyframe(selected->frame, frame)) {
				selected->x() = static_cast<float>(frame_to_position(frame));

				post_recreate_layout();

				invoke_callback(Event::kKeyMove);
			}
		} else {
			set_selected_frame(frame);
		}

		if(was_selected)
			set_selected_frame(frame);
	} else {
		set_selected_frame(-1);
	}

	post_damage();
}

void keyframe_editor_overlay::handle_keyframe_selection_change() {

	const auto selected = keyframes.get_selected();

	set_selected_frame(selected ? selected->frame : -1);

	post_damage();
}

void keyframe_editor_overlay::invoke_callback(Event e) {

	if(on_change_callback)
		on_change_callback(e);
}

void keyframe_editor_overlay::draw_scrollbar(cgv::render::context& ctx, cgv::g2d::canvas& cnvs) {

	auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");

	cgv::g2d::irect scrollbar_rect = layout.scrollbar_constraint;
	scrollbar_rect.scale(1);

	scrollbar_style.apply(ctx, rect_prog);
	// draw scrollbar track
	content_canvas.draw_shape(ctx, scrollbar_rect, layout.background_color);
	// draw scrollbar handle
	content_canvas.draw_shape(ctx, scrollbar[0], layout.control_color);

	// draw current frame marker on scrollbar
	rectangle_style.apply(ctx, rect_prog);

	cgv::g2d::irect line_rect = scrollbar_rect;
	line_rect.x() = frame_to_scrollbar_position(data->frame) - 1;
	line_rect.w() = 3;
	content_canvas.draw_shape(ctx, line_rect, layout.selection_color);

	// draw keyframe markers on scrollbar
	for(size_t i = 0; i < keyframes.size(); ++i) {
		const auto& keyframe = keyframes[i];

		line_rect = scrollbar_rect;
		line_rect.x() = frame_to_scrollbar_position(keyframe.frame);
		line_rect.w() = 1;

		if(i > 0 && i < keyframes.size() - 1)
			line_rect.scale(0, -3);

		content_canvas.draw_shape(ctx, line_rect, layout.keyframe_color);
	}

	content_canvas.disable_current_shader(ctx);
}

void keyframe_editor_overlay::draw_keyframes(context& ctx, cgv::g2d::canvas& cnvs) {

	auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");
	key_rect_style.apply(ctx, rect_prog);

	cgv::g2d::irect r = layout.timeline;
	r.translate(0, 3);
	r.w() = layout.frame_width - 5;
	r.h() = layout.timeline_height - 6;

	for(const auto& keyframe : keyframes) {
		r.x() = static_cast<int>(keyframe.x()) + 3;
		rgb color = selected_frame == keyframe.frame ? layout.highlight_color : layout.keyframe_color;
		content_canvas.draw_shape(ctx, r, color);
	}

	auto& circle_prog = content_canvas.enable_shader(ctx, "circle");
	key_circle_style.apply(ctx, circle_prog);

	r.size = 7;
	r.y() += layout.timeline_height - 15;

	for(const auto& keyframe : keyframes) {
		r.x() = static_cast<int>(keyframe.x()) + 5;
		content_canvas.draw_shape(ctx, r);
	}

	content_canvas.disable_current_shader(ctx);
}

void keyframe_editor_overlay::draw_time_marker_and_labels(cgv::render::context& ctx, cgv::g2d::canvas& cnvs) {

	auto num_digits = [](size_t number) {
		if(!number)
			return 1;

		int digits = 0;

		while(number) {
			number /= 10;
			++digits;
		}

		return digits;
	};

	// draw frame number labels
	auto& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx);
	font_renderer.render(ctx, content_canvas, labels, label_style, 1);

	// draw time marker line
	int marker_x = static_cast<int>(marker[0].x()) + 7;

	cgv::g2d::irect r = layout.timeline;
	r.w() = 1;

	auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");
	rectangle_style.apply(ctx, rect_prog);

	content_canvas.draw_shape(ctx, ivec2(marker_x, r.y() + 1), ivec2(3, r.h() + 4), layout.selection_color);

	// draw time marker handle
	vec2 pos0 = static_cast<vec2>(ivec2(marker_x, layout.total_height(padding()) - 10));
	vec2 pos1 = vec2(pos0.x(), pos0.y() - layout.marker_height);
	pos0.x() += 1.5f;
	pos1.x() += 1.5f;

	float marker_handle_width = static_cast<float>(layout.marker_width) + num_digits(data->frame) * 5.0f;

	marker_handle_style.stem_width = marker_handle_width;
	marker_handle_style.head_width = marker_handle_width;

	auto& arrow_prog = content_canvas.enable_shader(ctx, "arrow");
	marker_handle_style.apply(ctx, arrow_prog);
	content_canvas.draw_shape2(ctx, pos0, pos1);

	content_canvas.disable_current_shader(ctx);

	// draw time marker frame number label
	labels.set_text(0, std::to_string(data->frame));
	labels.set_position(0, vec2(pos0.x(), pos1.y() + 7));
	font_renderer.render(ctx, content_canvas, labels, label_style, 0, 1);
}

void keyframe_editor_overlay::init_styles() {
	
	// get theme info and colors
	auto& theme = cgv::gui::theme_info::instance();
	layout.keyframe_color = rgb(theme.is_dark() ? 0.75f : 0.25f);
	layout.highlight_color = theme.highlight();
	layout.selection_color = theme.selection();
	layout.background_color = theme.background();
	layout.control_color = theme.control();

	// configure style for the border rectangles
	border_style.fill_color = theme.border();
	border_style.border_color = theme.background();
	border_style.feather_width = 0.0f;
	border_style.border_width = 1.0f;

	rectangle_style.use_fill_color = false;
	rectangle_style.feather_width = 0.0f;

	line_style.fill_color = theme.background();
	line_style.feather_width = 0.0f;

	key_rect_style.use_fill_color = false;
	key_rect_style.border_radius = 6.0f;
	key_rect_style.feather_width = 1.0f;
	key_rect_style.use_blending = true;

	key_circle_style.fill_color = theme.border();
	key_circle_style.border_width = 0.0f;
	key_circle_style.use_blending = true;

	marker_handle_style.fill_color = theme.selection();
	marker_handle_style.border_radius = 3.0f;
	marker_handle_style.stem_width = 1.0f;
	marker_handle_style.head_width = 1.0f;
	marker_handle_style.absolute_head_length = 6.0f;
	marker_handle_style.head_length_is_relative = false;
	marker_handle_style.use_blending = true;

	scrollbar_style.use_fill_color = false;
	scrollbar_style.border_radius = 4.0f;
	scrollbar_style.use_blending = true;

	label_style.fill_color = theme.text();
	label_style.font_size = 12.0f;
}

void keyframe_editor_overlay::create_gui_impl() {

	add_decorator("Keyframes", "heading", "level=4");
	help.create_gui(this);
	
	if(data && selected_frame != -1) {
		add_decorator("Frame " + std::to_string(selected_frame), "heading", "level=4");

		add_member_control(this, "Easing Function", easing_function_id, "dropdown", "enums='" + easing_functions::names_string() + "';tooltip='Change the easing function of the selected keyframe'");

		auto curr_pair = data->ref_keyframes().bounds(selected_frame);

		size_t delta_before = -1;
		std::string duration_before = "-";
		std::string duration_after = "-";

		if(curr_pair.first != data->ref_keyframes().end() && curr_pair.second != data->ref_keyframes().end()) {
			size_t delta = curr_pair.second->first - curr_pair.first->first;
			duration_after = cgv::utils::to_string(data->frame_to_time(delta), static_cast<unsigned>(-1), 3u) + " s   (" + std::to_string(delta) + " frames)";
		}
		
		if(curr_pair.first != data->ref_keyframes().end() && curr_pair.first->first > 0) {
			auto prev_pair = data->ref_keyframes().bounds(curr_pair.first->first - 1);

			if(prev_pair.second != data->ref_keyframes().end())
				delta_before = prev_pair.second->first;

			if(prev_pair.first != data->ref_keyframes().end())
				delta_before -= prev_pair.first->first;
		} else {
			delta_before = 0ull;
		}

		if(delta_before != -1)
			duration_before = cgv::utils::to_string(data->frame_to_time(delta_before), static_cast<unsigned>(-1), 3u) + " s   (" + std::to_string(delta_before) + " frames)";

		add_view("Duration Before", duration_before);
		add_view("Duration After", duration_after);

		add_decorator("Change Duration", "heading", "level=4;font_style='regular'");
		
		add_member_control(this, "", new_frame_count, "value", "w=64;min=0;max=120;step=1", "");
		add_decorator(" frames", "heading", "w=58;level=4;font_style='regular'", "");
		
		add_member_control(this, "", new_duration, "value", "w=64;min=0;max=4;step=0.01", "");
		add_decorator(" s", "heading", "w=14;level=4;font_style='regular'");

		connect_copy(add_button("Change Before", "w=94;tooltip='Set the transition duration to the next keyframe'", " ")->click, rebind(this, &keyframe_editor_overlay::change_duration, cgv::signal::const_expression<bool>(true)));
		connect_copy(add_button("Change After", "w=94;tooltip='Set the transition duration from the previous keyframe'")->click, rebind(this, &keyframe_editor_overlay::change_duration, cgv::signal::const_expression<bool>(false)));

		add_decorator("", "separator");

		connect_copy(add_button("Remove Selected", "tooltip='Remove the selected keyframe'")->click, rebind(this, &keyframe_editor_overlay::erase_selected_keyframe));
	}

	add_decorator("", "separator");
	connect_copy(add_button("Set", "tooltip='Add or update keyframe at the current time'")->click, rebind(this, &keyframe_editor_overlay::add_keyframe));
}
