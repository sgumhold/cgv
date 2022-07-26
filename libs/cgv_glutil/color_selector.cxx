#include "color_selector.h"

#include <cgv/defines/quote.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
//#include <cgv/math/ftransform.h>
//#include <cgv/utils/advanced_scan.h>
//#include <cgv/utils/file.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
namespace glutil {

color_selector::color_selector() {

	set_name("Color Selector");

	layout.padding = 13; // 10px plus 3px border
	//layout.total_height = supports_opacity ? 200 : 60;

	set_overlay_alignment(AO_END, AO_END);
	set_overlay_stretch(SO_NONE);
	set_overlay_margin(ivec2(-3));
	set_overlay_size(ivec2(300, 280));
	
	fbc.add_attachment("color", "uint8[R,G,B,A]");
	fbc.set_size(get_overlay_size());

	content_canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);
	content_canvas.register_shader("circle", canvas::shaders_2d::circle);
	
	overlay_canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);

	color_points.set_constraint(layout.color_rect);
	color_points.set_drag_callback(std::bind(&color_selector::handle_color_point_drag, this));

	hue_points.set_constraint(layout.hue_rect);
	hue_points.set_drag_callback(std::bind(&color_selector::handle_hue_point_drag, this));

	//color_handle_renderer = generic_renderer(canvas::shaders_2d::arrow);
	//opacity_handle_renderer = generic_renderer(canvas::shaders_2d::rectangle);
	//line_renderer = generic_renderer(canvas::shaders_2d::line);
	//polygon_renderer = generic_renderer(canvas::shaders_2d::polygon);
}

void color_selector::clear(cgv::render::context& ctx) {

	content_canvas.destruct(ctx);
	overlay_canvas.destruct(ctx);
	fbc.clear(ctx);

	//color_handle_renderer.destruct(ctx);
	//opacity_handle_renderer.destruct(ctx);
	//line_renderer.destruct(ctx);
	//polygon_renderer.destruct(ctx);
}

bool color_selector::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	if (!show)
		return false;

	if (et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&)e;

		/*if (ke.get_action() == cgv::gui::KA_PRESS) {
			switch (ke.get_key()) {
			case cgv::gui::KEY_Left_Ctrl:
				show_cursor = true;
				cursor_drawtext = "+";
				post_redraw();
				break;
			case cgv::gui::KEY_Left_Alt:
				show_cursor = true;
				cursor_drawtext = "-";
				post_redraw();
				break;
			}
		}
		else if (ke.get_action() == cgv::gui::KA_RELEASE) {
			switch (ke.get_key()) {
			case cgv::gui::KEY_Left_Ctrl:
				show_cursor = false;
				post_redraw();
				break;
			case cgv::gui::KEY_Left_Alt:
				show_cursor = false;
				post_redraw();
				break;
			}
		}*/
	}
	else if (et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&)e;
		cgv::gui::MouseAction ma = me.get_action();

		/*switch (ma) {
		case cgv::gui::MA_ENTER:
			mouse_is_on_overlay = true;
			return true;
		case cgv::gui::MA_LEAVE:
			mouse_is_on_overlay = false;
			post_redraw();
			return true;
		case cgv::gui::MA_MOVE:
		case cgv::gui::MA_DRAG:
			if(get_context())
				cursor_pos = ivec2(me.get_x(), get_context()->get_height() - 1 - me.get_y());
			if (show_cursor)
				post_redraw();
			break;
		}

		if (me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if (ma == cgv::gui::MA_PRESS && modifiers > 0) {
				ivec2 mpos = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()));

				switch (modifiers) {
				case cgv::gui::EM_CTRL:
					if (!get_hit_point(mpos))
						add_point(mpos);
					break;
				case cgv::gui::EM_ALT:
				{
					draggable* hit_point = get_hit_point(mpos);
					if (hit_point)
						remove_point(hit_point);
				}
				break;
				}
			}
		}*/

		if(color_points.handle(e, last_viewport_size, container))
			return true;
		if(hue_points.handle(e, last_viewport_size, container))
			return true;
	}
	return false;
}

void color_selector::on_set(void* member_ptr) {

	//if(member_ptr == &layout.total_height) {
	//	ivec2 size = get_overlay_size();
	//	size.y() = layout.total_height;
	//	set_overlay_size(size);
	//}

	has_damage = true;
	update_member(member_ptr);
	post_redraw();
}

bool color_selector::init(cgv::render::context& ctx) {
	
	bool success = true;

	success &= fbc.ensure(ctx);
	success &= content_canvas.init(ctx);
	success &= overlay_canvas.init(ctx);
	//success &= color_handle_renderer.init(ctx);
	//success &= opacity_handle_renderer.init(ctx);
	//success &= line_renderer.init(ctx);
	//success &= polygon_renderer.init(ctx);

	if(success)
		init_styles(ctx);
	
	init_texture(ctx);
	//update_color_map(false);

	/*rgb a(0.75f);
	rgb b(0.9f);
	std::vector<rgb> bg_data = { a, b, b, a };
	
	color_tex.destruct(ctx);
	cgv::data::data_view bg_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_FLT32, cgv::data::CF_RGB), bg_data.data());
	color_tex = texture("flt32[R,G,B]", TF_NEAREST, TF_NEAREST, TW_REPEAT, TW_REPEAT);
	success &= color_tex.create(ctx, bg_dv, 0);*/





	color_point p;
	p.pos = layout.color_rect.pos();
	p.update_val(layout);
	//p.col = cmc.cm->interpolate_color(p.val);
	color_points.add(p);






	return success;
}

void color_selector::init_frame(cgv::render::context& ctx) {

	if(ensure_overlay_layout(ctx) || update_layout) {
		update_layout = false;
		ivec2 container_size = get_overlay_size();
		layout.update(container_size);

		fbc.set_size(container_size);
		fbc.ensure(ctx);

		content_canvas.set_resolution(ctx, container_size);
		overlay_canvas.set_resolution(ctx, get_viewport_size());

		//update_point_positions();
		//sort_points();
		//update_geometry();
		color_points.set_constraint(layout.color_rect);
		hue_points.set_constraint(layout.hue_rect);

		has_damage = true;
	}

	// TODO: move functionality of testing for theme changes to overlay? (or use observer pattern for theme info)
	auto& ti = cgv::gui::theme_info::instance();
	int theme_idx = ti.get_theme_idx();
	if(last_theme_idx != theme_idx) {
		last_theme_idx = theme_idx;
		init_styles(ctx);
		handle_color = rgba(ti.text(), 1.0f);
		highlight_color = rgba(ti.highlight(), 1.0f);
		highlight_color_hex = ti.highlight_hex();
		//update_geometry();
	}
}

void color_selector::draw(cgv::render::context& ctx) {

	if(!show)
		return;

	glDisable(GL_DEPTH_TEST);

	if(has_damage)
		draw_content(ctx);
	
	// draw frame buffer texture to screen
	auto& overlay_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	fbc.enable_attachment(ctx, "color", 0);
	overlay_canvas.draw_shape(ctx, get_overlay_position(), get_overlay_size());
	fbc.disable_attachment(ctx, "color");
	overlay_canvas.disable_current_shader(ctx);

	glEnable(GL_DEPTH_TEST);
}

void color_selector::draw_content(cgv::render::context& ctx) {
	
	fbc.enable(ctx);

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ivec2 container_size = get_overlay_size();

	// draw container
	auto& rect_prog = content_canvas.enable_shader(ctx, "rectangle");
	container_style.apply(ctx, rect_prog);
	content_canvas.draw_shape(ctx, ivec2(0), container_size);

	// draw inner border
	border_style.apply(ctx, rect_prog);
	content_canvas.draw_shape(ctx, ivec2(layout.padding), container_size - 2 * layout.padding);

	texture_style.apply(ctx, rect_prog);
	color_tex.enable(ctx, 0);
	content_canvas.draw_shape(ctx, layout.color_rect.pos(), layout.color_rect.size());
	color_tex.disable(ctx);

	/*if(cmc.cm) {
		// draw color scale texture
		color_map_style.apply(ctx, rect_prog);
		preview_tex.enable(ctx, 0);
		content_canvas.draw_shape(ctx, layout.color_editor_rect.pos(), layout.color_editor_rect.size());
		preview_tex.disable(ctx);
		content_canvas.disable_current_shader(ctx);

		if(supports_opacity) {
			// draw opacity editor checkerboard background
			auto& bg_prog = canvas.enable_shader(ctx, "background");
			bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
			bg_tex.enable(ctx, 0);
			canvas.draw_shape(ctx, layout.opacity_editor_rect.pos(), layout.opacity_editor_rect.size());
			bg_tex.disable(ctx);
			canvas.disable_current_shader(ctx);

			// draw histogram texture
			/*if(show_histogram && tfc.hist_tex.is_created()) {
				hist_style.fill_color = histogram_color;
				hist_style.border_color = histogram_border_color;
				hist_style.border_width = float(histogram_border_width);

				auto& hist_prog = canvas.enable_shader(ctx, "histogram");
				hist_prog.set_uniform(ctx, "max_value", tfc.hist_max);
				hist_prog.set_uniform(ctx, "nearest_linear_mix", histogram_smoothing);
				hist_style.apply(ctx, hist_prog);

				tfc.hist_tex.enable(ctx, 1);
				canvas.draw_shape(ctx, layout.editor_rect.pos(), layout.editor_rect.size());
				tfc.hist_tex.disable(ctx);
				canvas.disable_current_shader(ctx);
			}*

			preview_tex.enable(ctx, 0);
			// draw transfer function area polygon
			auto& poly_prog = polygon_renderer.ref_prog();
			poly_prog.enable(ctx);
			canvas.set_view(ctx, poly_prog);
			poly_prog.disable(ctx);
			polygon_renderer.render(ctx, PT_TRIANGLE_STRIP, cmc.triangles);

			// draw transfer function lines
			auto& line_prog = line_renderer.ref_prog();
			line_prog.enable(ctx);
			canvas.set_view(ctx, line_prog);
			line_prog.disable(ctx);
			line_renderer.render(ctx, PT_LINE_STRIP, cmc.lines);
			preview_tex.disable(ctx);

			// draw separator line
			rect_prog = canvas.enable_shader(ctx, "rectangle");
			border_style.apply(ctx, rect_prog);
			canvas.draw_shape(ctx,
				ivec2(layout.color_editor_rect.pos().x(), layout.color_editor_rect.box.get_max_pnt().y()),
				ivec2(container_size.x() - 2 * layout.padding, 1)
			);
			canvas.disable_current_shader(ctx);
		}

		// draw control points
		// color handles
		auto& color_handle_prog = color_handle_renderer.ref_prog();
		color_handle_prog.enable(ctx);
		canvas.set_view(ctx, color_handle_prog);
		color_handle_prog.disable(ctx);
		color_handle_renderer.render(ctx, PT_LINES, cmc.color_handles);

		if(supports_opacity) {
			// opacity handles
			auto& opacity_handle_prog = opacity_handle_renderer.ref_prog();
			opacity_handle_prog.enable(ctx);
			canvas.set_view(ctx, opacity_handle_prog);
			// size is constant for all points
			opacity_handle_prog.set_attribute(ctx, "size", vec2(2.0f*6.0f));
			opacity_handle_prog.disable(ctx);
			opacity_handle_renderer.render(ctx, PT_POINTS, cmc.opacity_handles);
		}
	} else {
		canvas.disable_current_shader(ctx);
	}*/

	glDisable(GL_BLEND);

	fbc.disable(ctx);

	has_damage = false;
}

void color_selector::create_gui() {

	create_overlay_gui();

	if(begin_tree_node("Settings", layout, false)) {
		align("\a");
		//std::string height_options = "min=";
		//height_options += supports_opacity ? "80" : "40";
		//height_options += ";max=500;step=10;ticks=true";
		//add_member_control(this, "Height", layout.total_height, "value_slider", height_options);
		//add_member_control(this, "Resolution", resolution, "dropdown", "enums='2=2,4=4,8=8,16=16,32=32,64=64,128=128,256=256,512=512,1024=1024,2048=2048'");
		//add_member_control(this, "Opacity Scale Exponent", opacity_scale_exponent, "value_slider", "min=1.0;max=5.0;step=0.001;ticks=true");
		align("\b");
		end_tree_node(layout);
	}

	if(begin_tree_node("Color Points", color_points, true)) {
		align("\a");
		auto& points = color_points;
		for(unsigned i = 0; i < points.size(); ++i)
			add_member_control(this, "#" + std::to_string(i), points[i].col, "", &points[i] == color_points.get_selected() ? "label_color=" + highlight_color_hex : "");
		align("\b");
		end_tree_node(color_points);
	}

	if(begin_tree_node("Opacity Points", hue_points, true)) {
		align("\a");
		auto& points = hue_points;
		for(unsigned i = 0; i < points.size(); ++i)
			add_member_control(this, "#" + std::to_string(i), points[i].val[1], "", &points[i] == hue_points.get_selected() ? "label_color=" + highlight_color_hex : "");
		align("\b");
		end_tree_node(hue_points);
	}
}

bool color_selector::was_updated() {
	bool temp = has_updated;
	has_updated = false;
	return temp;
}

void color_selector::set_color(rgb color) {
	
	this->color = color;

	//update_point_positions();
	//update_color_map(false);

	update_member(&color);
	//post_recreate_gui();
}

void color_selector::init_styles(context& ctx) {
	// get theme colors
	auto& ti = cgv::gui::theme_info::instance();
	rgba background_color = rgba(ti.background(), 1.0f);
	rgba group_color = rgba(ti.group(), 1.0f);
	rgba border_color = rgba(ti.border(), 1.0f);

	// configure style for the container rectangle
	container_style.apply_gamma = false;
	container_style.fill_color = group_color;
	container_style.border_color = background_color;
	container_style.border_width = 3.0f;
	container_style.feather_width = 0.0f;
	
	// configure style for the border rectangles
	border_style = container_style;
	border_style.fill_color = border_color;
	border_style.border_width = 0.0f;
	
	// configure style for the color scale rectangle
	texture_style = border_style;
	texture_style.texcoord_scaling = vec2(0.5f);
	texture_style.texcoord_offset = vec2(0.25f);
	texture_style.use_texture = true;

	// configure style for background
	//bg_style.use_texture = true;
	//bg_style.apply_gamma = false;
	//bg_style.feather_width = 0.0f;

	//auto& bg_prog = canvas.enable_shader(ctx, "background");
	//bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
	//bg_style.apply(ctx, bg_prog);
	//canvas.disable_current_shader(ctx);

	// configure style for color handle
	cgv::glutil::arrow2d_style color_handle_style;
	color_handle_style.use_blending = true;
	color_handle_style.apply_gamma = false;
	color_handle_style.use_fill_color = false;
	color_handle_style.position_is_center = true;
	color_handle_style.border_color = rgba(ti.border(), 1.0f);
	color_handle_style.border_width = 1.5f;
	color_handle_style.border_radius = 2.0f;
	color_handle_style.stem_width = 12.0f;
	color_handle_style.head_width = 12.0f;

	//auto& color_handle_prog = color_handle_renderer.ref_prog();
	//color_handle_prog.enable(ctx);
	//color_handle_style.apply(ctx, color_handle_prog);
	//color_handle_prog.disable(ctx);

	// configure style for hue handle
	cgv::glutil::shape2d_style hue_handle_style;
	hue_handle_style.use_blending = true;
	hue_handle_style.apply_gamma = false;
	hue_handle_style.use_fill_color = false;
	hue_handle_style.position_is_center = true;
	hue_handle_style.border_color = rgba(ti.border(), 1.0f);
	hue_handle_style.border_width = 1.5f;

	//auto& opacity_handle_prog = opacity_handle_renderer.ref_prog();
	//opacity_handle_prog.enable(ctx);
	//opacity_handle_style.apply(ctx, opacity_handle_prog);
	//opacity_handle_prog.disable(ctx);

	// configure style for final blending of overlay into main frame buffer
	cgv::glutil::shape2d_style overlay_style;
	overlay_style.fill_color = rgba(1.0f);
	overlay_style.use_texture = true;
	overlay_style.use_blending = false;
	overlay_style.feather_width = 0.0f;

	auto& overlay_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	overlay_style.apply(ctx, overlay_prog);
	overlay_canvas.disable_current_shader(ctx);
}

void color_selector::init_texture(context& ctx) {

	std::vector<uint8_t> data(3*4);

	data[0] = 0u;
	data[1] = 0u;
	data[2] = 0u;

	data[3] = 0u;
	data[4] = 0u;
	data[5] = 0u;

	data[6] = 255u;
	data[7] = 255u;
	data[8] = 255u;

	data[9] = 255u;
	data[10] = 0u;
	data[11] = 0u;

	color_tex.destruct(ctx);
	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_UINT8, cgv::data::CF_RGB), data.data());
	color_tex = texture("uint8[R,G,B,A]", TF_LINEAR, TF_LINEAR);
	color_tex.create(ctx, dv, 0);
}

void color_selector::handle_color_point_drag() {

	color_points.get_dragged()->update_val(layout);
	//update_color_map(true);
	post_redraw();
}

void color_selector::handle_hue_point_drag() {

	hue_points.get_dragged()->update_val(layout);
	//update_color_map(true);
	post_redraw();
}

/*void color_selector::update_point_positions() {

	for(unsigned i = 0; i < cmc.color_points.size(); ++i)
		cmc.color_points[i].update_pos(layout);

	for(unsigned i = 0; i < cmc.opacity_points.size(); ++i)
		cmc.opacity_points[i].update_pos(layout, opacity_scale_exponent);
}*/

/*void color_selector::update_color_map(bool is_data_change) {
	
	context* ctx_ptr = get_context();
	if(!ctx_ptr || !cmc.cm) return;
	context& ctx = *ctx_ptr;

	auto& cm = *cmc.cm;
	auto& color_points = cmc.color_points;
	auto& opacity_points = cmc.opacity_points;
	
	sort_points();

	cm.clear();

	for(unsigned i = 0; i < color_points.size(); ++i) {
		const color_point& p = color_points[i];
		cm.add_color_point(p.val, p.col);
	}

	if(supports_opacity) {
		for(unsigned i = 0; i < opacity_points.size(); ++i) {
			const opacity_point& p = opacity_points[i];
			cm.add_opacity_point(p.val.x(), p.val.y());
		}
	} else {
		// add one fully opaque point that will be removed later on
		cm.add_opacity_point(0.0f, 1.0f);
	}

	size_t size = static_cast<size_t>(resolution);
	std::vector<rgba> cs_data = cm.interpolate(size);

	std::vector<uint8_t> data(4 * 2 * size);
	for(size_t i = 0; i < size; ++i) {
		rgba col = cs_data[i];
		uint8_t r = static_cast<uint8_t>(255.0f * col.R());
		uint8_t g = static_cast<uint8_t>(255.0f * col.G());
		uint8_t b = static_cast<uint8_t>(255.0f * col.B());
		uint8_t a = static_cast<uint8_t>(255.0f * col.alpha());

		unsigned idx = 4 * i;
		data[idx + 0] = r;
		data[idx + 1] = g;
		data[idx + 2] = b;
		data[idx + 3] = a;
		idx += 4*size;
		data[idx + 0] = r;
		data[idx + 1] = g;
		data[idx + 2] = b;
		data[idx + 3] = a;
	}

	preview_tex.destruct(ctx);
	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(size, 2, TI_UINT8, cgv::data::CF_RGBA), data.data());
	preview_tex = texture("uint8[R,G,B,A]", TF_LINEAR, TF_LINEAR, TW_CLAMP_TO_EDGE, TW_CLAMP_TO_EDGE);
	preview_tex.create(ctx, dv, 0);

	if(!supports_opacity)
		cm.clear_opacity_points();

	update_geometry();

	has_updated = true;
	has_damage = true;
}*/

}
}
