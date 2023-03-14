#include "color_map_legend.h"

#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>

namespace cgv {
namespace app {

color_map_legend::color_map_legend() {

	set_name("Color Map Legend");
	block_events = false;
	blend_overlay = true;

	layout.padding = 13; // 10px plus 3px border
	layout.total_size = ivec2(300, 60);

	set_overlay_alignment(AO_START, AO_END);
	set_overlay_stretch(SO_NONE);
	set_overlay_margin(ivec2(-3));
	set_overlay_size(layout.total_size);

	tick_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::canvas::shaders_2d::rectangle);

	title = "";
	range = vec2(0.0f, 1.0f);
	num_ticks = 3;
	label_precision = 0;
	label_auto_precision = true;
	label_integer_mode = false;
	title_align = AO_START;
}

void color_map_legend::clear(cgv::render::context& ctx) {

	canvas_overlay::clear(ctx);

	tex.destruct(ctx);

	cgv::g2d::ref_msdf_font(ctx, -1);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);

	tick_renderer.destruct(ctx);
	ticks.destruct(ctx);
}

void color_map_legend::on_set(void* member_ptr) {

	if(member_ptr == &layout.total_size[0] || member_ptr == &layout.total_size[1]) {
		vec2 size = get_overlay_size();

		// TODO: minimum width and height depend on other layout parameters
		layout.total_size.y() = std::max(layout.total_size.y(), 2 * layout.padding + 4 + layout.label_space);

		set_overlay_size(layout.total_size);
	}

	if(member_ptr == &show_background || member_ptr == &invert_color) {
		auto ctx_ptr = get_context();
		if(ctx_ptr)
			init_styles(*ctx_ptr);
	}

	if(member_ptr == &num_ticks) {
		num_ticks = cgv::math::clamp(num_ticks, 2u, 100u);
	}

	if(member_ptr == &title) {
		layout.title_space = title == "" ? 0 : 12;
		post_recreate_layout();
	}

	if(member_ptr == &layout.orientation ||
		member_ptr == &layout.label_alignment ||
		member_ptr == &range ||
		member_ptr == &num_ticks ||
		member_ptr == &label_precision ||
		member_ptr == &label_auto_precision ||
		member_ptr == &label_integer_mode) {
		post_recreate_layout();
	}

	update_member(member_ptr);
	post_damage();
}

bool color_map_legend::init(cgv::render::context& ctx) {

	register_shader("rectangle", cgv::g2d::canvas::shaders_2d::rectangle);

	bool success = canvas_overlay::init(ctx);

	success &= tick_renderer.init(ctx);

	cgv::g2d::msdf_font& font = cgv::g2d::ref_msdf_font(ctx, 1);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);

	if(success)
		init_styles(ctx);

	if(font.is_initialized()) {
		labels.set_msdf_font(&font);
		labels.set_font_size(font_size);
	}

	return success;
}

void color_map_legend::init_frame(cgv::render::context& ctx) {

	if(ensure_layout(ctx)) {
		ivec2 container_size = get_overlay_size();
		create_labels();
		layout.update(container_size);
		create_ticks();
	}

	if(ensure_theme())
		init_styles(ctx);
}

void color_map_legend::draw_content(cgv::render::context& ctx) {

	begin_content(ctx);

	ivec2 container_size = get_overlay_size();

	auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");

	// draw container background
	if(show_background) {
		container_style.apply(ctx, rect_prog);
		content_canvas.draw_shape(ctx, ivec2(0), container_size);
	}

	// draw inner border
	border_style.apply(ctx, rect_prog);
	content_canvas.draw_shape(ctx, layout.color_map_rect.pos() - 1, layout.color_map_rect.size() + 2);

	if(tex.is_created()) {
		content_canvas.push_modelview_matrix();
		ivec2 pos = layout.color_map_rect.pos();
		ivec2 size = layout.color_map_rect.size();
		float angle = 0.0f;

		if(layout.orientation == OO_VERTICAL) {
			pos.x() += layout.color_map_rect.size().x();
			std::swap(size.x(), size.y());
			angle = 90.0f;
		}

		content_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(pos));
		content_canvas.mul_modelview_matrix(ctx, cgv::math::rotate2h(angle));

		// draw color scale texture
		color_map_style.apply(ctx, rect_prog);
		tex.enable(ctx, 0);
		content_canvas.draw_shape(ctx, ivec2(0), size);
		tex.disable(ctx);

		content_canvas.pop_modelview_matrix(ctx);
	}
	content_canvas.disable_current_shader(ctx);

	// draw tick marks
	tick_renderer.render(ctx, content_canvas, cgv::render::PT_POINTS, ticks);

	// draw tick labels
	auto& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx);
	if(font_renderer.enable(ctx, content_canvas, labels, text_style)) {
		font_renderer.draw(ctx, labels, 0, int(labels.size()) - 1);

		content_canvas.push_modelview_matrix();
		content_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(layout.title_position));
		content_canvas.mul_modelview_matrix(ctx, cgv::math::rotate2h(layout.title_angle));

		font_renderer.draw(ctx, content_canvas, labels, labels.size() - 1, 1);

		content_canvas.pop_modelview_matrix(ctx);
		font_renderer.disable(ctx, labels);
	}

	end_content(ctx);
}

void color_map_legend::create_gui_impl() {

	add_member_control(this, "Width", layout.total_size[0], "value_slider", "min=40;max=500;step=1;ticks=true");
	add_member_control(this, "Height", layout.total_size[1], "value_slider", "min=40;max=500;step=1;ticks=true");

	add_member_control(this, "Background", show_background, "check", "w=100", " ");
	add_member_control(this, "Invert Color", invert_color, "check", "w=88");

	add_member_control(this, "Orientation", layout.orientation, "dropdown", "enums='Horizontal,Vertical'");
	add_member_control(this, "Label Alignment", layout.label_alignment, "dropdown", "enums='-,Before,Inside,After'");

	add_member_control(this, "Ticks", num_ticks, "value", "min=2;max=10;step=1");
	add_member_control(this, "Number Precision", label_precision, "value", "w=60;min=0;max=10;step=1", " ");
	add_member_control(this, "Auto", label_auto_precision, "check", "w=44", " ");
	add_member_control(this, "Integers", label_integer_mode, "check", "w=72");
}

void color_map_legend::set_color_map(cgv::render::context& ctx, cgv::render::color_map& cm) {

	cgv::render::TextureFilter filter = cgv::render::TF_LINEAR;
	if(cm.has_texture_support()) {
		cgv::render::gl_color_map* gl_cm_ptr = dynamic_cast<cgv::render::gl_color_map*>(&cm);
		filter = gl_cm_ptr->is_linear_filtering_enabled() ? cgv::render::TF_LINEAR : cgv::render::TF_NEAREST;
	}

	unsigned resolution = cm.get_resolution();
	std::vector<rgb> data = cm.interpolate_color(static_cast<size_t>(resolution));

	std::vector<uint8_t> data_8(2 * 3 * data.size());
	for(unsigned i = 0; i < data.size(); ++i) {
		rgba col = data[i];
		data_8[3 * i + 0] = static_cast<uint8_t>(255.0f * col.R());
		data_8[3 * i + 1] = static_cast<uint8_t>(255.0f * col.G());
		data_8[3 * i + 2] = static_cast<uint8_t>(255.0f * col.B());
	}

	std::copy(data_8.begin(), data_8.begin() + 3*resolution, data_8.begin() + 3*resolution);

	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(resolution, 2u, TI_UINT8, cgv::data::CF_RGB), data_8.data());

	unsigned width = tex.get_width();

	bool replaced = false;
	if(tex.is_created() && width == resolution && tex.get_nr_components() == 3) {
		tex.set_min_filter(filter);
		tex.set_mag_filter(filter);
		replaced = tex.replace(ctx, 0, 0, dv);
	}

	if(!replaced) {
		tex.destruct(ctx);
		tex = cgv::render::texture("uint8[R,G,B]", filter, filter);
		tex.create(ctx, dv, 0);
	}

	post_damage();
}

void color_map_legend::set_width(size_t w) {
	layout.total_size.x() = int(w);
	on_set(&layout.total_size.x());
}

void color_map_legend::set_height(size_t h) {
	layout.total_size.y() = int(h);
	on_set(&layout.total_size.y());
}

void color_map_legend::set_title(const std::string& t) {
	title = t;
	on_set(&title);
}

void color_map_legend::set_range(vec2 r) {
	range = r;
	on_set(&range);
}

void color_map_legend::set_num_ticks(unsigned n) {
	num_ticks = n;
	on_set(&num_ticks);
}

void color_map_legend::set_label_precision(unsigned p) {
	label_precision = p;
	on_set(&label_precision);
}

void color_map_legend::set_label_auto_precision(bool f) {
	label_auto_precision = f;
	on_set(&label_auto_precision);
}

void color_map_legend::set_label_integer_mode(bool enabled) {
	label_integer_mode = enabled;
	on_set(&label_integer_mode);
}

void color_map_legend::init_styles(cgv::render::context& ctx) {
	// get theme colors
	auto& ti = cgv::gui::theme_info::instance();
	rgba background_color = rgba(ti.background(), 1.0f);
	rgba group_color = rgba(ti.group(), 1.0f);
	rgba border_color = rgba(ti.border(), 1.0f);
	rgb tick_color = ti.text();

	if(invert_color) {
		tick_color.R() = pow(1.0f - pow(tick_color.R(), 2.2f), 1.0f/2.2f);
		tick_color.G() = pow(1.0f - pow(tick_color.G(), 2.2f), 1.0f/2.2f);
		tick_color.B() = pow(1.0f - pow(tick_color.B(), 2.2f), 1.0f/2.2f);
	}

	// configure style for the container rectangle
	container_style.fill_color = group_color;
	container_style.border_color = background_color;
	container_style.border_width = 3.0f;
	container_style.feather_width = 0.0f;

	// configure style for the border rectangle
	border_style = container_style;
	border_style.fill_color = rgba(tick_color, 1.0);
	border_style.border_width = 0.0f;

	// configure style for the color scale rectangle
	color_map_style = border_style;
	color_map_style.use_texture = true;
	color_map_style.use_texture_alpha = false;

	// configure text style
	float label_border_alpha = 0.0f;
	float border_width = 0.25f;
	if(!show_background) {
		label_border_alpha = 1.0f;
		border_width = 0.0f;
	}

	text_style.fill_color = rgba(tick_color, 1.0f);
	text_style.border_color = rgba(tick_color, label_border_alpha);
	text_style.border_width = border_width;
	text_style.feather_origin = 0.5f;
	text_style.use_blending = true;

	// configure style for tick marks
	cgv::g2d::shape2d_style tick_style;
	tick_style.position_is_center = true;
	tick_style.fill_color = rgba(tick_color, 1.0f);
	tick_style.feather_width = 0.0f;

	tick_renderer.set_style(ctx, tick_style);
}

void color_map_legend::create_labels() {

	labels.clear();

	if(layout.label_alignment == AO_FREE)
		return;

	unsigned precision = label_precision;
	if(label_auto_precision) {
		float m = std::max(abs(range.x()), abs(range.y()));
		if(m >= 10000.0f) {
			precision = 0;
		} else {
			if(m >= 1000.0f)
				precision = 4;
			else if(m >= 100.0f)
				precision = 3;
			else if(m >= 10.0f)
				precision = 2;
			else if(m >= 1.0f)
				precision = 1;
			else
				precision = 0;
		}
	}

	float max_length = -1.0f;

	for(size_t i = 0; i < num_ticks; ++i) {
		float fi = static_cast<float>(i);
		float t = fi / static_cast<float>(num_ticks - 1);
		float val = cgv::math::lerp(range.x(), range.y(), t);

		std::string str = "";

		if(label_integer_mode)
			str = std::to_string(static_cast<int>(round(val)));
		else
			str = cgv::utils::to_string(val, -1, precision);

		labels.add_text(str, ivec2(0), cgv::render::TextAlignment::TA_NONE);
		max_length = std::max(max_length, labels.ref_texts().back().size.x());
	}

	if(labels.size() > 1) {
		if(layout.orientation == OO_HORIZONTAL)
			layout.x_label_size = std::max(labels.ref_texts().front().size.x(), labels.ref_texts().back().size.x()) * labels.get_font_size();
		else
			layout.x_label_size = int(max_length * labels.get_font_size());
	} else {
		layout.x_label_size = 0;
	}

	labels.add_text(title, ivec2(0), cgv::render::TextAlignment::TA_BOTTOM_LEFT);
}

void color_map_legend::create_ticks() {

	ticks.clear();

	if(layout.label_alignment == AO_FREE)
		return;

	ivec2 tick_size(1, 6);

	int axis = 0;
	int label_offset = 4;
	AlignmentOption label_alignment = layout.label_alignment;
	cgv::render::TextAlignment text_h_start = cgv::render::TextAlignment::TA_LEFT;
	cgv::render::TextAlignment text_h_end = cgv::render::TextAlignment::TA_RIGHT;
	cgv::render::TextAlignment text_v_start = cgv::render::TextAlignment::TA_TOP;
	cgv::render::TextAlignment text_v_end = cgv::render::TextAlignment::TA_BOTTOM;
	cgv::render::TextAlignment title_alignment_1 = cgv::render::TextAlignment::TA_TOP;
	cgv::render::TextAlignment title_alignment_2 = cgv::render::TextAlignment::TA_BOTTOM;

	layout.title_angle = 0.0f;

	if(layout.orientation == OO_VERTICAL) {
		axis = 1;
		label_offset = 6;

		std::swap(tick_size.x(), tick_size.y());

		if(label_alignment == AO_START) label_alignment = AO_END;
		else if(label_alignment == AO_END) label_alignment = AO_START;

		text_h_start = cgv::render::TextAlignment::TA_BOTTOM;
		text_h_end = cgv::render::TextAlignment::TA_TOP;
		text_v_start = cgv::render::TextAlignment::TA_RIGHT;
		text_v_end = cgv::render::TextAlignment::TA_LEFT;
		std::swap(title_alignment_1, title_alignment_2);

		layout.title_angle = 90.0f;
	}

	ivec2 color_rect_pos = layout.color_map_rect.pos();
	ivec2 color_rect_size = layout.color_map_rect.size();

	int length = color_rect_size[axis];
	float step = static_cast<float>(length + 1) / static_cast<float>(num_ticks - 1);

	ivec2 title_pos = color_rect_pos;
	ivec2 tick_start = color_rect_pos;

	cgv::render::TextAlignment title_alignment, text_alignment;
	title_alignment = title_alignment_1;

	bool inside = false;
	switch(label_alignment) {
	case AO_START:
		title_pos[1 - axis] -= 4;
		tick_start[1 - axis] += color_rect_size[1 - axis] + 3;
		text_alignment = text_v_end;
		break;
	case AO_CENTER:
		title_pos[axis] += 2;
		title_pos[1 - axis] += color_rect_size[1 - axis] - (axis ? 3 : 1);
		tick_start[1 - axis] += 3;
		inside = true;
		text_alignment = text_v_end;
		break;
	case AO_END:
		title_alignment = title_alignment_2;
		title_pos[1 - axis] += color_rect_size[1 - axis] + 4;
		tick_start[1 - axis] -= 3;
		label_offset = -label_offset;
		text_alignment = text_v_start;
		break;
	default: break;
	}

	title_alignment = static_cast<cgv::render::TextAlignment>(title_alignment + cgv::render::TextAlignment::TA_LEFT);

	for(size_t i = 0; i < num_ticks; ++i) {
		float fi = static_cast<float>(i);
		int offset = static_cast<int>(round(fi * step));

		ivec2 tick_pos = tick_start;
		tick_pos[axis] += offset;

		ivec2 label_pos = tick_start;
		label_pos[1 - axis] += label_offset;
		label_pos[axis] += offset;

		cgv::render::TextAlignment alignment = text_alignment;
		if(inside) {
			if(i == 0) {
				label_pos[axis] += 3;
				alignment = static_cast<cgv::render::TextAlignment>(alignment | text_h_start);
			} else if(i == num_ticks - 1) {
				label_pos[axis] -= 3;
				alignment = static_cast<cgv::render::TextAlignment>(alignment | text_h_end);
			}
		}

		ticks.add(tick_pos, tick_size);

		labels.set_position(int(i), label_pos);
		labels.set_alignment(int(i), alignment);
	}

	layout.title_position = title_pos;

	if(labels.size() > 0)
		labels.set_alignment(unsigned(labels.ref_texts().size() - 1), title_alignment);
}

}
}
