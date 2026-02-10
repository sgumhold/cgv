#include "color_scale_legend.h"

#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/algorithm.h>
#include <cgv/utils/scan.h>
#include <cgv_g2d/msdf_gl_font_renderer.h>

namespace cgv {
namespace app {

color_scale_legend::color_scale_legend() {

	set_name("Color Map Legend");

	// TODO: Remove padding from layout and use get_content_rect() as a starting point instead.
	layout.padding = padding();
	layout.total_size = ivec2(300, 60);

	set_size(layout.total_size);

	tick_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::rectangle);

	label_format.precision = 0;
	label_format.trailing_zeros = false;
	label_format.decimal_integers = false;
	label_format.fixed = true;
	label_format.grouping = true;
}

void color_scale_legend::clear(cgv::render::context& ctx) {

	cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx, -1);

	canvas_overlay::clear(ctx);

	tex.destruct(ctx);

	tick_renderer.destruct(ctx);
	tick_geometry.destruct(ctx);
	label_geometry.destruct(ctx);
}

void color_scale_legend::handle_member_change(const cgv::utils::pointer_test& m) {

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

	if(m.one_of(layout.orientation, layout.label_alignment, num_ticks, nice_ticks, auto_precision) || m.member_of(label_format))
		post_recreate_layout();
}

bool color_scale_legend::init(cgv::render::context& ctx) {

	cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx, 1);

	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("grid", cgv::g2d::shaders::grid);

	bool success = canvas_overlay::init(ctx);
	
	success &= tick_renderer.init(ctx);
	success &= tick_geometry.init(ctx);
	success &= label_geometry.init(ctx);
	
	return success;
}

void color_scale_legend::init_frame(cgv::render::context& ctx) {

	if(ensure_layout(ctx)) {
		update_layout(get_rectangle().size);
		if(create_ticks(ctx)) {
			post_recreate_layout();
			post_damage();
		}

		float width_factor = static_cast<float>(layout.color_ramp_rect.w());
		float height_factor = static_cast<float>(layout.color_ramp_rect.h());
		background_style.texcoord_scaling = vec2(width_factor, height_factor) / 10.0f;
	}

	create_texture();
}

void color_scale_legend::draw_content(cgv::render::context& ctx) {

	begin_content(ctx);

	// draw inner border
	content_canvas.enable_shader(ctx, "rectangle");
	content_canvas.set_style(ctx, border_style);
	content_canvas.draw_shape(ctx, layout.color_ramp_rect.position - 1, layout.color_ramp_rect.size + 2);

	// draw background grid as contrast for transparent color maps or indicator that no color map is set
	content_canvas.enable_shader(ctx, "grid");
	content_canvas.set_style(ctx, background_style);
	content_canvas.draw_shape(ctx, layout.color_ramp_rect);

	if(tex.is_created()) {
		// draw the color map texture
		content_canvas.push_modelview_matrix();
		ivec2 pos = layout.color_ramp_rect.position;
		ivec2 size = layout.color_ramp_rect.size;
		float angle = 0.0f;

		if(layout.orientation == Orientation::kVertical) {
			pos.x() += layout.color_ramp_rect.size.x();
			std::swap(size.x(), size.y());
			angle = 90.0f;
		}

		content_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(pos));
		content_canvas.mul_modelview_matrix(ctx, cgv::math::rotate2h(angle));

		// draw color scale texture
		color_ramp_style.use_texture_alpha = show_opacity;

		if(flip_texture)
			color_ramp_style.texcoord_scaling.x() *= -1.0f;

		content_canvas.enable_shader(ctx, "rectangle");
		content_canvas.set_style(ctx, color_ramp_style);
		tex.enable(ctx, 0);
		content_canvas.draw_shape(ctx, ivec2(0), size);
		tex.disable(ctx);

		content_canvas.pop_modelview_matrix(ctx);
	}
	content_canvas.disable_current_shader(ctx);

	// draw tick marks
	tick_renderer.render(ctx, content_canvas, cgv::render::PT_POINTS, tick_geometry, tick_style);

	// draw tick labels
	auto& font_renderer = cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx);
	font_renderer.render(ctx, content_canvas, label_geometry, text_style);

	end_content(ctx);
}

void color_scale_legend::create_gui_impl() {

	add_member_control(this, "Width", layout.total_size[0], "value_slider", "min=40;max=500;step=1;ticks=true");
	add_member_control(this, "Height", layout.total_size[1], "value_slider", "min=40;max=500;step=1;ticks=true");

	add_member_control(this, "Background", background_visible_, "check", "w=100", " ");
	add_member_control(this, "Invert Color", invert_color, "check", "w=88");
	add_member_control(this, "Show Opacity", show_opacity, "check");

	add_member_control(this, "Orientation", layout.orientation, "dropdown", "enums='Horizontal,Vertical'");
	add_member_control(this, "Label Alignment", layout.label_alignment, "dropdown", "enums='-,Before,Inside,After'");

	add_member_control(this, "Ticks", num_ticks, "value", "min=2;max=20;step=1");
	add_member_control(this, "Number Precision", label_format.precision, "value", "w=28;min=0;max=10;step=1", " ");
	add_member_control(this, "Auto", auto_precision, "check", "w=52", "");
	add_member_control(this, "Show 0s", label_format.trailing_zeros, "check", "w=74", "");
	add_member_control(this, "Int", label_format.decimal_integers, "check", "w=40");
	add_member_control(this, "Fixed", label_format.fixed, "check");
	add_member_control(this, "Nice", nice_ticks, "check");
}

void color_scale_legend::set_color_scale(std::shared_ptr<const cgv::media::color_scale> color_scale) {
	if(this->color_scale != color_scale) {
		this->color_scale = color_scale;
		build_time.reset();
	}
	post_damage();
}

void color_scale_legend::set_width(size_t w) {
	layout.total_size.x() = int(w);
	on_set(&layout.total_size.x());
}

void color_scale_legend::set_height(size_t h) {
	layout.total_size.y() = int(h);
	on_set(&layout.total_size.y());
}

void color_scale_legend::set_title(const std::string& t) {
	title = t;
	on_set(&title);
}

void color_scale_legend::set_orientation(Orientation orientation) {
	layout.orientation = orientation;
	on_set(&layout.orientation);
}

void color_scale_legend::set_label_alignment(AlignmentOption alignment) {
	layout.label_alignment = alignment;
	on_set(&layout.label_alignment);
}

void color_scale_legend::set_invert_color(bool flag) {
	invert_color = flag;
	on_set(&invert_color);
}

void color_scale_legend::set_num_ticks(unsigned n) {
	num_ticks = n;
	on_set(&num_ticks);
}

void color_scale_legend::set_label_precision(unsigned p) {
	label_format.precision = p;
	on_set(&label_format.precision);
}

void color_scale_legend::set_label_auto_precision(bool f) {
	auto_precision = f;
	on_set(&auto_precision);
}

void color_scale_legend::set_label_prune_trailing_zeros(bool f) {
	label_format.trailing_zeros = !f;
	on_set(&label_format.trailing_zeros);
}

void color_scale_legend::set_label_integer_mode(bool enabled) {
	label_format.decimal_integers = enabled;
	on_set(&label_format.decimal_integers);
}

void color_scale_legend::set_show_opacity(bool enabled) {
	show_opacity = enabled;
	on_set(&show_opacity);
}

void color_scale_legend::init_styles() {
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
	color_ramp_style = border_style;
	color_ramp_style.use_texture = true;
	color_ramp_style.use_texture_alpha = true;
	color_ramp_style.use_blending = true;

	// configure text style
	text_style.fill_color = tick_color;
	text_style.font_size = 12.0f;
	
	// configure style for tick marks
	tick_style.position_is_center = true;
	tick_style.fill_color = tick_color;
	tick_style.feather_width = 0.0f;
}

void color_scale_legend::update_layout(const ivec2& parent_size) {
	ivec2 offset(0, 0);
	ivec2 size(parent_size);

	switch(layout.label_alignment) {
	case AO_START:
		if(layout.orientation == Orientation::kHorizontal) {
			offset.x() = layout.x_label_size / 2;
			offset.y() = layout.title_space;
			size.x() -= layout.x_label_size;
			size.y() -= layout.label_space + layout.title_space;
		} else {
			offset.x() = layout.x_label_size + 4;
			offset.y() = 0;
			size.x() -= layout.x_label_size + 4 + layout.title_space;
		}
		break;
	case AO_END:
		if(layout.orientation == Orientation::kHorizontal) {
			offset.x() = layout.x_label_size / 2;
			offset.y() = layout.label_space;
			size.x() -= layout.x_label_size;
			size.y() -= layout.label_space + layout.title_space;
		} else {
			offset.x() = layout.title_space;
			size.x() -= layout.x_label_size + 4 + layout.title_space;
		}
		break;
	default: break;
	}

	layout.color_ramp_rect.position = offset + layout.padding;
	layout.color_ramp_rect.size = size - 2 * layout.padding;
}

void color_scale_legend::create_texture() {
	if(!get_context())
		return;

	if(color_scale && color_scale->get_modified_time() > build_time.get_modified_time()) {
		const cgv::render::TextureFilter filter = color_scale->is_discrete() ? cgv::render::TF_NEAREST : cgv::render::TF_LINEAR;

		size_t resolution = 256;
		std::vector<rgba> colors = color_scale->quantize(resolution);
		resolution = colors.size();

		std::vector<cgv::rgba8> texture_data;
		texture_data.reserve(resolution);
		std::transform(colors.begin(), colors.end(), std::back_inserter(texture_data), [](const cgv::rgba& color) {
			return cgv::rgba8(color);
		});

		cgv::data::data_format data_format(resolution, 1, cgv::type::info::TI_UINT8, cgv::data::CF_RGBA);
		cgv::data::data_view data_view(&data_format, texture_data.data());

		tex.set_min_filter(filter);
		tex.set_mag_filter(filter);
		tex.create(*get_context(), data_view, 0);

		build_time.modified();
		post_recreate_layout();
		post_damage();
	}
}

bool color_scale_legend::create_ticks(const cgv::render::context& ctx) {
	label_geometry.clear();
	tick_geometry.clear();

	if(!color_scale || layout.label_alignment == AO_FREE) {
		layout.x_label_size = 0;
		return true;
	}

	const cgv::vec2 domain = color_scale->get_domain();

	std::vector<float> ticks;
	if(nice_ticks)
		ticks = color_scale->get_ticks(num_ticks);
	else
		cgv::utils::subdivision_sequence(std::back_inserter(ticks), domain[0], domain[1], num_ticks);

	if(ticks.empty()) {
		layout.x_label_size = 0;
		return true;
	}

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
	float title_angle = 0.0f;

	if(layout.orientation == Orientation::kVertical) {
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

		title_angle = 90.0f;
	}

	ivec2 title_position = layout.color_ramp_rect.position;
	ivec2 tick_start = layout.color_ramp_rect.position;

	cgv::render::TextAlignment title_alignment, text_alignment;
	title_alignment = title_alignment_1;
	text_alignment = text_v_end;

	bool inside = false;
	switch(label_alignment) {
	case AO_START:
		title_position[1 - axis] -= 4;
		tick_start[1 - axis] += layout.color_ramp_rect.size[1 - axis] + 3;
		break;
	case AO_CENTER:
		title_position[axis] += 2;
		title_position[1 - axis] += layout.color_ramp_rect.size[1 - axis] - (axis ? 3 : 1);
		tick_start[1 - axis] += 3;
		inside = true;
		break;
	case AO_END:
		title_alignment = title_alignment_2;
		title_position[1 - axis] += layout.color_ramp_rect.size[1 - axis] + 4;
		tick_start[1 - axis] -= 3;
		label_offset = -label_offset;
		text_alignment = text_v_start;
		break;
	default: break;
	}

	title_alignment = static_cast<cgv::render::TextAlignment>(title_alignment + cgv::render::TextAlignment::TA_LEFT);

	// formatting
	unsigned last_precision = label_format.precision;
	if(auto_precision)
		label_format.precision_from_range(domain[0], domain[1]);

	int length = layout.color_ramp_rect.size[axis] + 2; // +2 for border
	
	for(float tick : ticks) {
		int offset = static_cast<int>(std::round(color_scale->normalize_value(tick) * length));
		offset = cgv::math::clamp(offset, 0, length - 1);

		ivec2 tick_pos = tick_start;
		tick_pos[axis] += offset;

		ivec2 label_pos = tick_start;
		label_pos[1 - axis] += label_offset;
		label_pos[axis] += offset;

		tick_geometry.add(tick_pos, tick_size);
		label_geometry.texts.push_back(label_format.convert(tick));
		label_geometry.positions.push_back(vec3(label_pos, 0.0f));
	}

	// Restore precision
	label_format.precision = last_precision;

	label_geometry.alignments.resize(ticks.size(), text_alignment);
	label_geometry.rotations.resize(ticks.size());

	// Align first and last label inside rectangle if requested.
	if(inside) {
		label_geometry.positions.front()[axis] += 3.0f;
		label_geometry.alignments.front() = static_cast<cgv::render::TextAlignment>(text_alignment | text_h_start);
		label_geometry.positions.back()[axis] -= 3.0f;
		label_geometry.alignments.back() = static_cast<cgv::render::TextAlignment>(text_alignment | text_h_end);
	}

	label_geometry.texts.push_back(title);

	// title:
	label_geometry.positions.push_back(cgv::vec3(static_cast<cgv::vec2>(title_position), 0.0f));
	label_geometry.rotations.push_back(cgv::quat(cgv::vec3(0.0f, 0.0f, 1.0f), cgv::math::deg2rad(title_angle)));
	label_geometry.alignments.push_back(title_alignment);

	label_geometry.create(ctx);
	const std::vector<cgv::g2d::msdf_text_geometry::text_info>& text_infos = label_geometry.ref_text_infos();
	
	int x_label_size = 0;

	if(label_geometry.size() > 1) {
		if(layout.orientation == Orientation::kHorizontal) {
			x_label_size = static_cast<int>(std::max(text_infos.front().normalized_width, text_infos[text_infos.size() - 2].normalized_width) * text_style.font_size);
		} else {
			float max_width = -1.0f;
			for(size_t i = 0; i < ticks.size(); ++i)
				max_width = std::max(max_width, text_infos[i].normalized_width);
			x_label_size = static_cast<int>(max_width * text_style.font_size);
		}
	}
	
	if(layout.x_label_size != x_label_size) {
		layout.x_label_size = x_label_size;
		return true;
	}
	return false;
}

}
}
