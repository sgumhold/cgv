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

	shaders.add("rectangle", "rect2d.glpr");
	shaders.add("circle", "circle2d.glpr");
	shaders.add("polygon", "poly2d.glpr");
	//shaders.add("line", "line2d.glpr");
	shaders.add("histogram", "hist2d.glpr");
	shaders.add("background", "bg2d.glpr");

	show = true;

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
}

bool transfer_function_editor::on_exit_request() {
	if(has_unsaved_changes) {
		return cgv::gui::question("The transfer function has unsaved changes. Are you sure you want to quit?");
	}
	return true;
}

void transfer_function_editor::clear(cgv::render::context& ctx) {

	shaders.clear(ctx);
	fbc.clear(ctx);
}

bool transfer_function_editor::self_reflect(cgv::reflect::reflection_handler& _rh) {

	return _rh.reflect_member("file_name", file_name);
}

bool transfer_function_editor::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	if(!show)
		return false;

	if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&) e;

		if(ke.get_action() == cgv::gui::KA_PRESS) {
			switch(ke.get_key()) {
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
		} else if(ke.get_action() == cgv::gui::KA_RELEASE) {
			switch(ke.get_key()) {
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
	} else if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		switch(ma) {
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
			if(show_cursor)
				post_redraw();
			break;
		}

		if(me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if(ma == cgv::gui::MA_PRESS && modifiers > 0) {
				ivec2 mpos = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()));

				switch(modifiers) {
				case cgv::gui::EM_CTRL:
					if(!get_hit_point(mpos))
						add_point(mpos);
					break;
				case cgv::gui::EM_ALT:
				{
					point* hit_point = get_hit_point(mpos);
					if(hit_point)
						remove_point(hit_point);
				}
				break;
				}
			}
		}

		return tfc.points.handle(e, last_viewport_size, container);

		return false;
	} else {
		return false;
	}
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
	success &= shaders.load_shaders(ctx);
	success &= line_renderer.init(ctx);

	shader_program& rect_prog = shaders.get("rectangle");
	rect_prog.enable(ctx);
	rect_prog.set_uniform(ctx, "tex", 0);
	rect_prog.set_uniform(ctx, "use_blending", false);
	rect_prog.set_uniform(ctx, "use_color", true);
	rect_prog.set_uniform(ctx, "apply_gamma", false);
	rect_prog.disable(ctx);

	shader_program& point_prog = shaders.get("circle");
	point_prog.enable(ctx);
	point_prog.set_uniform(ctx, "use_blending", true);
	point_prog.set_uniform(ctx, "use_color", true);
	point_prog.set_uniform(ctx, "apply_gamma", false);
	point_prog.disable(ctx);

	shader_program& poly_prog = shaders.get("polygon");
	poly_prog.enable(ctx);
	poly_prog.set_uniform(ctx, "use_blending", true);
	poly_prog.set_uniform(ctx, "apply_gamma", false);
	poly_prog.disable(ctx);

	//shader_program& line_prog = shaders.get("line");
	//line_prog.enable(ctx);
	//line_prog.set_uniform(ctx, "use_blending", true);
	//line_prog.set_uniform(ctx, "apply_gamma", false);
	//line_prog.set_uniform(ctx, "width", 3.0f);
	//line_prog.disable(ctx);

	shader_program& line_prog = line_renderer.ref_prog();
	line_prog.enable(ctx);
	line_prog.set_uniform(ctx, "use_blending", true);
	line_prog.set_uniform(ctx, "apply_gamma", false);
	line_prog.set_uniform(ctx, "width", 3.0f);
	line_prog.disable(ctx);

	shader_program& hist_prog = shaders.get("histogram");
	hist_prog.enable(ctx);
	hist_prog.set_uniform(ctx, "use_blending", true);
	hist_prog.set_uniform(ctx, "apply_gamma", false);
	hist_prog.disable(ctx);

	shader_program& bg_prog = shaders.get("background");
	bg_prog.enable(ctx);
	bg_prog.set_uniform(ctx, "tex", 0);
	bg_prog.set_uniform(ctx, "use_blending", false);
	bg_prog.set_uniform(ctx, "use_color", false);
	bg_prog.set_uniform(ctx, "apply_gamma", false);
	bg_prog.set_uniform(ctx, "scale_exponent", 1.0f);
	bg_prog.disable(ctx);

	//tfc.lines.init(ctx);

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

		shader_program& point_prog = shaders.get("circle");
		point_prog.enable(ctx);
		point_prog.set_uniform(ctx, "resolution", container_size);
		point_prog.disable(ctx);

		shader_program& poly_prog = shaders.get("polygon");
		poly_prog.enable(ctx);
		poly_prog.set_uniform(ctx, "resolution", container_size);
		poly_prog.disable(ctx);

		//shader_program& line_prog = shaders.get("line");
		shader_program& line_prog = line_renderer.ref_prog();
		line_prog.enable(ctx);
		line_prog.set_uniform(ctx, "resolution", container_size);
		line_prog.disable(ctx);

		shader_program& hist_prog = shaders.get("histogram");
		hist_prog.enable(ctx);
		hist_prog.set_uniform(ctx, "resolution", container_size);
		hist_prog.set_uniform(ctx, "position", ivec2(layout.padding));
		hist_prog.set_uniform(ctx, "size", layout.editor_rect.size());
		hist_prog.disable(ctx);

		shader_program& bg_prog = shaders.get("background");
		bg_prog.enable(ctx);
		bg_prog.set_uniform(ctx, "resolution", container_size);
		bg_prog.set_uniform(ctx, "position", ivec2(layout.editor_rect.pos()));
		bg_prog.set_uniform(ctx, "size", layout.editor_rect.size());
		bg_prog.disable(ctx);

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

	// TODO: flag to place border on outside or inside
	shader_program& rect_prog = shaders.get("rectangle");
	rect_prog.enable(ctx);
	rect_prog.set_uniform(ctx, "resolution", container_size);
	rect_prog.set_uniform(ctx, "use_color", true);
	rect_prog.set_uniform(ctx, "use_blending", false);
	rect_prog.set_uniform(ctx, "apply_gamma", false);
	rect_prog.set_uniform(ctx, "position", ivec2(0));
	rect_prog.set_uniform(ctx, "size", container_size);
	rect_prog.set_uniform(ctx, "color", vec4(0.9f, 0.9f, 0.9f, 1.0f));
	rect_prog.set_uniform(ctx, "border_color", vec4(0.2f, 0.2f, 0.2f, 1.0f));
	rect_prog.set_uniform(ctx, "border_width", 1.0f);
	rect_prog.set_uniform(ctx, "feather_width", 0.0f);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	// draw inner border
	rect_prog.set_uniform(ctx, "position", ivec2(layout.padding - 1));
	rect_prog.set_uniform(ctx, "size", container_size - 2*layout.padding + 2);
	rect_prog.set_uniform(ctx, "color", vec4(0.2f, 0.2f, 0.2f, 1.0f));
	rect_prog.set_uniform(ctx, "border_width", 0.0f);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// draw color scale texture
	rect_prog.set_uniform(ctx, "use_color", false);
	rect_prog.set_uniform(ctx, "position", layout.color_scale_rect.pos());
	rect_prog.set_uniform(ctx, "size", layout.color_scale_rect.size());
	rect_prog.set_uniform(ctx, "tex_scaling", vec2(1.0f));
	rect_prog.set_uniform(ctx, "apply_gamma", false);

	tfc.tex.enable(ctx, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	tfc.tex.disable(ctx);

	rect_prog.disable(ctx);

	// draw editor checkerboard background
	shader_program& bg_prog = shaders.get("background");
	bg_prog.enable(ctx);
	bg_prog.set_uniform(ctx, "tex", 0);
	bg_prog.set_uniform(ctx, "feather_width", 0.0f);
	float width_factor = static_cast<float>(layout.editor_rect.size().x()) / static_cast<float>(layout.editor_rect.size().y());
	bg_prog.set_uniform(ctx, "tex_scaling", vec2(5.0f * width_factor, 5.0f));
	bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
	
	bg_tex.enable(ctx, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	bg_tex.disable(ctx);
	bg_prog.disable(ctx);

	// draw histogranm texture
	if(show_histogram && tfc.hist_tex.is_created()) {
		shader_program& hist_prog = shaders.get("histogram");
		hist_prog.enable(ctx);
		hist_prog.set_uniform(ctx, "hist_tex", 0);
		hist_prog.set_uniform(ctx, "position", layout.editor_rect.pos());
		hist_prog.set_uniform(ctx, "size", layout.editor_rect.size());
		hist_prog.set_uniform(ctx, "max_value", tfc.hist_max);
		hist_prog.set_uniform(ctx, "nearest_linear_mix", histogram_smoothing);

		hist_prog.set_uniform(ctx, "color", histogram_color);
		hist_prog.set_uniform(ctx, "border_color", histogram_border_color);
		hist_prog.set_uniform(ctx, "border_width_in_pixel", histogram_border_width);
		hist_prog.set_uniform(ctx, "feather_width", 0.0f);

		tfc.hist_tex.enable(ctx, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		tfc.hist_tex.disable(ctx);

		hist_prog.disable(ctx);
	}

	// draw transfer function area polygon
	tfc.triangles.render(ctx, PT_TRIANGLE_STRIP, shaders.get("polygon"));
	// draw transfer function lines

	/*if(tfc.lines.enable(ctx)) {
		auto& lines_prog = shaders.get("line");
		lines_prog.enable(ctx);

		GLenum pt = gl::map_to_gl(PT_LINE_STRIP);
		glDrawArrays(pt, (GLint)0, (GLsizei)tfc.line_vertex_count);

		lines_prog.disable(ctx);
		tfc.lines.disable(ctx);
	}*/
	line_renderer.render(ctx, lines, PT_LINE_STRIP);


	//tfc.lines.render(ctx, PT_LINE_STRIP, shaders.get("line"));
	
	// draw separator line
	rect_prog.enable(ctx);
	rect_prog.set_uniform(ctx, "position", ivec2(
		layout.color_scale_rect.pos().x(),
		layout.color_scale_rect.box.get_max_pnt().y()
	));
	rect_prog.set_uniform(ctx, "size", ivec2(container_size.x() - 2 * layout.padding, 1));
	rect_prog.set_uniform(ctx, "color", vec4(0.2f, 0.2f, 0.2f, 1.0f));
	rect_prog.set_uniform(ctx, "use_color", true);
	rect_prog.set_uniform(ctx, "use_blending", true);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	rect_prog.disable(ctx);

	// draw control points
	shader_program& point_prog = shaders.get("circle");
	point_prog.enable(ctx);
	
	point_prog.set_uniform(ctx, "position_is_center", true);
	point_prog.set_uniform(ctx, "border_color", rgba(0.2f, 0.2f, 0.2f, 1.0f));
	point_prog.set_uniform(ctx, "border_width", 1.5f);
	
	for(unsigned i = 0; i < tfc.points.size(); ++i) {
		const point& p = tfc.points[i];

		ivec2 pos = p.get_render_position();
		ivec2 size = p.get_render_size();

		point_prog.set_uniform(ctx, "position", pos);
		point_prog.set_uniform(ctx, "size", size);
		point_prog.set_uniform(ctx, "color",
			tfc.points.get_selected() == &p ? vec4(0.5f, 0.5f, 0.5f, 1.0f) : vec4(0.9f, 0.9f, 0.9f, 1.0f)
		);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	point_prog.disable(ctx);
	
	glDisable(GL_BLEND);

	fbc.disable(ctx);

	// draw frame buffer texture to screen
	rect_prog.enable(ctx);
	rect_prog.set_uniform(ctx, "resolution", get_viewport_size());
	rect_prog.set_uniform(ctx, "position", get_overlay_position());
	rect_prog.set_uniform(ctx, "size", container_size);
	rect_prog.set_uniform(ctx, "border_width", 0.0f);
	rect_prog.set_uniform(ctx, "feather_width", 0.0f);
	rect_prog.set_uniform(ctx, "use_color", false);
	rect_prog.set_uniform(ctx, "use_blending", false);
	rect_prog.set_uniform(ctx, "apply_gamma", true);
	
	fbc.enable_attachment(ctx, "color", 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	fbc.disable_attachment(ctx, "color");

	rect_prog.disable(ctx);

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

	add_decorator("Transfer Function Editor", "heading", "level=2");

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

void transfer_function_editor::is_visible(bool visible) {

	show = visible;
}

void transfer_function_editor::toggle_visibility() {

	show = !show;
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
	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(data.size(), TI_FLT32, cgv::data::CF_R), fdata.data());
	tfc.hist_tex = texture("flt32[I]", TF_LINEAR, TF_LINEAR);
	tfc.hist_tex.create(*ctx_ptr, dv, 0);
	return true;
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

void transfer_function_editor::init_transfer_function_texture(context& ctx) {

	std::vector<uint8_t> data(resolution * 4, 0u);

	tf_tex.destruct(ctx);
	cgv::data::data_view tf_dv = cgv::data::data_view(new cgv::data::data_format(resolution, TI_UINT8, cgv::data::CF_RGBA), data.data());
	tf_tex = texture("uint8[R,G,B,A]", TF_LINEAR, TF_LINEAR);
	tf_tex.create(ctx, tf_dv, 0);
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
	//auto& lines = tfc.lines;
	auto& triangles = tfc.triangles;

	triangles.clear(ctx);
	lines.clear();
	//std::vector<vec2> line_positions;
	//std::vector<rgb> line_colors;
	
	bool success = true;

	if(points.size() > 1) {
		const point& pl = points[0];
		rgba coll = tf.interpolate(pl.val.x());

		lines.add(vec2(layout.editor_rect.pos().x(), pl.center().y()), rgb(coll));
		//line_positions.push_back(vec2(layout.editor_rect.pos().x(), pl.center().y()));
		//line_colors.push_back(rgb(coll));

		triangles.add(vec2(layout.editor_rect.pos().x(), pl.center().y()), coll);
		triangles.add(layout.editor_rect.pos(), coll);

		for(unsigned i = 0; i < points.size(); ++i) {
			vec2 pos = points[i].center();
			rgba col = tf.interpolate(points[i].val.x());

			lines.add(pos, rgb(col));
			//line_positions.push_back(pos);
			//line_colors.push_back(rgb(col));

			triangles.add(pos, col);
			triangles.add(vec2(pos.x(), layout.editor_rect.pos().y()), col);
		}

		const point& pr = points[points.size() - 1];
		rgba colr = tf.interpolate(pr.val.x());
		vec2 max_pos = layout.editor_rect.pos() + vec2(1.0f, 0.0f) * layout.editor_rect.size();

		lines.add(vec2(max_pos.x(), pr.center().y()), rgb(colr));
		//line_positions.push_back(vec2(max_pos.x(), pr.center().y()));
		//line_colors.push_back(rgb(colr));

		triangles.add(vec2(max_pos.x(), pr.center().y()), colr);
		triangles.add(max_pos, colr);

		success &= triangles.create(ctx, shaders.get("polygon"));
		


		//success &= lines.create(ctx, shaders.get("line"));

		//tfc.line_vertex_count = line_positions.size();
		//auto& line_prog = shaders.get("line");
		//lines.set_attribute_array(ctx, line_prog.get_attribute_location(ctx, "position"), line_positions);
		//lines.set_attribute_array(ctx, line_prog.get_attribute_location(ctx, "color"), line_colors);

		//line_renderer.set_vertex_count(line_positions.size());
		////auto& line_prog = shaders.get("line");
		//line_renderer.set_attribute_array(ctx, "position", line_positions);
		//line_renderer.set_attribute_array(ctx, "color", line_colors);
		lines.set_out_of_date();

	} else {
		success = false;
	}
	return success;
}

// TODO: move these three methods to some string lib
static std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {

	str.erase(0, str.find_first_not_of(chars));
	return str;
}

static std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {

	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

static std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {

	return ltrim(rtrim(str, chars), chars);
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
		int value_i = 0.0f;

		try {
			value_i = stoi(value_str);
		} catch(const std::invalid_argument& e) {
			return false;
		} catch(const std::out_of_range& e) {
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
		} catch(const std::invalid_argument& e) {
			return false;
		} catch(const std::out_of_range& e) {
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

		trim(line);

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
