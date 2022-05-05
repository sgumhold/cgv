#include "color_map_legend.h"

//#include <cgv/math/ftransform.h>
//#include <cgv_gl/gl/gl.h>

#include <cgv/utils/convert_string.h>

#include <cgv/gui/theme_info.h>

namespace cgv {
namespace glutil {

color_map_legend::color_map_legend() {

	set_name("Color Map Legend");

	layout.padding = 13; // 10px plus 3px border
	//layout.band_height = 40;
	layout.total_height = 50;

	set_overlay_alignment(AO_START, AO_END);
	set_overlay_stretch(SO_NONE);
	set_overlay_margin(ivec2(-3));
	set_overlay_size(ivec2(300, layout.total_height));

	fbc.add_attachment("color", "flt32[R,G,B,A]");
	fbc.set_size(get_overlay_size());

	canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);
	//canvas.register_shader("color_maps", "color_maps.glpr");

	overlay_canvas.register_shader("rectangle", canvas::shaders_2d::rectangle);

	//texture_ptr = nullptr;

	tick_renderer = generic_renderer(canvas::shaders_2d::rectangle);

	//size = uvec2(140, 400);
	//padding = vec2(8, 16);

	// fbc.add_attachment("color", "uint8[R,G,B,A]");

	//shaders.add("rectangle", "rect2d.glpr");

	//lblr = label_renderer("Segoe UI", 16.0f, cgv::media::font::FFA_REGULAR);

	range = vec2(0.0f, 1.0f);
	num_ticks = 3;

	//show = true;
	//background = true;
	//transparent = true;
}

void color_map_legend::clear(cgv::render::context& ctx) {

	tex.clear();
	tex.destruct(ctx);

	msdf_font.destruct(ctx);
	font_renderer.destruct(ctx);

	tick_renderer.destruct(ctx);

	canvas.destruct(ctx);
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

	if(member_ptr == &layout.total_height) {// || member_ptr == &layout.band_height) {
		//update_layout = true;
		vec2 size = get_overlay_size();

		//int h = static_cast<int>(tex.get_height());
		////layout.total_height = 2 * layout.padding + h * layout.band_height;
		//layout.total_height = 2 * layout.padding + layout.band_height;
		
		layout.total_height = std::max(layout.total_height, 2 * layout.padding + 4 + layout.label_space);

		size.y() = layout.total_height;
		set_overlay_size(size);
	}

	if(member_ptr == &font_size) {
		labels.set_font_size(font_size);
	}

	update_member(member_ptr);
	post_redraw();
}

bool color_map_legend::init(cgv::render::context& ctx) {

	bool success = true;

	success &= fbc.ensure(ctx);
	success &= canvas.init(ctx);
	success &= overlay_canvas.init(ctx);

	success &= font_renderer.init(ctx);
	success &= tick_renderer.init(ctx);

	if(success)
		init_styles(ctx);
#ifndef CGV_FORCE_STATIC 
	if(msdf_font.init(ctx)) {
		labels.set_msdf_font(&msdf_font);
		labels.set_font_size(font_size);
	}
#endif
	return success;

	//shaders.load_shaders(ctx);
	//
	//fbc.set_size(size);
	//fbc.ensure(ctx);
	//
	//lblr.initialize();
	//lblr.enable_culling(false);

	create_ticks();

	return true;
}

void color_map_legend::init_frame(cgv::render::context& ctx) {

#ifdef CGV_FORCE_STATIC 
	if(!msdf_font.is_initialized()) {
		if(msdf_font.init(ctx)) {
			//texts.set_msdf_font(&msdf_font);
			labels.set_msdf_font(&msdf_font);
			//texts.set_font_size(font_size);
			labels.set_font_size(font_size);
		}
	}
#endif
	if(ensure_overlay_layout(ctx) || update_layout) {
		ivec2 container_size = get_overlay_size();
		layout.update(container_size);

		fbc.set_size(container_size);
		fbc.ensure(ctx);

		canvas.set_resolution(ctx, container_size);
		overlay_canvas.set_resolution(ctx, get_viewport_size());

		create_ticks();

		update_layout = false;
	}

	int theme_idx = cgv::gui::theme_info::instance().get_theme_idx();
	if(last_theme_idx != theme_idx) {
		last_theme_idx = theme_idx;
		init_styles(ctx);
	}

	//if(texts_out_of_date)
	//	update_texts();

	//fbc.ensure(ctx);
}

void color_map_legend::draw(cgv::render::context& ctx) {

	// TODO: draw to framebuffer when stuff changes and only draw fb texture in this method
	if(!show || !tex.is_created())
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
	canvas.draw_shape(ctx, layout.color_map_rect.pos() - 1, layout.color_map_rect.size() + 2);
	//canvas.disable_current_shader(ctx);

	// draw color scale texture
	//auto& color_maps_prog = canvas.enable_shader(ctx, "rectangle");
	color_map_style.apply(ctx, rect_prog);
	tex.enable(ctx, 0);
	canvas.draw_shape(ctx, layout.color_map_rect.pos(), layout.color_map_rect.size());
	tex.disable(ctx);
	canvas.disable_current_shader(ctx);

	// draw tick marks
	auto& tick_prog = tick_renderer.ref_prog();
	tick_prog.enable(ctx);
	canvas.set_view(ctx, tick_prog);
	tick_prog.disable(ctx);
	tick_renderer.render(ctx, PT_POINTS, ticks);

	// draw tick labels
	auto& font_prog = font_renderer.ref_prog();
	font_prog.enable(ctx);
	text_style.apply(ctx, font_prog);
	canvas.set_view(ctx, font_prog);
	font_prog.disable(ctx);
	font_renderer.render(ctx, get_overlay_size(), labels);

	//glDisable(GL_BLEND);

	fbc.disable(ctx);

	// draw frame buffer texture to screen
	auto& blit_prog = overlay_canvas.enable_shader(ctx, "rectangle");
	fbc.enable_attachment(ctx, "color", 0);
	overlay_canvas.draw_shape(ctx, get_overlay_position(), container_size);
	fbc.disable_attachment(ctx, "color");

	overlay_canvas.disable_current_shader(ctx);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);










	/*ivec2 fb_size = fbc.get_size();

	shader_program& sprite2d_prog = shaders.get("rectangle");
	sprite2d_prog.enable(ctx);
	sprite2d_prog.set_uniform(ctx, "resolution", fb_size);
	sprite2d_prog.set_uniform(ctx, "use_blending", false);
	sprite2d_prog.set_uniform(ctx, "apply_gamma", false);
	sprite2d_prog.set_uniform(ctx, "use_texture", false);
	glDisable(GL_DEPTH_TEST);

	fbc.enable(ctx);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if(background) {
		sprite2d_prog.set_uniform(ctx, "use_blending", transparent);
		sprite2d_prog.set_uniform(ctx, "fill_color", vec4(0.9f, 0.9f, 0.9f, transparent ? 0.85f : 1.0f));
		sprite2d_prog.set_attribute(ctx, "position", vec2(0.0f));
		sprite2d_prog.set_attribute(ctx, "size", vec2(size));

		glDrawArrays(GL_POINTS, 0, 1);
	}

	// draw color scale texture
	tex.enable(ctx, 0);

	sprite2d_prog.set_uniform(ctx, "use_texture", true);
	sprite2d_prog.set_uniform(ctx, "use_blending", false);
	sprite2d_prog.set_uniform(ctx, "border_color", rgba(0.0f));
	sprite2d_prog.set_uniform(ctx, "border_width", 1.0f);
	sprite2d_prog.set_uniform(ctx, "feather_width", 0.0f);

	sprite2d_prog.set_attribute(ctx, "position", vec2(-1.0f) + padding);
	sprite2d_prog.set_attribute(ctx, "size", vec2(42.0f, size.y() - 2.0f*padding.y() + 2.0f));

	glDrawArrays(GL_POINTS, 0, 1);

	tex.disable(ctx);

	// draw ticks
	sprite2d_prog.set_uniform(ctx, "use_texture", false);
	sprite2d_prog.set_uniform(ctx, "fill_color", vec4(0.0f));
	sprite2d_prog.set_uniform(ctx, "feather_width", 0.0f);
	sprite2d_prog.set_attribute(ctx, "size", vec2(15.0f, 1.0f));

	for(unsigned i = 0; i < ticks.size(); ++i) {
		sprite2d_prog.set_attribute(ctx, "position", vec2(padding.x() + 40.0f, fb_size.y() - ticks[i]));
		glDrawArrays(GL_POINTS, 0, 1);
	}

	sprite2d_prog.disable(ctx);

	ctx.push_modelview_matrix();
	ctx.set_modelview_matrix(cgv::math::look_at4<double>(dvec3(0, 0, 2), dvec3(0, 0, 0), dvec3(0, 1, 0)));

	ctx.push_projection_matrix();
	ctx.set_projection_matrix(cgv::math::ortho4<double>(0.0, fb_size.x(), 0.0, fb_size.y(), 1, 4));

	lblr.render(ctx, labels, TA_BOTTOM_LEFT, rgb(0.0f));

	ctx.pop_modelview_matrix();
	ctx.pop_projection_matrix();

	fbc.disable(ctx);

	vec2 legend_pos = vec2(20, (float)ctx.get_height() - (float)fb_size.y() - 20 - 48 - 10);

	sprite2d_prog.enable(ctx);
	sprite2d_prog.set_uniform(ctx, "resolution", ivec2(static_cast<int>(ctx.get_width()), static_cast<int>(ctx.get_height())));
	sprite2d_prog.set_uniform(ctx, "use_texture", true);
	sprite2d_prog.set_uniform(ctx, "use_blending", true);
	sprite2d_prog.set_uniform(ctx, "apply_gamma", true);
	sprite2d_prog.set_attribute(ctx, "position", legend_pos);
	sprite2d_prog.set_attribute(ctx, "size", vec2(static_cast<float>(fb_size.x()), static_cast<float>(fb_size.y())));
	sprite2d_prog.set_uniform(ctx, "border_color", rgba(0.02f, 0.02f, 0.02f, 1.0f));
	sprite2d_prog.set_uniform(ctx, "border_width", 2.0f);

	fbc.enable_attachment(ctx, "color", 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_POINTS, 0, 1);

	glDisable(GL_BLEND);

	fbc.disable_attachment(ctx, "color");

	sprite2d_prog.disable(ctx);
	glEnable(GL_DEPTH_TEST);
	*/
}

void color_map_legend::create_gui() {

	create_overlay_gui();
	//add_member_control(this, "Band Height", layout.band_height, "value_slider", "min=5;max=50;step=5;ticks=true");
	add_member_control(this, "Height", layout.total_height, "value_slider", "min=40;max=80;step=1;ticks=true");

	add_gui("", text_style);
	add_member_control(this, "Font Size", font_size, "value_slider", "min=10;max=40;step=1");

	//p.add_member_control(bp, "Background", background, "check");
	//p.add_member_control(bp, "Transparent Background", transparent, "check");
}

void color_map_legend::create_gui(cgv::gui::provider& p) {

	//p.add_member_control(this, "Show", show, "check");
}

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
}

void color_map_legend::set_range(vec2 range) {

	this->range = range;
	create_ticks();
}

void color_map_legend::set_num_ticks(unsigned n) {

	this->num_ticks = n < 3 ? 3 : n;
	create_ticks();
}

void color_map_legend::init_styles(context& ctx) {
	// get theme colors
	auto& ti = cgv::gui::theme_info::instance();
	rgba background_color = rgba(ti.background(), 1.0f);
	rgba group_color = rgba(ti.group(), 1.0f);
	rgba border_color = rgba(ti.border(), 1.0f);

	// configure style for the container rectangle
	container_style.apply_gamma = false;
	//container_style.fill_color = rgba(0.9f, 0.9f, 0.9f, 1.0f);
	//container_style.border_color = rgba(0.2f, 0.2f, 0.2f, 1.0f);
	container_style.fill_color = group_color;
	container_style.border_color = background_color;
	container_style.border_width = 3.0f;
	container_style.feather_width = 0.0f;

	// configure style for the border rectangle
	border_style = container_style;
	//border_style.fill_color = border_color;
	//border_style.border_width = 0.0f;
	border_style.fill_color = rgba(rgb(0.0f), 1.0);
	//border_style.border_color = border_color;
	border_style.border_width = 0.0f;

	// configure style for the color scale rectangle
	color_map_style = border_style;
	color_map_style.use_texture = true;
	color_map_style.use_texture_alpha = false;
	
	// configure text style
	text_style.fill_color = rgba(rgb(0.0f), 1.0f);
	text_style.border_color = rgba(0.0f);// group_color;
	text_style.border_width = 0.5f;
	text_style.feather_origin = 0.5f; // 0.35f
	text_style.use_blending = true;

	shape2d_style tick_style;
	tick_style.position_is_center = true;
	tick_style.fill_color = rgba(rgb(0.0f), 1.0f);
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

	int width = layout.color_map_rect.size().x();

	float step = static_cast<float>(width + 1) / static_cast<float>(num_ticks - 1);

	int y_tick = layout.padding + 0.75f*layout.label_space;
	int y_label = layout.padding + 0.5f*layout.label_space;

	for(size_t i = 0; i < num_ticks; ++i) {
		float fi = static_cast<float>(i);
		int x = layout.padding + layout.label_space + static_cast<int>(round(fi * step));
		ticks.add(ivec2(x, y_tick), ivec2(1, 0.5f*layout.label_space));

		float t = fi / static_cast<float>(num_ticks - 1);
		float val = cgv::math::lerp(range.x(), range.y(), t);

		std::string str = cgv::utils::to_string(val);
		labels.add_text(str, ivec2(x, y_label), cgv::render::TextAlignment::TA_TOP);
	}



	/*texts.clear();
	if(names.size() == 0)
		return;

	int step = layout.color_map_rect.size().y() / names.size();
	ivec2 base = layout.color_map_rect.pos() + ivec2(layout.color_map_rect.size().x() / 2, step / 2 - static_cast<int>(0.333f*font_size));
	int i = 0;
	for(const auto& name : names) {
		ivec2 p = base;
		p.y() += (names.size() - 1 - i)*step;
		texts.add_text(name, p, TA_BOTTOM);
		++i;
	}
	texts_out_of_date = false;
	*/

	/*ticks.clear();
	labels.clear();

	float extent = range.y() - range.x();
	float step = extent / static_cast<float>(num_ticks - 1);

	unsigned pixels = size.y() - 2 * padding.y() + 1;

	float padding_scale = 2.0f*static_cast<float>(padding.y()) / static_cast<float>(size.y());

	float world_size = 2.0f - 2.0f*padding_scale;

	float world_min = -1.0f + padding_scale;
	float world_max = 1.0f - 1.5f*padding_scale;

	for(unsigned i = 0; i < num_ticks; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(num_ticks - 1);
		//unsigned pos = static_cast<unsigned>(round(padding.y() + t * pixels));
		unsigned pos = static_cast<unsigned>(padding.y() + t * pixels);
		ticks.push_back(pos);

		float world_y = cgv::math::lerp(world_min, world_max, t);

		float val = cgv::math::lerp(range.x(), range.y(), t);
		//labels.push_back(label_renderer::label_info(vec3(0.0f, world_y, 0.0f), util::ftos(val, 2)));
		labels.push_back(label_renderer::label_info(vec3(padding.x() + 65.0f, pos - 4.0f, 0.0f), util::ftos(val, 2)));
	}*/
}

}
}
