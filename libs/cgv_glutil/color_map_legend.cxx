#include "color_map_legend.h"

#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>

namespace cgv {
namespace glutil {

color_map_legend::color_map_legend() {

	set_name("Color Map Legend");

	layout.padding = 13; // 10px plus 3px border
	layout.total_size = ivec2(300, 300);

	set_overlay_alignment(AO_START, AO_END);
	set_overlay_stretch(SO_NONE);
	set_overlay_margin(ivec2(-3));
	set_overlay_size(layout.total_size);

	fbc.add_attachment("color", "flt32[R,G,B,A]");
	fbc.set_size(get_overlay_size());

	_canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);

	overlay_canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);

	tick_renderer = generic_renderer(canvas::shaders_2d::rectangle);

	range = vec2(0.0f, 1.0f);
	num_ticks = 3;
	label_auto_precision = true;
	label_precision = 0;
}

void color_map_legend::clear(cgv::render::context& ctx) {

	tex.clear();
	tex.destruct(ctx);

	font.destruct(ctx);
	font_renderer.destruct(ctx);

	tick_renderer.destruct(ctx);

	_canvas.destruct(ctx);
	overlay_canvas.destruct(ctx);
	fbc.clear(ctx);
}

bool color_map_legend::self_reflect(cgv::reflect::reflection_handler& _rh) {

	return false;
}

bool color_map_legend::handle_event(cgv::gui::event& e) {

	return false;
}

void color_map_legend::on_set(void* member_ptr) {

	if(member_ptr == &layout.total_size[0] || member_ptr == &layout.total_size[1]) {
		vec2 size = get_overlay_size();

		// TODO: minimum width and height depend on other layout parameters
		//layout.total_size.y() = std::max(layout.total_size.y(), 2 * layout.padding + 4 + layout.label_space);
		layout.total_size.y() = std::max(layout.total_size.y(), 2 * layout.padding + 4 + layout.label_space);

		set_overlay_size(layout.total_size);
	}

	if(member_ptr == &show_background || member_ptr == &invert_color) {
		auto ctx_ptr = get_context();
		if(ctx_ptr)
			init_styles(*ctx_ptr);
	}

	if(member_ptr == &range[0] || member_ptr == &range[1] || member_ptr == &layout.orientation || member_ptr == &layout.label_alignment || member_ptr == &layout.x_label_size) {
		layout.set_label_space();
		update_layout = true;
	}

	if(member_ptr == &range ||
		member_ptr == &num_ticks ||
		member_ptr == &label_auto_precision ||
		member_ptr == &label_precision) {
		num_ticks = cgv::math::clamp(num_ticks, 2u, 100u);
		create_ticks();
	}

	set_damaged();
	update_member(member_ptr);
	post_redraw();
}

bool color_map_legend::init(cgv::render::context& ctx) {

	bool success = true;

	success &= fbc.ensure(ctx);
	success &= _canvas.init(ctx);
	success &= overlay_canvas.init(ctx);

	success &= font_renderer.init(ctx);
	success &= tick_renderer.init(ctx);

	if(success)
		init_styles(ctx);
#ifndef CGV_FORCE_STATIC 
	if(font.init(ctx)) {
		labels.set_msdf_font(&font);
		labels.set_font_size(font_size);
	}
#endif
	return success;
}

void color_map_legend::init_frame(cgv::render::context& ctx) {

#ifdef CGV_FORCE_STATIC
	if(!font.is_initialized()) {
		if(font.init(ctx)) {
			labels.set_msdf_font(&font);
			labels.set_font_size(font_size);
		}
	}
#endif
	if(ensure_overlay_layout(ctx) || update_layout) {
		update_layout = false;

		ivec2 container_size = get_overlay_size();
		layout.update(container_size);

		fbc.set_size(container_size);
		fbc.ensure(ctx);

		_canvas.set_resolution(ctx, container_size);
		overlay_canvas.set_resolution(ctx, get_viewport_size());

		create_ticks();

		has_damage = true;
	}

	int theme_idx = cgv::gui::theme_info::instance().get_theme_idx();
	if(last_theme_idx != theme_idx) {
		last_theme_idx = theme_idx;
		init_styles(ctx);
		has_damage = true;
	}
}

void color_map_legend::draw(cgv::render::context& ctx) {

	if(!show)
		return;

	if(has_damage)
		draw_content(ctx);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw frame buffer texture to screen
	auto& blit_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	fbc.enable_attachment(ctx, "color", 0);
	overlay_canvas.draw_shape(ctx, get_overlay_position(), get_overlay_size());
	fbc.disable_attachment(ctx, "color");

	overlay_canvas.disable_current_shader(ctx);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void color_map_legend::draw_content(cgv::render::context& ctx) {

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//TODO: stupt proper bak to front blending equation

	fbc.enable(ctx);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ivec2 container_size = get_overlay_size();

	auto& rect_prog = _canvas.enable_shader(ctx, "rectangle");

	// draw container background
	if(show_background) {
		container_style.apply(ctx, rect_prog);
		_canvas.draw_shape(ctx, ivec2(0), container_size);
	}

	// draw inner border
	border_style.apply(ctx, rect_prog);
	_canvas.draw_shape(ctx, layout.color_map_rect.pos() - 1, layout.color_map_rect.size() + 2);

	if(tex.is_created()) {
		_canvas.push_modelview_matrix();
		ivec2 pos = layout.color_map_rect.pos();
		ivec2 size = layout.color_map_rect.size();
		float angle = 0.0f;

		if(layout.orientation == OO_VERTICAL) {
			pos.x() += layout.color_map_rect.size().x();
			std::swap(size.x(), size.y());
			angle = 90.0f;
		}

		_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(pos));
		_canvas.mul_modelview_matrix(ctx, cgv::math::rotate2h(angle));

		// draw color scale texture
		color_map_style.apply(ctx, rect_prog);
		tex.enable(ctx, 0);
		_canvas.draw_shape(ctx, ivec2(0), size);
		tex.disable(ctx);

		_canvas.pop_modelview_matrix(ctx);
	}
	_canvas.disable_current_shader(ctx);

	// draw tick marks
	auto& tick_prog = tick_renderer.ref_prog();
	tick_prog.enable(ctx);
	_canvas.set_view(ctx, tick_prog);
	tick_prog.disable(ctx);
	tick_renderer.render(ctx, PT_POINTS, ticks);

	// draw tick labels
	auto& font_prog = font_renderer.ref_prog();
	font_prog.enable(ctx);
	text_style.apply(ctx, font_prog);
	_canvas.set_view(ctx, font_prog);
	font_prog.disable(ctx);
	font_renderer.render(ctx, get_overlay_size(), labels);

	fbc.disable(ctx);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	has_damage = false;
}

void color_map_legend::create_gui() {

	create_overlay_gui();
	add_member_control(this, "Width", layout.total_size[0], "value_slider", "min=40;max=500;step=1;ticks=true");
	add_member_control(this, "Height", layout.total_size[1], "value_slider", "min=40;max=500;step=1;ticks=true");
	add_member_control(this, "LS", layout.x_label_size, "value_slider", "min=0;max=100;step=1;ticks=true");

	add_member_control(this, "Range A", range[0], "value_slider", "min=-10000;max=0;step=1;ticks=true;log=true");
	add_member_control(this, "Range B", range[1], "value_slider", "min=0;max=10000;step=1;ticks=true;log=true");

	add_member_control(this, "Background", show_background, "check", "w=100", " ");
	add_member_control(this, "Invert Color", invert_color, "check");

	add_member_control(this, "Orientation", layout.orientation, "dropdown", "enums='Horizontal,Vertical'");
	add_member_control(this, "Label Alignment", layout.label_alignment, "dropdown", "enums='-,Before,Inside,After'");

	add_member_control(this, "Ticks", num_ticks, "value", "min=2;max=10;step=1");
	std::string options = "w=140;min=0;max=10;step=1";//; active = ";
	//options += label_auto_precision ? "false" : "true";
	add_member_control(this, "Number Precision", label_precision, "value", options, " ");
	add_member_control(this, "Auto", label_auto_precision, "check");
}

void color_map_legend::create_gui(cgv::gui::provider& p) {}

void color_map_legend::set_color_map(cgv::render::context& ctx, color_map& cm) {

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
	if(tex.is_created() && width == resolution && tex.get_nr_components() == 3)
		replaced = tex.replace(ctx, 0, 0, dv);

	if(!replaced) {
		tex.destruct(ctx);
		tex = cgv::render::texture("uint8[R,G,B]", cgv::render::TF_LINEAR, cgv::render::TF_LINEAR);
		tex.create(ctx, dv, 0);
	}

	set_damaged();
	post_redraw();
}

void color_map_legend::set_range(vec2 r) {
	range = r;
	on_set(&range);
}

void color_map_legend::set_num_ticks(unsigned n) {
	num_ticks = n;
	on_set(&num_ticks);
}

void color_map_legend::set_label_auto_precision(bool f) {
	label_auto_precision = f;
	on_set(&label_auto_precision);
}

void color_map_legend::set_label_precision(unsigned p) {
	label_precision = p;
	on_set(&label_precision);
}

void color_map_legend::set_damaged() {
	has_damage = true;
}

void color_map_legend::init_styles(context& ctx) {
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
	container_style.apply_gamma = false;
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
	shape2d_style tick_style;
	tick_style.position_is_center = true;
	tick_style.fill_color = rgba(tick_color, 1.0f);
	tick_style.feather_width = 0.0f;

	auto& tick_prog = tick_renderer.ref_prog();
	tick_prog.enable(ctx);
	tick_style.apply(ctx, tick_prog);
	tick_prog.disable(ctx);

	// configure style for final blitting of overlay into main frame buffer
	shape2d_style blit_style;
	blit_style.fill_color = rgba(1.0f);
	blit_style.use_texture = true;
	blit_style.use_blending = true;
	blit_style.feather_width = 0.0f;

	auto& blit_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	blit_style.apply(ctx, blit_prog);
	overlay_canvas.disable_current_shader(ctx);
}

void color_map_legend::create_ticks() {

	ticks.clear();
	labels.clear();

	ivec2 tick_size(1, 6);

	int axis = 0;
	int label_offset = 4;
	AlignmentOption label_alignment = layout.label_alignment;
	cgv::render::TextAlignment text_h_start = cgv::render::TextAlignment::TA_LEFT;
	cgv::render::TextAlignment text_h_end = cgv::render::TextAlignment::TA_RIGHT;
	cgv::render::TextAlignment text_v_start = cgv::render::TextAlignment::TA_TOP;
	cgv::render::TextAlignment text_v_end = cgv::render::TextAlignment::TA_BOTTOM;

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
	}

	ivec2 color_rect_pos = layout.color_map_rect.pos();
	ivec2 color_rect_size = layout.color_map_rect.size();

	int length = color_rect_size[axis];
	float step = static_cast<float>(length + 1) / static_cast<float>(num_ticks - 1);

	ivec2 tick_start = color_rect_pos;
	
	cgv::render::TextAlignment text_alignment;

	bool inside = false;
	switch(label_alignment) {
	case AO_START:
		tick_start[1 - axis] += color_rect_size[1 - axis] + 3;
		text_alignment = text_v_end;
		break;
	case AO_CENTER:
		tick_start[1 - axis] += 3;
		inside = true;
		text_alignment = text_v_end;
		break;
	case AO_END:
		tick_start[1 - axis] -= 3;
		label_offset = -label_offset;
		text_alignment = text_v_start;
		break;
	default: break;
	}

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

	for(size_t i = 0; i < num_ticks; ++i) {
		float fi = static_cast<float>(i);
		int offset = static_cast<int>(round(fi * step));
		float t = fi / static_cast<float>(num_ticks - 1);
		float val = cgv::math::lerp(range.x(), range.y(), t);
		std::string str = cgv::utils::to_string(val, -1, precision);

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
		labels.add_text(str, label_pos, alignment);
	}
}

}
}
