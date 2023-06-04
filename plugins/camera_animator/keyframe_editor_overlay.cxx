#include "keyframe_editor_overlay.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>

using namespace cgv::render;

keyframe_editor_overlay::keyframe_editor_overlay() {

	set_name("Keyframe Editor");
	block_events = true;
	draw_in_finish_frame = true;
	blend_overlay = true;
	gui_options.allow_stretch = false;
	gui_options.allow_margin = false;
	
	set_overlay_alignment(AO_START, AO_START);
	set_overlay_stretch(SO_HORIZONTAL);
	set_overlay_margin(ivec2(-3));
	set_overlay_size(ivec2(100, layout.total_height()));

	scrollbar.set_drag_callback(std::bind(&keyframe_editor_overlay::handle_scrollbar_drag, this));
	marker.set_drag_callback(std::bind(&keyframe_editor_overlay::handle_marker_drag, this));
	keyframes.set_drag_callback(std::bind(&keyframe_editor_overlay::handle_keyframe_drag, this));
	keyframes.set_drag_end_callback(std::bind(&keyframe_editor_overlay::handle_keyframe_drag_end, this));
	keyframes.set_selection_change_callback(std::bind(&keyframe_editor_overlay::handle_keyframe_selection_change, this));

	help.add_bullet_point("Click on a keyframe to select it.");
	help.add_bullet_point("Drag a keyframe to change its time. Dragging onto another keyframe will revert the drag.");
	help.add_bullet_point("Click \"Set\" to add a new keyframe or change the existing keyframe at the current time.");
}

void keyframe_editor_overlay::clear(context& ctx) {

	canvas_overlay::clear(ctx);

	cgv::g2d::ref_msdf_font_regular(ctx, -1);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);
}

bool keyframe_editor_overlay::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();

	cgv::g2d::irect container = get_overlay_rectangle();
	container.position.x() -= layout.timeline_offset;

	if(keyframes.handle(e, get_viewport_size(), container))
		return true;

	if(et == cgv::gui::EID_KEY) {
		//cgv::gui::key_event& ke = (cgv::gui::key_event&) e;
		//cgv::gui::KeyAction ka = ke.get_action();

		/*
		ka is one of:
			cgv::gui::[
				KA_PRESS,
				KA_RELEASE,
				KA_REPEAT
			]
		*
		if(ka == cgv::gui::KA_PRESS) {
			switch(ke.get_key()) {
			case 'R':
			{
				auto& ctx = *get_context();
				ctx.disable_shader_file_cache();
				content_canvas.reload_shaders(ctx);
				ctx.enable_shader_file_cache();
				post_damage();
				return true;
			}
				break;
			default: break;
			}
		}

		return false;*/
	} else if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		
		if(me.get_button() == cgv::gui::MB_LEFT_BUTTON && ma == cgv::gui::MA_PRESS) {
			ivec2 local_mouse_position = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()));

			if(layout.marker_constraint.is_inside(local_mouse_position))
				set_frame(position_to_frame(local_mouse_position.x() + layout.timeline_offset));
			
			if(!scrollbar.empty()) {
				auto& handle = scrollbar[0];

				if(layout.scrollbar_constraint.is_inside(local_mouse_position) && !handle.is_inside(local_mouse_position)) {
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

		if(scrollbar.handle(e, get_viewport_size(), get_overlay_rectangle()))
			return true;
		
		if(marker.handle(e, get_viewport_size(), container))
			return true;
	}
	
	return false;
}

void keyframe_editor_overlay::handle_on_set(const cgv::app::on_set_evaluator& m) {

	if(m.is(easing_function_id)) {
		if(data) {
			if(keyframe* k = data->keyframe_at(selected_frame)) {
				k->ease(easing_function_id);

				if(on_change_callback)
					on_change_callback();
			}
		}
	}
}

bool keyframe_editor_overlay::init(context& ctx) {

	register_shader("background", cgv::g2d::shaders::grid);
	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("circle", cgv::g2d::shaders::circle);
	register_shader("arrow", cgv::g2d::shaders::arrow);
	
	bool success = canvas_overlay::init(ctx);

	cgv::g2d::msdf_font& font = cgv::g2d::ref_msdf_font_regular(ctx, 1);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);

	if(font.is_initialized())
		labels.set_msdf_font(&font);

	if(success)
		init_styles(ctx);

	return success;
}

void keyframe_editor_overlay::init_frame(context& ctx) {

	if(ensure_layout(ctx)) {
		layout.update(get_overlay_size(), data ? data->frame_count() : 0);

		if(scrollbar.empty()) {
			cgv::g2d::draggable handle;
			handle.position = static_cast<vec2>(layout.scrollbar_constraint.position);
			handle.size = vec2(16.0f, static_cast<float>(layout.scrollbar_height));
			scrollbar.add(handle);
		}

		scrollbar[0].size.x() = std::max(
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
			if(i % 5 == 0)
				labels.add_text(std::to_string(i), vec2(layout.padding + layout.frame_width * i + layout.frame_width / 2, layout.total_height() - 10 - layout.marker_height + 7), TA_BOTTOM);
		}
		
		create_keyframe_draggables();
		keyframes.set_constraint(layout.timeline);

		if(data) {
			if(data->frame > layout.timeline_frames) {
				data->frame = layout.timeline_frames;
				if(on_change_callback)
					on_change_callback();
			}
		}
	}
}

void keyframe_editor_overlay::draw_content(context& ctx) {

	begin_content(ctx);

	const auto& theme = cgv::gui::theme_info::instance();

	// draw container
	auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");
	container_style.apply(ctx, rect_prog);
	content_canvas.draw_shape(ctx, layout.container);

	// draw scrollbar
	cgv::g2d::irect scrollbar_rect = layout.scrollbar_constraint;
	scrollbar_rect.scale(1);

	scrollbar_style.apply(ctx, rect_prog);
	content_canvas.draw_shape(ctx, scrollbar_rect, theme.background());
	content_canvas.draw_shape(ctx, scrollbar[0], theme.control());

	if(data) {
		rectangle_style.apply(ctx, rect_prog);

		cgv::g2d::irect line_rect = scrollbar_rect;
		line_rect.position.x() = frame_to_scrollbar_position(data->frame) - 1;
		line_rect.size.x() = 3;
		content_canvas.draw_shape(ctx, line_rect, theme.selection());

		for(size_t i = 0; i < keyframes.size(); ++i) {
			const auto& keyframe = keyframes[i];

			line_rect = scrollbar_rect;
			line_rect.position.x() = frame_to_scrollbar_position(keyframe.frame);
			line_rect.size.x() = 1;

			if(i > 0 && i < keyframes.size() - 1)
				line_rect.scale(0, -3);

			content_canvas.draw_shape(ctx, line_rect, layout.keyframe_color);
		}

		


	
		content_canvas.push_modelview_matrix();
		content_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(vec2(static_cast<float>(-layout.timeline_offset), 0.0f)));

		// draw inner border
		border_style.apply(ctx, rect_prog);
		content_canvas.draw_shape(ctx, layout.timeline);

		// draw frame separation lines
		cgv::g2d::irect rectangle = layout.timeline;
		rectangle.size.x() = 1;

		line_style.apply(ctx, rect_prog);

		for(size_t i = 0; i < layout.timeline_frames; ++i) {
			rectangle.position.x() = layout.padding + layout.frame_width * static_cast<int>(i + 1);
			content_canvas.draw_shape(ctx, rectangle);
		}


		// draw frame number labels
		auto& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx);
		font_renderer.render(ctx, content_canvas, labels, label_style, 1);


		// draw time marker line
		int marker_x = static_cast<int>(marker[0].position.x()) + 7;

		rectangle_style.apply(ctx, rect_prog);
		content_canvas.draw_shape(ctx, ivec2(marker_x, rectangle.y() + 1), ivec2(3, rectangle.h() + 4), theme.selection());

		


		// draw keyframes
		rectangle.translate(0, 3);
		rectangle.size.x() = layout.frame_width - 5;
		rectangle.size.y() = layout.timeline_height - 6;

		key_rect_style.apply(ctx, rect_prog);
		
		for(size_t i = 0; i < keyframes.size(); ++i) {
			const auto& keyframe = keyframes[i];
			rectangle.position.x() = static_cast<int>(keyframe.position.x()) + 3;
			rgb color = selected_frame == keyframe.frame ? theme.highlight() : layout.keyframe_color;
			content_canvas.draw_shape(ctx, rectangle, color);
		}

		auto& circle_prog = content_canvas.enable_shader(ctx, "circle");

		rectangle.size = 7;
		rectangle.position.y() += layout.timeline_height - 15;

		key_circle_style.apply(ctx, circle_prog);
		for(size_t i = 0; i < keyframes.size(); ++i) {
			const auto& keyframe = keyframes[i];
			rectangle.position.x() = static_cast<int>(keyframe.position.x()) + 5;
			content_canvas.draw_shape(ctx, rectangle);
		}




		






		// draw time marker handle
		vec2 pos0 = static_cast<vec2>(ivec2(marker_x, layout.total_height() - 10));
		vec2 pos1 = static_cast<vec2>(ivec2(marker_x, pos0.y() - layout.marker_height));
		pos0.x() += 1.5f;
		pos1.x() += 1.5f;

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

		float marker_handle_width = static_cast<float>(layout.marker_width) + num_digits(data->frame) * 5.0f;

		marker_handle_style.stem_width = marker_handle_width;
		marker_handle_style.head_width = marker_handle_width;

		auto& arrow_prog = content_canvas.enable_shader(ctx, "arrow");
		marker_handle_style.apply(ctx, arrow_prog);
		//content_canvas.draw_shape2(ctx, ivec2(time_marker_position_x, padding), ivec2(time_marker_position_x, rectangle.y()));
		content_canvas.draw_shape2(ctx, pos0, pos1);

		// draw frame labels
		labels.set_text(0, std::to_string(data->frame));
		labels.set_position(0, vec2(pos0.x(), pos1.y() + 7));
		font_renderer.render(ctx, content_canvas, labels, label_style, 0, 1);






		

		content_canvas.pop_modelview_matrix(ctx);
	}

	content_canvas.disable_current_shader(ctx);

	end_content(ctx);
}

void keyframe_editor_overlay::set_view(cgv::render::view* view_ptr) {

	this->view_ptr = view_ptr;
}

void keyframe_editor_overlay::set_data(std::shared_ptr<animation_data> data) {

	this->data = data;
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

		if(keyframe* k = data->keyframe_at(data->frame))
			k->camera_state = view;
		else
			data->keyframes.insert(data->frame, keyframe(view, easing_functions::Id::kLinear));

		set_selected_frame(data->frame);

		if(on_change_callback)
			on_change_callback();
	}
}

void keyframe_editor_overlay::erase_selected_keyframe() {

	if(data) {
		data->keyframes.erase(selected_frame);

		set_selected_frame(-1);

		if(on_change_callback)
			on_change_callback();
	}
}

size_t keyframe_editor_overlay::position_to_frame(int position) {

	return (position - layout.padding) / layout.frame_width;
}

int keyframe_editor_overlay::frame_to_position(size_t frame) {

	return static_cast<int>(frame) * layout.frame_width + layout.padding;
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
		if(on_change_callback)
			on_change_callback();

		set_marker_position_from_frame();

		post_damage();
	}
}

void keyframe_editor_overlay::set_selected_frame(size_t frame) {

	selected_frame = frame;

	if(data) {
		if(keyframe* k = data->keyframe_at(selected_frame))
			easing_function_id = k->easing_id();
	}

	post_recreate_gui();
}

void keyframe_editor_overlay::create_keyframe_draggables() {

	if(data) {
		const auto& kfs = data->keyframes;

		keyframes.clear();
		for(const auto& kf : kfs) {
			keyframe_draggable d;

			d.frame = kf.first;
			d.x() = static_cast<float>(frame_to_position(kf.first));
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
		set_frame(position_to_frame(static_cast<int>(round(dragged->position.x()) + layout.padding / 2)));

	post_damage();
}

void keyframe_editor_overlay::handle_keyframe_drag() {

	const auto dragged = keyframes.get_dragged();

	if(dragged) {
		size_t frame = position_to_frame(static_cast<int>(round(dragged->position.x() + 0.5f * dragged->size.x())));
		dragged->x() = static_cast<float>(frame_to_position(frame));
	}

	post_damage();
}

void keyframe_editor_overlay::handle_keyframe_drag_end() {

	const auto selected = keyframes.get_selected();

	if(selected) {
		size_t frame = position_to_frame(static_cast<int>(round(selected->position.x() + 0.5f * selected->size.x())));
		selected->x() = static_cast<float>(frame_to_position(selected->frame));

		bool was_selected = selected->frame == selected_frame;

		if(selected->frame != frame && data) {

			if(data->keyframes.move(selected->frame, frame)) {
				selected->x() = static_cast<float>(frame_to_position(frame));

				post_recreate_layout();

				if(on_change_callback)
					on_change_callback();
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











void keyframe_editor_overlay::init_styles(context& ctx) {
	
	// get theme info and colors
	auto& theme = cgv::gui::theme_info::instance();
	layout.keyframe_color = rgb(theme.is_dark() ? 0.75f : 0.25f);

	// configure style for the container rectangle
	container_style.fill_color = theme.group();
	container_style.border_color = theme.background();
	container_style.border_width = 3.0f;
	container_style.feather_width = 0.0f;

	// configure style for the border rectangles
	border_style = container_style;
	border_style.fill_color = theme.border();
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

	label_style = cgv::g2d::text2d_style::preset_clear(theme.text());
	label_style.font_size = 12.0f;
}

void keyframe_editor_overlay::create_gui_impl() {

	add_decorator("Keyframes", "heading", "w=168;level=4", " ");
	help.create_button(this);
	
	if(selected_frame != -1) {
		add_decorator("Frame " + std::to_string(selected_frame), "heading", "level=4");

		add_member_control(this, "Easing Function", easing_function_id, "dropdown", "enums='" + easing_functions::names_string() + "'");

		connect_copy(add_button("Remove Selected", "tooltip='Remove the selected keyframe'")->click, rebind(this, &keyframe_editor_overlay::erase_selected_keyframe));
	}

	add_decorator("", "separator");
	connect_copy(add_button("Set", "tooltip='Add or update keyframe at the current time'")->click, rebind(this, &keyframe_editor_overlay::add_keyframe));
}
