#include "color_selector.h"

#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
namespace glutil {

color_selector::color_selector() {

	set_name("Color Selector");

	layout.padding = 13; // 10px plus 3px border

	set_overlay_alignment(AO_END, AO_END);
	set_overlay_stretch(SO_NONE);
	set_overlay_margin(ivec2(-3));
	set_overlay_size(ivec2(layout.size));
	
	register_shader("rectangle", cgv::g2d::canvas::shaders_2d::rectangle);
	register_shader("circle", cgv::g2d::canvas::shaders_2d::circle);
	register_shader("grid", cgv::g2d::canvas::shaders_2d::grid);
	
	selector_handles.set_drag_callback(std::bind(&color_selector::handle_selector_drag, this));
	selector_handles.set_use_individual_constraints(true);
}

void color_selector::clear(cgv::render::context& ctx) {

	canvas_overlay::clear(ctx);

	color_tex.destruct(ctx);
	hue_tex.destruct(ctx);

	cgv::g2d::ref_msdf_font(ctx, -1);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);
}

bool color_selector::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	if (!show)
		return false;

	if (et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&)e;
		cgv::gui::MouseAction ma = me.get_action();

		if (me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if (ma == cgv::gui::MA_PRESS) {
				ivec2 mpos = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()));

				int hit_index = -1;
				cgv::g2d::rect hit_rect;

				if(layout.color_rect.is_inside(mpos)) {
					hit_index = 0;
					hit_rect = layout.color_rect;
				}

				if(layout.hue_rect.is_inside(mpos)) {
					hit_index = 1;
					hit_rect = layout.hue_rect;
				}

				if(layout.opacity_rect.is_inside(mpos)) {
					hit_index = 2;
					hit_rect = layout.opacity_rect;
				}

				if(hit_index > -1 && hit_index < 4) {
					vec2 local_mpos = static_cast<vec2>(mpos - hit_rect.pos());
					vec2 val = local_mpos / static_cast<vec2>(hit_rect.size());
					if(hit_index > 0)
						val.x() = 0.0f;
					selector_handles[hit_index].val = val;
					selector_handles[hit_index].update_pos();

					update_color();

					if(hit_index == 1)
						update_color_texture();

					post_damage();
				}
			}
		}

		if(selector_handles.handle(e, last_viewport_size, container))
			return true;
	}
	return false;
}

void color_selector::on_set(void* member_ptr) {

	if(member_ptr == &rgb_color) {
		set_rgb_color(rgb_color);
	}

	if(member_ptr == &rgba_color) {
		set_rgba_color(rgba_color);
	}

	if(member_ptr == &layout.size) {
		set_overlay_size(ivec2(layout.size));
	}

	update_member(member_ptr);
	post_damage();
}

bool color_selector::init(cgv::render::context& ctx) {
	
	bool success = canvas_overlay::init(ctx);

	cgv::g2d::msdf_font& font = cgv::g2d::ref_msdf_font(ctx, 1);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);

	if(success)
		init_styles(ctx);
	
	init_textures(ctx);
	
	if(font.is_initialized()) {
		texts.set_msdf_font(&font);
		texts.set_font_size(14.0f);

		texts.add_text("R: ", ivec2(0), TA_BOTTOM_LEFT);
		texts.add_text("0", ivec2(0), TA_BOTTOM_RIGHT);
		texts.add_text("G: ", ivec2(0), TA_BOTTOM_LEFT);
		texts.add_text("0", ivec2(0), TA_BOTTOM_RIGHT);
		texts.add_text("B: ", ivec2(0), TA_BOTTOM_LEFT);
		texts.add_text("0", ivec2(0), TA_BOTTOM_RIGHT);
		texts.add_text("A:", ivec2(0), TA_BOTTOM_LEFT);
		texts.add_text("0", ivec2(0), TA_BOTTOM_RIGHT);
	}

	// saturation and value handle
	selector_handle sh;
	sh.size = vec2(16.0f);
	selector_handles.add(sh);

	// hue handle
	sh.is_rectangular = true;
	sh.size = vec2(20.0f, 10.0f);
	sh.position_is_center = false;
	sh.constraint_reference = cgv::g2d::draggable::CR_MIN_POINT;
	selector_handles.add(sh);

	// opacity handle
	selector_handles.add(sh);

	// set constraints
	selector_handles[0].set_constraint(&layout.color_rect);
	selector_handles[1].set_constraint(&layout.hue_constraint);
	selector_handles[2].set_constraint(&layout.opacity_constraint);

	if(has_opacity)
		set_rgba_color(rgba(0.0f, 0.0f, 0.0f, 1.0f));
	else
		set_rgb_color(rgb(0.0f));

	return success;
}

void color_selector::init_frame(cgv::render::context& ctx) {

	if(ensure_layout(ctx)) {
		ivec2 container_size = get_overlay_size();
		update_layout(container_size);

		for(size_t i = 0; i < 3; ++i)
			selector_handles[i].update_pos();
	
		int w = layout.opacity_rect.w();
		int h = layout.opacity_rect.h();
		opacity_bg_style.texcoord_scaling = vec2(1.0f, static_cast<float>(h) / static_cast<float>(w));

		ivec2 text_position = ivec2(layout.preview_rect.b().x() + 10, layout.preview_rect.y() + 5);
		for(size_t i = 0; i < texts.size(); ++i) {
			texts.set_position(i, text_position);
			text_position.x() += i & 1 ? 15 : 40;
		}
	}

	if(ensure_theme())
		init_styles(ctx);
}

void color_selector::draw_content(cgv::render::context& ctx) {
	
	begin_content(ctx);
	enable_blending();

	ivec2 container_size = get_overlay_size();

	// draw container
	auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");
	container_style.apply(ctx, rect_prog);
	content_canvas.draw_shape(ctx, ivec2(0), container_size);

	// draw inner border
	border_style.apply(ctx, rect_prog);

	auto& ti = cgv::gui::theme_info::instance();
	rgba border_color = rgba(ti.border(), 1.0f);
	rgba background_color = rgba(ti.background(), 1.0f);
	content_canvas.draw_shape(ctx, layout.border_rect, border_color);
	content_canvas.draw_shape(ctx, layout.preview_rect, rgb_color);

	cgv::g2d::rect text_bg = layout.preview_rect;
	text_bg.set_w(48);
	int n_labels = has_opacity ? 4 : 3;
	for(size_t i = 0; i < n_labels; ++i) {
		text_bg.set_x(texts.ref_texts()[2*i].position.x() - 4);
		content_canvas.draw_shape(ctx, text_bg, background_color);
	}
	
	color_texture_style.apply(ctx, rect_prog);
	color_tex.enable(ctx, 0);
	content_canvas.draw_shape(ctx, layout.color_rect);
	color_tex.disable(ctx);

	hue_texture_style.apply(ctx, rect_prog);
	hue_tex.enable(ctx, 0);
	content_canvas.draw_shape(ctx, layout.hue_rect);
	hue_tex.disable(ctx);

	if(has_opacity) {
		auto& grid_prog = content_canvas.enable_shader(ctx, "grid");
		opacity_bg_style.apply(ctx, grid_prog);
		content_canvas.draw_shape(ctx, layout.opacity_rect);
	}

	glEnable(GL_SCISSOR_TEST);
	glScissor(layout.color_rect.x(), layout.color_rect.y(), layout.color_rect.w(), layout.color_rect.h());

	auto& sh = selector_handles;
	auto& circle_prog = content_canvas.enable_shader(ctx, "circle");
	color_handle_style.apply(ctx, circle_prog);
	glScissor(layout.color_rect.x(), layout.color_rect.y(), layout.color_rect.w(), layout.color_rect.h());
	content_canvas.draw_shape(ctx, sh[0].pos + 0.5f, sh[0].size);

	rect_prog = content_canvas.enable_shader(ctx, "rectangle");
	hue_handle_style.apply(ctx, rect_prog);
	glScissor(layout.hue_rect.x(), layout.hue_rect.y(), layout.hue_rect.w(), layout.hue_rect.h());
	content_canvas.draw_shape(ctx, sh[1].pos, sh[1].size);
	
	if(has_opacity) {
		glScissor(layout.opacity_rect.x(), layout.opacity_rect.y(), layout.opacity_rect.w(), layout.opacity_rect.h());
		
		const auto& r = layout.opacity_rect;
		content_canvas.enable_shader(ctx, "rectangle");
		opacity_color_style.fill_color = rgba(rgb_color, 1.0f);
		opacity_color_style.feather_width = r.h();
		opacity_color_style.apply(ctx, rect_prog);
		content_canvas.draw_shape(ctx, ivec2(r.x(), r.y1() - 1), ivec2(r.w(), 1));

		hue_handle_style.apply(ctx, rect_prog);
		content_canvas.draw_shape(ctx, sh[2].pos, sh[2].size);
	}

	content_canvas.disable_current_shader(ctx);

	
	glDisable(GL_SCISSOR_TEST);

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, content_canvas, texts, text_style, 0, 2*n_labels);

	disable_blending();
	end_content(ctx);
}

void color_selector::create_gui() {

	create_overlay_gui();

	if(begin_tree_node("Settings", layout, false)) {
		align("\a");
		add_member_control(this, "Size", layout.size, "value_slider", "min=120;max=1000;step=1;ticks=true");
		align("\b");
		end_tree_node(layout);
	}

	if(has_opacity)
		add_member_control(this, "Color", rgba_color);
	else
		add_member_control(this, "Color", rgb_color);
}

bool color_selector::was_updated() {
	bool temp = has_updated;
	has_updated = false;
	return temp;
}

void color_selector::set_rgb_color(rgb color) {
	set_color(rgba(color, 1.0f), false);
}

void color_selector::set_rgba_color(rgba color) {
	set_color(color, true);
}

void color_selector::update_layout(const ivec2& parent_size) {

	auto& l = layout;
	const int slider_width = 20;

	l.border_rect.set_pos(ivec2(l.padding));
	l.border_rect.set_size(ivec2(parent_size - 2 * l.padding));
	l.border_rect.a() += ivec2(0, 23);

	cgv::g2d::rect content_rect = l.border_rect;
	content_rect.translate(1, 1);
	content_rect.resize(-2, -2);

	int mult = has_opacity ? 2 : 1;
	
	l.hue_rect.set_pos(content_rect.x1() - mult* slider_width - (mult-1), content_rect.y());
	l.hue_rect.set_size(slider_width, content_rect.h());

	if(has_opacity) {
		l.opacity_rect = l.hue_rect;
		l.opacity_rect.translate(slider_width + 1, 0);
	}

	l.color_rect = content_rect;
	l.color_rect.resize(-21 * mult, 0);

	l.preview_rect.set_pos(ivec2(l.padding));
	l.preview_rect.set_size(20, 20);


	l.hue_constraint = l.hue_rect;
	l.hue_constraint.translate(0, -5);
	l.hue_constraint.set_w(0);

	l.opacity_constraint = l.opacity_rect;
	l.opacity_constraint.translate(0, -5);
	l.opacity_constraint.set_w(0);

}

void color_selector::init_styles(context& ctx) {
	// get theme colors
	auto& ti = cgv::gui::theme_info::instance();
	rgba background_color = rgba(ti.background(), 1.0f);
	rgba group_color = rgba(ti.group(), 1.0f);
	rgba border_color = rgba(ti.border(), 1.0f);

	// configure style for the container rectangle
	container_style.fill_color = group_color;
	container_style.border_color = background_color;
	container_style.border_width = 3.0f;
	container_style.feather_width = 0.0f;
	
	// configure style for the border rectangles
	border_style = container_style;
	border_style.border_width = 0.0f;
	border_style.use_fill_color = false;

	opacity_bg_style.feather_width = 0.0f;
	opacity_bg_style.fill_color = rgba(rgb(0.75f), 1.0f);
	opacity_bg_style.border_color = rgba(rgb(0.9f), 1.0f);
	opacity_bg_style.pattern = cgv::g2d::grid2d_style::GP_CHECKER;
	opacity_bg_style.scale = 0.5f;
	
	opacity_color_style.use_blending = true;
	opacity_color_style.feather_origin = 1.0f;
	
	// configure style for the color and hue texture rectangles
	color_texture_style = border_style;
	color_texture_style.use_texture = true;
	
	hue_texture_style = color_texture_style;
	color_texture_style.texcoord_scaling = vec2(0.5f);
	color_texture_style.texcoord_offset = vec2(0.25f);

	// configure style for color handle
	color_handle_style.use_blending = true;
	color_handle_style.use_fill_color = true;
	color_handle_style.position_is_center = true;
	color_handle_style.border_color = rgba(rgb(1.0f), 0.75f);
	color_handle_style.border_width = 1.0f;
	color_handle_style.fill_color = rgba(rgb(0.0f), 1.0f);
	color_handle_style.ring_width = 2.5f;

	hue_handle_style = color_handle_style;
	hue_handle_style.border_color = rgba(rgb(1.0f), 0.6f);
	hue_handle_style.position_is_center = false;
	
	// configure style for hue handle
	cgv::g2d::shape2d_style hue_handle_style;
	hue_handle_style.use_blending = true;
	hue_handle_style.use_fill_color = false;
	hue_handle_style.position_is_center = true;
	hue_handle_style.border_color = rgba(ti.border(), 1.0f);
	hue_handle_style.border_width = 1.5f;

	// configure text style
	float label_border_alpha = 0.0f;
	float border_width = 0.25f;
	
	text_style.fill_color = rgba(ti.text(), 1.0f);
	text_style.border_color = rgba(ti.text(), label_border_alpha);
	text_style.border_width = border_width;
	text_style.feather_origin = 0.5f;
	text_style.use_blending = true;
}

void color_selector::init_textures(context& ctx) {

	std::vector<uint8_t> data(3*4, 0u);

	data[6] = 255u;
	data[7] = 255u;
	data[8] = 255u;
	data[9] = 255u;
	
	color_tex.destruct(ctx);
	cgv::data::data_view color_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_UINT8, cgv::data::CF_RGB), data.data());
	color_tex = texture("uint8[R,G,B]", TF_LINEAR, TF_LINEAR);
	color_tex.create(ctx, color_dv, 0);

	std::vector<uint8_t> hue_data(2*3*256);

	for(size_t i = 0; i < 256; ++i) {
		float hue = static_cast<float>(i) / 255.0f;
		rgb color = cgv::media::color<float, cgv::media::HLS>(hue, 0.5f, 1.0f);

		uint8_t col8[3];
		col8[0] = static_cast<uint8_t>(round(color.R() * 255.0f));
		col8[1] = static_cast<uint8_t>(round(color.G() * 255.0f));
		col8[2] = static_cast<uint8_t>(round(color.B() * 255.0f));

		hue_data[6 * i + 0] = col8[0];
		hue_data[6 * i + 1] = col8[1];
		hue_data[6 * i + 2] = col8[2];
		hue_data[6 * i + 3] = col8[0];
		hue_data[6 * i + 4] = col8[1];
		hue_data[6 * i + 5] = col8[2];
	}

	hue_tex.destruct(ctx);
	cgv::data::data_view hue_dv = cgv::data::data_view(new cgv::data::data_format(2, 256, TI_UINT8, cgv::data::CF_RGB), hue_data.data());
	hue_tex = texture("uint8[R,G,B]", TF_LINEAR, TF_LINEAR);
	hue_tex.create(ctx, hue_dv, 0);
}

void color_selector::update_color_texture() {

	if(!color_tex.is_created())
		return;
	
	std::vector<uint8_t> data(3 * 4, 0u);
	data[6] = 255u;
	data[7] = 255u;
	data[8] = 255u;
	
	const auto& hp = selector_handles[1];
	rgb color = cgv::media::color<float, cgv::media::HLS>(hp.val.y(), 0.5f, 1.0f);

	data[9]  = static_cast<uint8_t>(round(color.R() * 255.0f));
	data[10] = static_cast<uint8_t>(round(color.G() * 255.0f));
	data[11] = static_cast<uint8_t>(round(color.B() * 255.0f));

	cgv::data::data_view color_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_UINT8, cgv::data::CF_RGB), data.data());

	if(auto* ctx_ptr = get_context())
		color_tex.replace(*ctx_ptr, 0, 0, color_dv);
}

void color_selector::update_color() {

	const auto& cp = selector_handles[0];
	const auto& hp = selector_handles[1];
	const auto& op = selector_handles[2];
	rgb_color = cgv::media::color<float, cgv::media::HLS>(hp.val.y(), 0.5f, 1.0f);

	float s = cp.val.x(); // saturation
	float v = cp.val.y(); // value

	rgb_color = v * rgb_color;
	rgb_color = (1.0f - s)*rgb(v) + s * rgb_color;

	rgba_color = rgba(rgb_color, op.val.y());

	update_texts();

	update_member(&rgb_color);
	has_updated = true;
}

void color_selector::update_texts() {

	ivec4 components;
	components[0] = static_cast<int>(round(rgba_color.R() * 255.0f));
	components[1] = static_cast<int>(round(rgba_color.G() * 255.0f));
	components[2] = static_cast<int>(round(rgba_color.B() * 255.0f));
	components[3] = static_cast<int>(round(rgba_color.alpha() * 255.0f));

	components = cgv::math::clamp(components, 0, 255);

	texts.set_text(1, std::to_string(components[0]));
	texts.set_text(3, std::to_string(components[1]));
	texts.set_text(5, std::to_string(components[2]));
	texts.set_text(7, std::to_string(components[3]));
}

void color_selector::handle_selector_drag() {

	auto* p = selector_handles.get_dragged();
	p->update_val();

	update_color();
	if(p == &selector_handles[1])
		update_color_texture();

	post_damage();
}

void color_selector::set_color(rgba color, bool opacity) {

	this->rgb_color.R() = color.R();
	this->rgb_color.G() = color.G();
	this->rgb_color.B() = color.B();
	this->rgba_color = color;

	float h = color.H();
	float v = v = color.S() * std::min(color.L(), 1.0f - color.L()) + color.L();
	float s = v ? 2.0f - 2.0f * color.L() / v : 0.0f;

	selector_handles[0].val = vec2(s, v);
	selector_handles[1].val.y() = h;
	selector_handles[2].val.y() = color.alpha();

	selector_handles[0].update_pos();
	selector_handles[1].update_pos();
	selector_handles[2].update_pos();

	has_opacity = opacity;

	update_color_texture();
	update_texts();

	post_recreate_layout();
	post_recreate_gui();
	post_damage();
}

}
}
