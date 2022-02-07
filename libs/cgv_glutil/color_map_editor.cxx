#include "color_map_editor.h"

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

color_map_editor::color_map_editor() {

	set_name("Color Scale Editor");
#ifdef CGV_FORCE_STATIC
	file_name = "";
#else
	file_name = QUOTE_SYMBOL_VALUE(INPUT_DIR);
	file_name += "/res/default.xml";
#endif

	layout.padding = 8;
	layout.total_height = 60;

	set_overlay_alignment(AO_START, AO_START);
	set_overlay_stretch(SO_HORIZONTAL);
	set_overlay_margin(ivec2(20));
	set_overlay_size(ivec2(600u, layout.total_height));
	
	fbc.add_attachment("color", "flt32[R,G,B,A]");
	fbc.set_size(get_overlay_size());

	canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);
	canvas.register_shader("circle", canvas::shaders_2d::circle);
	canvas.register_shader("histogram", "hist2d.glpr");
	canvas.register_shader("background", canvas::shaders_2d::background);

	overlay_canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);

	opacity_scale_exponent = 1.0f;
	resolution = (cgv::type::DummyEnum)256;

	mouse_is_on_overlay = false;
	show_cursor = false;
	cursor_pos = ivec2(-100);
	cursor_drawtext = "";

	cmc.points.set_drag_callback(std::bind(&color_map_editor::handle_drag, this));
	cmc.points.set_drag_end_callback(std::bind(&color_map_editor::handle_drag_end, this));
	cmc.points.set_constraint(layout.handles_rect);

	// TODO: make static members for these strings
	handle_renderer = generic_renderer(canvas::shaders_2d::arrow);
}

bool color_map_editor::on_exit_request() {
	// TODO: does not seem to fire when window is maximized
	if(has_unsaved_changes) {
		return cgv::gui::question("The transfer function has unsaved changes. Are you sure you want to quit?");
	}
	return true;
}

void color_map_editor::clear(cgv::render::context& ctx) {

	canvas.destruct(ctx);
	overlay_canvas.destruct(ctx);
	fbc.clear(ctx);
}

bool color_map_editor::self_reflect(cgv::reflect::reflection_handler& _rh) {

	return _rh.reflect_member("file_name", file_name);
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

		return cmc.points.handle(e, last_viewport_size, container);
	}
	return false;
}

void color_map_editor::on_set(void* member_ptr) {

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
		//if(!load_from_xml(file_name))
		//	cmc.reset();

		update_point_positions();
		update_color_map(false);

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
			init_texture(*ctx_ptr);
		update_color_map(false);
	}

	for(unsigned i = 0; i < cmc.points.size(); ++i) {
		if(member_ptr == &cmc.points[i].col) {
			update_color_map(true);
			break;
		}
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
	success &= handle_renderer.init(ctx);

	if(success)
		init_styles(ctx);

	//if(!load_from_xml(file_name))
	//	cmc.reset();
	
	init_texture(ctx);
	update_color_map(false);

	rgb a(0.75f);
	rgb b(0.9f);
	std::vector<rgb> bg_data = { a, b, b, a };
	
	bg_tex.destruct(ctx);
	cgv::data::data_view bg_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_FLT32, cgv::data::CF_RGB), bg_data.data());
	bg_tex = texture("flt32[R,G,B]", TF_NEAREST, TF_NEAREST, TW_CLAMP_TO_EDGE, TW_CLAMP_TO_EDGE);
	success &= bg_tex.create(ctx, bg_dv, 0);

	return success;
}

void color_map_editor::init_frame(cgv::render::context& ctx) {

	if(ensure_overlay_layout(ctx)) {
		ivec2 container_size = get_overlay_size();
		layout.update(container_size);

		fbc.set_size(container_size);
		fbc.ensure(ctx);

		canvas.set_resolution(ctx, container_size);
		overlay_canvas.set_resolution(ctx, get_viewport_size());

		auto& bg_prog = canvas.enable_shader(ctx, "background");
		//float width_factor = static_cast<float>(layout.handles_rect.size().x()) / static_cast<float>(layout.handles_rect.size().y());
		float width_factor = static_cast<float>(layout.color_map_rect.size().x()) / static_cast<float>(layout.color_map_rect.size().y());
		bg_style.texcoord_scaling = vec2(5.0f * width_factor, 5.0f);
		bg_style.apply(ctx, bg_prog);
		canvas.disable_current_shader(ctx);

		update_point_positions();
		sort_points();
		update_geometry();
		cmc.points.set_constraint(layout.handles_rect);
	}
}

void color_map_editor::draw(cgv::render::context& ctx) {

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
	canvas.draw_shape(ctx, ivec2(layout.padding - 1) + ivec2(0, 4), container_size - 2*layout.padding + 2 - ivec2(0, 4));
	
	if(cmc.cm) {
		// draw color scale texture
		color_map_style.apply(ctx, rect_prog);
		preview_tex.enable(ctx, 0);
		canvas.draw_shape(ctx, layout.color_map_rect.pos(), layout.color_map_rect.size());
		preview_tex.disable(ctx);
		canvas.disable_current_shader(ctx);
	} else {
		canvas.disable_current_shader(ctx);
		// draw editor checkerboard background
		auto& bg_prog = canvas.enable_shader(ctx, "background");
		bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
		bg_tex.enable(ctx, 0);
		//canvas.draw_shape(ctx, layout.handles_rect.pos(), layout.handles_rect.size());
		canvas.draw_shape(ctx, layout.color_map_rect.pos(), layout.color_map_rect.size());
		bg_tex.disable(ctx);
		canvas.disable_current_shader(ctx);
	}

	// draw separator line
	rect_prog = canvas.enable_shader(ctx, "rectangle");
	border_style.apply(ctx, rect_prog);
	canvas.draw_shape(ctx,
		ivec2(layout.color_map_rect.pos().x(), layout.color_map_rect.box.get_max_pnt().y()),
		ivec2(container_size.x() - 2 * layout.padding, 1)
	);
	canvas.disable_current_shader(ctx);

	// draw control points
	auto& handle_prog = handle_renderer.ref_prog();
	handle_prog.enable(ctx);
	canvas.set_view(ctx, handle_prog);
	handle_prog.disable(ctx);
	handle_renderer.render(ctx, PT_LINES, cmc.handles);

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

	add_decorator("Control Points", "heading", "level=3");
	// TODO: add controls for t?
	auto& points = cmc.points;
	for(unsigned i = 0; i < points.size(); ++i)
		add_member_control(this, "Color " + std::to_string(i), points[i].col, "", &points[i] == cmc.points.get_selected() ? "label_color=0x4080ff" : "");
}

void color_map_editor::create_gui(cgv::gui::provider& p) {

	p.add_member_control(this, "Show", show, "check");
}

//texture& color_map_editor::ref_tex() {
//
//	return cm_tex;
//}

void color_map_editor::init_styles(context& ctx) {

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
	color_map_style = border_style;
	color_map_style.use_texture = true;

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

	// configure style for control handles
	cgv::glutil::arrow2d_style handle_style;
	handle_style.use_blending = true;
	handle_style.apply_gamma = false;
	handle_style.use_fill_color = false;
	handle_style.position_is_center = true;
	handle_style.border_color = rgba(0.2f, 0.2f, 0.2f, 1.0f);
	handle_style.border_width = 1.5f;
	handle_style.border_radius = 2.0f;
	handle_style.stem_width = 12.0f;
	handle_style.head_width = 12.0f;

	auto& handle_prog = handle_renderer.ref_prog();
	handle_prog.enable(ctx);
	handle_style.apply(ctx, handle_prog);
	handle_prog.disable(ctx);

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

	std::vector<uint8_t> data(resolution * 3 * 2, 0u);

	preview_tex.destruct(ctx);
	cgv::data::data_view tf_dv = cgv::data::data_view(new cgv::data::data_format(resolution, 2, TI_UINT8, cgv::data::CF_RGB), data.data());
	preview_tex = texture("uint8[R,G,B]", TF_LINEAR, TF_LINEAR);
	preview_tex.create(ctx, tf_dv, 0);
}

void color_map_editor::add_point(const vec2& pos) {

	if(cmc.cm) {
		point p;
		p.pos = ivec2(pos.x(), layout.handles_rect.pos().y());
		p.update_val(layout, opacity_scale_exponent);
		p.col = cmc.cm->interpolate_color(p.val);
		cmc.points.add(p);

		update_color_map(true);
	}
}

void color_map_editor::remove_point(const point* ptr) {

	if(cmc.points.size() <= 2)
		return;

	bool removed = false;
	std::vector<point> next_points;
	for(unsigned i = 0; i < cmc.points.size(); ++i) {
		if(&cmc.points[i] != ptr)
			next_points.push_back(cmc.points[i]);
		else
			removed = true;
	}
	cmc.points.ref_draggables() = std::move(next_points);
	
	if(removed)
		update_color_map(true);
}

color_map_editor::point* color_map_editor::get_hit_point(const color_map_editor::vec2& pos) {

	point* hit = nullptr;
	for(unsigned i = 0; i < cmc.points.size(); ++i) {
		point& p = cmc.points[i];
		if(p.is_inside(pos))
			hit = &p;
	}

	return hit;
}

void color_map_editor::handle_drag() {

	cmc.points.get_dragged()->update_val(layout, opacity_scale_exponent);
	update_color_map(true);
	post_redraw();
}

void color_map_editor::handle_drag_end() {

	post_recreate_gui();
	post_redraw();
}

void color_map_editor::sort_points() {

	auto& points = cmc.points;

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

void color_map_editor::update_point_positions() {

	for(unsigned i = 0; i < cmc.points.size(); ++i)
		cmc.points[i].update_pos(layout, opacity_scale_exponent);
}

void color_map_editor::update_color_map(bool is_data_change) {
	
	context* ctx_ptr = get_context();
	if(!ctx_ptr || !cmc.cm) return;
	context& ctx = *ctx_ptr;

	auto& cm = *cmc.cm;
	//auto& tex = cmc.tex;
	auto& points = cmc.points;
	
	sort_points();

	cm.clear();

	for(unsigned i = 0; i < points.size(); ++i) {
		const point& p = points[i];
		cm.add_color_point(p.val, p.col);
	}

	size_t size = static_cast<size_t>(resolution);
	std::vector<rgb> cs_data = cm.interpolate_color(size);

	std::vector<uint8_t> data(3 * 2 * size);
	for(size_t i = 0; i < size; ++i) {
		rgb col = cs_data[i];
		uint8_t r = static_cast<uint8_t>(255.0f * col.R());
		uint8_t g = static_cast<uint8_t>(255.0f * col.G());
		uint8_t b = static_cast<uint8_t>(255.0f * col.B());

		unsigned idx = 3 * i;
		data[idx + 0] = r;
		data[idx + 1] = g;
		data[idx + 2] = b;
		idx += 3*size;
		data[idx + 0] = r;
		data[idx + 1] = g;
		data[idx + 2] = b;
	}

	preview_tex.destruct(ctx);
	cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(size, 2, TI_UINT8, cgv::data::CF_RGB), data.data());
	preview_tex = texture("uint8[R,G,B]", TF_LINEAR, TF_LINEAR, TW_CLAMP_TO_EDGE, TW_CLAMP_TO_EDGE);
	preview_tex.create(ctx, dv, 0);

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

	has_updated = true;
}

bool color_map_editor::update_geometry() {

	context* ctx_ptr = get_context();
	if(!ctx_ptr || !cmc.cm) return false;
	context& ctx = *ctx_ptr;

	auto& cm = *cmc.cm;
	auto& points = cmc.points;
	auto& handles = cmc.handles;

	handles.clear();
	
	bool success = points.size() > 0;

	for(unsigned i = 0; i < points.size(); ++i) {
		const auto& p = points[i];
		vec2 pos = p.center();
		rgb col = cm.interpolate(points[i].val);

		handles.add(pos + vec2(0.0f, 2.0f), cmc.points.get_selected() == &p ? rgba(0.5f, 0.5f, 0.5f, 1.0f) : rgba(0.9f, 0.9f, 0.9f, 1.0f));
		handles.add(pos + vec2(0.0f, 20.0f), cmc.points.get_selected() == &p ? rgba(0.5f, 0.5f, 0.5f, 1.0f) : rgba(0.9f, 0.9f, 0.9f, 1.0f));
	}

	handles.set_out_of_date();

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

bool color_map_editor::load_from_xml(const std::string& file_name) {

	if(!cgv::utils::file::exists(file_name) || cgv::utils::to_upper(cgv::utils::file::get_extension(file_name)) != "XML")
		return false;

	std::string content;
	cgv::utils::file::read(file_name, content, true);

	bool read = true;
	size_t nl_pos = content.find_first_of("\n");
	size_t line_offset = 0;
	bool first_line = true;

	int active_stain_idx = -1;

	cmc.points.clear();

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

		if(tokens.size() == 1 && to_string(tokens[0]) == "ColorScale") {
			// is a color scale
		}

		if(tokens.size() == 6 && to_string(tokens[0]) == "Point") {
			float pos = -1.0f;
			int r = -1;
			int g = -1;
			int b = -1;

			xml_attribute_to_float(to_string(tokens[1]), pos);
			xml_attribute_to_int(to_string(tokens[2]), r);
			xml_attribute_to_int(to_string(tokens[3]), g);
			xml_attribute_to_int(to_string(tokens[4]), b);
				
			if(!(pos < 0.0f)) {
				rgb col(0.0f);

				if(!(r < 0 || g < 0 || b < 0)) {
					col[0] = cgv::math::clamp(static_cast<float>(r / 255.0f), 0.0f, 1.0f);
					col[1] = cgv::math::clamp(static_cast<float>(g / 255.0f), 0.0f, 1.0f);
					col[2] = cgv::math::clamp(static_cast<float>(b / 255.0f), 0.0f, 1.0f);
				}

				point p;
				p.col = col;
				p.val = cgv::math::clamp(pos, 0.0f, 1.0f);
				cmc.points.add(p);
			}
		}
	}

	return true;
}

bool color_map_editor::save_to_xml(const std::string& file_name) {

	auto to_col_uint8 = [](const float& val) {
		int ival = cgv::math::clamp(static_cast<int>(255.0f * val + 0.5f), 0, 255);
		return static_cast<unsigned char>(ival);
	};

	std::string content = "";
	content += "<ColorScale>\n";
	std::string tab = "  ";

	for(unsigned i = 0; i < cmc.points.size(); ++i) {
		const point& p = cmc.points[i];

		content += tab + "<Point ";
		content += "position=\"" + std::to_string(p.val) + "\" ";
		content += "r=\"" + std::to_string(to_col_uint8(p.col.R())) + "\" ";
		content += "g=\"" + std::to_string(to_col_uint8(p.col.G())) + "\" ";
		content += "b=\"" + std::to_string(to_col_uint8(p.col.B())) + "\" ";
		content += "/>\n";
	}
	
	content += "</ColorScale>\n";

	return cgv::utils::file::write(file_name, content, true);
}

}
}
