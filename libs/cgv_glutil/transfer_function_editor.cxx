#include "transfer_function_editor.h"

#include <cgv/defines/quote.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
namespace glutil {

transfer_function_editor::transfer_function_editor() {

	set_name("Transfer Function Editor");
#ifdef CGV_FORCE_STATIC
	file_name = "";
#else
	file_name = QUOTE_SYMBOL_VALUE(INPUT_DIR);
	file_name += "/res/default.xml";
#endif

	layout.padding = 8;
	layout.total_height = 200;
	layout.color_scale_height = 30;

	set_overlay_alignment(AO_START, AO_START);
	set_overlay_stretch(SO_HORIZONTAL);
	set_overlay_margin(ivec2(20));
	set_overlay_size(ivec2(600u, layout.total_height));
	
	fbc.add_attachment("color", "flt32[R,G,B,A]");
	fbc.set_size(get_overlay_size());

	canvas.register_shader("rectangle", "rect2d.glpr");
	canvas.register_shader("circle", "circle2d.glpr");
	canvas.register_shader("histogram", "hist2d.glpr");
	canvas.register_shader("background", "bg2d.glpr");

	overlay_canvas.register_shader("rectangle", "rect2d.glpr");

	opacity_scale_exponent = 1.0f;
	resolution = (cgv::type::DummyEnum)512;

	show_histogram = true;
	histogram_color = rgba(0.5f, 0.5f, 0.5f, 0.75f);
	histogram_border_color = rgba(0.0f, 0.0f, 0.0f, 1.0f);
	histogram_border_width = 0u;
	histogram_smoothing = 0.5f;

	mouse_is_on_overlay = false;
	show_cursor = false;
	cursor_pos = ivec2(-100);
	cursor_drawtext = "";

	tfc.points.set_drag_callback(std::bind(&transfer_function_editor::handle_drag, this));
	tfc.points.set_drag_end_callback(std::bind(&transfer_function_editor::handle_drag_end, this));
	tfc.points.set_constraint(layout.editor_rect);

	line_renderer = generic_renderer("line2d.glpr");
	polygon_renderer = generic_renderer("poly2d.glpr");
	point_renderer = generic_renderer("circle2d.glpr");
}

bool transfer_function_editor::on_exit_request() {
	// TODO: does not seem to fire when window is maximized
	if(has_unsaved_changes) {
		return cgv::gui::question("The transfer function has unsaved changes. Are you sure you want to quit?");
	}
	return true;
}

void transfer_function_editor::clear(cgv::render::context& ctx) {

	canvas.destruct(ctx);
	overlay_canvas.destruct(ctx);
	fbc.clear(ctx);
}

bool transfer_function_editor::self_reflect(cgv::reflect::reflection_handler& _rh) {

	return _rh.reflect_member("file_name", file_name);
}

bool transfer_function_editor::handle_event(cgv::gui::event& e) {

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
			cursor_pos = ivec2(me.get_x(), me.get_y());
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
					point* hit_point = get_hit_point(mpos);
					if (hit_point)
						remove_point(hit_point);
				}
				break;
				}
			}
		}

		return tfc.points.handle(e, last_viewport_size, container);
	}
	return false;
}

void transfer_function_editor::on_set(void* member_ptr) {

	if(member_ptr == &file_name) {
/*
#ifndef CGV_FORCE_STATIC
		// TODO: implemenmt
		std::cout << "IMPLEMENT" << std::endl;
		//std::filesystem::path file_path(file_name);
		//if(file_path.is_relative()) {
		//	std::string debug_file_name = QUOTE_SYMBOL_VALUE(INPUT_DIR);
		//	file_name = debug_file_name + "/" + file_name;
		//}
#endif
*/
		if(!load_from_xml(file_name))
			tfc.reset();

		update_point_positions();
		update_transfer_function(false);

		has_unsaved_changes = false;
		on_set(&has_unsaved_changes);
		
		post_recreate_gui();
	}

	if(member_ptr == &save_file_name) {
		std::string extension = cgv::utils::file::get_extension(save_file_name);

		if(extension == "") {
			extension = "xml";
			save_file_name += "." + extension;
		}

		if(cgv::utils::to_upper(extension) == "XML") {
			if(save_to_xml(save_file_name)) {
				file_name = save_file_name;
				update_member(&file_name);
				has_unsaved_changes = false;
				on_set(&has_unsaved_changes);
			} else {
				std::cout << "Error: Could not write transfer function to file: " << save_file_name << std::endl;
			}
		} else {
			std::cout << "Please specify a xml file name." << std::endl;
		}
	}

	if(member_ptr == &has_unsaved_changes) {
		auto ctrl = find_control(file_name);
		if(ctrl)
			ctrl->set("color", has_unsaved_changes ? "0xff6666" : "0xffffff");
	}
	
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
			init_transfer_function_texture(*ctx_ptr);
		update_transfer_function(false);
	}

	for(unsigned i = 0; i < tfc.points.size(); ++i) {
		if(member_ptr == &tfc.points[i].col) {
			update_transfer_function(true);
			break;
		}
	}
	
	update_member(member_ptr);
	post_redraw();
}

bool transfer_function_editor::init(cgv::render::context& ctx) {
	
	// get a bold font face to use for the cursor
	auto font = cgv::media::font::find_font("Arial");
	if(!font.empty()) {
		cursor_font_face = font->get_font_face(cgv::media::font::FFA_BOLD);
	}

	bool success = true;

	success &= fbc.ensure(ctx);
	success &= canvas.init(ctx);
	success &= overlay_canvas.init(ctx);
	success &= line_renderer.init(ctx);
	success &= polygon_renderer.init(ctx);
	success &= point_renderer.init(ctx);

	if(success)
		init_styles(ctx);

	if(!load_from_xml(file_name))
		tfc.reset();
	
	init_transfer_function_texture(ctx);
	update_transfer_function(false);

	rgb a(0.75f);
	rgb b(0.9f);
	std::vector<rgb> bg_data = { a, b, b, a };
	
	bg_tex.destruct(ctx);
	cgv::data::data_view bg_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_FLT32, cgv::data::CF_RGB), bg_data.data());
	bg_tex = texture("flt32[R,G,B]", TF_NEAREST, TF_NEAREST, TW_REPEAT, TW_REPEAT);
	success &= bg_tex.create(ctx, bg_dv, 0);

	return success;
}

void transfer_function_editor::init_frame(cgv::render::context& ctx) {

	if(ensure_overlay_layout(ctx)) {
		ivec2 container_size = get_overlay_size();
		layout.update(container_size);

		fbc.set_size(container_size);
		fbc.ensure(ctx);

		canvas.set_resolution(ctx, container_size);
		overlay_canvas.set_resolution(ctx, get_viewport_size());

		auto& bg_prog = canvas.enable_shader(ctx, "background");
		float width_factor = static_cast<float>(layout.editor_rect.size().x()) / static_cast<float>(layout.editor_rect.size().y());
		bg_style.texcoord_scaling = vec2(5.0f * width_factor, 5.0f);
		bg_style.apply(ctx, bg_prog);
		canvas.disable_current_shader(ctx);

		update_point_positions();
		sort_points();
		update_geometry();
		tfc.points.set_constraint(layout.editor_rect);
	}
}

void transfer_function_editor::draw(cgv::render::context& ctx) {

	if(!show)
		return;

	fbc.enable(ctx);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	ivec2 container_size = get_overlay_size();
	
	// draw container
	auto& rect_prog = canvas.enable_shader(ctx, "rectangle");
	container_style.apply(ctx, rect_prog);
	canvas.draw_shape(ctx, ivec2(0), container_size);
	
	// draw inner border
	border_style.apply(ctx, rect_prog);
	canvas.draw_shape(ctx, ivec2(layout.padding - 1), container_size - 2*layout.padding + 2);
	
	// draw color scale texture
	color_scale_style.apply(ctx, rect_prog);
	tfc.tex.enable(ctx, 0);
	canvas.draw_shape(ctx, layout.color_scale_rect.pos(), layout.color_scale_rect.size());
	tfc.tex.disable(ctx);
	canvas.disable_current_shader(ctx);

	// draw editor checkerboard background
	auto& bg_prog = canvas.enable_shader(ctx, "background");
	bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
	bg_tex.enable(ctx, 0);
	canvas.draw_shape(ctx, layout.editor_rect.pos(), layout.editor_rect.size());
	bg_tex.disable(ctx);
	canvas.disable_current_shader(ctx);

	// draw histogram texture
	if(show_histogram && tfc.hist_tex.is_created()) {
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
	}

	// draw transfer function area polygon
	auto& poly_prog = polygon_renderer.ref_prog();
	poly_prog.enable(ctx);
	canvas.set_view(ctx, poly_prog);
	poly_prog.disable(ctx);
	polygon_renderer.render(ctx, PT_TRIANGLE_STRIP, tfc.triangles);

	// draw transfer function lines
	auto& line_prog = line_renderer.ref_prog();
	line_prog.enable(ctx);
	canvas.set_view(ctx, line_prog);
	line_prog.disable(ctx);
	line_renderer.render(ctx, PT_LINE_STRIP, tfc.lines);

	// draw separator line
	rect_prog = canvas.enable_shader(ctx, "rectangle");
	border_style.apply(ctx, rect_prog);
	canvas.draw_shape(ctx,
		ivec2(layout.color_scale_rect.pos().x(), layout.color_scale_rect.box.get_max_pnt().y()),
		ivec2(container_size.x() - 2 * layout.padding, 1)
	);
	canvas.disable_current_shader(ctx);

	// draw control points
	auto& point_prog = point_renderer.ref_prog();
	point_prog.enable(ctx);
	canvas.set_view(ctx, point_prog);
	// size is constant for all points
	point_prog.set_attribute(ctx, "size", vec2(tfc.points[0].get_render_size()));
	point_prog.disable(ctx);
	point_renderer.render(ctx, PT_POINTS, tfc.point_geometry);

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

void transfer_function_editor::create_gui() {

	create_overlay_gui();

	add_decorator("File", "heading", "level=3");
	std::string filter = "XML Files (xml):*.xml|All Files:*.*";
	add_gui("File", file_name, "file_name", "title='Open Transfer Function';filter='" + filter + "';save=false;w=136;small_icon=true;align_gui=' ';color=" + (has_unsaved_changes ? "0xff6666" : "0xffffff"));
	add_gui("save_file_name", save_file_name, "file_name", "title='Save Transfer Function';filter='" + filter + "';save=true;control=false;small_icon=true");

	if(begin_tree_node("Settings", layout, false)) {
		align("\a");
		add_member_control(this, "Height", layout.total_height, "value_slider", "min=100;max=500;step=10;ticks=true");
		add_member_control(this, "Opacity Scale Exponent", opacity_scale_exponent, "value_slider", "min=1.0;max=5.0;step=0.001;ticks=true");
		add_member_control(this, "Resolution", resolution, "dropdown", "enums='2=2,4=4,8=8,16=16,32=32,64=64,128=128,256=256,512=512,1024=1024,2048=2048'");
		align("\b");
		end_tree_node(layout);
	}

	if(begin_tree_node("Histogram", show_histogram, false)) {
		align("\a");
		add_member_control(this, "Show", show_histogram, "check");
		add_member_control(this, "Fill Color", histogram_color, "");
		add_member_control(this, "Border Color", histogram_border_color, "");
		add_member_control(this, "Border Width", histogram_border_width, "value_slider", "min=0;max=10;step=1;ticks=true");
		add_member_control(this, "Smoothing", histogram_smoothing, "value_slider", "min=0;max=1;step=0.01;ticks=true");
		align("\b");
		end_tree_node(show_histogram);
	}

	add_decorator("Control Points", "heading", "level=3");
	// TODO: add parameters for t and alpha?
	auto& points = tfc.points;
	for(unsigned i = 0; i < points.size(); ++i)
		add_member_control(this, "Color " + std::to_string(i), points[i].col, "", &points[i] == tfc.points.get_selected() ? "label_color=0x4080ff" : "");
}

void transfer_function_editor::create_gui(cgv::gui::provider& p) {

	p.add_member_control(this, "Show", show, "check");
}

texture& transfer_function_editor::ref_tex() {

	return tf_tex;
}

bool transfer_function_editor::set_histogram(const std::vector<unsigned>& data) {

	context* ctx_ptr = get_context();
	if(!ctx_ptr)
		return false;

	std::vector<float> fdata(data.size());

	tfc.hist_max = 0;
	for(unsigned i = 0; i < data.size(); ++i) {
		unsigned value = data[i];
		tfc.hist_max = std::max(tfc.hist_max, value);
		fdata[i] = static_cast<float>(value);
	}

	tfc.hist_tex.destruct(*ctx_ptr);
	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(unsigned(data.size()), TI_FLT32, cgv::data::CF_R), fdata.data());
	tfc.hist_tex = texture("flt32[I]", TF_LINEAR, TF_LINEAR);
	tfc.hist_tex.create(*ctx_ptr, dv, 0);
	return true;
}

void transfer_function_editor::init_styles(context& ctx) {

	// configure style for the container rectangle
	container_style.apply_gamma = false;
	container_style.fill_color = rgba(0.9f, 0.9f, 0.9f, 1.0f);
	container_style.border_color = rgba(0.2f, 0.2f, 0.2f, 1.0f);
	container_style.border_width = 1.0f;
	container_style.feather_width = 0.0f;
	
	// configure style for the border rectangles
	border_style = container_style;
	border_style.fill_color = rgba(0.2f, 0.2f, 0.2f, 1.0f);
	border_style.border_width = 0.0f;
	
	// configure style for the color scale rectangle
	color_scale_style = border_style;
	color_scale_style.use_texture = true;

	// configure style for background
	bg_style.use_texture = true;
	bg_style.apply_gamma = false;
	bg_style.feather_width = 0.0f;
	bg_style.texcoord_scaling = vec2(5.0f, 5.0f);

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

	// configure style for control points
	cgv::glutil::shape2d_style point_style;
	point_style.use_blending = true;
	point_style.apply_gamma = false;
	point_style.use_fill_color = false;
	point_style.position_is_center = true;
	point_style.border_color = rgba(0.2f, 0.2f, 0.2f, 1.0f);
	point_style.border_width = 1.5f;

	auto& point_prog = point_renderer.ref_prog();
	point_prog.enable(ctx);
	point_style.apply(ctx, point_prog);
	point_prog.disable(ctx);

	// configure style for the lines and polygon
	cgv::glutil::line2d_style line_style;
	line_style.use_blending = true;
	line_style.use_fill_color = false;
	line_style.apply_gamma = false;
	line_style.width = 3.0f;

	auto& line_prog = line_renderer.ref_prog();
	line_prog.enable(ctx);
	line_style.apply(ctx, line_prog);
	line_prog.disable(ctx);

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

void transfer_function_editor::init_transfer_function_texture(context& ctx) {

	std::vector<uint8_t> data(resolution * 4, 0u);

	tf_tex.destruct(ctx);
	cgv::data::data_view tf_dv = cgv::data::data_view(new cgv::data::data_format(resolution, TI_UINT8, cgv::data::CF_RGBA), data.data());
	tf_tex = texture("uint8[R,G,B,A]", TF_LINEAR, TF_LINEAR);
	tf_tex.create(ctx, tf_dv, 0);
}

void transfer_function_editor::add_point(const vec2& pos) {

	point p;
	p.pos = pos;
	p.update_val(layout, opacity_scale_exponent);
	p.col = tfc.tf.interpolate_color(p.val.x());
	tfc.points.add(p);

	update_transfer_function(true);
}

void transfer_function_editor::remove_point(const point* ptr) {

	if(tfc.points.size() < 3)
		return;

	bool removed = false;
	std::vector<point> next_points;
	for(unsigned i = 0; i < tfc.points.size(); ++i) {
		if(&tfc.points[i] != ptr)
			next_points.push_back(tfc.points[i]);
		else
			removed = true;
	}
	tfc.points.ref_draggables() = std::move(next_points);
	
	if(removed)
		update_transfer_function(true);
}

transfer_function_editor::point* transfer_function_editor::get_hit_point(const transfer_function_editor::vec2& pos) {

	point* hit = nullptr;
	for(unsigned i = 0; i < tfc.points.size(); ++i) {
		point& p = tfc.points[i];
		if(p.is_inside(pos))
			hit = &p;
	}

	return hit;
}

void transfer_function_editor::handle_drag() {

	tfc.points.get_dragged()->update_val(layout, opacity_scale_exponent);
	update_transfer_function(true);
	post_redraw();
}

void transfer_function_editor::handle_drag_end() {

	post_recreate_gui();
	post_redraw();
}

void transfer_function_editor::sort_points() {

	auto& points = tfc.points;

	if(points.size() > 1) {
		int dragged_point_idx = -1;
		int selected_point_idx = -1;

		const point* dragged_point = points.get_dragged();
		const point* selected_point = points.get_selected();

		std::vector<std::pair<point, int>> sorted(points.size());

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

void transfer_function_editor::update_point_positions() {

	for(unsigned i = 0; i < tfc.points.size(); ++i)
		tfc.points[i].update_pos(layout, opacity_scale_exponent);
}

void transfer_function_editor::update_transfer_function(bool is_data_change) {
	
	context* ctx_ptr = get_context();
	if(!ctx_ptr) return;
	context& ctx = *ctx_ptr;

	auto& tf = tfc.tf;
	auto& tex = tfc.tex;
	auto& points = tfc.points;
	
	sort_points();

	tf.clear();

	for(unsigned i = 0; i < points.size(); ++i) {
		const point& p = points[i];
		tf.add_color_point(p.val.x(), p.col);
		tf.add_opacity_point(p.val.x(), p.val.y());
	}

	std::vector<rgba> tf_data;

	unsigned size = resolution;
	float step = 1.0f / static_cast<float>(size - 1);

	for(unsigned i = 0; i < size; ++i) {
		float t = i * step;
		rgba col = tf.interpolate(t);
		tf_data.push_back(col);
	}

	std::vector<rgba> data2d(2 * size);
	for(unsigned i = 0; i < size; ++i) {
		data2d[i + 0] = tf_data[i];
		data2d[i + size] = tf_data[i];
	}

	tex.destruct(ctx);
	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(size, 2, TI_FLT32, cgv::data::CF_RGBA), data2d.data());
	tex = texture("flt32[R,G,B,A]", TF_LINEAR, TF_LINEAR);
	tex.create(ctx, dv, 0);

	if(tf_tex.is_created()) {
		std::vector<uint8_t> tf_data_8(4*tf_data.size());
		for(unsigned i = 0; i < tf_data.size(); ++i) {
			rgba col = tf_data[i];
			tf_data_8[4 * i + 0] = static_cast<uint8_t>(255.0f * col.R());
			tf_data_8[4 * i + 1] = static_cast<uint8_t>(255.0f * col.G());
			tf_data_8[4 * i + 2] = static_cast<uint8_t>(255.0f * col.B());
			tf_data_8[4 * i + 3] = static_cast<uint8_t>(255.0f * col.alpha());
		}

		cgv::data::data_view dv1d = cgv::data::data_view(new cgv::data::data_format(size, TI_UINT8, cgv::data::CF_RGBA), tf_data_8.data());
		tf_tex.replace(ctx, 0, dv1d);
	}

	if(is_data_change) {
		has_unsaved_changes = true;
		on_set(&has_unsaved_changes);
	} else {
		bool had_unsaved_changes = has_unsaved_changes;
		if(has_unsaved_changes != had_unsaved_changes) {
			has_unsaved_changes = has_unsaved_changes;
			has_unsaved_changes = false;
			on_set(&has_unsaved_changes);
		}
	}

	update_geometry();
}

bool transfer_function_editor::update_geometry() {

	context* ctx_ptr = get_context();
	if(!ctx_ptr) return false;
	context& ctx = *ctx_ptr;

	auto& tf = tfc.tf;
	auto& points = tfc.points;
	auto& lines = tfc.lines;
	auto& triangles = tfc.triangles;
	auto& point_geometry = tfc.point_geometry;

	lines.clear();
	triangles.clear();
	point_geometry.clear();
	
	bool success = true;

	if(points.size() > 1) {
		const point& pl = points[0];
		rgba coll = tf.interpolate(pl.val.x());

		lines.add(vec2(float(layout.editor_rect.pos().x()), pl.center().y()), rgb(coll));
		
		triangles.add(vec2(float(layout.editor_rect.pos().x()), pl.center().y()), coll);
		triangles.add(layout.editor_rect.pos(), coll);

		for(unsigned i = 0; i < points.size(); ++i) {
			const auto& p = points[i];
			vec2 pos = p.center();
			rgba col = tf.interpolate(points[i].val.x());

			lines.add(pos, rgb(col));
			triangles.add(pos, col);
			triangles.add(vec2(pos.x(), (float)layout.editor_rect.pos().y()), col);
			point_geometry.add(pos,
				tfc.points.get_selected() == &p ? rgba(0.5f, 0.5f, 0.5f, 1.0f) : rgba(0.9f, 0.9f, 0.9f, 1.0f)
			);
		}

		const point& pr = points[points.size() - 1];
		rgba colr = tf.interpolate(pr.val.x());
		vec2 max_pos = layout.editor_rect.pos() + vec2(1.0f, 0.0f) * layout.editor_rect.size();

		lines.add(vec2(max_pos.x(), pr.center().y()), rgb(colr));
		
		triangles.add(vec2(max_pos.x(), pr.center().y()), colr);
		triangles.add(max_pos, colr);

		lines.set_out_of_date();
		triangles.set_out_of_date();
		point_geometry.set_out_of_date();

	} else {
		success = false;
	}
	return success;
}

static std::string xml_attribute_value(const std::string& attribute) {

	size_t pos_start = attribute.find_first_of("\"");
	size_t pos_end = attribute.find_last_of("\"");

	if(pos_start != std::string::npos &&
		pos_end != std::string::npos &&
		pos_start < pos_end &&
		attribute.length() > 2) {
		return attribute.substr(pos_start + 1, pos_end - pos_start - 1);
	}

	return "";
}

static std::pair<std::string, std::string> xml_attribute_pair(const std::string& attribute) {

	std::string name = "";
	std::string value = "";

	size_t pos = attribute.find_first_of('=');

	if(pos != std::string::npos) {
		name = attribute.substr(0, pos);
	}

	size_t pos_start = attribute.find_first_of("\"", pos);
	size_t pos_end = attribute.find_last_of("\"");

	if(pos_start != std::string::npos &&
		pos_end != std::string::npos &&
		pos_start < pos_end &&
		attribute.length() > 2) {
		value = attribute.substr(pos_start + 1, pos_end - pos_start - 1);
	}

	return { name, value };
}

static bool xml_attribute_to_int(const std::string& attribute, int& value) {

	std::string value_str = xml_attribute_value(attribute);

	if(!value_str.empty()) {
		int value_i = 0;

		try {
			value_i = stoi(value_str);
		} catch(const std::invalid_argument&) {
			return false;
		} catch(const std::out_of_range&) {
			return false;
		}

		value = value_i;
		return true;
	}

	return false;
}

static bool xml_attribute_to_float(const std::string& attribute, float& value) {

	std::string value_str = xml_attribute_value(attribute);

	if(!value_str.empty()) {
		float value_f = 0.0f;

		try {
			value_f = stof(value_str);
		} catch(const std::invalid_argument&) {
			return false;
		} catch(const std::out_of_range&) {
			return false;
		}

		value = value_f;
		return true;
	}

	return false;
}

bool transfer_function_editor::load_from_xml(const std::string& file_name) {

	if(!cgv::utils::file::exists(file_name) || cgv::utils::to_upper(cgv::utils::file::get_extension(file_name)) != "XML")
		return false;

	std::string content;
	cgv::utils::file::read(file_name, content, true);

	bool read = true;
	size_t nl_pos = content.find_first_of("\n");
	size_t line_offset = 0;
	bool first_line = true;

	int active_stain_idx = -1;

	tfc.points.clear();

	while(read) {
		std::string line = "";

		if(nl_pos == std::string::npos) {
			read = false;
			line = content.substr(line_offset, std::string::npos);
		} else {
			size_t next_line_offset = nl_pos;
			line = content.substr(line_offset, next_line_offset - line_offset);
			line_offset = next_line_offset + 1;
			nl_pos = content.find_first_of('\n', line_offset);
		}

		cgv::utils::trim(line);

		if(line.length() < 3)
			continue;
		
		line = line.substr(1, line.length() - 2);

		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(line, tokens, "", true, "", "");

		if(tokens.size() == 1 && to_string(tokens[0]) == "TransferFunction") {
			// is a transfer function
		}

		if(tokens.size() == 6 && to_string(tokens[0]) == "Point") {
			float pos = -1.0f;
			int r = -1;
			int g = -1;
			int b = -1;
			float a = -1.0f;

			xml_attribute_to_float(to_string(tokens[1]), pos);
			xml_attribute_to_int(to_string(tokens[2]), r);
			xml_attribute_to_int(to_string(tokens[3]), g);
			xml_attribute_to_int(to_string(tokens[4]), b);
			xml_attribute_to_float(to_string(tokens[5]), a);
				
			if(!(pos < 0.0f)) {
				rgb col(0.0f);
				float alpha = 0.0f;

				if(!(r < 0 || g < 0 || b < 0)) {
					col[0] = cgv::math::clamp(static_cast<float>(r / 255.0f), 0.0f, 1.0f);
					col[1] = cgv::math::clamp(static_cast<float>(g / 255.0f), 0.0f, 1.0f);
					col[2] = cgv::math::clamp(static_cast<float>(b / 255.0f), 0.0f, 1.0f);
				}

				if(!(a < 0.0f)) {
					alpha = cgv::math::clamp(a, 0.0f, 1.0f);
				}

				point p;
				p.col = col;
				p.val.x() = cgv::math::clamp(pos, 0.0f, 1.0f);
				p.val.y() = alpha;
				tfc.points.add(p);
			}
		}
	}

	return true;
}

bool transfer_function_editor::save_to_xml(const std::string& file_name) {

	auto to_col_uint8 = [](const float& val) {
		int ival = cgv::math::clamp(static_cast<int>(255.0f * val + 0.5f), 0, 255);
		return static_cast<unsigned char>(ival);
	};

	std::string content = "";
	content += "<TransferFunction>\n";
	std::string tab = "  ";

	for(unsigned i = 0; i < tfc.points.size(); ++i) {
		const point& p = tfc.points[i];

		content += tab + "<Point ";
		content += "position=\"" + std::to_string(p.val.x()) + "\" ";
		content += "r=\"" + std::to_string(to_col_uint8(p.col.R())) + "\" ";
		content += "g=\"" + std::to_string(to_col_uint8(p.col.G())) + "\" ";
		content += "b=\"" + std::to_string(to_col_uint8(p.col.B())) + "\" ";
		content += "opacity=\"" + std::to_string(p.val.y()) + "\"";
		content += "/>\n";
	}
	
	content += "</TransferFunction>\n";

	return cgv::utils::file::write(file_name, content, true);
}

}
}
