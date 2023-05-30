#include "keyframe_editor_overlay.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>

using namespace cgv::render;

keyframe_editor_overlay::keyframe_editor_overlay() {

	set_name("Keyframe Editor");
	block_events = true;
	blend_overlay = true;
	
	set_overlay_alignment(AO_START, AO_START);
	set_overlay_stretch(SO_HORIZONTAL);
	set_overlay_margin(ivec2(-3));
	set_overlay_size(ivec2(100, layout.total_height()));

	scrollbar.set_drag_callback(std::bind(&keyframe_editor_overlay::handle_scrollbar_drag, this));
	marker.set_drag_callback(std::bind(&keyframe_editor_overlay::handle_marker_drag, this));
}

void keyframe_editor_overlay::clear(context& ctx) {

	canvas_overlay::clear(ctx);

	cgv::g2d::ref_msdf_font_regular(ctx, -1);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);
}

void keyframe_editor_overlay::on_set(void* member_ptr) {

	on_set(cgv::app::on_set_evaluator(member_ptr));
	update_member(member_ptr);
	post_damage();
}

void keyframe_editor_overlay::on_set(const cgv::app::on_set_evaluator& m) {

	/*if(m.component_of(layout.total_size)) {
		vec2 size = get_overlay_size();

		// TODO: minimum width and height depend on other layout parameters
		layout.total_size.y() = std::max(layout.total_size.y(), 2 * layout.padding + 4 + layout.label_space);

		set_overlay_size(layout.total_size);
	}*/

	/*if(m.is(show_background, invert_color)) {
		auto ctx_ptr = get_context();
		if(ctx_ptr)
			init_styles(*ctx_ptr);
	}*/

	/*if(member_ptr == &layout.orientation ||
		member_ptr == &layout.label_alignment ||
		member_ptr == &range ||
		member_ptr == &num_ticks ||
		member_ptr == &label_precision ||
		member_ptr == &label_auto_precision ||
		member_ptr == &label_integer_mode) {
		post_recreate_layout();
	}*/
}

bool keyframe_editor_overlay::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();

	/*auto damage = [this]() {
		post_damage();
		return true;
	};

	cgv::g2d::irect button_container = get_overlay_rectangle();

	button_container.translate(button_container.size - buttons.get_default_button_size() - padding);

	if(buttons.handle(e, get_viewport_size(), button_container)) {
		return damage();
	}

	if(editor->handle(e, get_viewport_size(), get_overlay_rectangle())) {
		return damage();
	}

	//if(node_draggables.handle(e, get_overlay_size(), container)) {
	//	post_damage();
	//	return true;
	//}
	*/
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

			bool hit_marker_area = layout.marker_constraint.is_inside(local_mouse_position);
			bool hit_timeline = layout.timeline.is_inside(local_mouse_position);

			size_t frame = get_frame_from_position(local_mouse_position.x() + layout.timeline_offset);

			if(hit_timeline) {
				auto it = data_ptr->keyframes.find(frame);
				if(it != data_ptr->keyframes.end())
					selected_frame = frame;
			}

			if(hit_marker_area || hit_timeline)
				set_frame(frame);
			else
				selected_frame = -1;

			post_damage();
		}

		//if(ma == cgv::gui::MA_PRESS && )

		/*
		ma is one of:
			cgv::gui::[
				MA_DRAG,
				MA_ENTER,
				MA_LEAVE,
				MA_MOVE,
				MA_PRESS,
				MA_RELEASE,
				MA_WHEEL
			]
		*/
		if(scrollbar.handle(e, get_viewport_size(), get_overlay_rectangle()))
			return true;

		cgv::g2d::irect container = get_overlay_rectangle();
		container.position.x() -= layout.timeline_offset;
		if(marker.handle(e, get_viewport_size(), container))
			return true;

		return false;
	} else {
		return false;
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

	//buttons.init(ctx);
	//buttons.set_default_button_size(ivec2(24));
	//buttons.set_default_callback(std::bind(&node_editor_overlay::handle_button_click, this, std::placeholders::_1));
	//buttons.add("[   ]", ivec2(0));

	if(success)
		init_styles(ctx);

	return success;
}

void keyframe_editor_overlay::init_frame(context& ctx) {

	if(ensure_layout(ctx)) {
		//ivec2 container_size = get_overlay_size();
		//create_labels();
		layout.update(get_overlay_size(), data_ptr ? data_ptr->frame_count() : 0);
		//create_ticks();

		if(!scrollbar.size()) {
			cgv::g2d::draggable handle;
			handle.position = static_cast<vec2>(layout.scrollbar_constraint.position);
			handle.size = vec2(16.0f, layout.scrollbar_height);
			scrollbar.add(handle);
		}

		scrollbar[0].size.x() = std::max(
			layout.scrollbar_constraint.w() * static_cast<float>(layout.scrollbar_constraint.w()) / static_cast<float>(layout.timeline.w()),
			24.0f);

		scrollbar.set_constraint(layout.scrollbar_constraint);

		if(!marker.size()) {
			cgv::g2d::draggable handle;
			handle.position = static_cast<vec2>(layout.marker_constraint.position);
			// TODO: adjust width based on frame number
			handle.size = vec2(layout.marker_height, layout.marker_width);
			marker.add(handle);
		}

		marker.set_constraint(layout.marker_constraint);

		if(data_ptr) {
			size_t frame_count = data_ptr->frame_count();

			labels.clear();
			labels.add_text("0", vec2(0.0f), TA_BOTTOM);

			for(size_t i = 0; i < frame_count; ++i) {
				if(i % 5 == 0)
					labels.add_text(std::to_string(i), vec2(layout.padding + layout.frame_width * i + layout.frame_width / 2, layout.total_height() - 10 - layout.marker_height + 7), TA_BOTTOM);
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

	cgv::g2d::irect scrollbar_rect = layout.scrollbar_constraint;
	scrollbar_rect.scale(1);

	scrollbar_style.apply(ctx, rect_prog);
	content_canvas.draw_shape(ctx, scrollbar_rect, theme.background());
	content_canvas.draw_shape(ctx, scrollbar[0].position, scrollbar[0].size, theme.control());

	if(data_ptr) {
		content_canvas.push_modelview_matrix();
		content_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(vec2(-layout.timeline_offset, 0.0f)));

		// draw inner border
		border_style.apply(ctx, rect_prog);
		content_canvas.draw_shape(ctx, layout.timeline);

		// draw frame separation lines
		cgv::g2d::irect rectangle = layout.timeline;
		rectangle.size.x() = 1;

		line_style.apply(ctx, rect_prog);

		for(size_t i = 0; i < data_ptr->frame_count(); ++i) {
			rectangle.position.x() = layout.padding + layout.frame_width * static_cast<int>(i + 1);
			content_canvas.draw_shape(ctx, rectangle);
		}


		// draw frame number labels
		auto& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx);
		font_renderer.render(ctx, content_canvas, labels, label_style, 1);


		// draw time marker line
		//int time_marker_position_x = layout.padding + static_cast<int>(data_ptr->frame) * layout.frame_width + 7;
		int marker_x = static_cast<int>(marker[0].position.x()) + 7;

		marker_style.apply(ctx, rect_prog);
		content_canvas.draw_shape(ctx, ivec2(marker_x, rectangle.y() + 1), ivec2(3, rectangle.h() + 4));



		// draw keyframes
		rectangle.translate(0, 3);
		rectangle.size.x() = layout.frame_width - 5;
		rectangle.size.y() = layout.timeline_height - 6;

		key_rect_style.apply(ctx, rect_prog);
		for(const auto& keyframe : data_ptr->keyframes) {
			size_t index = keyframe.first;
			rectangle.position.x() = layout.padding + layout.frame_width * static_cast<int>(index) + 3;

			rgb color = rgb(theme.is_dark() ? 0.75f : 0.25f);
			if(selected_frame == index)
				color = theme.selection();

			content_canvas.draw_shape(ctx, rectangle, color);
		}

		auto& circle_prog = content_canvas.enable_shader(ctx, "circle");

		rectangle.size = 7;
		rectangle.position.y() += layout.timeline_height - 15;

		key_circle_style.apply(ctx, circle_prog);
		for(const auto& keyframe : data_ptr->keyframes) {
			size_t index = keyframe.first;
			rectangle.position.x() = layout.padding + layout.frame_width * static_cast<int>(index) + 5;

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

		float marker_handle_width = static_cast<float>(layout.marker_width) + num_digits(data_ptr->frame) * 5.0f;

		marker_handle_style.stem_width = marker_handle_width;
		marker_handle_style.head_width = marker_handle_width;

		auto& arrow_prog = content_canvas.enable_shader(ctx, "arrow");
		marker_handle_style.apply(ctx, arrow_prog);
		//content_canvas.draw_shape2(ctx, ivec2(time_marker_position_x, padding), ivec2(time_marker_position_x, rectangle.y()));
		content_canvas.draw_shape2(ctx, pos0, pos1);

		labels.set_text(0, std::to_string(data_ptr->frame));
		labels.set_position(0, vec2(pos0.x(), pos1.y() + 7));
		font_renderer.render(ctx, content_canvas, labels, label_style, 0, 1);






		

		content_canvas.pop_modelview_matrix(ctx);
	}

	content_canvas.disable_current_shader(ctx);

	end_content(ctx);
}

/*void keyframe_editor_overlay::handle_node_editor_update() {

	post_damage();
}*/

void keyframe_editor_overlay::init_styles(context& ctx) {
	
	// get theme info and colors
	auto& theme = cgv::gui::theme_info::instance();
	
	// configure style for the container rectangle
	container_style.fill_color = theme.group();
	container_style.border_color = theme.background();
	container_style.border_width = 3.0f;
	container_style.feather_width = 0.0f;

	// configure style for the border rectangles
	border_style = container_style;
	border_style.fill_color = theme.border();
	border_style.border_width = 1.0f;

	line_style.fill_color = theme.background();
	line_style.feather_width = 0.0f;

	key_rect_style.use_fill_color = false;
	//key_rect_style.fill_color = rgb(theme.is_dark() ? 0.75f : 0.25f);
	key_rect_style.border_radius = 6.0f;
	key_rect_style.feather_width = 1.0f;
	key_rect_style.use_blending = true;

	key_circle_style.fill_color = theme.border();
	key_circle_style.border_width = 0.0f;
	key_circle_style.use_blending = true;

	marker_style.fill_color = theme.selection();
	marker_style.feather_width = 0.0f;

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

	//inline_object_gui(editor);

	//add_gui("", test_style);

	/*add_member_control(this, "Width", layout.total_size[0], "value_slider", "min=40;max=500;step=1;ticks=true");
	add_member_control(this, "Height", layout.total_size[1], "value_slider", "min=40;max=500;step=1;ticks=true");

	add_member_control(this, "Background", show_background, "check", "w=100", " ");
	add_member_control(this, "Invert Color", invert_color, "check", "w=88");

	add_member_control(this, "Orientation", layout.orientation, "dropdown", "enums='Horizontal,Vertical'");
	add_member_control(this, "Label Alignment", layout.label_alignment, "dropdown", "enums='-,Before,Inside,After'");

	add_member_control(this, "Ticks", num_ticks, "value", "min=2;max=10;step=1");
	add_member_control(this, "Number Precision", label_precision, "value", "w=60;min=0;max=10;step=1", " ");
	add_member_control(this, "Auto", label_auto_precision, "check", "w=44", " ");
	add_member_control(this, "Integers", label_integer_mode, "check", "w=72");*/
}
