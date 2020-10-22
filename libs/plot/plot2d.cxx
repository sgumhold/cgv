#include "plot2d.h"
#include <libs/cgv_gl/gl/gl.h>

namespace cgv {
	namespace plot {

/// overloaded in derived classes to compute complete tick render information
void plot2d::compute_tick_render_information()
{
	collect_tick_geometry(0, 1, &domain_min(0), &domain_max(0), &extent(0));
	collect_tick_geometry(1, 0, &domain_min(0), &domain_max(0), &extent(0));
}

void plot2d::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i)
{
	plot_base::set_uniforms(ctx, prog, i);
	if (i >= 0 && i < get_nr_sub_plots()) {
		prog.set_uniform(ctx, "line_color", ref_sub_plot2d_config(i).line_color);
		float w1 = ref_sub_plot2d_config(i).bar_percentual_width*(float)((domain_max(0)-domain_min(0)) / (ref_sub_plot_samples(i).size() - 1.0f));
		float w2 = w1;
		if (ref_sub_plot_strips(i).size() > 1)
			w2 *= ref_sub_plot_strips(i).size();
		prog.set_uniform(ctx, "bar_width", w2);
	}
}


/** extend common plot configuration with parameters specific to 1d plot */
plot2d_config::plot2d_config(const std::string& _name) : plot_base_config(_name)
{
	show_lines = true;
	line_width = 1;
	line_color = rgb(1,0.5f,0);
	configure_chart(CT_LINE_CHART);
};

/// configure the sub plot to a specific chart type
void plot2d_config::configure_chart(ChartType chart_type)
{
	plot_base_config::configure_chart(chart_type);
	show_lines = chart_type == CT_LINE_CHART;
}

///
void plot2d_config::set_colors(const rgb& base_color)
{
	plot_base_config::set_colors(base_color);
	line_color = 0.25f*rgb(1, 1, 1) + 0.75f*base_color;
}

/// construct empty plot with default domain [0..1,0..1]
plot2d::plot2d() : plot_base(2)
{
}

bool plot2d::compute_sample_coordinate_interval(int i, int ai, float& samples_min, float& samples_max)
{
	// compute bounding box
	bool found_sample = false;
	float min_value, max_value;
	for (unsigned j = 0; j < samples[i].size(); ++j) {
		if (found_sample) {
			min_value = std::min(min_value, samples[i][j](ai));
			max_value = std::max(max_value, samples[i][j](ai));
		}
		else {
			min_value = samples[i][j](ai);
			max_value = samples[i][j](ai);
			found_sample = true;
		}
	}
	if (found_sample) {
		samples_min = min_value;
		samples_max = max_value;
		return true;
	}
	return false;
}


unsigned plot2d::add_sub_plot(const std::string& name)
{
	// determine index of new sub plot
	unsigned i = get_nr_sub_plots();

	// create new config
	if (i == 0)
		configs.push_back(new plot2d_config(name));
	else {
		configs.push_back(new plot2d_config(ref_sub_plot2d_config(i - 1)));
		ref_sub_plot_config(i).name = name;
	}

	// create new point container
	samples.push_back(std::vector<plot2d::vec2>());
	strips.push_back(std::vector<unsigned>());
	attribute_sources.push_back(std::vector<attribute_source>());
	attribute_sources.back().push_back(attribute_source(i, 0, 0, 2 * sizeof(float)));
	attribute_sources.back().push_back(attribute_source(i, 1, 0, 2 * sizeof(float)));
	// return sub plot index
	return i;
}

void plot2d::delete_sub_plot(unsigned i)
{
	delete configs[i];
	configs[i] = 0;
	configs.erase(configs.begin() + i);
	samples.erase(samples.begin() + i);
	strips.erase(strips.begin() + i);
}

/// return a reference to the plot base configuration of the i-th plot
plot2d_config& plot2d::ref_sub_plot2d_config(unsigned i)
{
	return static_cast<plot2d_config&>(ref_sub_plot_config(i));
}

std::vector<plot2d::vec2>& plot2d::ref_sub_plot_samples(unsigned i)
{
	return samples[i];
}

/// return the strip definition of the i-th sub plot
std::vector<unsigned>& plot2d::ref_sub_plot_strips(unsigned i)
{
	return strips[i];
}

void plot2d::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot2d_config& pbc = ref_sub_plot2d_config(i);
	bool show = p.begin_tree_node("lines", pbc.show_lines, false, "level=3;options='w=142';align=' '");
	p.add_member_control(bp, "show", pbc.show_lines, "toggle", "w=50");
	if (show) {
		p.align("\a");
			p.add_member_control(bp, "width", pbc.line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "color", pbc.line_color);
		p.align("\b");
		p.end_tree_node(pbc.show_lines);
	}

	plot_base::create_config_gui(bp, p, i);
}

bool plot2d::init(cgv::render::context& ctx)
{
	if (!prog.build_program(ctx, "plot2d.glpr")) {
		std::cerr << "could not build GLSL program from plot2d.glpr" << std::endl;
		return false;
	}
	if (!stick_prog.build_program(ctx, "plot2d_stick.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_stick.glpr" << std::endl;
		return false;
	}
	if (!bar_prog.build_program(ctx, "plot2d_bar.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_bar.glpr" << std::endl;
		return false;
	}
	if (!bar_outline_prog.build_program(ctx, "plot2d_bar_outline.glpr")) {
		std::cerr << "could not build GLSL program from bar_outline_prog.glpr" << std::endl;
		return false;
	}
	return true;
}

void plot2d::clear(cgv::render::context& ctx)
{
	prog.destruct(ctx);
	stick_prog.destruct(ctx);
	bar_prog.destruct(ctx);
	bar_outline_prog.destruct(ctx);
}


void plot2d::draw_sub_plot(cgv::render::context& ctx, unsigned i)
{
	GLsizei count = (GLsizei)set_attributes(ctx, i, samples);
	if (count == 0)
		return;
	const plot2d_config& spc = ref_sub_plot2d_config(i);
	if (spc.show_bars) {
		set_uniforms(ctx, bar_prog, i);
		glDisable(GL_CULL_FACE);

		bar_prog.enable(ctx);
		set_default_attributes(ctx, bar_prog, 2);

		ctx.set_color(spc.bar_color);
		glDrawArrays(GL_POINTS, 0, count);
		bar_prog.disable(ctx);

		glEnable(GL_CULL_FACE);

		if (spc.bar_outline_width > 0) {

			glLineWidth(spc.bar_outline_width);
			set_uniforms(ctx, bar_outline_prog, i);

			bar_outline_prog.enable(ctx);
			set_default_attributes(ctx, bar_outline_prog, 2);

			ctx.set_color(spc.bar_outline_color);
			glDrawArrays(GL_POINTS, 0, count);
			bar_outline_prog.disable(ctx);
		}
	}

	if (spc.show_sticks) {
		set_uniforms(ctx, stick_prog, i);
		glLineWidth(spc.stick_width);
		stick_prog.enable(ctx);
		set_default_attributes(ctx, stick_prog, 2);
		ctx.set_color(spc.stick_color);
		glDrawArrays(GL_POINTS, 0, count);
		stick_prog.disable(ctx);
	}

	if (spc.show_points || spc.show_lines) {
		set_uniforms(ctx, prog, i);
		prog.enable(ctx);
		prog.set_uniform(ctx, "feature_offset", 0.001f * extent.length());
		set_default_attributes(ctx, prog, 2);
	}

	if (spc.show_lines) {
		ctx.set_color(spc.line_color);
		glLineWidth(spc.line_width);
		if (strips[i].empty())
			glDrawArrays(GL_LINE_STRIP, 0, count);
		else {
			unsigned fst = 0;
			for (unsigned j = 0; j < strips[i].size(); ++j) {
				glDrawArrays(GL_LINE_STRIP, fst, strips[i][j]);
				fst += strips[i][j];
			}
		}
	}

	if (spc.show_points) {
		ctx.set_color(spc.point_color);
		glPointSize(spc.point_size);
		glDrawArrays(GL_POINTS, 0, count);
	}

	if(spc.show_points || spc.show_lines) {
		prog.set_uniform(ctx, "feature_offset", 0.0f);
		prog.disable(ctx);
	}
}

void plot2d::draw_domain(cgv::render::context& ctx)
{
	std::vector<vec2> P;
	box2 domain(vec2(2, &domain_min(0)), vec2(2, &domain_max(0)));
	if (get_domain_config_ptr()->fill) {
		ctx.set_color(get_domain_config_ptr()->color);
		P.push_back(domain.get_corner(0));
		P.push_back(domain.get_corner(1));
		P.push_back(domain.get_corner(3));
		P.push_back(domain.get_corner(2));
		set_attributes(ctx, P);
		glDrawArrays(GL_QUADS, 0, 4);
		P.clear();
	}
	else {
		for (unsigned ai = 0; ai < 2; ++ai) {
			axis_config& ac = get_domain_config_ptr()->axis_configs[ai];
			ctx.set_color(get_domain_config_ptr()->color);
			glLineWidth(ac.line_width);
			P.push_back(domain.get_corner(0));
			P.push_back(domain.get_corner(1 + ai));
			P.push_back(domain.get_corner(2 - ai));
			P.push_back(domain.get_corner(3));
			set_attributes(ctx, P);
			glDrawArrays(GL_LINES, 0, 4);
			P.clear();
		}
	}
}

void plot2d::draw_axes(cgv::render::context& ctx)
{
	std::vector<vec2> P;
	for (unsigned ai = 0; ai < 2; ++ai) {
		unsigned aj = 1 - ai;
		axis_config& ac = get_domain_config_ptr()->axis_configs[ai];
		axis_config& ao = get_domain_config_ptr()->axis_configs[aj];
		ctx.set_color(ac.color);
		glLineWidth(ac.line_width);
		// min line
		vec2 p = vec2(2,&domain_min(0)); P.push_back(p);
		p(aj) = domain_max(aj); P.push_back(p);
		// max line
		p = vec2(2, &domain_max(0)); P.push_back(p);
		p(aj) = domain_min(aj); P.push_back(p);
		// axis line
		if (domain_min(ai) < 0 && domain_max(ai) > 0) {
			p(ai) = 0.0f;
			p(aj) = domain_min(aj); P.push_back(p);
			p(aj) = domain_max(aj); P.push_back(p);
		}
		if (!P.empty()) {
			set_attributes(ctx, P);
			glDrawArrays(GL_LINES, 0, GLsizei(P.size()));
			P.clear();
		}
	}
}

void plot2d::draw_ticks(cgv::render::context& ctx)
{
	if (tick_vertices.empty())
		return;
	set_attributes(ctx, tick_vertices);
	for (const auto& tbc : tick_batches) if (tbc.vertex_count > 0) {
		const axis_config& ac = get_domain_config_ptr()->axis_configs[tbc.ai];
		const tick_config& tc = tbc.primary ? ac.primary_ticks : ac.secondary_ticks;
		glLineWidth(tc.line_width);
		ctx.set_color(ac.color);
		glDrawArrays(GL_LINES, tbc.first_vertex, tbc.vertex_count);
	}
}

void plot2d::draw_tick_labels(cgv::render::context& ctx)
{
	if (tick_labels.empty())
		return;
	set_attributes(ctx, tick_vertices);
	for (const auto& tbc : tick_batches) if (tbc.label_count > 0) {
		ctx.set_color(get_domain_config_ptr()->axis_configs[tbc.ai].color);
		for (unsigned i = tbc.first_label; i < tbc.first_label + tbc.label_count; ++i) {
			const label_info& li = tick_labels[i];
			ctx.set_cursor(transform_to_world(li.position.to_vec()).to_vec(), li.label, li.align);
			ctx.output_stream() << li.label;
			ctx.output_stream().flush();
		}
	}
}

void plot2d::draw(cgv::render::context& ctx)
{
	GLboolean line_smooth = glIsEnabled(GL_LINE_SMOOTH); glEnable(GL_LINE_SMOOTH);
	GLboolean point_smooth = glIsEnabled(GL_POINT_SMOOTH); glEnable(GL_POINT_SMOOTH);
	GLboolean blend = glIsEnabled(GL_BLEND); glEnable(GL_BLEND);
	GLenum blend_src, blend_dst, depth;
	glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
	glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
	glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth));
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	enable_attributes(ctx, 2);
	set_uniforms(ctx, prog);
	prog.enable(ctx);
	set_default_attributes(ctx, prog, 2);
	if (get_domain_config_ptr()->show_domain)
		draw_domain(ctx);
	draw_axes(ctx);
	draw_ticks(ctx);
	prog.disable(ctx);
	disable_attributes(ctx, 2);

	ctx.enable_font_face(label_font_face, get_domain_config_ptr()->label_font_size);
	draw_tick_labels(ctx);

	enable_attributes(ctx, 2);
	for (unsigned i = 0; i<samples.size(); ++i) {
		// skip unvisible and empty sub plots
		if (!ref_sub_plot2d_config(i).show_plot)
			continue;
		draw_sub_plot(ctx, i);
	}
	disable_attributes(ctx, 2);

	if (!line_smooth)
		glDisable(GL_LINE_SMOOTH);
	if (!point_smooth)
		glDisable(GL_POINT_SMOOTH);
	if (!blend)
		glDisable(GL_BLEND);
	glDepthFunc(depth);
	glBlendFunc(blend_src, blend_dst);
}

	}
}