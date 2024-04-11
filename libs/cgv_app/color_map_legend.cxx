#include "color_map_legend.h"

#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/scan.h>

namespace cgv {
namespace app {

color_map_legend::color_map_legend() {

	set_name("Color Map Legend");

	// TODO: Remove padding from layout and use get_content_rect() as a starting point instead.
	layout.padding = padding();
	layout.total_size = ivec2(300, 60);

	set_size(layout.total_size);

	tick_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::rectangle);
}

void color_map_legend::clear(cgv::render::context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);

	canvas_overlay::clear(ctx);

	tex.destruct(ctx);

	tick_renderer.destruct(ctx);
	ticks.destruct(ctx);
	labels.destruct(ctx);
}

void color_map_legend::handle_member_change(const cgv::utils::pointer_test& m) {

	if(m.member_of(layout.total_size)) {
		// TODO: minimum width and height depend on other layout parameters
		layout.total_size.y() = std::max(layout.total_size.y(), 2 * layout.padding + 4 + layout.label_space);
		set_size(layout.total_size);
	}

	if(m.one_of(background_visible_, invert_color))
		init_styles();

	if(m.is(num_ticks))
		num_ticks = cgv::math::clamp(num_ticks, 2u, 100u);

	if(m.is(title)) {
		layout.title_space = title == "" ? 0 : 12;
		post_recreate_layout();
	}

	if(m.one_of(layout.orientation, layout.label_alignment, value_range, num_ticks) || m.member_of(label_format))
		post_recreate_layout();	
}

bool color_map_legend::init(cgv::render::context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);

	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("grid", cgv::g2d::shaders::grid);

	bool success = canvas_overlay::init(ctx);

	success &= tick_renderer.init(ctx);

	labels.init(ctx);
	
	return success;
}

void color_map_legend::init_frame(cgv::render::context& ctx) {

	if(ensure_layout(ctx)) {
		create_labels();
		layout.update(get_rectangle().size);
		create_ticks();

		float width_factor = static_cast<float>(layout.color_map_rect.w());
		float height_factor = static_cast<float>(layout.color_map_rect.h());
		background_style.texcoord_scaling = vec2(width_factor, height_factor) / 10.0f;
	}
}

void color_map_legend::draw_content(cgv::render::context& ctx) {

	begin_content(ctx);

	// draw inner border
	content_canvas.enable_shader(ctx, "rectangle");
	content_canvas.set_style(ctx, border_style);
	content_canvas.draw_shape(ctx, layout.color_map_rect.position - 1, layout.color_map_rect.size + 2);

	// draw background grid as contrast for transparent color maps or indicator that no color map is set
	content_canvas.enable_shader(ctx, "grid");
	content_canvas.set_style(ctx, background_style);
	content_canvas.draw_shape(ctx, layout.color_map_rect);

	if(tex.is_created()) {
		// draw the color map texture
		content_canvas.push_modelview_matrix();
		ivec2 pos = layout.color_map_rect.position;
		ivec2 size = layout.color_map_rect.size;
		float angle = 0.0f;

		if(layout.orientation == OO_VERTICAL) {
			pos.x() += layout.color_map_rect.size.x();
			std::swap(size.x(), size.y());
			angle = 90.0f;
		}

		content_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(pos));
		content_canvas.mul_modelview_matrix(ctx, cgv::math::rotate2h(angle));

		// draw color scale texture
		color_map_style.use_texture_alpha = show_opacity;

		color_map_style.texcoord_offset.x() = display_range[0];
		color_map_style.texcoord_scaling.x() = display_range[1] - display_range[0];
		
		if(flip_texture)
			color_map_style.texcoord_scaling.x() *= -1.0f;

		content_canvas.enable_shader(ctx, "rectangle");
		content_canvas.set_style(ctx, color_map_style);
		tex.enable(ctx, 0);
		content_canvas.draw_shape(ctx, ivec2(0), size);
		tex.disable(ctx);

		content_canvas.pop_modelview_matrix(ctx);
	}
	content_canvas.disable_current_shader(ctx);

	// draw tick marks
	tick_renderer.render(ctx, content_canvas, cgv::render::PT_POINTS, ticks, tick_style);

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

	add_member_control(this, "Background", background_visible_, "check", "w=100", " ");
	add_member_control(this, "Invert Color", invert_color, "check", "w=88");
	add_member_control(this, "Show Opacity", show_opacity, "check");

	add_member_control(this, "Orientation", layout.orientation, "dropdown", "enums='Horizontal,Vertical'");
	add_member_control(this, "Label Alignment", layout.label_alignment, "dropdown", "enums='-,Before,Inside,After'");

	add_member_control(this, "Ticks", num_ticks, "value", "min=2;max=10;step=1");
	add_member_control(this, "Number Precision", label_format.precision, "value", "w=28;min=0;max=10;step=1", " ");
	add_member_control(this, "Auto", label_format.auto_precision, "check", "w=52", "");
	add_member_control(this, "Show 0s", label_format.trailing_zeros, "check", "w=74", "");
	add_member_control(this, "Int", label_format.integers, "check", "w=40");

	add_gui("", color_map_style);
}

void color_map_legend::set_color_map(cgv::render::context& ctx, const cgv::render::color_map& cm) {

	cgv::render::TextureFilter filter = cgv::render::TF_LINEAR;
	if(cm.has_texture_support()) {
		const cgv::render::gl_color_map* gl_cm_ptr = dynamic_cast<const cgv::render::gl_color_map*>(&cm);
		filter = gl_cm_ptr->is_linear_filtering_enabled() ? cgv::render::TF_LINEAR : cgv::render::TF_NEAREST;
	}

	unsigned resolution = cm.get_resolution();
	std::vector<rgb> color_data = cm.interpolate_color(static_cast<size_t>(resolution));
	std::vector<float> opacity_data(static_cast<size_t>(resolution), 1.0f);
	
	if(!cm.ref_opacity_points().empty())
		opacity_data = cm.interpolate_opacity(static_cast<size_t>(resolution));
	
	std::vector<uint8_t> data_8(2 * 4 * color_data.size());
	for(unsigned i = 0; i < color_data.size(); ++i) {
		rgba col = color_data[i];
		data_8[4 * i + 0] = static_cast<uint8_t>(255.0f * col.R());
		data_8[4 * i + 1] = static_cast<uint8_t>(255.0f * col.G());
		data_8[4 * i + 2] = static_cast<uint8_t>(255.0f * col.B());
		data_8[4 * i + 3] = static_cast<uint8_t>(255.0f * opacity_data[i]);
	}

	std::copy(data_8.begin(), data_8.begin() + 4 * resolution, data_8.begin() + 4 * resolution);

	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(resolution, 2u, TI_UINT8, cgv::data::CF_RGBA), data_8.data());

	unsigned width = (unsigned)tex.get_width();

	bool replaced = false;
	if(tex.is_created() && width == resolution && tex.get_nr_components() == 4) {
		tex.set_min_filter(filter);
		tex.set_mag_filter(filter);
		replaced = tex.replace(ctx, 0, 0, dv);
	}

	if(!replaced) {
		tex.destruct(ctx);
		tex = cgv::render::texture("uint8[R,G,B,A]", filter, filter);
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
	flip_texture = r.x() > r.y();
	if(flip_texture)
		std::swap(r.x(), r.y());

	value_range = r;
	on_set(&value_range);
}

void color_map_legend::set_display_range(vec2 r) {
	display_range = r;
	on_set(&display_range);
}

void color_map_legend::set_invert_color(bool flag) {
	invert_color = flag;
	on_set(&invert_color);
}

void color_map_legend::set_num_ticks(unsigned n) {
	num_ticks = n;
	on_set(&num_ticks);
}

void color_map_legend::set_label_precision(unsigned p) {
	label_format.precision = p;
	on_set(&label_format.precision);
}

void color_map_legend::set_label_auto_precision(bool f) {
	label_format.auto_precision = f;
	on_set(&label_format.auto_precision);
}

void color_map_legend::set_label_prune_trailing_zeros(bool f) {
	label_format.trailing_zeros = !f;
	on_set(&label_format.trailing_zeros);
}

void color_map_legend::set_label_integer_mode(bool enabled) {
	label_format.integers = enabled;
	on_set(&label_format.integers);
}

void color_map_legend::set_show_opacity(bool enabled) {
	show_opacity = enabled;
	on_set(&show_opacity);
}

void color_map_legend::init_styles() {
	auto& theme = cgv::gui::theme_info::instance();
	rgb tick_color = theme.text();

	if(invert_color)
		tick_color = pow(rgb(1.0f) - pow(tick_color, 2.2f), 1.0f / 2.2f);

	// configure style for the border rectangle
	border_style.feather_width = 0.0f;
	border_style.fill_color = tick_color;
	border_style.border_width = 0.0f;

	// configure style for the background rectangle
	background_style.fill_color = rgb(0.9f);
	background_style.border_color = rgb(0.75f);
	background_style.feather_width = 0.0f;
	background_style.pattern = cgv::g2d::grid2d_style::GridPattern::GP_CHECKER;
	
	// configure style for the color scale rectangle
	color_map_style = border_style;
	color_map_style.use_texture = true;
	color_map_style.use_texture_alpha = true;
	color_map_style.use_blending = true;

	// configure text style
	text_style.fill_color = tick_color;
	text_style.font_size = 12.0f;
	
	// configure style for tick marks
	tick_style.position_is_center = true;
	tick_style.fill_color = tick_color;
	tick_style.feather_width = 0.0f;
}

void color_map_legend::create_labels() {

	labels.clear();

	if(layout.label_alignment == AO_FREE)
		return;

	unsigned precision = label_format.precision;
	
	if(label_format.auto_precision) {
		precision = 0;
		const float delta = std::abs(value_range[1] - value_range[0]);
		const unsigned max_precision = 7;

		if(delta > 5.0f) {
			precision = 1;
		} else {
			float limit = 1.0f;
			for(unsigned i = 2; i <= max_precision; ++i) {
				if(delta > limit || i == max_precision) {
					precision = i;
					break;
				}
				limit /= 2.0f;
			}
		}
	}

	float max_length = -1.0f;

	for(size_t i = 0; i < num_ticks; ++i) {
		float fi = static_cast<float>(i);
		float t = fi / static_cast<float>(num_ticks - 1);
		float val = cgv::math::lerp(value_range.x(), value_range.y(), t);

		std::string str;

		if(label_format.integers)
			str = std::to_string(static_cast<int>(round(val)));
		else {
			str = cgv::utils::to_string(val, -1, precision, true);
			if(!label_format.trailing_zeros && str.length() > 1)
				cgv::utils::rtrim(cgv::utils::rtrim(str, "0"), ".");
		}

		labels.add_text(str, ivec2(0), cgv::render::TextAlignment::TA_NONE);
		max_length = std::max(max_length, labels.ref_texts().back().size.x());
	}

	if(labels.size() > 1) {
		if(layout.orientation == OO_HORIZONTAL)
			layout.x_label_size = static_cast<int>(std::max(labels.ref_texts().front().size.x(), labels.ref_texts().back().size.x()) * text_style.font_size);
		else
			layout.x_label_size = static_cast<int>(max_length * text_style.font_size);
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

	ivec2 color_rect_pos = layout.color_map_rect.position;
	ivec2 color_rect_size = layout.color_map_rect.size;

	int length = color_rect_size[axis];
	float step = static_cast<float>(length + 1) / static_cast<float>(num_ticks - 1);

	ivec2 title_pos = color_rect_pos;
	ivec2 tick_start = color_rect_pos;

	cgv::render::TextAlignment title_alignment, text_alignment;
	title_alignment = title_alignment_1;
	text_alignment = text_v_end;

	bool inside = false;
	switch(label_alignment) {
	case AO_START:
		title_pos[1 - axis] -= 4;
		tick_start[1 - axis] += color_rect_size[1 - axis] + 3;
		break;
	case AO_CENTER:
		title_pos[axis] += 2;
		title_pos[1 - axis] += color_rect_size[1 - axis] - (axis ? 3 : 1);
		tick_start[1 - axis] += 3;
		inside = true;
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
