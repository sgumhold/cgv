#include "transfer_function_editor.h"

#include <algorithm>
#include <numeric>

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_g2d/msdf_font.h>
#include <cgv_g2d/msdf_gl_font_renderer.h>

namespace cgv {
namespace app {

const vec2 transfer_function_editor::color_point_size = { 12.0f, 18.0f };
const vec2 transfer_function_editor::opacity_point_size = { 12.0f };

transfer_function_editor::transfer_function_editor() {
	set_name("Color Scale Editor");
	blocks_events(true);

	resolution = (cgv::type::DummyEnum)256;
	opacity_scale_exponent = 1.0f;
	supports_opacity = false;
	use_interpolation = true;
	use_linear_filtering = true;
	range = vec2(0.0f, 1.0f);

	layout.total_height = supports_opacity ? 200 : 60;
	
	set_size(ivec2(600u, layout.total_height));
	
	color_draggables.set_constraint(layout.color_draggables_rect);
	color_draggables.callback = std::bind(&transfer_function_editor::handle_drag, this, std::placeholders::_1, DraggableType::kColor);

	opacity_draggables.set_constraint(layout.opacity_editor_rect);
	opacity_draggables.callback = std::bind(&transfer_function_editor::handle_drag, this, std::placeholders::_1, DraggableType::kOpacity);

	color_handle_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::arrow);
	opacity_handle_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::rectangle);
	line_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::line);
	polygon_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::polygon);
}

void transfer_function_editor::clear(cgv::render::context& ctx) {
	cgv::g2d::ref_msdf_font_regular(ctx, -1);
	cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx, -1);

	canvas_overlay::clear(ctx);

	color_handle_renderer.destruct(ctx);
	opacity_handle_renderer.destruct(ctx);
	line_renderer.destruct(ctx);
	polygon_renderer.destruct(ctx);

	background_tex.destruct(ctx);
	preview_tex.destruct(ctx);
	histogram_tex.destruct(ctx);
}

//bool transfer_function_editor::handle_key_event(cgv::gui::key_event& e) {
//	return false;
//}

bool transfer_function_editor::handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 local_mouse_pos) {
	bool capture_event = false;
	
	//bool request_clear_selection = false;

	if(e.get_action() == cgv::gui::MA_PRESS) {
		if(e.get_button() & cgv::gui::MB_LEFT_BUTTON) {
			//request_clear_selection = is_hit_local(local_mouse_pos);

			if(!get_hit_point(local_mouse_pos)) {
				add_point(local_mouse_pos);
				//request_clear_selection = false;
			}
			capture_event = true;
		} else if(e.get_button() & cgv::gui::MB_RIGHT_BUTTON) {
			cgv::g2d::draggable* hit_point = get_hit_point(local_mouse_pos);
			if(hit_point)
				erase_point(hit_point);
		}
	}

	if(color_draggables.handle(e, get_viewport_size(), get_rectangle()))
		return true;
	if(opacity_draggables.handle(e, get_viewport_size(), get_rectangle()))
		return true;
	/*
	if(request_clear_selection) {
		selected_color_draggable = nullptr;
		selected_opacity_draggable = nullptr;
		handle_selection_change();
	}*/
	if (capture_event)
		return true;
	return false;
}

void transfer_function_editor::handle_member_change(const cgv::utils::pointer_test& m) {
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
		//if(cmc.cm)
		//	cmc.cm->set_resolution(resolution);
		//update_color_map(false);
	}

	if(m.is(use_interpolation)) {
		cgv::render::context* ctx_ptr = get_context();
		if(transfer_function)
			//cmc.cm->enable_interpolation(use_interpolation);
			transfer_function->use_interpolation = use_interpolation;
		//update_color_map(false);
	}

	/*
	if(m.is(use_linear_filtering)) {
		cgv::render::context* ctx_ptr = get_context();
		if(cmc.cm) {
			cgv::render::gl_color_map* gl_cm = cmc.get_gl_color_map();
			if(gl_cm)
				gl_cm->enable_linear_filtering(use_linear_filtering);
		}
		update_color_map(false);
	}
	*/

	for(const auto& draggable : color_draggables) {
		if(m.is(draggable.data)) {
			update_transfer_function_from_data();
			create_preview_texture();
			update_geometry();
			post_damage();
			break;
		}
	}

	for(auto& draggable : opacity_draggables) {
		if(m.is(draggable.uv.y())) {
			draggable.set_uv_and_update_position(draggable.uv);
			update_transfer_function_from_data();
			update_geometry();
			post_damage();
			break;
		}
	}
	
	if(m.is(supports_opacity)) {
		// Todo: Fixme: This will overwrite the user-defined total height.
		layout.total_height = supports_opacity ? 200 : 60;
		on_set(&layout.total_height);

		if(!supports_opacity) {
			if(transfer_function) {
				// Todo: Clear opacity points or just disable editing them?
				transfer_function->clear_opacity_points();
				opacity_draggables.clear();

				update_point_positions();
				//update_color_map(false);
			}
		}

		post_recreate_layout();
		post_recreate_gui();
	}
}

bool transfer_function_editor::init(cgv::render::context& ctx) {
	cgv::g2d::ref_msdf_font_regular(ctx, 1);
	cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx, 1);
	
	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("circle", cgv::g2d::shaders::circle);
	register_shader("histogram", "heightfield1d.glpr");
	//Todo: Rename shader program?
	register_shader("background", "color_map_editor_bg.glpr");

	bool success = canvas_overlay::init(ctx);

	success &= color_handle_renderer.init(ctx);
	success &= opacity_handle_renderer.init(ctx);
	success &= line_renderer.init(ctx);
	success &= polygon_renderer.init(ctx);
	success &= create_background_texture();

	//update_color_map(false);

	return success;
}

void transfer_function_editor::init_frame(cgv::render::context& ctx) {
	if(ensure_layout(ctx)) {
		ivec2 container_size = get_rectangle().size;

		update_layout(container_size);
		update_data_from_transfer_function();

		auto& bg_prog = content_canvas.enable_shader(ctx, "background");
		float width_factor = static_cast<float>(layout.opacity_editor_rect.w()) / static_cast<float>(layout.opacity_editor_rect.h());
		bg_style.texcoord_scaling = vec2(5.0f * width_factor, 5.0f);
		bg_style.apply(ctx, bg_prog);
		content_canvas.disable_current_shader(ctx);

		//update_point_positions();
		////sort_points();
		//update_geometry();
		
		color_draggables.set_constraint(layout.color_draggables_rect);
		opacity_draggables.set_constraint(layout.opacity_editor_rect);
	}

	//update_data_from_transfer_function();
}

void transfer_function_editor::draw_content(cgv::render::context& ctx) {
	begin_content(ctx);
	
	// draw inner border
	ivec2 container_size = get_rectangle().size;
	content_canvas.enable_shader(ctx, "rectangle");
	content_canvas.set_style(ctx, border_style);
	content_canvas.draw_shape(ctx, ivec2(padding() - 1) + ivec2(0, 10), container_size - 2 * padding() + 2 - ivec2(0, 10));
	
	if(transfer_function && preview_tex.is_created()) {
		// draw color scale texture
		content_canvas.set_style(ctx, color_map_style);
		preview_tex.enable(ctx, 0);
		content_canvas.draw_shape(ctx, layout.color_editor_rect);
		preview_tex.disable(ctx);

		if(supports_opacity) {
			// draw opacity editor checkerboard background
			auto& bg_prog = content_canvas.enable_shader(ctx, "background");
			bg_style.apply(ctx, bg_prog);
			bg_prog.set_uniform(ctx, "scale_exponent", opacity_scale_exponent);
			background_tex.enable(ctx, 0);
			content_canvas.draw_shape(ctx, layout.opacity_editor_rect);
			background_tex.disable(ctx);
			content_canvas.disable_current_shader(ctx);

			// draw histogram
			if(histogram_type != HistogramType::kNone && histogram_tex.is_created()) {
				auto& hist_prog = content_canvas.enable_shader(ctx, "histogram");
				hist_prog.set_uniform(ctx, "max_value", hist_norm_ignore_zero ? hist_max_non_zero : hist_max);
				hist_prog.set_uniform(ctx, "norm_gamma", hist_norm_gamma);
				hist_prog.set_uniform(ctx, "sampling_type", cgv::math::clamp(static_cast<unsigned>(histogram_type) - 1, 0u, 2u));
				hist_style.apply(ctx, hist_prog);

				histogram_tex.enable(ctx, 1);
				content_canvas.draw_shape(ctx, layout.opacity_editor_rect);
				histogram_tex.disable(ctx);
				content_canvas.disable_current_shader(ctx);
			}

			preview_tex.enable(ctx, 0);
			// draw transfer function area polygon
			polygon_renderer.render(ctx, content_canvas, cgv::render::PT_TRIANGLE_STRIP, triangle_geometry, polygon_style);

			// draw transfer function lines
			line_renderer.render(ctx, content_canvas, cgv::render::PT_LINE_STRIP, line_geometry, line_style);
			preview_tex.disable(ctx);

			// draw separator line
			content_canvas.enable_shader(ctx, "rectangle");
			content_canvas.set_style(ctx, border_style);
			content_canvas.draw_shape(ctx,
				ivec2(layout.color_editor_rect.x(), layout.color_editor_rect.y1()),
				ivec2(container_size.x() - 2 * padding(), 1)
			);
			content_canvas.disable_current_shader(ctx);
		}

		// draw control points
		// color handles
		color_handle_renderer.render(ctx, content_canvas, cgv::render::PT_LINES, color_draggables_geometry, color_handle_style);

		// opacity handles
		if(supports_opacity) {
			auto& opacity_handle_prog = opacity_handle_renderer.enable_prog(ctx);
			opacity_handle_prog.set_attribute(ctx, "size", opacity_point_size); // size is constant for all points
			opacity_handle_renderer.render(ctx, content_canvas, cgv::render::PT_POINTS, opacity_draggables_geometry, opacity_handle_style);
		}
	} else {
		content_canvas.disable_current_shader(ctx);
	}

	auto& font = cgv::g2d::ref_msdf_font_regular(ctx);
	auto& font_renderer = cgv::g2d::ref_msdf_gl_font_renderer_2d(ctx);
	cgv::g2d::msdf_gl_font_renderer::text_render_info text_render_info;

	if(!value_label.empty()) {
		cgv::g2d::irect rectangle = value_label_rectangle;
		rectangle.translate(0, 5);
		rectangle.size += ivec2(10, 6);

		content_canvas.enable_shader(ctx, "rectangle");
		content_canvas.set_style(ctx, label_box_style);
		content_canvas.draw_shape(ctx, rectangle);
		content_canvas.disable_current_shader(ctx);

		text_render_info.alignment = cgv::render::TextAlignment::TA_BOTTOM;
		font_renderer.render(ctx, content_canvas, font, value_label, value_label_rectangle.position, text_render_info, value_label_style);
	}

	end_content(ctx);
}

void transfer_function_editor::handle_theme_change(const cgv::gui::theme_info& theme) {
	themed_canvas_overlay::handle_theme_change(theme);
	update_geometry();
	post_recreate_gui();
}

void transfer_function_editor::create_gui_impl() {
	if(begin_tree_node("Settings", layout, false)) {
		align("\a");
		std::string height_options = "min=";
		height_options += supports_opacity ? "80" : "40";
		height_options += ";max=500;step=10;ticks=true";
		add_member_control(this, "Height", layout.total_height, "value_slider", height_options);
		add_member_control(this, "Opacity Scale Exponent", opacity_scale_exponent, "value_slider", "min=1.0;max=5.0;step=0.001;ticks=true");
		add_member_control(this, "Resolution", resolution, "dropdown", "enums='2=2,4=4,8=8,16=16,32=32,64=64,128=128,256=256,512=512,1024=1024,2048=2048';w=106", " ");
		add_member_control(this, "Interpolate", use_interpolation, "check", "w=82");
		
		//if(cmc.cm && cmc.cm->has_texture_support()) {
		//	add_member_control(this, "Linear Filtering (Texture)", use_linear_filtering, "check");
		//}
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

	if(begin_tree_node("Color Points", color_draggables, true)) {
		align("\a");
		auto& points = color_draggables;
		for(unsigned i = 0; i < points.size(); ++i) {
			std::string label_prefix = "";
			std::string options = "w=48";
			if(&points[i] == selected_color_draggable) {
				label_prefix = "> ";
				options += ";label_color=" + cgv::media::to_hex(highlight_color);
			}

			add_view(label_prefix + std::to_string(i), points[i].uv.x(), "", options, " ");
			add_member_control(this, "", points[i].data, "", "w=140");
		}
		align("\b");
		end_tree_node(color_draggables);
	}

	if(supports_opacity) {
		if(begin_tree_node("Opacity Points", opacity_draggables, true)) {
			align("\a");
			auto& points = opacity_draggables;
			for(unsigned i = 0; i < points.size(); ++i) {
				std::string label_prefix = "";
				std::string options = "w=48";
				if(&points[i] == selected_opacity_draggable) {
					label_prefix = "> ";
					options += ";label_color=" + cgv::media::to_hex(highlight_color);
				}

				add_view(label_prefix + std::to_string(i), points[i].uv.x(), "", options, " ");
				add_member_control(this, "", points[i].uv.y(), "value", "w=140");
			}
			align("\b");
			end_tree_node(opacity_draggables);
		}
	}
}

void transfer_function_editor::set_opacity_support(bool flag) {
	supports_opacity = flag;
	//set_transfer_function(transfer_function);
	on_set(&supports_opacity);
}

void transfer_function_editor::set_transfer_function(std::shared_ptr<cgv::media::transfer_function> transfer_function) {
	//if(this->transfer_function != transfer_function) {
	//	this->transfer_function = transfer_function;
	//	build_time.reset();
	//}

	this->transfer_function = transfer_function;
	update_data_from_transfer_function();
	post_damage();
	
	/*
	if(cmc.cm) {
		const std::vector<std::pair<float, cgv::rgb>>& cp = cmc.cm->get_color_draggables();
		
		for(size_t i = 0; i < cp.size(); ++i) {
			color_point p;
			p.val = cgv::math::clamp(cp[i].first, 0.0f, 1.0f);
			p.col = cp[i].second;
			cmc.color_draggables.add(p);
		}

		if(supports_opacity) {
			const std::vector<std::pair<float, float>>& ap = cmc.cm->get_opacity_draggables();
			for(size_t i = 0; i < ap.size(); ++i) {
				opacity_point p;
				p.val.x() = cgv::math::clamp(ap[i].first, 0.0f, 1.0f);
				p.val.y() = cgv::math::clamp(ap[i].second, 0.0f, 1.0f);
				cmc.opacity_draggables.add(p);
			}
		}

		use_interpolation = cmc.cm->use_interpolation;
		//use_linear_filtering = true;
		
		//cgv::render::gl_color_map* gl_cm = cmc.get_gl_color_map();
		//if(gl_cm)
		//	use_linear_filtering = gl_cm->is_linear_filtering_enabled();

		update_point_positions();
		update_color_map(false);

		post_recreate_gui();
	}
	*/
}

void transfer_function_editor::set_histogram_data(const std::vector<unsigned> data) {
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

	if(get_context()) {
		cgv::data::data_view data_view = cgv::data::data_view(new cgv::data::data_format(unsigned(histogram.size()), cgv::type::info::TI_FLT32, cgv::data::CF_R), float_data.data());
		histogram_tex.create(*get_context(), data_view, 0);
		post_damage();
	}
}

void transfer_function_editor::set_selected_color(rgb color) {
	if(selected_color_draggable) {
		selected_color_draggable->data = color;
		on_set(&selected_color_draggable->data);
		post_damage();
	}
}

void transfer_function_editor::init_styles() {
	auto& theme = cgv::gui::theme_info::instance();
	// Todo: Check if alpha is set to 1.
	handle_color = theme.text();
	highlight_color = theme.highlight();

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
	color_handle_style.stem_width = color_point_size.x();
	color_handle_style.head_width = color_point_size.x();

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

void transfer_function_editor::update_layout(const ivec2& parent_size) {
	int content_height = layout.total_height - 10 - 2 * padding();
	if(supports_opacity) {
		layout.color_editor_height = static_cast<int>(floor(0.15f * static_cast<float>(content_height)));
		layout.color_editor_height = cgv::math::clamp(layout.color_editor_height, 4, 80);
		layout.opacity_editor_height = content_height - layout.color_editor_height - 1;
	} else {
		layout.color_editor_height = content_height;
		layout.opacity_editor_height = 0;
	}

	int y_off = padding();

	layout.color_draggables_rect = {
		ivec2(padding(), 16),
		ivec2(parent_size.x() - 2 * padding(), 0)
	};

	// move 10px up to clear some space for the color handles rect
	y_off += 10;

	layout.color_editor_rect.position = ivec2(padding(), y_off);
	layout.color_editor_rect.size = ivec2(parent_size.x() - 2 * padding(), layout.color_editor_height);

	y_off += layout.color_editor_height + 1; // plus 1px border

	layout.opacity_editor_rect.position = ivec2(padding(), y_off);
	layout.opacity_editor_rect.size = ivec2(parent_size.x() - 2 * padding(), layout.opacity_editor_height);
}

bool transfer_function_editor::create_preview_texture() {
	const size_t size = static_cast<size_t>(resolution);
	std::vector<rgba> cs_data = transfer_function->quantize(size);

	std::vector<cgv::rgba8> texture_data;
	texture_data.reserve(resolution);
	std::transform(cs_data.begin(), cs_data.end(), std::back_inserter(texture_data), [](const cgv::rgba& color) {
		return cgv::rgba8(color);
	});

	cgv::data::data_format data_format(resolution, 1, cgv::type::info::TI_UINT8, cgv::data::CF_RGBA);
	cgv::data::data_view data_view(&data_format, texture_data.data());

	const cgv::render::TextureFilter filter = cgv::render::TF_LINEAR;

	if(get_context()) {
		preview_tex.set_min_filter(filter);
		preview_tex.set_mag_filter(filter);
		return preview_tex.create(*get_context(), data_view, 0);
	}
	return false;
}

bool transfer_function_editor::create_background_texture() {
	const rgb dark(0.75f);
	const rgb light(0.9f);
	std::vector<rgb> data = { dark, light, light, dark };
	cgv::data::data_format format(2, 2, cgv::type::info::TI_FLT32, cgv::data::CF_RGB);
	cgv::data::data_view data_view = cgv::data::data_view(&format, data.data());
	if(get_context())
		return background_tex.create(*get_context(), data_view, 0);
	return false;
}

void transfer_function_editor::add_point(const vec2& pos) {
	if(!transfer_function)
		return;

	ivec2 test_pos = static_cast<ivec2>(pos);

	selected_color_draggable = nullptr;
	selected_opacity_draggable = nullptr;

	if(layout.color_editor_rect.contains(test_pos)) {
		color_point draggable = make_color_point();
		draggable.set_position_and_update_uv({ pos.x(), 0.0f });
		draggable.data = transfer_function->get_mapped_color(draggable.uv.x());
		size_t index = color_draggables.add(draggable);
		// Todo: Allow drag after add.
		//color_draggables.set_dragged(index);
		selected_color_draggable = &color_draggables[index];
	} else if(supports_opacity && layout.opacity_editor_rect.contains(test_pos)) {
		opacity_point draggable = make_opacity_point();
		draggable.set_position_and_update_uv(pos);
		size_t index = opacity_draggables.add(draggable);
		selected_opacity_draggable = &opacity_draggables[index];
	}

	sort_points();
	handle_selection_change();
	update_geometry();
	update_transfer_function_from_data();
	post_damage();
	// Todo: notify change
	//update_color_map(true);
}

void transfer_function_editor::erase_point(const cgv::g2d::draggable* point) {
	const auto& try_erase_point_from = [point](auto& draggables) {
		if(draggables.size() > 1) {
			auto it = std::remove_if(draggables.begin(), draggables.end(), [point](const cgv::g2d::draggable& draggable) {
				return &draggable == point;
			});
			if(it != draggables.end()) {
				draggables.ref_draggables().erase(it);
				return true;
			}
		}
		return false;
	};

	if(try_erase_point_from(color_draggables) || try_erase_point_from(opacity_draggables)) {
		selected_color_draggable = nullptr;
		selected_opacity_draggable = nullptr;
		handle_selection_change();
		update_geometry();
		post_damage();
		
		// Todo:
		// update tf and notify
	}
}

cgv::g2d::draggable* transfer_function_editor::get_hit_point(const vec2& pos) {
	const auto& contains = [&pos](const auto& draggable) {
		return draggable.contains(pos);
	};

	auto color_it = std::find_if(color_draggables.begin(), color_draggables.end(), contains);
	if(color_it != color_draggables.end())
		return &*color_it;

	auto opacity_it = std::find_if(opacity_draggables.begin(), opacity_draggables.end(), contains);
	if(opacity_it != opacity_draggables.end())
		return &*opacity_it;
	
	return nullptr;
}

void transfer_function_editor::set_value_label(vec2 position, const std::string& text) {
	value_label = text;
	value_label_rectangle.position = position;

	if(get_context()) {
		auto& font = cgv::g2d::ref_msdf_font_regular(*get_context());
		value_label_rectangle.size = font.compute_render_size(value_label, value_label_style.font_size);
	}

	cgv::g2d::rect constraint = get_content_rectangle();
	constraint.scale(-0.5f * (value_label_rectangle.size + 4.0f));
	value_label_rectangle.position = cgv::math::clamp(value_label_rectangle.position, constraint.a(), constraint.b());
}

void transfer_function_editor::handle_drag(cgv::g2d::DragAction action, DraggableType type) {
	bool modified = false;

	switch(action) {
	case cgv::g2d::DragAction::kDragStart:
	case cgv::g2d::DragAction::kDrag:
		if(type == DraggableType::kColor) {
			color_point* dragged = color_draggables.get_dragged();
			if(dragged) {
				dragged->set_position_and_update_uv(dragged->position);
				set_value_label(
					dragged->position + cgv::vec2(0.0f, 25.0f),
					value_to_string(dragged->uv.x())
				);
				selected_color_draggable = dragged;
				selected_opacity_draggable = nullptr;
				modified = true;
			}
		} else if(type == DraggableType::kOpacity) {
			opacity_point* dragged = opacity_draggables.get_dragged();
			if(dragged) {
				dragged->set_position_and_update_uv(dragged->position);

				set_value_label(
					dragged->position + cgv::vec2(0.0f, 10.0f),
					// Todo: Use value mapping and then convert to string.
					value_to_string(dragged->uv.x()) + ", " + cgv::utils::to_string(dragged->uv.y(), -1, 3u)
				);
				selected_opacity_draggable = dragged;
				selected_color_draggable = nullptr;
				modified = true;
			}
		}
		if(modified)
			handle_selection_change();
		break;
	case cgv::g2d::DragAction::kDragEnd:
		handle_selection_change();
		value_label.clear();
		modified = true;
		break;
	default:
		break;
	}

	// Todo: somehow deselect points
	//press_inside = has_constraint && !use_individual_constraints ? constraint_area.contains(mouse_position) : true;

	if(modified) {
		sort_points();
		update_geometry();
		create_preview_texture();
		update_transfer_function_from_data();
		post_damage();
	}
}

void transfer_function_editor::handle_selection_change() {
	if(selected_color_draggable) {
		if(on_color_point_select_callback)
			on_color_point_select_callback(selected_color_draggable->data);
	} else {
		if(on_color_point_deselect_callback)
			on_color_point_deselect_callback();
	}
	post_recreate_gui();
}

std::string transfer_function_editor::value_to_string(float value) {
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

// Todo: Move to utils algorithm
template<typename T>
std::vector<size_t> sort_indices(const std::vector<T>& v) {
	// Initialize original index locations.
	std::vector<size_t> indices(v.size());
	std::iota(indices.begin(), indices.end(), 0);

	// Sort indices based on comparing values in v.
	std::sort(indices.begin(), indices.end(), [&v](size_t i1, size_t i2) {
		return v[i1] < v[i2];
	});

	return indices;
}

template<typename T>
void sort_draggables(cgv::g2d::draggable_collection<T>& draggables, T*& selected) {
	if(draggables.empty())
		return;

	int dragged_idx = -1;
	int selected_idx = -1;

	for(size_t i = 0; i < draggables.size(); ++i) {
		if(&draggables[i] == draggables.get_dragged())
			dragged_idx = static_cast<int>(i);
		if(&draggables[i] == selected)
			selected_idx = static_cast<int>(i);
	}

	std::vector<T> draggables_copy = draggables.ref_draggables();
	std::vector<size_t> permutation = sort_indices(draggables_copy);

	for(size_t i = 0; i < permutation.size(); ++i) {
		int permuted_index = static_cast<int>(permutation[i]);
		draggables[i] = draggables_copy[permuted_index];
		if(permuted_index == dragged_idx)
			draggables.set_dragged(i);
		if(permuted_index == selected_idx)
			selected = &draggables[i];
	}
}


void transfer_function_editor::sort_points() {
	sort_draggables(color_draggables, selected_color_draggable);
	if(supports_opacity)
		sort_draggables(opacity_draggables, selected_opacity_draggable);
}

void transfer_function_editor::update_point_positions() {
	for(auto& draggable : color_draggables)
		draggable.set_position_and_update_uv(draggable.position);
	
	for(auto& draggable : opacity_draggables)
		draggable.set_position_and_update_uv(draggable.position);
}

/*
void transfer_function_editor::update_color_map(bool is_data_change) {
	cgv::render::context* ctx_ptr = get_context();
	if(!ctx_ptr || !transfer_function) return;
	cgv::render::context& ctx = *ctx_ptr;

	//sort_points();

	//cm.clear();

	std::vector<std::pair<float, rgb>> colors;
	std::vector<std::pair<float, float>> alphas;
	colors.reserve(color_draggables.size());
	alphas.reserve(opacity_draggables.size());

	std::transform(color_draggables.begin(), color_draggables.end(), std::back_inserter(colors), [](const color_point& point) { return std::make_pair(point.val, point.col); });
	std::transform(opacity_draggables.begin(), opacity_draggables.end(), std::back_inserter(alphas), [](const opacity_point& point) { return std::make_pair(point.val[0], point.val[1]); });

	transfer_function->set_color_draggables(colors);
	transfer_function->set_opacity_draggables(alphas);

	create_preview_texture();
	
	update_geometry();

	if(on_change_callback)
		on_change_callback();

	post_damage();
}
*/

void transfer_function_editor::update_geometry() {
	color_draggables_geometry.clear();
	opacity_draggables_geometry.clear();
	line_geometry.clear();
	triangle_geometry.clear();

	vec2 pos_offset = vec2(0.0f, 0.5f * color_point_size.y());

	for(const auto& draggable : color_draggables) {
		vec2 pos = draggable.center();
		rgba col = selected_color_draggable == &draggable ? highlight_color : handle_color;
		color_draggables_geometry.add(pos - pos_offset, col);
		color_draggables_geometry.add(pos + pos_offset, col);
	}

	if(!opacity_draggables.empty()) {
		const auto& first = *opacity_draggables.begin();

		vec2 tex_coord(0.0f, 0.5f);

		line_geometry.add(vec2(static_cast<float>(layout.opacity_editor_rect.x()), first.center().y()), tex_coord);
		
		triangle_geometry.add(vec2(static_cast<float>(layout.opacity_editor_rect.x()), first.center().y()), tex_coord);
		triangle_geometry.add(layout.opacity_editor_rect.position, tex_coord);

		for(const auto& draggable : opacity_draggables) {
			vec2 pos = draggable.center();
			rgba col = selected_opacity_draggable == &draggable ? highlight_color : handle_color;
			opacity_draggables_geometry.add(pos, col);

			tex_coord.x() = draggable.uv.x();

			line_geometry.add(pos, tex_coord);

			triangle_geometry.add(pos, tex_coord);
			triangle_geometry.add(vec2(pos.x(), static_cast<float>(layout.opacity_editor_rect.y())), tex_coord);
		}

		const auto& last = *(--opacity_draggables.end());
		vec2 max_pos = layout.opacity_editor_rect.position + vec2(1.0f, 0.0f) * layout.opacity_editor_rect.size;

		tex_coord.x() = 1.0f;

		line_geometry.add(vec2(max_pos.x(), last.center().y()), tex_coord);

		triangle_geometry.add(vec2(max_pos.x(), last.center().y()), tex_coord);
		triangle_geometry.add(max_pos, tex_coord);
	}
}

}
}
