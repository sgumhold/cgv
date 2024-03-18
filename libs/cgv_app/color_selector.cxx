#include "color_selector.h"

#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
namespace app {

color_selector::color_selector() {

	set_name("Color Selector");
	block_events = true;

	layout.padding = padding();

	set_size(ivec2(layout.size));
	
	selector_handles.set_drag_callback(std::bind(&color_selector::handle_selector_drag, this));
	selector_handles.set_use_individual_constraints(true);
}

void color_selector::clear(cgv::render::context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);

	canvas_overlay::clear(ctx);

	color_tex.destruct(ctx);
	hue_tex.destruct(ctx);
	texts.destruct(ctx);
}

bool color_selector::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	if (et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&)e;
		cgv::gui::MouseAction ma = me.get_action();

		if (me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if (ma == cgv::gui::MA_PRESS) {
				ivec2 mpos = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()));

				int hit_index = -1;
				cgv::g2d::irect hit_rect;

				if(layout.color_rect.contains(mpos)) {
					hit_index = 0;
					hit_rect = layout.color_rect;
				}

				if(layout.hue_rect.contains(mpos)) {
					hit_index = 1;
					hit_rect = layout.hue_rect;
				}

				if(layout.opacity_rect.contains(mpos)) {
					hit_index = 2;
					hit_rect = layout.opacity_rect;
				}

				if(hit_index > -1 && hit_index < 4) {
					vec2 local_mpos = static_cast<vec2>(mpos - hit_rect.position);
					vec2 val = local_mpos / static_cast<vec2>(hit_rect.size);
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

		if(selector_handles.handle(e, get_viewport_size(), get_rectangle()))
			return true;
	}
	return false;
}

void color_selector::handle_member_change(const cgv::utils::pointer_test& m) {

	if(m.is(rgb_color))
		set_rgb_color(rgb_color);

	if(m.is(rgba_color))
		set_rgba_color(rgba_color);

	if(m.is(layout.size))
		set_size(ivec2(layout.size));
}

bool color_selector::init(cgv::render::context& ctx) {
	
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);
	
	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("circle", cgv::g2d::shaders::circle);
	register_shader("grid", cgv::g2d::shaders::grid);
	
	bool success = canvas_overlay::init(ctx);

	success &= texts.init(ctx);

	if(success) {
		texts.add_text("R: ", ivec2(0), cgv::render::TA_LEFT);
		texts.add_text("0", ivec2(0), cgv::render::TA_RIGHT);
		texts.add_text("G: ", ivec2(0), cgv::render::TA_LEFT);
		texts.add_text("0", ivec2(0), cgv::render::TA_RIGHT);
		texts.add_text("B: ", ivec2(0), cgv::render::TA_LEFT);
		texts.add_text("0", ivec2(0), cgv::render::TA_RIGHT);
		texts.add_text("A:", ivec2(0), cgv::render::TA_LEFT);
		texts.add_text("0", ivec2(0), cgv::render::TA_RIGHT);
	}

	init_textures(ctx);

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
		set_color(rgba(0.0f, 0.0f, 0.0f, 1.0f), true, true);
	else
		set_color(rgb(0.0f), false, true);

	return success;
}

void color_selector::init_frame(cgv::render::context& ctx) {

	if(ensure_layout(ctx)) {
		update_layout(get_rectangle().size);

		for(size_t i = 0; i < 3; ++i)
			selector_handles[i].update_pos();
	
		int w = layout.opacity_rect.w();
		int h = layout.opacity_rect.h();
		opacity_bg_style.texcoord_scaling = vec2(1.0f, static_cast<float>(h) / static_cast<float>(w));

		ivec2 text_position = ivec2(layout.preview_rect.b().x() + 10, layout.preview_rect.center().y());
		for(unsigned i = 0; i < texts.size(); ++i) {
			texts.set_position(i, text_position);
			text_position.x() += i & 1 ? 15 : 40;
		}
	}
}

void color_selector::draw_content(cgv::render::context& ctx) {
	
	begin_content(ctx);
	
	// draw inner border
	content_canvas.enable_shader(ctx, "rectangle");
	content_canvas.set_style(ctx, border_style);

	auto& theme = cgv::gui::theme_info::instance();
	rgba border_color = rgba(theme.border(), 1.0f);
	rgba text_background_color = rgba(theme.text_background(), 1.0f);
	content_canvas.draw_shape(ctx, layout.border_rect, border_color);
	content_canvas.draw_shape(ctx, layout.preview_rect, rgb_color);

	cgv::g2d::irect text_bg = layout.preview_rect;
	text_bg.size.x() = 48;
	int n_labels = has_opacity ? 4 : 3;
	for(size_t i = 0; i < n_labels; ++i) {
		text_bg.position.x() = static_cast<int>(texts.ref_texts()[2 * i].position.x() - 4.0f);
		content_canvas.draw_shape(ctx, text_bg, text_background_color);
	}

	content_canvas.set_style(ctx, color_texture_style);
	color_tex.enable(ctx, 0);
	content_canvas.draw_shape(ctx, layout.color_rect);
	color_tex.disable(ctx);

	content_canvas.set_style(ctx, hue_texture_style);
	hue_tex.enable(ctx, 0);
	content_canvas.draw_shape(ctx, layout.hue_rect);
	hue_tex.disable(ctx);

	if(has_opacity) {
		content_canvas.enable_shader(ctx, "grid");
		content_canvas.set_style(ctx, opacity_bg_style);
		content_canvas.draw_shape(ctx, layout.opacity_rect);
	}

	glEnable(GL_SCISSOR_TEST);
	glScissor(layout.color_rect.x(), layout.color_rect.y(), layout.color_rect.w(), layout.color_rect.h());

	auto& sh = selector_handles;
	content_canvas.enable_shader(ctx, "circle");
	content_canvas.set_style(ctx, color_handle_style);
	glScissor(layout.color_rect.x(), layout.color_rect.y(), layout.color_rect.w(), layout.color_rect.h());
	content_canvas.draw_shape(ctx, sh[0].position + 0.5f, sh[0].size);

	content_canvas.enable_shader(ctx, "rectangle");
	content_canvas.set_style(ctx, hue_handle_style);
	glScissor(layout.hue_rect.x(), layout.hue_rect.y(), layout.hue_rect.w(), layout.hue_rect.h());
	content_canvas.draw_shape(ctx, sh[1]);

	if(has_opacity) {
		glScissor(layout.opacity_rect.x(), layout.opacity_rect.y(), layout.opacity_rect.w(), layout.opacity_rect.h());

		const auto& r = layout.opacity_rect;
		content_canvas.enable_shader(ctx, "rectangle");
		opacity_color_style.fill_color = rgba(rgb_color, 1.0f);
		opacity_color_style.feather_width = static_cast<float>(r.h());
		content_canvas.set_style(ctx, opacity_color_style);
		content_canvas.draw_shape(ctx, ivec2(r.x(), r.y1() - 1), ivec2(r.w(), 1));

		content_canvas.set_style(ctx, hue_handle_style);
		content_canvas.draw_shape(ctx, sh[2]);
	}

	content_canvas.disable_current_shader(ctx);

	glDisable(GL_SCISSOR_TEST);

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, content_canvas, texts, text_style, 0, 2 * n_labels);

	end_content(ctx);
}

void color_selector::create_gui_impl() {

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

void color_selector::set_rgb_color(rgb color) {
	set_color(rgba(color, 1.0f), false);
}

void color_selector::set_rgba_color(rgba color) {
	set_color(color, true);
}

void color_selector::update_layout(const ivec2& parent_size) {

	auto& l = layout;
	const int slider_width = 20;

	l.border_rect.position = ivec2(l.padding);
	l.border_rect.size = ivec2(parent_size - 2 * l.padding);
	l.border_rect.position.y() += 23;
	l.border_rect.size.y() -= 23;

	cgv::g2d::irect content_rect = l.border_rect;
	content_rect.translate(1, 1);
	content_rect.size -= 2;

	int mult = has_opacity ? 2 : 1;
	
	l.hue_rect.position = ivec2(content_rect.x1() - mult* slider_width - (mult-1), content_rect.y());
	l.hue_rect.size = ivec2(slider_width, content_rect.h());

	if(has_opacity) {
		l.opacity_rect = l.hue_rect;
		l.opacity_rect.translate(slider_width + 1, 0);
	}

	l.color_rect = content_rect;
	l.color_rect.w() -= 21 * mult;

	l.preview_rect.position = ivec2(l.padding);
	l.preview_rect.size = ivec2(20, 20);

	l.hue_constraint = l.hue_rect;
	l.hue_constraint.translate(0, -5);
	l.hue_constraint.size.x() = 0;

	l.opacity_constraint = l.opacity_rect;
	l.opacity_constraint.translate(0, -5);
	l.opacity_constraint.size.x() = 0;

}

void color_selector::init_styles() {
	auto& theme = cgv::gui::theme_info::instance();
	
	// configure style for the border rectangles
	border_style.border_width = 0.0f;
	border_style.use_fill_color = false;
	border_style.feather_width = 0.0f;

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
	color_handle_style.ring_width = 4.0f;

	// configure style for hue handle
	hue_handle_style = color_handle_style;
	hue_handle_style.position_is_center = false;
	
	// configure text style
	text_style.fill_color = theme.text();
	text_style.font_size = 14.0f;
}

void color_selector::init_textures(cgv::render::context& ctx) {

	std::vector<uint8_t> data(3*4, 0u);

	data[6] = 255u;
	data[7] = 255u;
	data[8] = 255u;
	data[9] = 255u;
	
	color_tex.destruct(ctx);
	cgv::data::data_view color_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_UINT8, cgv::data::CF_RGB), data.data());
	color_tex = cgv::render::texture("uint8[R,G,B]", cgv::render::TF_LINEAR, cgv::render::TF_LINEAR);
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
	hue_tex = cgv::render::texture("uint8[R,G,B]", cgv::render::TF_LINEAR, cgv::render::TF_LINEAR);
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

	if(has_opacity) {
		if(on_change_rgba_callback)
			on_change_rgba_callback(rgba_color);
	} else {
		if(on_change_rgb_callback)
			on_change_rgb_callback(rgb_color);
	}

	update_member(&rgb_color);
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

void color_selector::set_color(rgba color, bool opacity, bool init) {

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

	if(has_opacity != opacity) {
		// need to recreate the layout and gui in case the opacity usage changed
		post_recreate_layout();
		post_recreate_gui();
		has_opacity = opacity;
	}
	
	update_color_texture();
	update_texts();

	if(!init && auto_show)
		set_visibility(true);

	post_damage();
}

}
}
