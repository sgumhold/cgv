#include "color_map_editor.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
namespace app {

const float color_map_editor::color_point::default_width = 12.0f;
const float color_map_editor::color_point::default_height = 18.0f;

const float color_map_editor::opacity_point::default_size = 12.0f;

color_map_editor::color_map_editor() {

	set_name("Color Scale Editor");
	block_events = true;

	resolution = (cgv::type::DummyEnum)256;
	opacity_scale_exponent = 1.0f;
	supports_opacity = false;
	use_interpolation = true;
	use_linear_filtering = true;
	range = vec2(0.0f, 1.0f);

	layout.padding = padding();
	layout.total_height = supports_opacity ? 200 : 60;

	set_size(ivec2(600u, layout.total_height));
	
	mouse_is_on_overlay = false;
	cursor_pos = ivec2(-100);
	cursor_label_index = -1;
	show_value_label = false;

	cmc.color_points.set_constraint(layout.color_handles_rect);
	cmc.color_points.set_drag_callback(std::bind(&color_map_editor::handle_color_point_drag, this));
	cmc.color_points.set_drag_end_callback(std::bind(&color_map_editor::handle_drag_end, this));
	cmc.color_points.set_selection_change_callback(std::bind(&color_map_editor::handle_drag_end, this));

	cmc.opacity_points.set_constraint(layout.opacity_editor_rect);
	cmc.opacity_points.set_drag_callback(std::bind(&color_map_editor::handle_opacity_point_drag, this));
	cmc.opacity_points.set_drag_end_callback(std::bind(&color_map_editor::handle_drag_end, this));
	cmc.opacity_points.set_selection_change_callback(std::bind(&color_map_editor::handle_drag_end, this));

	color_handle_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::arrow);
	opacity_handle_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::rectangle);
	line_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::line);
	polygon_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::polygon);
}

void color_map_editor::clear(cgv::render::context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);

	canvas_overlay::clear(ctx);

	color_handle_renderer.destruct(ctx);
	opacity_handle_renderer.destruct(ctx);
	line_renderer.destruct(ctx);
	polygon_renderer.destruct(ctx);

	bg_tex.destruct(ctx);
	preview_tex.destruct(ctx);
	hist_tex.destruct(ctx);

	cursor_labels.destruct(ctx);
	value_labels.destruct(ctx);
}

bool color_map_editor::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	if (et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&)e;

		if (ke.get_action() == cgv::gui::KA_PRESS) {
			switch (ke.get_key()) {
			case cgv::gui::KEY_Left_Ctrl:
				cursor_label_index = 1;
				post_damage();
				break;
			case cgv::gui::KEY_Left_Alt:
				cursor_label_index = 0;
				post_damage();
				break;
			}
		}
		else if (ke.get_action() == cgv::gui::KA_RELEASE) {
			switch (ke.get_key()) {
			case cgv::gui::KEY_Left_Ctrl:
			case cgv::gui::KEY_Left_Alt:
				cursor_label_index = -1;
				post_damage();
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
			return true;
		case cgv::gui::MA_LEAVE:
			mouse_is_on_overlay = false;
			post_damage();
			return true;
		case cgv::gui::MA_MOVE:
		case cgv::gui::MA_DRAG:
			if(get_context())
				cursor_pos = ivec2(me.get_x(), get_context()->get_height() - 1 - me.get_y());
			if(cursor_label_index > -1)
				post_damage();
			break;
		}

		bool request_clear_selection = false;

		if (me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if (ma == cgv::gui::MA_PRESS) {
				ivec2 mpos = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()));
				
				request_clear_selection = is_hit(mpos);

				switch (modifiers) {
				case cgv::gui::EM_CTRL:
					if(!get_hit_point(mpos))
						add_point(mpos);
					break;
				case cgv::gui::EM_ALT:
				{
					cgv::g2d::draggable* hit_point = get_hit_point(mpos);
					if(hit_point)
						remove_point(hit_point);
				}
				break;
				default: break;
				}
			}
		}

		if(cmc.color_points.handle(e, get_viewport_size(), get_rectangle()))
			return true;
		if(cmc.opacity_points.handle(e, get_viewport_size(), get_rectangle()))
			return true;

		if(request_clear_selection) {
			cmc.color_points.clear_selected();
			cmc.opacity_points.clear_selected();
			handle_drag_end();
		}
	}
	return false;
}

void color_map_editor::handle_member_change(const cgv::utils::pointer_test& m) {

	if(m.is(layout.total_height)) {
		ivec2 size = get_rectangle().size;
		size.y() = layout.total_height;
		set_size(size);
	}

	if(m.is(opacity_scale_exponent)) {
		opacity_scale_exponent = cgv::math::clamp(opacity_scale_exponent, 1.0f, 5.0f);

		update_point_positions();
		sort_points();
		update_geometry();
	}

	if(m.is(resolution)) {
		cgv::render::context* ctx_ptr = get_context();
		if(cmc.cm)
			cmc.cm->set_resolution(resolution);
		update_color_map(false);
	}

	if(m.is(use_interpolation)) {
		cgv::render::context* ctx_ptr = get_context();
		if(cmc.cm)
			cmc.cm->enable_interpolation(use_interpolation);
		update_color_map(false);
	}

	if(m.is(use_linear_filtering)) {
		cgv::render::context* ctx_ptr = get_context();
		if(cmc.cm) {
			cgv::render::gl_color_map* gl_cm = cmc.get_gl_color_map();
			if(gl_cm)
				gl_cm->enable_linear_filtering(use_linear_filtering);
		}
		update_color_map(false);
	}

	for(unsigned i = 0; i < cmc.color_points.size(); ++i) {
		if(m.is(cmc.color_points[i].col)) {
			update_color_map(true);
			break;
		}
	}

	for(unsigned i = 0; i < cmc.opacity_points.size(); ++i) {
		if(m.is(cmc.opacity_points[i].val[1])) {
			cmc.opacity_points[i].update_pos(layout, opacity_scale_exponent);
			update_color_map(true);
			break;
		}
	}
	
	if(m.is(supports_opacity)) {
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

		post_recreate_layout();
		post_recreate_gui();
	}
}

bool color_map_editor::init(cgv::render::context& ctx) {
	
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);
	
	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("circle", cgv::g2d::shaders::circle);
	register_shader("histogram", "heightfield1d.glpr");
	register_shader("background", "color_map_editor_bg.glpr");

	bool success = canvas_overlay::init(ctx);

	success &= color_handle_renderer.init(ctx);
	success &= opacity_handle_renderer.init(ctx);
	success &= line_renderer.init(ctx);
	success &= polygon_renderer.init(ctx);
	success &= cursor_labels.init(ctx);
	success &= value_labels.init(ctx);

	if(success) {
		cursor_labels.add_text("-", vec2(0.0f));
		cursor_labels.add_text("+", vec2(0.0f));

		value_labels.add_text("", ivec2(0), cgv::render::TA_BOTTOM);
	}

	init_preview_texture(ctx);
	update_color_map(false);

	rgb a(0.75f);
	rgb b(0.9f);
	std::vector<rgb> bg_data = { a, b, b, a };
	
	bg_tex.destruct(ctx);
	cgv::data::data_view bg_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_FLT32, cgv::data::CF_RGB), bg_data.data());
	bg_tex = cgv::render::texture("flt32[R,G,B]", cgv::render::TF_NEAREST, cgv::render::TF_NEAREST, cgv::render::TW_REPEAT, cgv::render::TW_REPEAT);
	success &= bg_tex.create(ctx, bg_dv, 0);

	return success;
}

void color_map_editor::init_frame(cgv::render::context& ctx) {

	if(ensure_layout(ctx)) {
		ivec2 container_size = get_rectangle().size;
		layout.update(container_size, supports_opacity);

		auto& bg_prog = content_canvas.enable_shader(ctx, "background");
		float width_factor = static_cast<float>(layout.opacity_editor_rect.w()) / static_cast<float>(layout.opacity_editor_rect.h());
		bg_style.texcoord_scaling = vec2(5.0f * width_factor, 5.0f);
		bg_style.apply(ctx, bg_prog);
		content_canvas.disable_current_shader(ctx);

		update_point_positions();
		sort_points();
		update_geometry();
		cmc.color_points.set_constraint(layout.color_handles_rect);
		cmc.opacity_points.set_constraint(layout.opacity_editor_rect);
	}
}

void color_map_editor::draw_content(cgv::render::context& ctx) {
	
	begin_content(ctx);
	
	// draw inner border
	ivec2 container_size = get_rectangle().size;
	content_canvas.enable_shader(ctx, "rectangle");
	content_canvas.set_style(ctx, border_style);
	content_canvas.draw_shape(ctx, ivec2(layout.padding - 1) + ivec2(0, 10), container_size - 2 * layout.padding + 2 - ivec2(0, 10));
	
	if(cmc.cm) {
		// draw color scale texture
		content_canvas.set_style(ctx, color_map_style);
		preview_tex.enable(ctx, 0);
		content_canvas.draw_shape(ctx, layout.color_editor_rect);
		preview_tex.disable(ctx);
		//content_canvas.disable_current_shader(ctx);

		if(supports_opacity) {
			// draw opacity editor checkerboard background
			auto& bg_prog = content_canvas.enable_shader(ctx, "background");
			bg_style.apply(ctx, bg_prog);
			bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
			bg_tex.enable(ctx, 0);
			content_canvas.draw_shape(ctx, layout.opacity_editor_rect);
			bg_tex.disable(ctx);
			content_canvas.disable_current_shader(ctx);

			// draw histogram
			if(histogram_type != (cgv::type::DummyEnum)0 && hist_tex.is_created()) {
				auto& hist_prog = content_canvas.enable_shader(ctx, "histogram");
				hist_prog.set_uniform(ctx, "max_value", hist_norm_ignore_zero ? hist_max_non_zero : hist_max);
				hist_prog.set_uniform(ctx, "norm_gamma", hist_norm_gamma);
				hist_prog.set_uniform(ctx, "sampling_type", cgv::math::clamp(static_cast<unsigned>(histogram_type) - 1, 0u, 2u));
				hist_style.apply(ctx, hist_prog);

				hist_tex.enable(ctx, 1);
				content_canvas.draw_shape(ctx, layout.opacity_editor_rect);
				hist_tex.disable(ctx);
				content_canvas.disable_current_shader(ctx);
			}

			preview_tex.enable(ctx, 0);
			// draw transfer function area polygon
			polygon_renderer.render(ctx, content_canvas, cgv::render::PT_TRIANGLE_STRIP, cmc.triangles, polygon_style);

			// draw transfer function lines
			line_renderer.render(ctx, content_canvas, cgv::render::PT_LINE_STRIP, cmc.lines, line_style);
			preview_tex.disable(ctx);

			// draw separator line
			content_canvas.enable_shader(ctx, "rectangle");
			content_canvas.set_style(ctx, border_style);
			content_canvas.draw_shape(ctx,
				ivec2(layout.color_editor_rect.x(), layout.color_editor_rect.y1()),
				ivec2(container_size.x() - 2 * layout.padding, 1)
			);
			content_canvas.disable_current_shader(ctx);
		}

		// draw control points
		// color handles
		color_handle_renderer.render(ctx, content_canvas, cgv::render::PT_LINES, cmc.color_handles, color_handle_style);

		// opacity handles
		if(supports_opacity) {
			auto& opacity_handle_prog = opacity_handle_renderer.enable_prog(ctx);
			opacity_handle_prog.set_attribute(ctx, "size", vec2(opacity_point::default_size)); // size is constant for all points
			opacity_handle_renderer.render(ctx, content_canvas, cgv::render::PT_POINTS, cmc.opacity_handles, opacity_handle_style);
		}
	} else {
		content_canvas.disable_current_shader(ctx);
	}

	auto& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx);

	if(show_value_label) {
		content_canvas.enable_shader(ctx, "rectangle");
		content_canvas.set_style(ctx, label_box_style);

		cgv::g2d::irect rectangle(
			value_labels.ref_texts()[0].position,
			static_cast<ivec2>(value_labels.get_text_render_size(0, value_label_style.font_size))
		);
		rectangle.translate(0, 5);
		rectangle.size += ivec2(10, 6);

		content_canvas.draw_shape(ctx, rectangle);
		content_canvas.disable_current_shader(ctx);

		font_renderer.render(ctx, content_canvas, value_labels, value_label_style);
	}

	// draw cursor decorators to show interaction hints
	if(mouse_is_on_overlay && cursor_label_index > -1) {
		ivec2 pos = cursor_pos + ivec2(14, 10);
		content_canvas.push_modelview_matrix();
		content_canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(pos));

		font_renderer.render(ctx, content_canvas, cursor_labels, cursor_label_style,  cursor_label_index, 1);

		content_canvas.pop_modelview_matrix(ctx);
	}

	end_content(ctx);
}

void color_map_editor::handle_theme_change(const cgv::gui::theme_info& theme) {

	themed_canvas_overlay::handle_theme_change(theme);
	update_geometry();
	post_recreate_gui();
}

void color_map_editor::create_gui_impl() {

	if(begin_tree_node("Settings", layout, false)) {
		align("\a");
		std::string height_options = "min=";
		height_options += supports_opacity ? "80" : "40";
		height_options += ";max=500;step=10;ticks=true";
		add_member_control(this, "Height", layout.total_height, "value_slider", height_options);
		add_member_control(this, "Opacity Scale Exponent", opacity_scale_exponent, "value_slider", "min=1.0;max=5.0;step=0.001;ticks=true");
		add_member_control(this, "Resolution", resolution, "dropdown", "enums='2=2,4=4,8=8,16=16,32=32,64=64,128=128,256=256,512=512,1024=1024,2048=2048';w=106", " ");
		add_member_control(this, "Interpolate", use_interpolation, "check", "w=82");
		
		if(cmc.cm && cmc.cm->has_texture_support()) {
			add_member_control(this, "Linear Filtering (Texture)", use_linear_filtering, "check");
		}
		align("\b");
		end_tree_node(layout);

		add_decorator("Histogram", "heading", "level=4");
		add_member_control(this, "Type", histogram_type, "dropdown", "enums='None,Nearest,Linear,Smooth'");
		add_member_control(this, "Ignore Zero for Normalization", hist_norm_ignore_zero, "check");
		add_member_control(this, "Gamma", hist_norm_gamma, "value_slider", "min=0.001;max=2;step=0.001;ticks=true");
		add_member_control(this, "Fill Color", hist_style.fill_color);
		add_member_control(this, "Border Color", hist_style.border_color);
		add_member_control(this, "Border Width", hist_style.border_width, "value_slider", "min=0;max=10;step=0.5;ticks=true");
	}

	if(begin_tree_node("Color Points", cmc.color_points, true)) {
		align("\a");
		auto& points = cmc.color_points;
		for(unsigned i = 0; i < points.size(); ++i) {
			std::string label_prefix = "";
			std::string options = "w=48";
			if(&points[i] == cmc.color_points.get_selected()) {
				label_prefix = "> ";
				options += ";label_color=" + highlight_color_hex;
			}

			add_view(label_prefix + std::to_string(i), points[i].val, "", options, " ");
			add_member_control(this, "", points[i].col, "", "w=140");
		}
		align("\b");
		end_tree_node(cmc.color_points);
	}

	if(supports_opacity) {
		if(begin_tree_node("Opacity Points", cmc.opacity_points, true)) {
			align("\a");
			auto& points = cmc.opacity_points;
			for(unsigned i = 0; i < points.size(); ++i) {
				std::string label_prefix = "";
				std::string options = "w=48";
				if(&points[i] == cmc.opacity_points.get_selected()) {
					label_prefix = "> ";
					options += ";label_color=" + highlight_color_hex;
				}

				add_view(label_prefix + std::to_string(i), points[i].val[0], "", options, " ");
				add_member_control(this, "", points[i].val[1], "value", "w=140");
			}
			align("\b");
			end_tree_node(cmc.opacity_points);
		}
	}
}

void color_map_editor::set_opacity_support(bool flag) {
	
	supports_opacity = flag;
	set_color_map(cmc.cm);
	on_set(&supports_opacity);
}

void color_map_editor::set_color_map(cgv::render::color_map* cm) {
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

		use_interpolation = cm.is_interpolation_enabled();
		use_linear_filtering = true;
		
		cgv::render::gl_color_map* gl_cm = cmc.get_gl_color_map();
		if(gl_cm)
			use_linear_filtering = gl_cm->is_linear_filtering_enabled();

		update_point_positions();
		update_color_map(false);

		post_recreate_gui();
	}
}

void color_map_editor::set_histogram_data(const std::vector<unsigned> data) {
	histogram = data;

	std::vector<float> float_data(histogram.size(), 0.0f);
	hist_max = 1;
	hist_max_non_zero = 1;
	for(size_t i = 0; i < histogram.size(); ++i) {
		unsigned count = histogram[i];
		hist_max = std::max(hist_max, count);
		if(i > 0)
			hist_max_non_zero = std::max(hist_max_non_zero, count);
		float_data[i] = static_cast<float>(count);
	}

	if(auto ctx_ptr = get_context()) {
		auto& ctx = *ctx_ptr;
		hist_tex.destruct(ctx);

		cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(unsigned(histogram.size()), TI_FLT32, cgv::data::CF_R), float_data.data());
		hist_tex = cgv::render::texture("flt32[R]");
		hist_tex.create(ctx, dv, 0);

		post_damage();
	}
}

void color_map_editor::set_selected_color(rgb color) {
	
	auto selected_point = cmc.color_points.get_selected();
	if(selected_point) {
		selected_point->col = color;
		update_color_map(true);
		update_member(&selected_point->col);
		post_damage();
	}
}

void color_map_editor::init_styles() {
	auto& theme = cgv::gui::theme_info::instance();
	handle_color = rgba(theme.text(), 1.0f);
	highlight_color = rgba(theme.highlight(), 1.0f);
	highlight_color_hex = theme.highlight_hex();

	// configure style for the border rectangles
	border_style.fill_color = theme.border();
	border_style.border_width = 0.0f;
	border_style.feather_width = 0.0f;
	
	// configure style for the color scale rectangle
	color_map_style = border_style;
	color_map_style.use_texture = true;

	// configure style for background
	bg_style.use_texture = true;
	bg_style.feather_width = 0.0f;

	// configure style for histogram
	hist_style.use_blending = true;
	hist_style.feather_width = 1.0f;
	hist_style.feather_origin = 0.0f;
	hist_style.fill_color = rgba(rgb(0.5f), 0.666f);
	hist_style.border_color = rgba(rgb(0.0f), 0.666f);
	hist_style.border_width = 1.0f;

	// configure style for color handles
	color_handle_style.use_blending = true;
	color_handle_style.use_fill_color = false;
	color_handle_style.position_is_center = true;
	color_handle_style.border_color = theme.border();
	color_handle_style.border_width = 1.5f;
	color_handle_style.border_radius = 2.0f;
	color_handle_style.stem_width = color_point::default_width;
	color_handle_style.head_width = color_point::default_width;

	label_box_style.position_is_center = true;
	label_box_style.use_blending = true;
	label_box_style.fill_color = handle_color;
	label_box_style.border_color = theme.border();
	label_box_style.border_width = 1.5f;
	label_box_style.border_radius = 4.0f;

	// configure style for opacity handles
	opacity_handle_style.use_blending = true;
	opacity_handle_style.use_fill_color = false;
	opacity_handle_style.position_is_center = true;
	opacity_handle_style.border_color = theme.border();
	opacity_handle_style.border_width = 1.5f;

	// configure style for the lines and polygon
	line_style.use_blending = true;
	line_style.use_fill_color = false;
	line_style.use_texture = true;
	line_style.use_texture_alpha = false;
	line_style.width = 3.0f;

	polygon_style = static_cast<cgv::g2d::shape2d_style>(line_style);
	polygon_style.use_texture_alpha = true;

	// label style
	cursor_label_style.fill_color = rgb(0.0f);
	cursor_label_style.font_size = 16.0f;

	value_label_style.fill_color = theme.group();
	value_label_style.font_size = 12.0f;
}

void color_map_editor::setup_preview_texture(cgv::render::context& ctx) {

	if(preview_tex.is_created())
		preview_tex.destruct(ctx);

	cgv::render::TextureFilter filter = use_linear_filtering ? cgv::render::TF_LINEAR : cgv::render::TF_NEAREST;
	preview_tex = cgv::render::texture("uint8[R,G,B,A]", filter, filter);
}

void color_map_editor::init_preview_texture(cgv::render::context& ctx) {

	std::vector<uint8_t> data(resolution * 4 * 2, 0u);

	setup_preview_texture(ctx);
	cgv::data::data_view tf_dv = cgv::data::data_view(new cgv::data::data_format(resolution, 2, TI_UINT8, cgv::data::CF_RGBA), data.data());
	preview_tex.create(ctx, tf_dv, 0);
}

void color_map_editor::add_point(const vec2& pos) {

	if(cmc.cm) {
		ivec2 test_pos = static_cast<ivec2>(pos);

		if(layout.color_editor_rect.contains(test_pos)) {
			// color point
			color_point p;
			p.position = ivec2(int(pos.x()), layout.color_handles_rect.y());
			p.update_val(layout);
			p.col = cmc.cm->interpolate_color(p.val);
			cmc.color_points.add(p);
		} else if(supports_opacity && layout.opacity_editor_rect.contains(test_pos)) {
			// opacity point
			opacity_point p;
			p.position = pos;
			p.update_val(layout, opacity_scale_exponent);
			cmc.opacity_points.add(p);
		}
		
		update_color_map(true);
	}
}

void color_map_editor::remove_point(const cgv::g2d::draggable* ptr) {

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

cgv::g2d::draggable* color_map_editor::get_hit_point(const vec2& pos) {

	cgv::g2d::draggable* hit = nullptr;

	for(unsigned i = 0; i < cmc.color_points.size(); ++i) {
		color_point& p = cmc.color_points[i];
		if(p.contains(pos))
			hit = &p;
	}

	for(unsigned i = 0; i < cmc.opacity_points.size(); ++i) {
		opacity_point& p = cmc.opacity_points[i];
		if(p.contains(pos))
			hit = &p;
	}

	return hit;
}

void color_map_editor::handle_color_point_drag() {

	auto dragged_point = cmc.color_points.get_dragged();
	dragged_point->update_val(layout);
	update_color_map(true);

	show_value_label = true;
	std::string value_label = value_to_string(dragged_point->val);
	value_labels.set_text(0, value_label);

	float width = value_labels.get_text_render_size(0, value_label_style.font_size).x();
	int padding = static_cast<int>(ceil(0.5f*width)) + 4;
	ivec2 label_position = dragged_point->position;
	label_position.x() = cgv::math::clamp(label_position.x(), layout.color_editor_rect.x() + padding, layout.color_editor_rect.x1() - padding);
	label_position.y() += 25;
	value_labels.set_position(0, label_position);
	
	if(dragged_point && on_color_point_select_callback)
		on_color_point_select_callback(dragged_point->col);

	post_damage();
}

void color_map_editor::handle_opacity_point_drag() {

	auto dragged_point = cmc.opacity_points.get_dragged();
	dragged_point->update_val(layout, opacity_scale_exponent);
	update_color_map(true);

	show_value_label = true;
	std::string x_label = value_to_string(dragged_point->val.x());
	std::string y_label = cgv::utils::to_string(dragged_point->val.y(), -1, 3u);
	std::string value_label = x_label + ", " + y_label;
	value_labels.set_text(0, value_label);

	float width = value_labels.get_text_render_size(0, value_label_style.font_size).x();
	int padding = static_cast<int>(ceil(0.5f*width)) + 4;
	ivec2 label_position = dragged_point->position;
	label_position.x() = cgv::math::clamp(label_position.x(), layout.opacity_editor_rect.x() + padding, layout.opacity_editor_rect.x1() - padding);
	label_position.y() = std::min(label_position.y() + 10, layout.opacity_editor_rect.y1() - 6);
	value_labels.set_position(0, label_position);

	post_damage();
}

void color_map_editor::handle_drag_end() {

	show_value_label = false;
	update_geometry();

	auto selected_point = cmc.color_points.get_selected();
	if(selected_point) {
		if(on_color_point_select_callback)
			on_color_point_select_callback(selected_point->col);
	} else {
		if(on_color_point_deselect_callback)
			on_color_point_deselect_callback();
	}

	post_recreate_gui();
	post_damage();
}

std::string color_map_editor::value_to_string(float value) {

	float display_value = cgv::math::lerp(range.x(), range.y(), value);

	float total = range.y() - range.x();
	if(total < 100.0f) {
		// show as float
		return cgv::utils::to_string(display_value, -1, 3u);
	} else {
		// show as int
		int display_value_int = static_cast<int>(round(display_value));
		return std::to_string(display_value_int);
	}
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
	
	cgv::render::context* ctx_ptr = get_context();
	if(!ctx_ptr || !cmc.cm) return;
	cgv::render::context& ctx = *ctx_ptr;

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

		size_t idx = 4 * i;
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

	setup_preview_texture(ctx);
	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(unsigned(size), 2, TI_UINT8, cgv::data::CF_RGBA), data.data());
	preview_tex.create(ctx, dv, 0);

	if(!supports_opacity)
		cm.clear_opacity_points();

	update_geometry();

	if(on_change_callback)
		on_change_callback();

	post_damage();
}

bool color_map_editor::update_geometry() {

	cgv::render::context* ctx_ptr = get_context();
	if(!ctx_ptr || !cmc.cm) return false;
	cgv::render::context& ctx = *ctx_ptr;

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
	vec2 pos_offset = vec2(0.0f, 0.5f * color_point::default_height);

	for(unsigned i = 0; i < color_points.size(); ++i) {
		const auto& p = color_points[i];
		vec2 pos = p.center();
		rgba col = color_points.get_selected() == &p ? highlight_color : handle_color;
		color_handles.add(pos - pos_offset, col);
		color_handles.add(pos + pos_offset, col);
	}

	// create opacity handles
	for(unsigned i = 0; i < opacity_points.size(); ++i) {
		const auto& p = opacity_points[i];
		vec2 pos = p.center();
		rgba col = opacity_points.get_selected() == &p ? highlight_color : handle_color;
		opacity_handles.add(pos, col);
	}

	if(opacity_points.size() > 0) {
		const auto& pl = opacity_points[0];

		vec2 tex_coord(0.0f, 0.5f);

		lines.add(vec2(float(layout.opacity_editor_rect.x()), pl.center().y()), tex_coord);
		
		triangles.add(vec2(float(layout.opacity_editor_rect.x()), pl.center().y()), tex_coord);
		triangles.add(layout.opacity_editor_rect.position, tex_coord);

		for(unsigned i = 0; i < opacity_points.size(); ++i) {
			const auto& p = opacity_points[i];
			vec2 pos = p.center();

			tex_coord.x() = p.val.x();

			lines.add(pos, tex_coord);

			triangles.add(pos, tex_coord);
			triangles.add(vec2(pos.x(), (float)layout.opacity_editor_rect.y()), tex_coord);
		}

		const auto& pr = opacity_points[opacity_points.size() - 1];
		vec2 max_pos = layout.opacity_editor_rect.position + vec2(1.0f, 0.0f) * layout.opacity_editor_rect.size;

		tex_coord.x() = 1.0f;

		lines.add(vec2(max_pos.x(), pr.center().y()), tex_coord);

		triangles.add(vec2(max_pos.x(), pr.center().y()), tex_coord);
		triangles.add(max_pos, tex_coord);
	} else {
		success = false;
	}

	return success;
}

}
}
