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

	last_viewport_size = vec2(-1.0f);

	update_layout = false;
	height = 200u;

	layout.margin = 20u;
	layout.padding = 10u;
	layout.container_rect.set_size(uvec2(600u, height));

	fbc.add_attachment("color", "flt32[R,G,B,A]");
	fbc.set_size(layout.container_rect.size());

	shaders.add("rectangle", "rect2d.glpr");
	shaders.add("ellipse", "ellipse2d.glpr");
	shaders.add("polygon", "poly2d.glpr");
	shaders.add("line", "line2d.glpr");
	shaders.add("histogram", "hist2d.glpr");
	shaders.add("background", "bg2d.glpr");

	show = true;
	left_mouse_button_pressed = false;
	has_captured_mouse = false;

	opacity_scale_exponent = 1.0f;
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

bool transfer_function_editor::handle(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	context* ctx_ptr = get_context();

	if(!show || !ctx_ptr)
		return false;

	if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();
		
		ivec2 mpos(me.get_x(), me.get_y());
		mpos.y() = last_viewport_size.y() - mpos.y();
		mpos -= layout.margin;

		if(me.get_button() == cgv::gui::MouseButton::MB_LEFT_BUTTON) {
			if(ma == cgv::gui::MouseAction::MA_RELEASE) {
				has_captured_mouse = false;

				if(dragged_point) {
					selected_point = dragged_point;
					dragged_point = nullptr;
				} else {
					selected_point = get_hit_point(mpos);
				}
				post_recreate_gui();
				post_redraw();
			}
		}

		if(me.get_button_state() & cgv::gui::MouseButton::MB_LEFT_BUTTON) {
			bool is_on_overlay = layout.is_hit(mpos);

			if(is_on_overlay && me.get_button() == cgv::gui::MouseButton::MB_LEFT_BUTTON) {
				has_captured_mouse = true;
			}

			if(is_on_overlay || has_captured_mouse) {
				if(ma == cgv::gui::MouseAction::MA_PRESS && modifiers > 0) {
					switch(modifiers) {
					case cgv::gui::EM_CTRL:
						if(!get_hit_point(mpos))
							add_point(*ctx_ptr, mpos);
						break;
					case cgv::gui::EM_ALT:
					{
						point* hit_point = get_hit_point(mpos);
						if(hit_point)
							if(remove_point(*ctx_ptr, hit_point))
								update_transfer_function(*ctx_ptr);
						if(hit_point == selected_point) {
							selected_point = nullptr;
							post_recreate_gui();
						}
					}
					break;
					}
				} else {
					if(dragged_point) {
						dragged_point->pos = mpos + offset_pos;
						dragged_point->update_val(layout, opacity_scale_exponent);
						update_transfer_function(*ctx_ptr);
					} else {
						if(ma == cgv::gui::MouseAction::MA_PRESS) {
							dragged_point = get_hit_point(mpos);
							if(dragged_point)
								offset_pos = dragged_point->pos - mpos;
							selected_point = dragged_point;
							post_recreate_gui();
						}
					}
				}

				post_redraw();
				return true;
			}

			return has_captured_mouse;
		}

		return false;
	} else {
		return false;
	}
}

void transfer_function_editor::on_set(void* member_ptr) {

	if(member_ptr == &file_name) {
#ifndef CGV_FORCE_STATIC
		// TODO: implemenmt
		std::cout << "IMPLEMENT" << std::endl;
		//std::filesystem::path file_path(file_name);
		//if(file_path.is_relative()) {
		//	std::string debug_file_name = QUOTE_SYMBOL_VALUE(INPUT_DIR);
		//	file_name = debug_file_name + "/" + file_name;
		//}
#endif
		if(!load_from_xml(file_name)) {
			tfc = tf_container();
		}

		context* ctx_ptr = get_context();
		if(ctx_ptr)
			update_all_transfer_functions(*ctx_ptr);

		post_recreate_gui();
	}

	if(member_ptr == &save_file_name) {
		if(cgv::utils::to_upper(cgv::utils::file::get_extension(save_file_name)) == "XML") {
			if(save_to_xml(save_file_name)) {
				file_name = save_file_name;
				update_member(&file_name);
				has_unsaved_changes = false;
				on_set(&has_unsaved_changes);
			} else {
				std::cout << "Error: Could not write transfer function to file: " << save_file_name << std::endl;
			}
		} else {
			// TODO: suffix name with .xml if not done by user
			std::cout << "Please specify a xml file name." << std::endl;
		}
	}

	if(member_ptr == &has_unsaved_changes) {
		auto ctrl = find_control(file_name);
		if(ctrl)
			ctrl->set("color", has_unsaved_changes ? "0xff6666" : "0xffffff");
	}
	
	if(member_ptr == &height) {
		layout.container_rect.set_size(ivec2(layout.container_rect.size().x(), height));
		update_layout = true;
	}

	if(member_ptr == &opacity_scale_exponent) {
		opacity_scale_exponent = cgv::math::clamp(opacity_scale_exponent, 1.0f, 5.0f);
		context* ctx_ptr = get_context();
		if(ctx_ptr)
			update_all_transfer_functions(*ctx_ptr);
	}

	auto& points = tfc.points;
	for(unsigned i = 0; i < points.size(); ++i) {
		if(member_ptr == &points[i].col) {
			context* ctx_ptr = get_context();
			if(ctx_ptr)
				update_transfer_function(*ctx_ptr);
			break;
		}
	}
	
	update_member(member_ptr);
	post_redraw();
}

bool transfer_function_editor::init(cgv::render::context& ctx) {
	
	fbc.ensure(ctx);
	shaders.load_shaders(ctx);

	shader_program& rect_prog = shaders.get("rectangle");
	rect_prog.enable(ctx);
	rect_prog.set_uniform(ctx, "tex", 0);
	rect_prog.set_uniform(ctx, "use_blending", false);
	rect_prog.set_uniform(ctx, "use_color", true);
	rect_prog.set_uniform(ctx, "apply_gamma", false);
	rect_prog.disable(ctx);

	shader_program& point_prog = shaders.get("ellipse");
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

	shader_program& line_prog = shaders.get("line");
	line_prog.enable(ctx);
	line_prog.set_uniform(ctx, "use_blending", false);
	line_prog.set_uniform(ctx, "apply_gamma", false);
	line_prog.set_uniform(ctx, "width", 2.0f);
	line_prog.disable(ctx);

	shader_program& hist_prog = shaders.get("histogram");
	hist_prog.enable(ctx);
	hist_prog.set_uniform(ctx, "color", rgba(1.0f, 1.0f, 1.0f, 0.5f));
	hist_prog.set_uniform(ctx, "border_color", rgba(0.0f, 0.0f, 0.0f, 1.0f));
	hist_prog.set_uniform(ctx, "border_width_in_pixel", 0u);
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

	if(!load_from_xml(file_name)) {
		tfc = tf_container();
	}
	init_transfer_function_texture(ctx);

	rgb a(0.6f);
	rgb b(0.8f);
	std::vector<rgb> bg_data = { a, b, b, a };
	
	bg_tex.destruct(ctx);
	cgv::data::data_view bg_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_FLT32, cgv::data::CF_RGB), bg_data.data());
	bg_tex = texture("flt32[R,G,B]", TF_NEAREST, TF_NEAREST, TW_REPEAT, TW_REPEAT);
	bg_tex.create(ctx, bg_dv, 0);

	return true;
}

void transfer_function_editor::init_frame(cgv::render::context& ctx) {

	vec2 viewport_size(ctx.get_width(), ctx.get_height());
	if(last_viewport_size != viewport_size || update_layout) {
		last_viewport_size = viewport_size;

		layout.update(viewport_size);

		fbc.set_size(layout.container_rect.size());
		fbc.ensure(ctx);

		vec2 container_size(layout.container_rect.size());

		shader_program& point_prog = shaders.get("ellipse");
		point_prog.enable(ctx);
		point_prog.set_uniform(ctx, "resolution", container_size);
		point_prog.disable(ctx);

		shader_program& poly_prog = shaders.get("polygon");
		poly_prog.enable(ctx);
		poly_prog.set_uniform(ctx, "resolution", container_size);
		poly_prog.disable(ctx);

		shader_program& line_prog = shaders.get("line");
		line_prog.enable(ctx);
		line_prog.set_uniform(ctx, "resolution", container_size);
		line_prog.disable(ctx);

		shader_program& hist_prog = shaders.get("histogram");
		hist_prog.enable(ctx);
		hist_prog.set_uniform(ctx, "resolution", container_size);
		hist_prog.set_uniform(ctx, "position", vec2(layout.padding));
		hist_prog.set_uniform(ctx, "size", vec2(layout.editor_rect.size()));
		hist_prog.disable(ctx);

		shader_program& bg_prog = shaders.get("background");
		bg_prog.enable(ctx);
		bg_prog.set_uniform(ctx, "resolution", container_size);
		bg_prog.set_uniform(ctx, "position", vec2(layout.editor_rect.pos()));
		bg_prog.set_uniform(ctx, "size", vec2(layout.editor_rect.size()));
		bg_prog.disable(ctx);

		update_all_transfer_functions(ctx);
		update_layout = false;
	}
}

void transfer_function_editor::draw(cgv::render::context& ctx) {

	if(!show)
		return;

	fbc.enable(ctx);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	vec2 container_size(layout.container_rect.size());

	// draw container border
	shader_program& rect_prog = shaders.get("rectangle");
	rect_prog.enable(ctx);
	rect_prog.set_uniform(ctx, "resolution", container_size);
	rect_prog.set_uniform(ctx, "use_color", true);
	rect_prog.set_uniform(ctx, "use_blending", false);
	rect_prog.set_uniform(ctx, "position", vec2(0.0f));
	rect_prog.set_uniform(ctx, "size", container_size);
	rect_prog.set_uniform(ctx, "color", vec4(0.0f, 0.0f, 0.0f, 1.0f));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// draw container background
	rect_prog.set_uniform(ctx, "position", vec2(1.0f));
	rect_prog.set_uniform(ctx, "size", container_size - 2.0f);
	rect_prog.set_uniform(ctx, "color", vec4(0.6f, 0.6f, 0.6f, 1.0f));
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// draw inner border
	rect_prog.set_uniform(ctx, "position", vec2(layout.padding - 1.0f));
	rect_prog.set_uniform(ctx, "size", vec2(layout.container_rect.size() - 2*layout.padding) + 2.0f);
	rect_prog.set_uniform(ctx, "color", vec4(0.48f, 0.48f, 0.48f, 1.0f));
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	rect_prog.disable(ctx);

	// draw editor checkerboard background
	shader_program& bg_prog = shaders.get("background");
	bg_prog.enable(ctx);
	bg_prog.set_uniform(ctx, "tex", 0);
	float width_factor = static_cast<float>(layout.editor_rect.size().x()) / static_cast<float>(layout.editor_rect.size().y());
	bg_prog.set_uniform(ctx, "tex_scaling", vec2(5.0f * width_factor, 5.0f));
	bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
	
	bg_tex.enable(ctx, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	bg_tex.disable(ctx);
	bg_prog.disable(ctx);

	// draw color scale texture
	rect_prog.enable(ctx);
	rect_prog.set_uniform(ctx, "use_color", false);
	rect_prog.set_uniform(ctx, "position", vec2(layout.color_scale_rect.pos()));
	rect_prog.set_uniform(ctx, "size", vec2(layout.color_scale_rect.size()));
	rect_prog.set_uniform(ctx, "tex_scaling", vec2(1.0f));
	rect_prog.set_uniform(ctx, "apply_gamma", false);

	tfc.tex.enable(ctx, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	tfc.tex.disable(ctx);

	rect_prog.disable(ctx);
	
	// draw histogranm texture
	if(tfc.hist_tex.is_created()) {
		shader_program& hist_prog = shaders.get("histogram");
		hist_prog.enable(ctx);
		hist_prog.set_uniform(ctx, "hist_tex", 0);
		hist_prog.set_uniform(ctx, "position", vec2(layout.editor_rect.pos()));
		hist_prog.set_uniform(ctx, "size", vec2(layout.editor_rect.size()));
		hist_prog.set_uniform(ctx, "max_value", tfc.hist_max);

		tfc.hist_tex.enable(ctx, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		tfc.hist_tex.disable(ctx);

		hist_prog.disable(ctx);
	}

	// draw transfer function area polygon
	if(tfc.vertex_array.is_created()) {
		shader_program& poly_prog = shaders.get("polygon");
		poly_prog.enable(ctx);

		tfc.vertex_array.enable(ctx);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)tfc.vertices.size());
		tfc.vertex_array.disable(ctx);

		poly_prog.disable(ctx);
	}

	// draw transfer function lines
	if(tfc.line_vertex_array.is_created()) {
		shader_program& line_prog = shaders.get("line");
		line_prog.enable(ctx);

		tfc.line_vertex_array.enable(ctx);
		glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)tfc.line_vertices.size());
		tfc.line_vertex_array.disable(ctx);

		line_prog.disable(ctx);
	}

	// draw control points
	shader_program& point_prog = shaders.get("ellipse");
	point_prog.enable(ctx);
	
	for(unsigned i = 0; i < tfc.points.size(); ++i) {
		const point& p = tfc.points[i];
		point_prog.set_uniform(ctx, "position", p.pos);
		point_prog.set_uniform(ctx, "size", vec2(2.0f * p.radius));
		point_prog.set_uniform(ctx, "color", rgba(0.0f, 0.0f, 0.0f, 1.0f));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		point_prog.set_uniform(ctx, "position", p.pos + 2.0f);
		point_prog.set_uniform(ctx, "size", vec2(2.0f * p.radius - 4.0f));
		point_prog.set_uniform(ctx, "color",
			selected_point == &p ? vec4(0.25f, 0.5f, 1.0f, 1.0f) : vec4(0.9f, 0.9f, 0.9f, 1.0f)
		);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	point_prog.disable(ctx);
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	fbc.disable(ctx);

	// draw frame buffer texture to screen
	rect_prog.enable(ctx);
	rect_prog.set_uniform(ctx, "resolution", last_viewport_size);
	rect_prog.set_uniform(ctx, "position", vec2(layout.margin));
	rect_prog.set_uniform(ctx, "size", container_size);
	rect_prog.set_uniform(ctx, "use_color", false);
	rect_prog.set_uniform(ctx, "use_blending", false);
	rect_prog.set_uniform(ctx, "apply_gamma", true);

	fbc.enable_attachment(ctx, "color", 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	fbc.disable_attachment(ctx, "color");

	rect_prog.disable(ctx);
}

void transfer_function_editor::create_gui() {

	add_decorator("Transfer Function Editor", "heading", "level=2");

	add_decorator("File", "heading", "level=3");
	std::string filter = "XML Files (xml):*.xml|All Files:*.*";
	add_gui("File", file_name, "file_name", "title='Open Transfer Function';filter='" + filter + "';save=false;w=136;small_icon=true;align_gui=' ';color=" + (has_unsaved_changes ? "0xff6666" : "0xffffff"));
	add_gui("save_file_name", save_file_name, "file_name", "title='Save Transfer Function';filter='" + filter + "';save=true;control=false;small_icon=true");

	add_decorator("Settings", "heading", "level=3");
	add_member_control(this, "Height", height, "value_slider", "min=100;max=500;step=10;ticks=true");
	add_member_control(this, "Opacity Scale Exponent", opacity_scale_exponent, "value_slider", "min=1.0;max=5.0;step=0.001;ticks=true");

	add_decorator("Control Points", "heading", "level=3");
	// TODO: add parameters for t and alpha?
	auto& points = tfc.points;
	for(unsigned i = 0; i < points.size(); ++i)
		add_member_control(this, "Color " + std::to_string(i), points[i].col, "", &points[i] == selected_point ? "label_color=0x4080ff" : "");
}

void transfer_function_editor::create_gui(cgv::gui::provider& p) {

	p.add_member_control(this, "Show", show, "check");
}

bool transfer_function_editor::is_hit(const ivec2& mouse_pos) {

	ivec2 test_pos = mouse_pos;
	test_pos.y() = last_viewport_size.y() - test_pos.y();
	test_pos -= layout.margin;
	return layout.is_hit(test_pos);
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

void transfer_function_editor::add_point(context& ctx, const vec2& pos) {

	point p;
	p.pos = pos - p.radius;
	p.update_val(layout, opacity_scale_exponent);
	p.col = tfc.tf.interpolate_color(p.val.x());
	tfc.points.push_back(p);

	update_transfer_function(ctx);
}

bool transfer_function_editor::remove_point(context& ctx, const point* ptr) {

	if(tfc.points.size() < 3)
		return false;

	bool removed = false;
	std::vector<point> next_points;
	for(unsigned i = 0; i < tfc.points.size(); ++i) {
		if(&tfc.points[i] != ptr)
			next_points.push_back(tfc.points[i]);
		else
			removed = true;
	}
	tfc.points = std::move(next_points);
	return removed;
}

transfer_function_editor::point* transfer_function_editor::get_hit_point(const transfer_function_editor::vec2& pos) {

	point* hit = nullptr;
	for(unsigned i = 0; i < tfc.points.size(); ++i) {
		point& p = tfc.points[i];
		if(p.is_hit(pos))
			hit = &p;
	}

	return hit;
}

void transfer_function_editor::init_transfer_function_texture(context& ctx) {

	unsigned size = 512;
	std::vector<uint8_t> data(size * 4, 0u);

	tf_tex.destruct(ctx);
	cgv::data::data_view tf_dv = cgv::data::data_view(new cgv::data::data_format(size, TI_UINT8, cgv::data::CF_RGBA), data.data());
	tf_tex = texture("uint8[R,G,B,A]", TF_LINEAR, TF_LINEAR);
	tf_tex.create(ctx, tf_dv, 0);
}

void transfer_function_editor::update_all_transfer_functions(context& ctx) {

	for(unsigned i = 0; i < tfc.points.size(); ++i)
		tfc.points[i].update_pos(layout, opacity_scale_exponent);

	update_transfer_function(ctx);
	
	has_unsaved_changes = false;
	on_set(&has_unsaved_changes);
}

void transfer_function_editor::update_transfer_function(context& ctx) {
	
	auto& tf = tfc.tf;
	auto& tex = tfc.tex;
	auto& points = tfc.points;
	auto& vertices = tfc.vertices;
	auto& line_vertices = tfc.line_vertices;

	tf.clear();

	for(unsigned i = 0; i < points.size(); ++i) {
		const point& p = points[i];
		tf.add_color_point(p.val.x(), p.col);

		//float opacity = std::pow(p.val.y(), 1.0f / opacity_scale_exponent);
		//opacity = cgv::math::clamp(opacity, 0.0f, 1.0f);
		tf.add_opacity_point(p.val.x(), p.val.y());
	}

	std::vector<rgba> tf_data;

	unsigned size = 512;
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

	vertices.clear();
	tfc.vb.destruct(ctx);
	tfc.vertex_array.destruct(ctx);

	line_vertices.clear();
	tfc.line_vb.destruct(ctx);
	tfc.line_vertex_array.destruct(ctx);

	// TODO: return success
	bool success = true;

	if(points.size() > 1) {
		int dragged_point_idx = -1;
		int selected_point_idx = -1;

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

		for(unsigned i = 0; i < sorted.size(); ++i) {
			points[i] = sorted[i].first;
			if(dragged_point_idx == sorted[i].second) {
				dragged_point = &points[i];
			}
			if(selected_point_idx == sorted[i].second) {
				selected_point = &points[i];
			}
		}

		std::vector<point>& sorted_points = points;

		const point& pl = sorted_points[0];
		rgba coll = tf.interpolate(pl.val.x());

		line_vertices.push_back({ vec2(layout.editor_rect.pos().x(), pl.center().y()) , rgb(coll) });

		vertices.push_back({ vec2(layout.editor_rect.pos().x(), pl.center().y()) , coll });
		vertices.push_back({ layout.editor_rect.pos(), coll });

		for(unsigned i = 0; i < sorted_points.size(); ++i) {
			vec2 pos = sorted_points[i].center();
			rgba col = tf.interpolate(sorted_points[i].val.x());

			line_vertices.push_back({ pos, rgb(col) });

			vertices.push_back({ pos, col });
			vertices.push_back({ vec2(pos.x(), layout.editor_rect.pos().y()), col });
		}

		const point& pr = sorted_points[sorted_points.size() - 1];
		rgba colr = tf.interpolate(pr.val.x());
		vec2 max_pos = layout.editor_rect.pos() + vec2(1.0f, 0.0f) * layout.editor_rect.size();
		
		line_vertices.push_back({ vec2(max_pos.x(), pr.center().y()) , rgb(colr) });

		vertices.push_back({ vec2(max_pos.x(), pr.center().y()) , colr });
		vertices.push_back({ max_pos, colr });

		type_descriptor vec2_type = cgv::render::element_descriptor_traits<cgv::render::render_types::vec2>::get_type_descriptor(vertices[0].pos);
		type_descriptor	rgba_type = cgv::render::element_descriptor_traits<cgv::render::render_types::rgba>::get_type_descriptor(vertices[0].col);
		type_descriptor	rgb_type = cgv::render::element_descriptor_traits<cgv::render::render_types::rgb>::get_type_descriptor(line_vertices[0].col);

		// TODO: replace if size is the same

		// create buffer objects
		if(tfc.vb.is_created())
			tfc.vb.destruct(ctx);
		if(tfc.vertex_array.is_created())
			tfc.vertex_array.destruct(ctx);

		shader_program& poly_prog = shaders.get("polygon");
		success &= tfc.vb.create(ctx, &(vertices[0]), vertices.size());
		success &= tfc.vertex_array.create(ctx);
		success &= tfc.vertex_array.set_attribute_array(ctx, poly_prog.get_position_index(), vec2_type, tfc.vb, 0, vertices.size(), sizeof(vertex));
		success &= tfc.vertex_array.set_attribute_array(ctx, poly_prog.get_color_index(), rgba_type, tfc.vb, sizeof(cgv::render::render_types::vec2), vertices.size(), sizeof(vertex));

		if(tfc.line_vb.is_created())
			tfc.line_vb.destruct(ctx);
		if(tfc.line_vertex_array.is_created())
			tfc.line_vertex_array.destruct(ctx);

		shader_program& line_prog = shaders.get("line");
		success &= tfc.line_vb.create(ctx, &(line_vertices[0]), line_vertices.size());
		success &= tfc.line_vertex_array.create(ctx);
		success &= tfc.line_vertex_array.set_attribute_array(ctx, line_prog.get_position_index(), vec2_type, tfc.line_vb, 0, line_vertices.size(), sizeof(vertex2));
		success &= tfc.line_vertex_array.set_attribute_array(ctx, line_prog.get_color_index(), rgb_type, tfc.line_vb, sizeof(cgv::render::render_types::vec2), line_vertices.size(), sizeof(vertex2));
	} else {
		success = false;
	}

	has_unsaved_changes = true;
	on_set(&has_unsaved_changes);
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
				tfc.points.push_back(p);
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

	const auto& points = tfc.points;

	//content += tab + "<TransferFunction>\n";
	//tab = "    ";

	for(unsigned j = 0; j < points.size(); ++j) {
		const point& p = points[j];

		content += tab + "<Point ";
		content += "position=\"" + std::to_string(p.val.x()) + "\" ";
		content += "r=\"" + std::to_string(to_col_uint8(p.col.R())) + "\" ";
		content += "g=\"" + std::to_string(to_col_uint8(p.col.G())) + "\" ";
		content += "b=\"" + std::to_string(to_col_uint8(p.col.B())) + "\" ";
		content += "opacity=\"" + std::to_string(p.val.y()) + "\"";
		content += "/>\n";
	}
	//tab = "  ";
	//content += tab + "</TransferFunction>\n";
	
	content += "</TransferFunction>\n";

	return cgv::utils::file::write(file_name, content, true);
}

}
}
