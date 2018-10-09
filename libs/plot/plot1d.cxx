#include "plot1d.h"
#include <libs/cgv_gl/gl/gl.h>
#include <libs/cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace plot {


/** extend common plot configuration with parameters specific to 1d plot */
plot1d_config::plot1d_config()
{
	show_lines = true;
	line_width = 1;
	line_color = Clr(1,0.5f,0);
};

/// construct empty plot with default domain [0..1,0..1]
plot1d::plot1d()
{
	domain.ref_min_pnt() = P2D(0,0);
	domain.ref_max_pnt() = P2D(1,1);
	extent               = V2D(1,1);
	axis_directions[0]   = V3D(1,0,0);
	axis_directions[1]   = V3D(0,1,0);
	center_location      = P3D(0,0,0);
	tick_line_width[1]   = 1;
	axes[0].ticks[1].type = TT_LINE;
	axes[1].ticks[1].type = TT_LINE;
}

/// return number of axis
unsigned plot1d::get_nr_axes() const
{
	return 2;
}

axis_config& plot1d::ref_axis_config(unsigned ai)
{
	return axes[ai];
}

void plot1d::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i)
{
	prog.set_uniform(ctx, "x_axis", axis_directions[0]);
	prog.set_uniform(ctx, "y_axis", axis_directions[1]);
	prog.set_uniform(ctx, "x_axis_log_scale", axes[0].log_scale);
	prog.set_uniform(ctx, "y_axis_log_scale", axes[1].log_scale);
	prog.set_uniform(ctx, "extent", extent);
	prog.set_uniform(ctx, "domain_min_pnt", domain.get_min_pnt());
	prog.set_uniform(ctx, "domain_max_pnt", domain.get_max_pnt());
	prog.set_uniform(ctx, "center_location", center_location);

	if (i < get_nr_sub_plots()) {
		plot_base::set_uniforms(ctx, prog, i);
		prog.set_uniform(ctx, "line_color", (cgv::math::fvec<Clr::component_type, Clr::nr_components>&) ref_sub_plot1d_config(i).line_color);
		prog.set_uniform(ctx, "bar_width", ref_sub_plot1d_config(i).bar_percentual_width*(float)(domain.get_extent()(0) / (ref_sub_plot_samples(i).size() - 1.0f)));
	}
}

/// adjust the domain with respect to \c ai th axis to the data
void plot1d::adjust_domain_axis_to_data(unsigned ai, bool adjust_min, bool adjust_max)
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
void plot1d::adjust_domain_axis_to_visible_data(unsigned ai, bool adjust_min, bool adjust_max)
{
	// compute bounding box
	bool found_sample = false;
	Crd min_value, max_value;
	for (unsigned i = 0; i<samples.size(); ++i) {
		if (ref_sub_plot1d_config(i).show_plot) {
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
void plot1d::adjust_tick_marks_to_domain(unsigned max_nr_primary_ticks)
{
	V2D de = domain.get_extent();

	for (int ai=0; ai<2; ++ai) {
		if (axes[ai].log_scale)
			de(ai) = (log(domain.get_max_pnt()(ai)) - log(domain.get_min_pnt()(ai))) / log(10.0f);

		Crd step = de(ai) / max_nr_primary_ticks;
		Crd step2;
		Crd scale = (Crd)pow(10,-floor(log10(step)));
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
		axes[ai].ticks[0].step = step;
		axes[ai].ticks[1].step = step2;
	}
}

/// extend domain such that given axis is included
void plot1d::include_axis_to_domain(unsigned ai)
{
	if (domain.get_min_pnt()(1-ai) > 0)
		domain.ref_min_pnt()(1-ai) = 0;
	if (domain.get_max_pnt()(1-ai) < 0)
		domain.ref_max_pnt()(1-ai) = 0;
}

/// adjust all axes of domain to data
void plot1d::adjust_domain_to_data(bool adjust_x_axis, bool adjust_y_axis)
{
	if (adjust_x_axis)
		adjust_domain_axis_to_data(0);
	if (adjust_y_axis)
		adjust_domain_axis_to_data(1);
}

/// adjust all axes of domain to data
void plot1d::adjust_domain_to_visible_data(bool adjust_x_axis, bool adjust_y_axis)
{
	if (adjust_x_axis)
		adjust_domain_axis_to_visible_data(0);
	if (adjust_y_axis)
		adjust_domain_axis_to_visible_data(1);
}

/// query the plot extend in 2D coordinates
const plot1d::V2D& plot1d::get_extent() const
{
	return extent;
}

/// set the plot extend in 2D coordinates
void plot1d::set_extent(const V2D& new_extent)
{
	extent = new_extent;
}

/// set the plot width to given value and if constrained == true the height, such that the aspect ration is the same as the aspect ratio of the domain
void plot1d::set_width(Crd new_width, bool constrained)
{
	extent(0) = new_width;
	if (constrained) {
		V2D e = domain.get_extent();
		extent(1) = new_width*e(1)/e(0);
	}
}

/// set the plot height to given value and if constrained == true the width, such that the aspect ration is the same as the aspect ratio of the domain
void plot1d::set_height(Crd new_height, bool constrained)
{
	extent(1) = new_height;
	if (constrained) {
		V2D e = domain.get_extent();
		extent(0) = new_height*e(0)/e(1);
	}
}

/// set the direction of x or y axis
void plot1d::set_axis_direction(unsigned ai, const V3D& new_axis_direction) 
{
	axis_directions[ai] = new_axis_direction;
}

/// place the origin of the plot in 3D to the given location
void plot1d::place_origin(const P3D& new_origin_location)
{
	center_location += new_origin_location - get_origin();
}

/// place the plot extent center in 3D to the given location (this might can change the current origin location) 
void plot1d::place_center(const P3D& new_center_location)
{
	center_location = new_center_location;
}

/// place a corner (0 .. lower left, 1 .. lower right, 2 .. upper left, 3 .. upper right) to a given 3D location ((this might can change the current origin / center location) 
void plot1d::place_corner(unsigned corner_index, const P3D& new_corner_location)
{
	center_location += new_corner_location - get_corner(corner_index);
}

/// return the current origin in 3D coordinates
plot1d::P3D plot1d::get_origin() const
{
	V2D delta = - domain.get_center();
	delta /= domain.get_extent();
	delta *= extent;
	return get_center() + delta(0) * get_axis_direction(0) + delta(1) * get_axis_direction(1);
}

/// return the current plot center in 3D coordinates
const plot1d::P3D& plot1d::get_center() const
{
	return center_location;
}

/// return the i-th plot corner in 3D coordinates
plot1d::P3D plot1d::get_corner(unsigned i) const
{
	V2D delta = domain.get_corner(i) - domain.get_center();
	delta /= domain.get_extent();
	delta *= extent;
	return get_center() + delta(0) * get_axis_direction(0) + delta(1) * get_axis_direction(1);
}

/// set the direction of x or y axis
const plot1d::V3D& plot1d::get_axis_direction(unsigned ai) const
{
	return axis_directions[ai];
}

/// set the colors for all plot features as variation of the given color
void plot1d::set_sub_plot_colors(unsigned i, const Clr& base_color)
{
	ref_sub_plot1d_config(i).line_color        = 0.25f*Clr(1,1,1)+0.75f*base_color;
	ref_sub_plot1d_config(i).stick_color       = 0.25f*Clr(0,0,0)+0.75f*base_color;
	ref_sub_plot1d_config(i).point_color       = base_color;
	ref_sub_plot1d_config(i).bar_color         = 0.5f*Clr(1,1,1)+0.5f*base_color;
	ref_sub_plot1d_config(i).bar_outline_color = base_color;
}

unsigned plot1d::add_sub_plot(const std::string& name)
{
	// determine index of new sub plot
	unsigned i = get_nr_sub_plots();

	// create new config
	if (i == 0)
		configs.push_back(new plot1d_config());
	else
		configs.push_back(new plot1d_config(ref_sub_plot1d_config(i-1)));
	ref_sub_plot_config(i).name = name;

	// create new point container
	samples.push_back(std::vector<plot1d::P2D>());
	strips.push_back(std::vector<unsigned>());

	// return sub plot index
	return i;
}

void plot1d::delete_sub_plot(unsigned i)
{
	delete configs[i];
	configs[i] = 0;
	configs.erase(configs.begin() + i);
	samples.erase(samples.begin() + i);
	strips.erase(strips.begin() + i);
}

/// return a reference to the plot base configuration of the i-th plot
plot1d_config& plot1d::ref_sub_plot1d_config(unsigned i)
{
	return static_cast<plot1d_config&>(ref_sub_plot_config(i));
}

std::vector<plot1d::P2D>& plot1d::ref_sub_plot_samples(unsigned i)
{
	return samples[i];
}

/// return the strip definition of the i-th sub plot
std::vector<unsigned>& plot1d::ref_sub_plot_strips(unsigned i)
{
	return strips[i];
}

/// create the gui for the plot independent of the sub plots
void plot1d::create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p)
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

void plot1d::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot1d_config& pbc = ref_sub_plot1d_config(i);

	p.add_decorator("lines", "heading", "level=3;w=100", " ");
	p.add_member_control(bp, "show", pbc.show_lines, "toggle", "w=50");
	p.add_member_control(bp, "width", pbc.line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
	p.add_member_control(bp, "color", pbc.line_color);

	plot_base::create_config_gui(bp, p, i);
}

bool plot1d::init(cgv::render::context& ctx)
{
	return true;
}

void plot1d::clear(cgv::render::context& ctx)
{
	prog.destruct(ctx);
	stick_prog.destruct(ctx);
	bar_prog.destruct(ctx);
	bar_outline_prog.destruct(ctx);
}


plot1d::P3D plot1d::transform_to_world(const P2D& domain_point) const
{
	V2D delta;
	for (unsigned ai=0; ai<2; ++ai)
		if (axes[ai].log_scale)
			delta[ai] = extent[ai]*(log(domain_point[ai]) - 0.5f*(log(domain.get_min_pnt()[ai])+ log(domain.get_max_pnt()[ai])))/(log(domain.get_max_pnt()[ai])- log(domain.get_min_pnt()[ai]));
		else
			delta[ai] = extent[ai] * (domain_point[ai] - 0.5f*(domain.get_min_pnt()[ai] + domain.get_max_pnt()[ai])) / (domain.get_max_pnt()[ai] - domain.get_min_pnt()[ai]);

	return center_location + delta(0) * axis_directions[0] + delta(1) * axis_directions[1];
}

void plot1d::draw(cgv::render::context& ctx)
{
	if (!prog.is_created()) {
		if (!prog.build_program(ctx, "plot1d.glpr")) {
			std::cerr << "could not build GLSL program from plot1d.glpr" << std::endl;
			return;
		}
	}
	if (!stick_prog.is_created()) {
		if (!stick_prog.build_program(ctx, "plot1d_stick.glpr")) {
			std::cerr << "could not build GLSL program from plot1d_stick.glpr" << std::endl;
			return;
		}
	}
	if (!bar_prog.is_created()) {
		if (!bar_prog.build_program(ctx, "plot1d_bar.glpr")) {
			std::cerr << "could not build GLSL program from plot1d_bar.glpr" << std::endl;
			return;
		}
	}
	if (!bar_outline_prog.is_created()) {
		if (!bar_outline_prog.build_program(ctx, "plot1d_bar_outline.glpr")) {
			std::cerr << "could not build GLSL program from bar_outline_prog.glpr" << std::endl;
			return;
		}
	}
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	for (unsigned i = 0; i<samples.size(); ++i) {
		if (!ref_sub_plot1d_config(i).show_plot || samples[i].size() == 0)
			continue;
		glVertexPointer(2, GL_FLOAT, 0, &samples[i].front());
		glEnableClientState(GL_VERTEX_ARRAY);

		if (ref_sub_plot_config(i).show_bars) {
			set_uniforms(ctx, bar_prog, i);
			glColor3fv(&ref_sub_plot1d_config(i).bar_color[0]);
			glDisable(GL_CULL_FACE);

			bar_prog.enable(ctx);
			glDrawArrays(GL_POINTS, 0, samples[i].size());
			bar_prog.disable(ctx);

			glEnable(GL_CULL_FACE);

			if (ref_sub_plot1d_config(i).bar_outline_width > 0) {
				glLineWidth(ref_sub_plot1d_config(i).bar_outline_width);
				set_uniforms(ctx, bar_outline_prog, i);
				glColor3fv(&ref_sub_plot1d_config(i).bar_outline_color[0]);

				bar_outline_prog.enable(ctx);
				glDrawArrays(GL_POINTS, 0, samples[i].size());
				bar_outline_prog.disable(ctx);
			}
		}

		if (ref_sub_plot_config(i).show_sticks) {
			set_uniforms(ctx, stick_prog, i);
			glColor3fv(&ref_sub_plot1d_config(i).stick_color[0]);
			glLineWidth(ref_sub_plot1d_config(i).stick_width);
			stick_prog.enable(ctx);
			glDrawArrays(GL_POINTS, 0, samples[i].size());
			stick_prog.disable(ctx);
		}

		if (ref_sub_plot1d_config(i).show_points || ref_sub_plot1d_config(i).show_lines) {
			set_uniforms(ctx, prog, i);
			prog.enable(ctx);
		}

		if (ref_sub_plot1d_config(i).show_lines) {
			glColor3fv(&ref_sub_plot1d_config(i).line_color[0]);
			glLineWidth(ref_sub_plot1d_config(i).line_width);
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

		if (ref_sub_plot1d_config(i).show_points) {
			glColor3fv(&ref_sub_plot1d_config(i).point_color[0]);
			glPointSize(ref_sub_plot1d_config(i).point_size);
			glDrawArrays(GL_POINTS, 0, samples[i].size());
		}

		if (ref_sub_plot1d_config(i).show_points || ref_sub_plot1d_config(i).show_lines)
			prog.disable(ctx);

		glDisableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, 0);
	}

	if (show_axes) {
		glColor3fv(&axis_color[0]);
		glLineWidth(axis_line_width);
		set_uniforms(ctx, prog, 0);
		prog.enable(ctx);
		glBegin(GL_LINE_LOOP);
		glVertex2fv(domain.get_corner(0));
		glVertex2fv(domain.get_corner(1));
		glVertex2fv(domain.get_corner(3));
		glVertex2fv(domain.get_corner(2));
		glEnd();

		bool axis_inside[2] = { false, false };
		if (domain.get_min_pnt()(0) < 0 && domain.get_max_pnt()(0) > 0) {
			glBegin(GL_LINES);
			glVertex2f(0,domain.get_min_pnt()(1));
			glVertex2f(0,domain.get_max_pnt()(1));
			glEnd();
			axis_inside[1] = true;
		}
		if (domain.get_min_pnt()(1) < 0 && domain.get_max_pnt()(1) > 0) {
			glBegin(GL_LINES);
			glVertex2f(domain.get_min_pnt()(0),0);
			glVertex2f(domain.get_max_pnt()(0),0);
			glEnd();
			axis_inside[0] = true;
		}
		int ai;
		for (ai = 0; ai < 2; ++ai) {
			for (int ti = 0; ti < 2; ++ti) {
				if (axes[ai].ticks[ti].type != TT_NONE) {
					Crd min_val = domain.get_min_pnt()(ai);
					Crd max_val = domain.get_max_pnt()(ai);
					if (axes[ai].log_scale) {
						min_val = log(min_val) / log(10.0f);
						max_val = log(max_val) / log(10.0f);
					}
					int min_i = (int) ((min_val - fmod(min_val, axes[ai].ticks[ti].step) ) / axes[ai].ticks[ti].step);
					int max_i = (int) ((max_val - fmod(max_val, axes[ai].ticks[ti].step) ) / axes[ai].ticks[ti].step);

					// ignore ticks on domain boundary
					if (min_i * axes[ai].ticks[ti].step - min_val < std::numeric_limits<Crd>::epsilon())
						++min_i;
					if (max_i * axes[ai].ticks[ti].step - max_val > -std::numeric_limits<Crd>::epsilon())
						--max_i;
					
					glLineWidth(tick_line_width[ti]);
					Crd dash_length = tick_length[ti]*0.01f*domain.get_extent()(1-domain.get_max_extent_coord_index());
					if (extent(0) < extent(1)) {
						if (ai == 1)
							dash_length *= domain.get_extent()(0);
						else
							dash_length *= domain.get_extent()(1)*extent(0) / extent(1);
					}
					else {
						if (ai == 0)
							dash_length *= domain.get_extent()(1);
						else
							dash_length *= domain.get_extent()(0)*extent(1) / extent(0);
					}
					Crd s_min = domain.get_min_pnt()(1-ai);
					Crd s_max = domain.get_max_pnt()(1-ai);

					glBegin(GL_LINES);
					for (int i=min_i; i<=max_i; ++i) {
						Crd c[2];
						c[ai] = (Crd) (i*axes[ai].ticks[ti].step);
						if (axes[ai].log_scale)
							c[ai] = pow(10.0f, c[ai]);
						else // ignore ticks on axes
							if (fabs(c[ai]) < std::numeric_limits<Crd>::epsilon())
								continue;

						switch (axes[ai].ticks[ti].type) {
						case TT_DASH :
							c[1-ai] = s_min; 
							glVertex2fv(c);
							c[1 - ai] = s_min + dash_length;
							if (axes[1 - ai].log_scale) {
								float q = (c[1 - ai] - domain.get_center()(1 - ai)) / domain.get_extent()(1 - ai);
								c[1 - ai] = pow(10.0f, (q*(log(domain.get_max_pnt()(1-ai)) - log(domain.get_min_pnt()(1-ai))) + 0.5f*(log(domain.get_min_pnt()(1-ai)) + log(domain.get_max_pnt()(1-ai)))) / log(10.0f));
							}
							glVertex2fv(c);
							c[1-ai] = s_max; 
							glVertex2fv(c);
							c[1 - ai] = s_max - dash_length;
							if (axes[1 - ai].log_scale) {
								float q = (c[1 - ai] - domain.get_center()(1 - ai)) / domain.get_extent()(1 - ai);
								c[1 - ai] = pow(10.0f, (q*(log(domain.get_max_pnt()(1 - ai)) - log(domain.get_min_pnt()(1 - ai))) + 0.5f*(log(domain.get_min_pnt()(1 - ai)) + log(domain.get_max_pnt()(1 - ai)))) / log(10.0f));
							}
							glVertex2fv(c);
							
							// draw tick mark on axis
							if (!axes[1-ai].log_scale && s_min + dash_length < 0 && s_max - dash_length > 0) {
								c[1-ai] = -dash_length; 
								glVertex2fv(c);
								c[1-ai] =  dash_length; 
								glVertex2fv(c);
							}
							break;
						case TT_LINE : 
						case TT_PLANE : 
							c[1-ai] = s_min; glVertex2fv(c);
							c[1-ai] = s_max; glVertex2fv(c);
							break;
						}
					}
					glEnd();
				}
			}
		}
		prog.disable(ctx);
		for (ai = 0; ai < 2; ++ai) {
			for (int ti = 0; ti < 2; ++ti) if (label_ticks[ti]) {
				ctx.enable_font_face(label_font_face, label_font_size);
				if (axes[ai].ticks[ti].type != TT_NONE) {
					Crd min_val = domain.get_min_pnt()(ai);
					Crd max_val = domain.get_max_pnt()(ai);
					if (axes[ai].log_scale) {
						min_val = log(min_val) / log(10.0f);
						max_val = log(max_val) / log(10.0f);
					}
					int min_i = (int) ((min_val - fmod(min_val, axes[ai].ticks[ti].step) ) / axes[ai].ticks[ti].step);
					int max_i = (int) ((max_val - fmod(max_val, axes[ai].ticks[ti].step) ) / axes[ai].ticks[ti].step);

					if (min_i * axes[ai].ticks[ti].step - min_val < -std::numeric_limits<Crd>::epsilon())
						++min_i;
					if (max_i * axes[ai].ticks[ti].step - max_val > std::numeric_limits<Crd>::epsilon())
						--max_i;

					glLineWidth(tick_line_width[ti]);
					Crd dash_length = tick_length[0]*0.01f*domain.get_extent()(1-domain.get_max_extent_coord_index());
					if (extent(0) < extent(1)) {
						if (ai == 1)
							dash_length *= domain.get_extent()(0);
						else
							dash_length *= domain.get_extent()(1)*extent(0) / extent(1);
					}
					else {
						if (ai == 0)
							dash_length *= domain.get_extent()(1);
						else
							dash_length *= domain.get_extent()(0)*extent(1) / extent(0);
					}
					Crd s_min = domain.get_min_pnt()(1-ai);
					Crd s_max = domain.get_max_pnt()(1-ai);
					
					for (int i=min_i; i<=max_i; ++i) {
						V2D c;
						c(ai) = (Crd) (i*axes[ai].ticks[ti].step);
						if (axes[ai].log_scale)
							c(ai) = pow(10.0f, c(ai));

						if (c(ai) < domain.get_min_pnt()(ai))
							c(ai) = domain.get_min_pnt()(ai);
						std::string label = cgv::utils::to_string(c(ai));


						switch (axes[ai].ticks[ti].type) {
						case TT_DASH :
							// ignore ticks on axes
							if (fabs(c[ai]) > std::numeric_limits<Crd>::epsilon()) {
								if (s_min + dash_length < 0 && s_max - dash_length > 0) {
									c(1-ai) = -1.5f*dash_length;
									if (axes[1 - ai].log_scale) {
										float q = (c[1 - ai] - domain.get_center()(1 - ai)) / domain.get_extent()(1 - ai);
										c[1 - ai] = pow(10.0f, (q*(log(domain.get_max_pnt()(1 - ai)) - log(domain.get_min_pnt()(1 - ai))) + 0.5f*(log(domain.get_min_pnt()(1 - ai)) + log(domain.get_max_pnt()(1 - ai)))) / log(10.0f));
									}
									ctx.set_cursor(transform_to_world(c).to_vec(), label, ai == 0 ? cgv::render::TA_TOP : cgv::render::TA_RIGHT);
									ctx.output_stream() << label;
									ctx.output_stream().flush();
								}
							}
						case TT_LINE : 
						case TT_PLANE : 
							c(1-ai) = s_min - 0.5f*dash_length;
							if (axes[1 - ai].log_scale) {
								float q = (c[1 - ai] - domain.get_center()(1 - ai)) / domain.get_extent()(1 - ai);
								c[1 - ai] = pow(10.0f, (q*(log(domain.get_max_pnt()(1 - ai)) - log(domain.get_min_pnt()(1 - ai))) + 0.5f*(log(domain.get_min_pnt()(1 - ai)) + log(domain.get_max_pnt()(1 - ai)))) / log(10.0f));
							}
							ctx.set_cursor(transform_to_world(c).to_vec(), label, ai == 0 ? cgv::render::TA_TOP : cgv::render::TA_RIGHT);
							ctx.output_stream() << label;
							ctx.output_stream().flush();

							c(1-ai) = s_max + 0.5f*dash_length;
							if (axes[1 - ai].log_scale) {
								float q = (c[1 - ai] - domain.get_center()(1 - ai)) / domain.get_extent()(1 - ai);
								c[1 - ai] = pow(10.0f, (q*(log(domain.get_max_pnt()(1 - ai)) - log(domain.get_min_pnt()(1 - ai))) + 0.5f*(log(domain.get_min_pnt()(1 - ai)) + log(domain.get_max_pnt()(1 - ai)))) / log(10.0f));
							}
							ctx.set_cursor(transform_to_world(c).to_vec(), label, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT);
							ctx.output_stream() << label;
							ctx.output_stream().flush();
							break;
						}
					}
				}
			}
		}
	}

//	glDisable(GL_BLEND);
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_LINE_SMOOTH);
//	glDepthFunc(GL_LESS);
}

	}
}