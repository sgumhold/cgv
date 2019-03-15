#include "plot2d.h"
#include <libs/cgv_gl/gl/gl.h>
#include <cgv/render/attribute_array_binding.h>

namespace cgv {
	namespace plot {

/// overloaded in derived classes to compute complete tick render information
void plot2d::compute_tick_render_information()
{
	collect_tick_geometry(0, 1, &domain.get_min_pnt()(0), &domain.get_max_pnt()(0), &extent(0));
	collect_tick_geometry(1, 0, &domain.get_min_pnt()(0), &domain.get_max_pnt()(0), &extent(0));
}

void plot2d::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i)
{
	prog.set_uniform(ctx, "x_axis", axis_directions[0]);
	prog.set_uniform(ctx, "y_axis", axis_directions[1]);
	prog.set_uniform(ctx, "extent", extent);
	prog.set_uniform(ctx, "domain_min_pnt", domain.get_min_pnt());
	prog.set_uniform(ctx, "domain_max_pnt", domain.get_max_pnt());
	prog.set_uniform(ctx, "center_location", center_location);

	prog.set_uniform(ctx, "x_axis_log_scale", get_domain_config_ptr()->axis_configs[0].log_scale);
	prog.set_uniform(ctx, "y_axis_log_scale", get_domain_config_ptr()->axis_configs[0].log_scale);

	if (i >= 0 && i < get_nr_sub_plots()) {
		plot_base::set_uniforms(ctx, prog, i);
		prog.set_uniform(ctx, "line_color", ref_sub_plot2d_config(i).line_color);
		float w1 = ref_sub_plot2d_config(i).bar_percentual_width*(float)(domain.get_extent()(0) / (ref_sub_plot_samples(i).size() - 1.0f));
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

/// construct empty plot with default domain [0..1,0..1]
plot2d::plot2d() : plot_base(2)
{
	domain.ref_min_pnt() = vec2(0.0f,0.0f);
	domain.ref_max_pnt() = vec2(1.0f,1.0f);
	extent               = vec2(1.0f,1.0f);
	axis_directions[0]   = vec3(1.0f,0.0f,0.0f);
	axis_directions[1]   = vec3(0.0f,1.0f,0.0f);
	center_location      = vec3(0.0f,0.0f,0.0f);
}

/// query the plot extend in 2D coordinates
const plot2d::vec2& plot2d::get_extent() const
{
	return extent;
}

/// set the plot extend in 2D coordinates
void plot2d::set_extent(const vec2& new_extent)
{
	extent = new_extent;
}

/// set the plot width to given value and if constrained == true the height, such that the aspect ration is the same as the aspect ratio of the domain
void plot2d::set_width(float new_width, bool constrained)
{
	extent(0) = new_width;
	if (constrained) {
		vec2 e = domain.get_extent();
		extent(1) = new_width * e(1) / e(0);
	}
}

/// set the plot height to given value and if constrained == true the width, such that the aspect ration is the same as the aspect ratio of the domain
void plot2d::set_height(float new_height, bool constrained)
{
	extent(1) = new_height;
	if (constrained) {
		vec2 e = domain.get_extent();
		extent(0) = new_height * e(0) / e(1);
	}
}

/// set the direction of x or y axis
void plot2d::set_axis_direction(unsigned ai, const vec3& new_axis_direction)
{
	axis_directions[ai] = new_axis_direction;
}

/// place the origin of the plot in 3D to the given location
void plot2d::place_origin(const vec3& new_origin_location)
{
	center_location += new_origin_location - get_origin();
}

/// place the plot extent center in 3D to the given location (this might can change the current origin location) 
void plot2d::place_center(const vec3& new_center_location)
{
	center_location = new_center_location;
}

/// place a corner (0 .. lower left, 1 .. lower right, 2 .. upper left, 3 .. upper right) to a given 3D location ((this might can change the current origin / center location) 
void plot2d::place_corner(unsigned corner_index, const vec3& new_corner_location)
{
	center_location += new_corner_location - get_corner(corner_index);
}

/// return the current origin in 3D coordinates
plot2d::vec3 plot2d::get_origin() const
{
	vec2 delta = -domain.get_center();
	delta /= domain.get_extent();
	delta *= extent;
	return get_center() + delta(0) * get_axis_direction(0) + delta(1) * get_axis_direction(1);
}

/// return the current plot center in 3D coordinates
const plot2d::vec3& plot2d::get_center() const
{
	return center_location;
}

/// return the i-th plot corner in 3D coordinates
plot2d::vec3 plot2d::get_corner(unsigned i) const
{
	vec2 delta = domain.get_corner(i) - domain.get_center();
	delta /= domain.get_extent();
	delta *= extent;
	return get_center() + delta(0) * get_axis_direction(0) + delta(1) * get_axis_direction(1);
}

/// set the direction of x or y axis
const plot2d::vec3& plot2d::get_axis_direction(unsigned ai) const
{
	return axis_directions[ai];
}

/// adjust the domain with respect to \c ai th axis to the data
void plot2d::adjust_domain_axis_to_data(unsigned ai, bool adjust_min, bool adjust_max)
{
	if (samples.empty())
		return;
	bool found_samples = false;

	for (unsigned i = 0; i<samples.size(); ++i) {
		if (samples[i].empty())
			continue;
		if (!found_samples) {
			found_samples = true;
			// compute bounding box
			if (adjust_min)
				domain.ref_min_pnt()(ai) = samples[i].front()(ai);
			if (adjust_max)
				domain.ref_max_pnt()(ai) = samples[i].front()(ai);

		}
		for (unsigned j = 0; j<samples[i].size(); ++j) {
			if (adjust_min) {
				if (samples[i][j](ai) < domain.ref_min_pnt()(ai))
					domain.ref_min_pnt()(ai) = samples[i][j](ai);
			}
			if (adjust_max) {
				if (samples[i][j](ai) > domain.ref_max_pnt()(ai))
					domain.ref_max_pnt()(ai) = samples[i][j](ai);
			}
		}
	}
	if (found_samples) {
		if (domain.ref_min_pnt()(ai) == domain.ref_max_pnt()(ai))
			domain.ref_max_pnt()(ai) += 1;
	}
}

/// adjust the domain with respect to \c ai th axis to the data
void plot2d::adjust_domain_axis_to_visible_data(unsigned ai, bool adjust_min, bool adjust_max)
{
	// compute bounding box
	bool found_sample = false;
	float min_value, max_value;
	for (unsigned i = 0; i<samples.size(); ++i) {
		if (ref_sub_plot2d_config(i).show_plot) {
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
		}
	}
	if (adjust_min)
		domain.ref_min_pnt()(ai) = found_sample?min_value:0;
	if (adjust_max)
		domain.ref_max_pnt()(ai) = found_sample ? max_value:1;
	if (domain.ref_min_pnt()(ai) == domain.ref_max_pnt()(ai))
		domain.ref_max_pnt()(ai) += 1;
}

/// adjust tick marks
void plot2d::adjust_tick_marks_to_domain(unsigned max_nr_primary_ticks)
{
	vec2 de = domain.get_extent();

	for (int ai=0; ai<2; ++ai) {
		if (get_domain_config_ptr()->axis_configs[ai].log_scale)
			de(ai) = (log(domain.get_max_pnt()(ai)) - log(domain.get_min_pnt()(ai))) / log(10.0f);

		float step = de(ai) / max_nr_primary_ticks;
		float step2;
		float scale = (float)pow(10,-floor(log10(step)));
		if (scale * step < 1.5f) {
			step = 1.0f/scale;
			step2 = 5.0f/scale;
		}
		else if (scale * step < 3.5f) {
			step = 2.0f/scale;
			step2 = 10.0f/scale;
		}
		else if (scale * step < 7.5f) {
			step = 5.0f/scale;
			step2 = 20.0f/scale;
		}
		else {
			step = 10.0f/scale;
			step2 = 50.0f/scale;
		}
		get_domain_config_ptr()->axis_configs[ai].primary_ticks.step = step2;
		get_domain_config_ptr()->axis_configs[ai].secondary_ticks.step = step;
	}
}

/// extend domain such that given axis is included
void plot2d::include_axis_to_domain(unsigned ai)
{
	if (domain.get_min_pnt()(1-ai) > 0)
		domain.ref_min_pnt()(1-ai) = 0;
	if (domain.get_max_pnt()(1-ai) < 0)
		domain.ref_max_pnt()(1-ai) = 0;
}

/// adjust all axes of domain to data
void plot2d::adjust_domain_to_data(bool adjust_x_axis, bool adjust_y_axis)
{
	if (adjust_x_axis)
		adjust_domain_axis_to_data(0);
	if (adjust_y_axis)
		adjust_domain_axis_to_data(1);
}

/// adjust all axes of domain to data
void plot2d::adjust_domain_to_visible_data(bool adjust_x_axis, bool adjust_y_axis)
{
	if (adjust_x_axis)
		adjust_domain_axis_to_visible_data(0);
	if (adjust_y_axis)
		adjust_domain_axis_to_visible_data(1);
}


/// set the colors for all plot features as variation of the given color
void plot2d::set_sub_plot_colors(unsigned i, const rgb& base_color)
{
	ref_sub_plot2d_config(i).line_color        = 0.25f*rgb(1,1,1)+0.75f*base_color;
	ref_sub_plot2d_config(i).stick_color       = 0.25f*rgb(0,0,0)+0.75f*base_color;
	ref_sub_plot2d_config(i).point_color       = base_color;
	ref_sub_plot2d_config(i).bar_color         = 0.5f*rgb(1,1,1)+0.5f*base_color;
	ref_sub_plot2d_config(i).bar_outline_color = base_color;
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

/// create the gui for the plot independent of the sub plots
void plot2d::create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	if (p.begin_tree_node("dimensions", "heading", false, "level=3")) {
		p.align("\a");
			p.add_gui("center", center_location, "vector", "main_label='heading';gui_type='value_slider';options='min=-100;max=100;log=true;ticks=true'");
			p.add_gui("domain", domain, "box", "main_label='heading';gui_type='value_slider';options='min=-10;max=10;step=0.1;log=true;ticks=true'");
			p.add_gui("extent", extent, "vector", "main_label='heading';gui_type='value_slider';options='min=0.01;max=100;step=0.001;log=true;ticks=true'");
			p.add_gui("x-axis", axis_directions[0], "direction", "main_label='heading';gui_type='value_slider'");
			p.add_gui("y-axis", axis_directions[1], "direction", "main_label='heading';gui_type='value_slider'");
		p.align("\b");
	}

	plot_base::create_plot_gui(bp, p);
}

void plot2d::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot2d_config& pbc = ref_sub_plot2d_config(i);
	bool show = p.begin_tree_node("lines", pbc.show_lines, false, "level=3;w=100;align=' '");
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

plot2d::vec3 plot2d::transform_to_world(const vec2& domain_point) const
{
	vec2 delta;
	for (unsigned ai=0; ai<2; ++ai)
		if (get_domain_config_ptr()->axis_configs[ai].log_scale)
			delta[ai] = extent[ai]*(log(domain_point[ai]) - 0.5f*(log(domain.get_min_pnt()[ai])+ log(domain.get_max_pnt()[ai])))/(log(domain.get_max_pnt()[ai])- log(domain.get_min_pnt()[ai]));
		else
			delta[ai] = extent[ai] * (domain_point[ai] - 0.5f*(domain.get_min_pnt()[ai] + domain.get_max_pnt()[ai])) / (domain.get_max_pnt()[ai] - domain.get_min_pnt()[ai]);

	return center_location + delta(0) * axis_directions[0] + delta(1) * axis_directions[1];
}

void plot2d::draw_sub_plot(cgv::render::context& ctx, unsigned i)
{
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, samples[i]);

	if (ref_sub_plot_config(i).show_bars) {
		set_uniforms(ctx, bar_prog, i);
		glDisable(GL_CULL_FACE);

		bar_prog.enable(ctx);
		ctx.set_color(ref_sub_plot2d_config(i).bar_color);
		glDrawArrays(GL_POINTS, 0, samples[i].size());
		bar_prog.disable(ctx);

		glEnable(GL_CULL_FACE);

		if (ref_sub_plot2d_config(i).bar_outline_width > 0) {

			glLineWidth(ref_sub_plot2d_config(i).bar_outline_width);
			set_uniforms(ctx, bar_outline_prog, i);

			bar_outline_prog.enable(ctx);
			ctx.set_color(ref_sub_plot2d_config(i).bar_outline_color);
			glDrawArrays(GL_POINTS, 0, samples[i].size());
			bar_outline_prog.disable(ctx);
		}
	}

	if (ref_sub_plot_config(i).show_sticks) {
		set_uniforms(ctx, stick_prog, i);
		glLineWidth(ref_sub_plot2d_config(i).stick_width);
		stick_prog.enable(ctx);
		ctx.set_color(ref_sub_plot2d_config(i).stick_color);
		glDrawArrays(GL_POINTS, 0, samples[i].size());
		stick_prog.disable(ctx);
	}

	if (ref_sub_plot2d_config(i).show_points || ref_sub_plot2d_config(i).show_lines) {
		set_uniforms(ctx, prog, i);
		prog.enable(ctx);
	}

	if (ref_sub_plot2d_config(i).show_lines) {
		ctx.set_color(ref_sub_plot2d_config(i).line_color);
		glLineWidth(ref_sub_plot2d_config(i).line_width);
		if (strips[i].empty())
			glDrawArrays(GL_LINE_STRIP, 0, samples[i].size());
		else {
			unsigned fst = 0;
			for (unsigned j = 0; j < strips[i].size(); ++j) {
				glDrawArrays(GL_LINE_STRIP, fst, strips[i][j]);
				fst += strips[i][j];
			}
		}
	}

	if (ref_sub_plot2d_config(i).show_points) {
		ctx.set_color(ref_sub_plot2d_config(i).point_color);
		glPointSize(ref_sub_plot2d_config(i).point_size);
		glDrawArrays(GL_POINTS, 0, samples[i].size());
	}

	if (ref_sub_plot2d_config(i).show_points || ref_sub_plot2d_config(i).show_lines)
		prog.disable(ctx);
}

void plot2d::draw_domain(cgv::render::context& ctx)
{
	std::vector<vec2> P;
	if (get_domain_config_ptr()->fill) {
		ctx.set_color(get_domain_config_ptr()->color);
		P.push_back(domain.get_corner(0));
		P.push_back(domain.get_corner(1));
		P.push_back(domain.get_corner(3));
		P.push_back(domain.get_corner(2));
		cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, P);
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
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, P);
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
		vec2 p = domain.get_min_pnt(); P.push_back(p);
		p(aj) = domain.get_max_pnt()(aj); P.push_back(p);
		// max line
		p = domain.get_max_pnt(); P.push_back(p);
		p(aj) = domain.get_min_pnt()(aj); P.push_back(p);
		// axis line
		if (domain.get_min_pnt()(aj) < 0 && domain.get_max_pnt()(aj) > 0) {
			p(ai) = 0.0f;
			p(aj) = domain.get_min_pnt()(aj); P.push_back(p);
			p(aj) = domain.get_max_pnt()(aj); P.push_back(p);
		}
		if (!P.empty()) {
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, P);
			glDrawArrays(GL_LINES, 0, GLsizei(P.size()));
			P.clear();
		}
	}
}

void plot2d::draw_ticks(cgv::render::context& ctx)
{
	if (tick_vertices.empty())
		return;
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, tick_vertices);
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
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, tick_vertices);
	for (const auto& tbc : tick_batches) if (tbc.label_count > 0) {
		ctx.set_color(get_domain_config_ptr()->axis_configs[tbc.ai].color);
		for (unsigned i = tbc.first_label; i < tbc.first_label + tbc.label_count; ++i) {
			const label_info& li = tick_labels[i];
			ctx.set_cursor(transform_to_world(li.position).to_vec(), li.label, li.align);
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

	set_uniforms(ctx, prog);
	prog.enable(ctx);
	if (get_domain_config_ptr()->show_domain)
		draw_domain(ctx);
	draw_axes(ctx);
	draw_ticks(ctx);
	prog.disable(ctx);

	cgv::render::attribute_array_binding::disable_global_array(ctx, 0);
	ctx.enable_font_face(label_font_face, get_domain_config_ptr()->label_font_size);
	draw_tick_labels(ctx);

	cgv::render::attribute_array_binding::enable_global_array(ctx, 0);
	for (unsigned i = 0; i<samples.size(); ++i) {
		// skip unvisible and empty sub plots
		if (!ref_sub_plot2d_config(i).show_plot || samples[i].size() == 0)
			continue;
		draw_sub_plot(ctx, i);
	}

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