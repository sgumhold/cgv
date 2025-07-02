#include "framing_overlay.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/utils/convert_string.h>
#include <cgv_g2d/msdf_gl_font_renderer.h>

using namespace cgv::render;

float framing_overlay::corner_handle_size = 14.0f;
float framing_overlay::min_frame_size = 128.0f;

framing_overlay::framing_overlay() {
	set_name("Framing Editor");
	blocks_events(false);
	set_stretch(SO_BOTH);
	set_margin(0);
	set_size(-1);

	background_visible_ = false;
	gui_options.allow_stretch = false;
	gui_options.allow_margin = false;

	handles.callback = std::bind(&framing_overlay::handle_drag, this, std::placeholders::_1);
}

bool framing_overlay::init(context& ctx) {
	cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx, 1);

	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("circle", cgv::g2d::shaders::circle);

	bool success = themed_canvas_overlay::init(ctx);
	success &= labels.init(ctx);

	handles.set_constraint({ 0.0f, get_viewport_size() });
	set_frame({ 0.0f, 0.0f });

	return success;
}

void framing_overlay::clear(context& ctx) {
	cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx, -1);

	themed_canvas_overlay::clear(ctx);

	labels.destruct(ctx);
}

bool framing_overlay::handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 local_mouse_pos) {
	cgv::g2d::irect container = get_rectangle();

	if(!editing_enabled)
		return false;

	if(handles.handle_mouse_event(e, local_mouse_pos))
		return true;

	return is_frame_valid();
}

void framing_overlay::init_frame(context& ctx) {
	if(ensure_layout(ctx)) {
		frame_absolute = to_absolute_frame(frame_relative, get_viewport_size());
		handles.set_constraint({ 0.0f, get_viewport_size() });
		update_handles();
	}
}

void framing_overlay::draw_content(context& ctx) {
	begin_content(ctx);

	const auto& theme = cgv::gui::theme_info::instance();

	cgv::vec2 viewport_size = get_viewport_size();

	if(is_frame_valid()) {
		cgv::g2d::shape2d_style rect_style;
		const cgv::rgb frame_color = editing_enabled ? theme.warning() : theme.selection();
		const float frame_ring_width = 20.0f;

		// draw transparent frame shadow
		auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");

		rect_style.use_blending = true;
		rect_style.fill_color = { frame_color, 0.2f };
		rect_style.border_width = 0.0f;
		rect_style.ring_width = frame_ring_width;
		content_canvas.set_style(ctx, rect_style);
		content_canvas.draw_shape(ctx, frame_absolute);

		// draw frame border
		rect_style.fill_color = { 0.0f };
		rect_style.border_color = { frame_color, 1.0f };
		rect_style.border_width = 1.0f;
		rect_style.ring_width = 0.0f;

		content_canvas.set_style(ctx, rect_style);
		content_canvas.draw_shape(ctx, frame_absolute);

		// draw darkening shadow outside of frame
		rect_style.feather_width = 0.0f;
		rect_style.border_width = 0.0f;
		rect_style.fill_color = { 0.0f, 0.0f, 0.0f, 0.25f };

		content_canvas.set_style(ctx, rect_style);

		cgv::g2d::rect border;
		border.position = 0.0f;
		border.size = { viewport_size.x(), frame_absolute.y() };
		content_canvas.draw_shape(ctx, border);

		border.y() = frame_absolute.y1();
		border.h() = viewport_size.y() - frame_absolute.y1();
		content_canvas.draw_shape(ctx, border);

		border.position = { 0.0f, frame_absolute.y() };
		border.size = { frame_absolute.x() , frame_absolute.h() };
		content_canvas.draw_shape(ctx, border);

		border.x() = frame_absolute.x1();
		border.w() = viewport_size.x() - frame_absolute.x1();
		content_canvas.draw_shape(ctx, border);

		// draw corner handles
		if(!handles.empty() && editing_enabled) {
			rect_style.feather_width = 1.0f;
			rect_style.fill_color = { 1.0f };
			rect_style.border_color = { 1.0f, 0.0f, 0.0f, 1.0f };
			rect_style.border_width = 1.0f;
			rect_style.position_is_center = true;

			content_canvas.set_style(ctx, rect_style);
			for(size_t i = 0; i < 4; ++i)
				content_canvas.draw_shape(ctx, handles[i + 1]);
		}

		// draw circular center marker
		rect_style.fill_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		rect_style.border_width = 0.0f;
		rect_style.position_is_center = true;
		content_canvas.set_style(ctx, rect_style);

		const cgv::vec2 frame_center = frame_absolute.center();
		const float center_circle_diameter = 0.2f * std::min(frame_absolute.w(), frame_absolute.h());
		const float center_circle_radius = 0.5f * center_circle_diameter;

		cgv::g2d::rect line;
		line.size = { 1.0f, center_circle_radius };
		line.x() = frame_center.x();
		line.y() = frame_center.y() - center_circle_radius;
		content_canvas.draw_shape(ctx, line);

		line.y() = frame_center.y() + center_circle_radius;
		content_canvas.draw_shape(ctx, line);

		std::swap(line.w(), line.h());
		line.x() = frame_center.x() - center_circle_radius;
		line.y() = frame_center.y();
		content_canvas.draw_shape(ctx, line);

		line.x() = frame_center.x() + center_circle_radius;
		line.y() = frame_center.y();
		content_canvas.draw_shape(ctx, line);

		auto& circle_prog = content_canvas.enable_shader(ctx, "circle");
		cgv::g2d::circle2d_style center_circle_style;
		center_circle_style.use_blending = true;
		center_circle_style.fill_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		center_circle_style.ring_width = 2.0f;
		center_circle_style.position_is_center = true;
		content_canvas.set_style(ctx, center_circle_style);
		content_canvas.draw_shape(ctx, frame_center, { center_circle_diameter });

		// draw resolution info
		content_canvas.enable_shader(ctx, rect_prog);
		rect_style.fill_color = { 1.0f };
		rect_style.position_is_center = false;
		content_canvas.set_style(ctx, rect_style);

		cgv::g2d::rect label_rect;
		label_rect.size = { 100.0f, frame_ring_width };
		label_rect.align_inside(frame_absolute, cgv::g2d::Alignment::kTop);
		content_canvas.draw_shape(ctx, label_rect);

		cgv::g2d::rect viewport_label_rect = label_rect;
		viewport_label_rect.w() = 200.0f;
		viewport_label_rect.align_inside({ 0.0f, viewport_size }, cgv::g2d::Alignment::kBottom);
		content_canvas.draw_shape(ctx, viewport_label_rect);

		content_canvas.disable_current_shader(ctx);

		const auto to_resolution_string = [](cgv::vec2 resolution) {
			resolution = cgv::math::round(resolution);
			std::string str;
			str += std::to_string(static_cast<unsigned>(resolution.x())) + " x ";
			str += std::to_string(static_cast<unsigned>(resolution.y()));
			return str;
		};

		std::vector<std::string> texts;
		texts.push_back(to_resolution_string(frame_absolute.size));
		texts.push_back("Viewport size: " + to_resolution_string(viewport_size));

		labels.clear();
		labels.set_text_array(ctx, texts);

		labels.positions.push_back(cgv::vec3(label_rect.center(), 0.0f));
		labels.positions.push_back(cgv::vec3(viewport_label_rect.center(), 0.0f));
		labels.alignment = cgv::render::TextAlignment::TA_NONE;
		
		auto& font_renderer = cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx);
		font_renderer.render(ctx, content_canvas, labels, label_style);
	}

	content_canvas.disable_current_shader(ctx);

	end_content(ctx);
}

void framing_overlay::clear_frame() {
	set_frame({ 0.0f, 0.0f });
}

void framing_overlay::set_frame(const cgv::g2d::rect& frame) {
	frame_relative = frame;
	frame_absolute = to_absolute_frame(frame_relative, get_viewport_size());
	create_handles();
	post_damage();
}

cgv::g2d::rect framing_overlay::get_frame() const {
	// return a full-screen frame in case the actual frame is not valid
	if(!is_frame_valid())
		return { 0.0f, 1.0f };
	return frame_relative;
}

void framing_overlay::enable_editing() {
	editing_enabled = true;
	post_damage();
}

void framing_overlay::disable_editing() {
	editing_enabled = false;
	post_damage();
}

void framing_overlay::init_styles() {
	auto& theme = cgv::gui::theme_info::instance();

	label_style.fill_color = theme.text();
	label_style.font_size = 12.0f;
}

bool framing_overlay::is_frame_valid() const {
	return frame_relative.x() >= 0.0f && frame_relative.y() >= 0.0f && frame_relative.w() > 0.0f && frame_relative.h() > 0.0f;
}

cgv::g2d::rect framing_overlay::to_relative_frame(const cgv::g2d::rect& absolute_frame, cgv::vec2 viewport_size) {
	cgv::g2d::rect res = absolute_frame;
	res.position /= viewport_size;
	res.size /= viewport_size;
	return res;
}

cgv::g2d::rect framing_overlay::to_absolute_frame(const cgv::g2d::rect& relative_frame, cgv::vec2 viewport_size) {
	cgv::g2d::rect res = relative_frame;
	res.position *= viewport_size;
	res.size *= viewport_size;
	return res;
}

const float corner_handle_size = 14.0f;
const float min_frame_size = 128.0f;

void framing_overlay::create_handles() {
	handles.clear();

	if(is_frame_valid()) {
		// first handle is for complete frame
		handles.add({ 0.0f, 0.0f });
		// last 4 handles are for corners
		cgv::g2d::draggable rect({ 0.0f }, { corner_handle_size });
		rect.position_is_center = true;
		handles.add(rect);
		handles.add(rect);
		handles.add(rect);
		handles.add(rect);

		const auto center_constraint = cgv::g2d::draggable::ConstraintReference::CR_CENTER;
		handles[1].constraint_reference = center_constraint;
		handles[2].constraint_reference = center_constraint;
		handles[3].constraint_reference = center_constraint;
		handles[4].constraint_reference = center_constraint;

		update_handles();
	}
}

void framing_overlay::update_handles() {
	if(!handles.empty()) {
		cgv::vec2 a = frame_absolute.a();
		cgv::vec2 b = frame_absolute.b();

		// set first handle to position and size of frame
		handles[0].position = frame_absolute.position;
		handles[0].size = frame_absolute.size;
		// place next handles at corners, starting at bottom-left and going counter-clockwise
		handles[1].position = a;
		handles[2].position = { b.x(), a.y() };
		handles[3].position = b;
		handles[4].position = { a.x(), b.y() };
	}
}

void framing_overlay::handle_drag(cgv::g2d::DragAction action) {
	if(action == cgv::g2d::DragAction::kDrag) {
		cgv::g2d::draggable* handle = handles.get_dragged();
		if(handle) {
			size_t index = 0;
			for(size_t i = 0; i < handles.size(); ++i) {
				if(handle == &handles[i])
					index = i;
			}

			if(index == 0) {
				frame_absolute.position = handle->position;
			} else {
				if(index == 1 || index == 4) {
					handle->x() = std::min(handle->x(), frame_absolute.x1() - min_frame_size);
					frame_absolute.w() = frame_absolute.x1() - handle->x();
					frame_absolute.x() = handle->x();
				}

				if(index == 1 || index == 2) {
					handle->y() = std::min(handle->y(), frame_absolute.y1() - min_frame_size);
					frame_absolute.h() = frame_absolute.y1() - handle->y();
					frame_absolute.y() = handle->y();
				}

				if(index == 2 || index == 3) {
					handle->x() = std::max(handle->x(), frame_absolute.x() + min_frame_size);
					frame_absolute.w() = handle->x() - frame_absolute.x();
				}

				if(index == 3 || index == 4) {
					handle->y() = std::max(handle->y(), frame_absolute.y() + min_frame_size);
					frame_absolute.h() = handle->y() - frame_absolute.y();
				}
			}

			frame_relative = to_relative_frame(frame_absolute, get_viewport_size());
			update_handles();
			on_change();
			post_damage();
		}
	}
}
