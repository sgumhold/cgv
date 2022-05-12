#include "color_map_editor.h"

#include <cgv/defines/quote.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
namespace glutil {

color_map_editor::color_map_editor() {

	set_name("Color Scale Editor");

	resolution = (cgv::type::DummyEnum)256;
	opacity_scale_exponent = 1.0f;
	supports_opacity = false;

	layout.padding = 13; // 10px plus 3px border
	layout.total_height = supports_opacity ? 200 : 60;

	set_overlay_alignment(AO_START, AO_START);
	set_overlay_stretch(SO_HORIZONTAL);
	set_overlay_margin(ivec2(-3));
	set_overlay_size(ivec2(600u, layout.total_height));
	
	fbc.add_attachment("color", "flt32[R,G,B,A]");
	fbc.set_size(get_overlay_size());

	canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);
	canvas.register_shader("circle", canvas::shaders_2d::circle);
	canvas.register_shader("histogram", "hist2d.glpr");
	canvas.register_shader("background", canvas::shaders_2d::background);

	overlay_canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);

	mouse_is_on_overlay = false;
	show_cursor = false;
	cursor_pos = ivec2(-100);
	cursor_drawtext = "";

	cmc.color_points.set_constraint(layout.color_handles_rect);
	cmc.color_points.set_drag_callback(std::bind(&color_map_editor::handle_color_point_drag, this));
	cmc.color_points.set_drag_end_callback(std::bind(&color_map_editor::handle_drag_end, this));

	cmc.opacity_points.set_constraint(layout.opacity_editor_rect);
	cmc.opacity_points.set_drag_callback(std::bind(&color_map_editor::handle_opacity_point_drag, this));
	cmc.opacity_points.set_drag_end_callback(std::bind(&color_map_editor::handle_drag_end, this));

	color_handle_renderer = generic_renderer(canvas::shaders_2d::arrow);
	opacity_handle_renderer = generic_renderer(canvas::shaders_2d::rectangle);
	line_renderer = generic_renderer(canvas::shaders_2d::line);
	polygon_renderer = generic_renderer(canvas::shaders_2d::polygon);
}

void color_map_editor::clear(cgv::render::context& ctx) {

	canvas.destruct(ctx);
	overlay_canvas.destruct(ctx);
	fbc.clear(ctx);
}

bool color_map_editor::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	if (!show)
		return false;

	if (et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&)e;

		if (ke.get_action() == cgv::gui::KA_PRESS) {
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
		}
	}
	else if (et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&)e;
		cgv::gui::MouseAction ma = me.get_action();

		switch (ma) {
		case cgv::gui::MA_ENTER:
			mouse_is_on_overlay = true;
			break;
		case cgv::gui::MA_LEAVE:
			mouse_is_on_overlay = false;
			post_redraw();
			break;
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
		}

		if(cmc.color_points.handle(e, last_viewport_size, container))
			return true;
		if(cmc.opacity_points.handle(e, last_viewport_size, container))
			return true;
	}
	return false;
}

void color_map_editor::on_set(void* member_ptr) {

	if(member_ptr == &layout.total_height) {
		ivec2 size = get_overlay_size();
		size.y() = layout.total_height;
		set_overlay_size(size);
	}

	if(member_ptr == &opacity_scale_exponent) {
		opacity_scale_exponent = cgv::math::clamp(opacity_scale_exponent, 1.0f, 5.0f);

		update_point_positions();
		sort_points();
		update_geometry();
	}

	if(member_ptr == &resolution) {
		context* ctx_ptr = get_context();
		if(ctx_ptr)
			init_texture(*ctx_ptr);
		update_color_map(false);
	}

	for(unsigned i = 0; i < cmc.color_points.size(); ++i) {
		if(member_ptr == &cmc.color_points[i].col) {
			update_color_map(true);
			break;
		}
	}

	for(unsigned i = 0; i < cmc.opacity_points.size(); ++i) {
		if(member_ptr == &cmc.opacity_points[i].val[1]) {
			cmc.opacity_points[i].update_pos(layout, opacity_scale_exponent);
			update_color_map(true);
			break;
		}
	}
	
	if(member_ptr == &supports_opacity) {
		layout.total_height = supports_opacity ? 200 : 60;
		on_set(&layout.total_height);

		if(supports_opacity) {
			layout.total_height = 200;
		} else {
			layout.total_height = 60;
			if(cmc.cm) {
				cmc.cm->clear_opacity_points();
				cmc.opacity_points.clear();

				update_point_positions();
				update_color_map(false);
			}
		}

		update_layout = true;
		post_recreate_gui();
	}

	update_member(member_ptr);
	post_redraw();
}

bool color_map_editor::init(cgv::render::context& ctx) {
	
	// get a bold font face to use for the cursor
	auto font = cgv::media::font::find_font("Arial");
	if(!font.empty()) {
		cursor_font_face = font->get_font_face(cgv::media::font::FFA_BOLD);
	}

	bool success = true;

	success &= fbc.ensure(ctx);
	success &= canvas.init(ctx);
	success &= overlay_canvas.init(ctx);
	success &= color_handle_renderer.init(ctx);
	success &= opacity_handle_renderer.init(ctx);
	success &= line_renderer.init(ctx);
	success &= polygon_renderer.init(ctx);

	if(success)
		init_styles(ctx);
	
	init_texture(ctx);
	update_color_map(false);

	rgb a(0.75f);
	rgb b(0.9f);
	std::vector<rgb> bg_data = { a, b, b, a };
	
	bg_tex.destruct(ctx);
	cgv::data::data_view bg_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_FLT32, cgv::data::CF_RGB), bg_data.data());
	bg_tex = texture("flt32[R,G,B]", TF_NEAREST, TF_NEAREST, TW_REPEAT, TW_REPEAT);
	success &= bg_tex.create(ctx, bg_dv, 0);

	return success;
}

void color_map_editor::init_frame(cgv::render::context& ctx) {

	if(ensure_overlay_layout(ctx) || update_layout) {
		update_layout = false;
		ivec2 container_size = get_overlay_size();
		layout.update(container_size, supports_opacity);

		fbc.set_size(container_size);
		fbc.ensure(ctx);

		canvas.set_resolution(ctx, container_size);
		overlay_canvas.set_resolution(ctx, get_viewport_size());

		auto& bg_prog = canvas.enable_shader(ctx, "background");
		float width_factor = static_cast<float>(layout.opacity_editor_rect.size().x()) / static_cast<float>(layout.opacity_editor_rect.size().y());
		bg_style.texcoord_scaling = vec2(5.0f * width_factor, 5.0f);
		bg_style.apply(ctx, bg_prog);
		canvas.disable_current_shader(ctx);

		update_point_positions();
		sort_points();
		update_geometry();
		cmc.color_points.set_constraint(layout.color_handles_rect);
		cmc.opacity_points.set_constraint(layout.opacity_editor_rect);
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
		update_geometry();
	}
}

void color_map_editor::draw(cgv::render::context& ctx) {

	if(!show)
		return;

	fbc.enable(ctx);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	ivec2 container_size = get_overlay_size();
	
	// draw container
	auto& rect_prog = canvas.enable_shader(ctx, "rectangle");
	container_style.apply(ctx, rect_prog);
	canvas.draw_shape(ctx, ivec2(0), container_size);
	
	// draw inner border
	border_style.apply(ctx, rect_prog);
	canvas.draw_shape(ctx, ivec2(layout.padding - 1) + ivec2(0, 10), container_size - 2*layout.padding + 2 - ivec2(0, 10));
	
	if(cmc.cm) {
		// draw color scale texture
		color_map_style.apply(ctx, rect_prog);
		preview_tex.enable(ctx, 0);
		canvas.draw_shape(ctx, layout.color_editor_rect.pos(), layout.color_editor_rect.size());
		preview_tex.disable(ctx);
		canvas.disable_current_shader(ctx);
		
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
			}*/

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
	}
	
	glDisable(GL_BLEND);

	fbc.disable(ctx);

	// draw frame buffer texture to screen
	auto& final_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	fbc.enable_attachment(ctx, "color", 0);
	overlay_canvas.draw_shape(ctx, get_overlay_position(), container_size);
	fbc.disable_attachment(ctx, "color");

	overlay_canvas.disable_current_shader(ctx);

	// draw cursor decorators to show interaction hints
	if(mouse_is_on_overlay && show_cursor) {
		ivec2 pos = cursor_pos + ivec2(7, 4);

		auto fntf_ptr = ctx.get_current_font_face();
		auto s = ctx.get_current_font_size();

		ctx.enable_font_face(cursor_font_face, s);

		ctx.push_pixel_coords();
		ctx.set_color(rgb(0.0f));
		ctx.set_cursor(vecn(float(pos.x()), float(pos.y())), "", cgv::render::TA_TOP_LEFT);
		ctx.output_stream() << cursor_drawtext;
		ctx.output_stream().flush();
		ctx.pop_pixel_coords();

		ctx.enable_font_face(fntf_ptr, s);
	}
	
	glEnable(GL_DEPTH_TEST);
}

void color_map_editor::create_gui() {

	create_overlay_gui();

	if(begin_tree_node("Settings", layout, false)) {
		align("\a");
		std::string height_options = "min=";
		height_options += supports_opacity ? "80" : "40";
		height_options += ";max=500;step=10;ticks=true";
		add_member_control(this, "Height", layout.total_height, "value_slider", height_options);
		add_member_control(this, "Resolution", resolution, "dropdown", "enums='2=2,4=4,8=8,16=16,32=32,64=64,128=128,256=256,512=512,1024=1024,2048=2048'");
		add_member_control(this, "Opacity Scale Exponent", opacity_scale_exponent, "value_slider", "min=1.0;max=5.0;step=0.001;ticks=true");
		align("\b");
		end_tree_node(layout);
	}

	if(begin_tree_node("Color Points", cmc.color_points, true)) {
		align("\a");
		auto& points = cmc.color_points;
		for(unsigned i = 0; i < points.size(); ++i)
			add_member_control(this, "#" + std::to_string(i), points[i].col, "", &points[i] == cmc.color_points.get_selected() ? "label_color=" + highlight_color_hex : "");
		align("\b");
		end_tree_node(cmc.color_points);
	}
	if(supports_opacity) {
		if(begin_tree_node("Opacity Points", cmc.opacity_points, true)) {
			align("\a");
			auto& points = cmc.opacity_points;
			for(unsigned i = 0; i < points.size(); ++i)
				add_member_control(this, "#" + std::to_string(i), points[i].val[1], "", &points[i] == cmc.opacity_points.get_selected() ? "label_color=" + highlight_color_hex : "");
			align("\b");
			end_tree_node(cmc.color_points);
		}
	}
}

void color_map_editor::create_gui(cgv::gui::provider& p) {

	p.add_member_control(this, "Show", show, "check");
}

void color_map_editor::set_opacity_support(bool flag) {
	
	supports_opacity = flag;
	on_set(&supports_opacity);
}

bool color_map_editor::was_updated() {
	bool temp = has_updated;
	has_updated = false;
	return temp;
}

void color_map_editor::set_color_map(color_map* cm) {
	cmc.reset();
	cmc.cm = cm;

	if(cmc.cm) {
		auto& cm = *cmc.cm;
		auto& cp = cmc.cm->ref_color_points();
		for(size_t i = 0; i < cp.size(); ++i) {
			color_point p;
			p.val = cgv::math::clamp(cp[i].first, 0.0f, 1.0f);
			p.col = cp[i].second;
			cmc.color_points.add(p);
		}
		if(supports_opacity) {
			auto& op = cmc.cm->ref_opacity_points();
			for(size_t i = 0; i < op.size(); ++i) {
				opacity_point p;
				p.val.x() = cgv::math::clamp(op[i].first, 0.0f, 1.0f);
				p.val.y() = cgv::math::clamp(op[i].second, 0.0f, 1.0f);
				cmc.opacity_points.add(p);
			}
		}
		update_point_positions();
		update_color_map(false);

		post_recreate_gui();
	}
}

void color_map_editor::init_styles(context& ctx) {
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
	color_map_style = border_style;
	color_map_style.use_texture = true;

	// configure style for background
	bg_style.use_texture = true;
	bg_style.apply_gamma = false;
	bg_style.feather_width = 0.0f;

	auto& bg_prog = canvas.enable_shader(ctx, "background");
	bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
	bg_style.apply(ctx, bg_prog);
	canvas.disable_current_shader(ctx);

	// configure style for histogram
	hist_style.use_blending = true;
	hist_style.apply_gamma = false;
	hist_style.feather_width = 0.0f;

	auto& hist_prog = canvas.enable_shader(ctx, "histogram");
	hist_style.apply(ctx, hist_prog);
	canvas.disable_current_shader(ctx);

	// configure style for color handles
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

	auto& color_handle_prog = color_handle_renderer.ref_prog();
	color_handle_prog.enable(ctx);
	color_handle_style.apply(ctx, color_handle_prog);
	color_handle_prog.disable(ctx);

	// configure style for opacity handles
	cgv::glutil::shape2d_style opacity_handle_style;
	opacity_handle_style.use_blending = true;
	opacity_handle_style.apply_gamma = false;
	opacity_handle_style.use_fill_color = false;
	opacity_handle_style.position_is_center = true;
	opacity_handle_style.border_color = rgba(ti.border(), 1.0f);
	opacity_handle_style.border_width = 1.5f;

	auto& opacity_handle_prog = opacity_handle_renderer.ref_prog();
	opacity_handle_prog.enable(ctx);
	opacity_handle_style.apply(ctx, opacity_handle_prog);
	opacity_handle_prog.disable(ctx);

	// configure style for the lines and polygon
	cgv::glutil::line2d_style line_style;
	line_style.use_blending = true;
	line_style.use_fill_color = false;
	line_style.use_texture = true;
	line_style.use_texture_alpha = false;
	line_style.apply_gamma = false;
	line_style.width = 3.0f;

	auto& line_prog = line_renderer.ref_prog();
	line_prog.enable(ctx);
	line_style.apply(ctx, line_prog);
	line_prog.disable(ctx);

	line_style.use_texture_alpha = true;

	auto& poly_prog = polygon_renderer.ref_prog();
	poly_prog.enable(ctx);
	cgv::glutil::shape2d_style poly_style = static_cast<cgv::glutil::shape2d_style>(line_style);
	poly_style.apply(ctx, poly_prog);
	poly_prog.disable(ctx);

	// configure style for final blitting of overlay into main frame buffer
	cgv::glutil::shape2d_style final_style;
	final_style.fill_color = rgba(1.0f);
	final_style.use_texture = true;
	final_style.use_blending = false;
	final_style.feather_width = 0.0f;

	auto& final_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	final_style.apply(ctx, final_prog);
	overlay_canvas.disable_current_shader(ctx);
}

void color_map_editor::init_texture(context& ctx) {

	std::vector<uint8_t> data(resolution * 4 * 2, 0u);

	preview_tex.destruct(ctx);
	cgv::data::data_view tf_dv = cgv::data::data_view(new cgv::data::data_format(resolution, 2, TI_UINT8, cgv::data::CF_RGBA), data.data());
	preview_tex = texture("uint8[R,G,B,A]", TF_LINEAR, TF_LINEAR);
	preview_tex.create(ctx, tf_dv, 0);
}

void color_map_editor::add_point(const vec2& pos) {

	if(cmc.cm) {
		ivec2 test_pos = static_cast<ivec2>(pos);

		if(layout.color_editor_rect.is_inside(test_pos)) {
			// color point
			color_point p;
			p.pos = ivec2(pos.x(), layout.color_handles_rect.pos().y());
			p.update_val(layout);
			p.col = cmc.cm->interpolate_color(p.val);
			cmc.color_points.add(p);
		} else if(supports_opacity && layout.opacity_editor_rect.is_inside(test_pos)) {
			// opacity point
			opacity_point p;
			p.pos = pos;
			p.update_val(layout, opacity_scale_exponent);
			cmc.opacity_points.add(p);
		}
		
		update_color_map(true);
	}
}

void color_map_editor::remove_point(const draggable* ptr) {

	int color_point_idx = -1;
	int opacity_point_idx = -1;

	for(unsigned i = 0; i < cmc.color_points.size(); ++i) {
		if(&cmc.color_points[i] == ptr) {
			color_point_idx = i;
			break;
		}
	}

	for(unsigned i = 0; i < cmc.opacity_points.size(); ++i) {
		if(&cmc.opacity_points[i] == ptr) {
			opacity_point_idx = i;
			break;
		}
	}

	bool removed = false;

	if(color_point_idx > -1) {
		if(cmc.color_points.size() > 1) {
			cmc.color_points.ref_draggables().erase(cmc.color_points.ref_draggables().begin() + color_point_idx);
			removed = true;
		}
	}

	if(opacity_point_idx > -1) {
		if(cmc.opacity_points.size() > 1) {
			cmc.opacity_points.ref_draggables().erase(cmc.opacity_points.ref_draggables().begin() + opacity_point_idx);
			removed = true;
		}
	}

	if(removed)
		update_color_map(true);
}

draggable* color_map_editor::get_hit_point(const color_map_editor::vec2& pos) {

	draggable* hit = nullptr;

	for(unsigned i = 0; i < cmc.color_points.size(); ++i) {
		color_point& p = cmc.color_points[i];
		if(p.is_inside(pos))
			hit = &p;
	}

	for(unsigned i = 0; i < cmc.opacity_points.size(); ++i) {
		opacity_point& p = cmc.opacity_points[i];
		if(p.is_inside(pos))
			hit = &p;
	}

	return hit;
}

void color_map_editor::handle_color_point_drag() {

	cmc.color_points.get_dragged()->update_val(layout);
	update_color_map(true);
	post_redraw();
}

void color_map_editor::handle_opacity_point_drag() {

	cmc.opacity_points.get_dragged()->update_val(layout, opacity_scale_exponent);
	update_color_map(true);
	post_redraw();
}

void color_map_editor::handle_drag_end() {

	update_geometry();
	post_recreate_gui();
	post_redraw();
}

void color_map_editor::sort_points() {
	sort_color_points();
	if(supports_opacity)
		sort_opacity_points();
}

void color_map_editor::sort_color_points() {

	auto& points = cmc.color_points;

	if(points.size() > 1) {
		int dragged_point_idx = -1;
		int selected_point_idx = -1;

		const color_point* dragged_point = points.get_dragged();
		const color_point* selected_point = points.get_selected();

		std::vector<std::pair<color_point, int>> sorted(points.size());

		for(unsigned i = 0; i < points.size(); ++i) {
			sorted[i].first = points[i];
			sorted[i].second = i;

			if(dragged_point == &points[i])
				dragged_point_idx = i;
			if(selected_point == &points[i])
				selected_point_idx = i;
		}

		std::sort(sorted.begin(), sorted.end(),
			[](const auto& a, const auto& b) -> bool {
				return a.first.val < b.first.val;
			}
		);

		int new_dragged_point_idx = -1;
		int new_selected_point_idx = -1;

		for(unsigned i = 0; i < sorted.size(); ++i) {
			points[i] = sorted[i].first;
			if(dragged_point_idx == sorted[i].second) {
				new_dragged_point_idx = i;
			}
			if(selected_point_idx == sorted[i].second) {
				new_selected_point_idx = i;
			}
		}

		points.set_dragged(new_dragged_point_idx);
		points.set_selected(new_selected_point_idx);
	}
}

void color_map_editor::sort_opacity_points() {

	auto& points = cmc.opacity_points;

	if(points.size() > 1) {
		int dragged_point_idx = -1;
		int selected_point_idx = -1;

		const opacity_point* dragged_point = points.get_dragged();
		const opacity_point* selected_point = points.get_selected();

		std::vector<std::pair<opacity_point, int>> sorted(points.size());

		for(unsigned i = 0; i < points.size(); ++i) {
			sorted[i].first = points[i];
			sorted[i].second = i;

			if(dragged_point == &points[i])
				dragged_point_idx = i;
			if(selected_point == &points[i])
				selected_point_idx = i;
		}

		std::sort(sorted.begin(), sorted.end(),
			[](const auto& a, const auto& b) -> bool {
				return a.first.val.x() < b.first.val.x();
			}
		);

		int new_dragged_point_idx = -1;
		int new_selected_point_idx = -1;

		for(unsigned i = 0; i < sorted.size(); ++i) {
			points[i] = sorted[i].first;
			if(dragged_point_idx == sorted[i].second) {
				new_dragged_point_idx = i;
			}
			if(selected_point_idx == sorted[i].second) {
				new_selected_point_idx = i;
			}
		}

		points.set_dragged(new_dragged_point_idx);
		points.set_selected(new_selected_point_idx);
	}
}

void color_map_editor::update_point_positions() {

	for(unsigned i = 0; i < cmc.color_points.size(); ++i)
		cmc.color_points[i].update_pos(layout);

	for(unsigned i = 0; i < cmc.opacity_points.size(); ++i)
		cmc.opacity_points[i].update_pos(layout, opacity_scale_exponent);
}

void color_map_editor::update_color_map(bool is_data_change) {
	
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
}

bool color_map_editor::update_geometry() {

	context* ctx_ptr = get_context();
	if(!ctx_ptr || !cmc.cm) return false;
	context& ctx = *ctx_ptr;

	auto& cm = *cmc.cm;
	auto& color_points = cmc.color_points;
	auto& opacity_points = cmc.opacity_points;
	auto& color_handles = cmc.color_handles;
	auto& opacity_handles = cmc.opacity_handles;
	auto& lines = cmc.lines;
	auto& triangles = cmc.triangles;

	color_handles.clear();
	opacity_handles.clear();
	lines.clear();
	triangles.clear();

	bool success = color_points.size() > 0 && opacity_points.size() > 0;

	// create color handles
	for(unsigned i = 0; i < color_points.size(); ++i) {
		const auto& p = color_points[i];
		vec2 pos = p.center();
		rgba col = color_points.get_selected() == &p ? highlight_color : handle_color;
		color_handles.add(pos + vec2(0.0f, 2.0f),  col);
		color_handles.add(pos + vec2(0.0f, 20.0f), col);
	}

	// create opacity handles
	for(unsigned i = 0; i < opacity_points.size(); ++i) {
		const auto& p = opacity_points[i];
		vec2 pos = p.center();
		rgba col = opacity_points.get_selected() == &p ? highlight_color : handle_color;
		opacity_handles.add(pos, col);
	}

	// TODO: handle case with only 1 opacity handle
	if(opacity_points.size() > 0) {
		const auto& pl = opacity_points[0];

		lines.add(vec2(float(layout.opacity_editor_rect.pos().x()), pl.center().y()), vec2(0.0f, 0.5f));

		triangles.add(vec2(float(layout.opacity_editor_rect.pos().x()), pl.center().y()), vec2(0.0f, 0.5f));
		triangles.add(layout.opacity_editor_rect.pos(), vec2(0.0f, 0.5f));

		for(unsigned i = 0; i < opacity_points.size(); ++i) {
			const auto& p = opacity_points[i];
			vec2 pos = p.center();

			lines.add(pos, vec2(p.val.x(), 0.5f));
			triangles.add(pos, vec2(p.val.x(), 0.5f));
			triangles.add(vec2(pos.x(), (float)layout.opacity_editor_rect.pos().y()), vec2(p.val.x(), 0.5f));
		}

		const auto& pr = opacity_points[opacity_points.size() - 1];
		vec2 max_pos = layout.opacity_editor_rect.pos() + vec2(1.0f, 0.0f) * layout.opacity_editor_rect.size();

		lines.add(vec2(max_pos.x(), pr.center().y()), vec2(1.0f, 0.5f));

		triangles.add(vec2(max_pos.x(), pr.center().y()), vec2(1.0f, 0.5f));
		triangles.add(max_pos, vec2(1.0f, 0.5f));
	} else {
		success = false;
	}

	return success;
}

}
}
