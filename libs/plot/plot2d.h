#pragma once

#include "plot_base.h"
#include <cgv/render/shader_program.h>

#include "lib_begin.h"

namespace cgv {
	namespace plot {

/** extend common plot configuration with parameters specific to 2d plot */
struct CGV_API plot2d_config : public plot_base_config
{
	/// whether to connect data points with lines
	bool show_lines;
	/// line width
	float line_width;
	/// line color
	rgb line_color;
	/// set default values
	plot2d_config(const std::string& _name);
	/// configure the sub plot to a specific chart type
	void configure_chart(ChartType chart_type);
};

/** The \c plot2d class draws 2d plots with potentially several sub plots of different chart types */
class CGV_API plot2d : public plot_base
{
protected:
	cgv::render::shader_program prog;
	cgv::render::shader_program stick_prog;
	cgv::render::shader_program bar_prog, bar_outline_prog;

	/// overloaded in derived classes to compute complete tick render information
	void compute_tick_render_information();
	///
	void draw_sub_plot(cgv::render::context& ctx, unsigned i);
	void draw_domain(cgv::render::context& ctx);
	void draw_axes(cgv::render::context& ctx);
	void draw_ticks(cgv::render::context& ctx);
	void draw_tick_labels(cgv::render::context& ctx);
protected:
	void set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i = -1);

	/**@name domain to world transformation*/
	//@{
	/// 2d box of plotted domain
	box2 domain;
	/// extent of domain in world coordinates
	vec2 extent;
	/// direction of domain axis in world coordinates
	vec3 axis_directions[2];
	/// center location of domain in world coordinates
	vec3 center_location;
	/// function to transform form 2d domain points to world coordinates
	vec3 transform_to_world(const vec2& domain_point) const;
	//@}

	/// store 2d samples for data series
	std::vector<std::vector<vec2> > samples;
	/// allow to split series into connected strips that are represented by the number of contained samples
	std::vector <std::vector<unsigned> > strips;
public:
	/// construct empty plot with default domain [0..1,0..1]
	plot2d();

	/**@name configure domain to world transform*/
	//@{
	/// return domain shown in plot
	const box2& get_domain() { return domain; }
	/// reference domain shown in plot
	box2& ref_domain() { return domain; }
	/// set the plot extend in 2D coordinates
	void set_extent(const vec2& new_extent);
	/// query the plot extend in 2D coordinates
	const vec2& get_extent() const;
	/// set the plot width to given value and if constrained == true the height, such that the aspect ration is the same as the aspect ratio of the domain
	void set_width(float new_width, bool constrained = true);
	/// set the plot height to given value and if constrained == true the width, such that the aspect ration is the same as the aspect ratio of the domain
	void set_height(float new_height, bool constrained = true);
	/// set the direction of x or y axis
	void set_axis_direction(unsigned ai, const vec3& new_axis_direction);
	/// place the origin of the plot in 3D to the given location
	void place_origin(const vec3& new_origin_location);
	/// place the plot extent center in 3D to the given location (this might can change the current origin location) 
	void place_center(const vec3& new_center_location);
	/// place a corner (0 .. lower left, 1 .. lower right, 2 .. upper left, 3 .. upper right) to a given 3D location ((this might can change the current origin / center location) 
	void place_corner(unsigned corner_index, const vec3& new_corner_location);
	/// return the current origin in 3D coordinates
	vec3 get_origin() const;
	/// return the current plot center in 3D coordinates
	const vec3& get_center() const;
	/// return the i-th plot corner in 3D coordinates
	vec3 get_corner(unsigned i) const;
	/// set the direction of x or y axis
	const vec3& get_axis_direction(unsigned ai) const;
	//@}

	/**@name helper functions to adjust axes*/
	//@{
	/// adjust the domain with respect to \c ai th axis to the data
	void adjust_domain_axis_to_data(unsigned ai, bool adjust_min = true, bool adjust_max = true);
	/// adjust the domain with respect to \c ai th axis to the data in the currently visible plots
	void adjust_domain_axis_to_visible_data(unsigned ai, bool adjust_min = true, bool adjust_max = true);
	/// adjust selected axes of domain to data
	void adjust_domain_to_data(bool adjust_x_axis = true, bool adjust_y_axis = true);
	/// adjust selected axes of domain to data in the currently visible plots
	void adjust_domain_to_visible_data(bool adjust_x_axis = true, bool adjust_y_axis = true);
	/// extend domain such that given axis is included
	void include_axis_to_domain(unsigned ai);
	/// adjust tick marks
	void adjust_tick_marks_to_domain(unsigned max_nr_primary_ticks = 20);
	//@}

	/**@name management of sub plots*/
	//@{
	/// add sub plot and return sub plot index 
	unsigned add_sub_plot(const std::string& name);
	/// delete the i-th sub plot
	void delete_sub_plot(unsigned i);
	/// return a reference to the plot1d configuration of the i-th plot
	plot2d_config& ref_sub_plot2d_config(unsigned i = 0);
	/// set the colors for all plot features of the i-th sub plot as variation of the given color
	void set_sub_plot_colors(unsigned i, const rgb& base_color);
	/// return the samples of the i-th sub plot
	std::vector<vec2>& ref_sub_plot_samples(unsigned i = 0);
	/// return the strip definition of the i-th sub plot
	std::vector<unsigned>& ref_sub_plot_strips(unsigned i = 0);
	//@}

	/// create the gui for the plot independent of the sub plots
	void create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p);
	/// create the gui for a configuration, overload to specialize for extended configs
	void create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);
	/// construct shader programs
	bool init(cgv::render::context& ctx);
	/// draw plot
	void draw(cgv::render::context& ctx);
	/// destruct shader programs
	void clear(cgv::render::context& ctx);
};

	}
}

#include <cgv/config/lib_end.h>