#include "performance_monitor.h"

#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>

namespace cgv {
namespace app {

performance_monitor::performance_monitor() {

	set_name("Performance Monitor");
	gui_options.allow_stretch = false;

	layout.padding = padding();
	layout.total_size = ivec2(180, 90);

	set_size(layout.total_size);

	bar_renderer = cgv::g2d::generic_2d_renderer(cgv::g2d::shaders::rectangle);
}

void performance_monitor::clear(cgv::render::context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);

	canvas_overlay::clear(ctx);

	bar_renderer.destruct(ctx);
	bars.destruct(ctx);
	texts.destruct(ctx);
	labels.destruct(ctx);
}

void performance_monitor::handle_member_change(const cgv::utils::pointer_test& m) {

	if(m.is(show_plot)) {
		layout.total_size.y() = show_plot ? 90 : 55;
		set_size(layout.total_size);
	}

	if(m.one_of(background_visible_, invert_color))
		init_styles();

	if(m.is(monitor.enabled)) {
		if(monitor.enabled)
			monitor.reset();
	}
}

bool performance_monitor::init(cgv::render::context& ctx) {

	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);

	register_shader("rectangle", cgv::g2d::shaders::rectangle);
	register_shader("line", cgv::g2d::shaders::line);

	bool success = canvas_overlay::init(ctx);

	success &= bar_renderer.init(ctx);
	success &= texts.init(ctx);
	success &= labels.init(ctx);

	plot_color_map.add_color_point(0.0f, rgb(0.5f, 1.0f, 0.5f));
	plot_color_map.add_color_point(0.25f, rgb(0.0f, 0.9f, 0.0f));
	plot_color_map.add_color_point(0.5f, rgb(0.8f, 0.9f, 0.0f));
	plot_color_map.add_color_point(1.0f, rgb(0.9f, 0.0f, 0.0f));

	return success;
}

void performance_monitor::init_frame(cgv::render::context& ctx) {

	if(ensure_layout(ctx)) {
		layout.update(get_rectangle().size);
		create_texts();
		create_labels();
	}

	bool enabled = monitor.enabled;
	if(monitor.enabled_only_when_visible && !is_visible()) {
		enabled = false;
	}
	if(enabled) {
		if(show_plot)
			update_plot();
		update_stats_texts();
	}
}

void performance_monitor::draw_content(cgv::render::context& ctx) {

	begin_content(ctx);
	auto& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx);

	if(show_plot) {
		// draw plot border
		content_canvas.enable_shader(ctx, "rectangle");
		content_canvas.set_style(ctx, border_style);
		content_canvas.draw_shape(ctx, layout.plot_rect.position - 1, layout.plot_rect.size + 2);

		// draw plot bars
		bar_renderer.render(ctx, content_canvas, cgv::render::PT_POINTS, bars, bar_style);

		// draw line
		const auto& r = layout.plot_rect;
		ivec2 a(r.x() + 12, r.center().y());
		ivec2 b = a;
		b.x() = r.x1();

		content_canvas.enable_shader(ctx, "line");
		content_canvas.set_style(ctx, line_style);
		content_canvas.draw_shape2(ctx, a, b);
		content_canvas.disable_current_shader(ctx);

		// draw labels
		font_renderer.render(ctx, content_canvas, labels, label_style);
	}

	// draw text
	font_renderer.render(ctx, content_canvas, texts, text_style);

	end_content(ctx);
}

void performance_monitor::after_finish(cgv::render::context& ctx) {

	themed_canvas_overlay::after_finish(ctx);

	bool enabled = monitor.enabled;
	if(monitor.enabled_only_when_visible && !is_visible())
		enabled = false;
	
	if(enabled) {
		++monitor.total_frame_count;
		++monitor.interval_frame_count;
		
		double seconds_since_start = monitor.timer.get_elapsed_time();
		monitor.delta_time = seconds_since_start - monitor.last_seconds_since_start;
		
		monitor.running_time += monitor.delta_time;

		monitor.last_seconds_since_start = seconds_since_start;

		if(monitor.running_time >= monitor.interval) {
			monitor.avg_fps = (double)monitor.interval_frame_count / monitor.running_time;
			monitor.running_time = 0.0;
			monitor.interval_frame_count = 0u;
		}
	}
}

void performance_monitor::set_invert_color(bool flag) {

	invert_color = flag;
	on_set(&invert_color);
}

void performance_monitor::enable_monitoring(bool enabled) {
	monitor.enabled = enabled;
	on_set(&monitor.enabled);
}

void performance_monitor::enable_monitoring_only_when_visible(bool enabled) {
	monitor.enabled_only_when_visible = enabled;
}

void performance_monitor::on_visibility_change() {

	if(monitor.enabled_only_when_visible && is_visible()) {
		if(monitor.enabled)
			monitor.reset();
	}
}

void performance_monitor::create_gui_impl() {

	add_member_control(this, "Enable", monitor.enabled, "check", "w=110", " ");
	add_member_control(this, "Show Plot", show_plot, "check", "w=78");
	add_member_control(this, "Measure Interval (s)", monitor.interval, "value_slider", "min=0.01;max=1;step=0.01;ticks=true");
	
	add_member_control(this, "Background", background_visible_, "check", "w=100", " ");
	add_member_control(this, "Invert Color", invert_color, "check", "w=88");
}

void performance_monitor::init_styles() {
	auto& theme = cgv::gui::theme_info::instance();
	rgb border_color = theme.text();

	if(invert_color)
		border_color = pow(rgb(1.0f) - pow(border_color, 2.2f), 1.0f / 2.2f);

	// configure style for the border rectangle
	border_style.fill_color = background_visible_ ? rgba(theme.text_background(), 1.0f) : rgba(0.0f);
	border_style.border_color = rgba(border_color, 1.0);
	border_style.border_width = 1.0f;
	border_style.feather_width = 0.0f;
	border_style.use_blending = true;

	line_style.use_blending = true;
	line_style.fill_color = rgba(border_color, invert_color ? 0.666f : 0.333f);
	line_style.feather_width = 0.0f;
	line_style.dash_length = 10.0f;
	
	bar_style.use_fill_color = false;
	bar_style.feather_width = 0.0f;

	// configure text style
	text_style.fill_color = border_color;
	text_style.font_size = 12.0f;

	label_style = text_style;
	label_style.font_size = 10.0f;
}

void performance_monitor::create_texts() {

	texts.clear();

	const int line_spacing = static_cast<int>(1.25f* text_style.font_size);

	ivec2 caret_pos = ivec2(layout.content_rect.x(), layout.content_rect.y1() - (int)text_style.font_size);
	texts.add_text("Frames per Second:", caret_pos, cgv::render::TA_BOTTOM_LEFT);
	caret_pos.y() -= line_spacing;
	texts.add_text("Frametime (ms):", caret_pos, cgv::render::TA_BOTTOM_LEFT);
	
	caret_pos = ivec2(layout.content_rect.x1(), layout.content_rect.y1() - (int)text_style.font_size);
	texts.add_text("", caret_pos, cgv::render::TA_BOTTOM_RIGHT);
	caret_pos.y() -= line_spacing;
	texts.add_text("", caret_pos, cgv::render::TA_BOTTOM_RIGHT);
}

void performance_monitor::update_stats_texts() {

	if(texts.size() > 3) {
		std::stringstream ss;
		ss.precision(2);
		ss << std::fixed;
		ss << monitor.avg_fps;

		std::string str = ss.str();
		texts.set_text(2, ss.str());

		ss.str(std::string());
		if(monitor.avg_fps < 0.001f)
			ss << "-";
		else
			ss << 1000.0 / monitor.avg_fps;

		texts.set_text(3, ss.str());

		post_damage();
	}
}

void performance_monitor::create_labels() {
	
	labels.clear();

	ivec2 caret_pos = ivec2(layout.plot_rect.x(), layout.plot_rect.y1());
	labels.add_text("30", caret_pos, cgv::render::TA_TOP_LEFT);
	caret_pos.y() = layout.plot_rect.center().y();
	labels.add_text("60", caret_pos, cgv::render::TA_LEFT);
	caret_pos.y() = layout.plot_rect.y();
	labels.add_text("120", caret_pos, cgv::render::TA_BOTTOM_LEFT);
}

void performance_monitor::update_plot() {

	ivec2 plot_size = layout.plot_rect.size;

	float a = static_cast<float>(1000.0 * monitor.delta_time / 33.333333333);
	float b = std::min(a, 1.0f);
	float bar_height = plot_size.y() * b;
	bar_height = std::max(bar_height, 1.0f);

	rgb bar_color = a > 1.0f ? rgb(0.7f, 0.0f, 0.0f) : plot_color_map.interpolate_color(b);

	if(bars.render_count() < plot_size.x()) {
		for(auto& position : bars.position)
			position.x() -= 1.0f;

		int x = layout.plot_rect.x1() - 1;
		x = std::max(x, 0);
		float bar_x = static_cast<float>(x);
		bars.add(vec2(bar_x, static_cast<float>(layout.plot_rect.y())), vec2(1.0f, bar_height), bar_color);

	} else {
		for(size_t i = 0; i < bars.position.size() - 1; ++i) {
			bars.size[i].y() = bars.size[i + 1].y();
			bars.color[i] = bars.color[i + 1];
		}

		bars.size.back() = vec2(1.0f, bar_height);
		bars.color.back() = bar_color;
	}

	bars.set_out_of_date();
	post_damage();
}

}
}
