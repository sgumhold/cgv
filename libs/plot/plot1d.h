#pragma once

#include "plot_base.h"
#include <cgv/render/shader_program.h>

#include "lib_begin.h"

namespace cgv {
	namespace plot {


/** extend common plot configuration with parameters specific to 1d plot */
struct CGV_API plot1d_config : public plot_base_config
{
	bool show_lines;
	float line_width;
	Clr line_color;

	plot1d_config();
};

/** The \c plot1d class draws 1d plots with potentially several sub plots of different plot configuration */
class CGV_API plot1d : public plot_base
{
	cgv::render::shader_program prog;
	cgv::render::shader_program stick_prog;
	cgv::render::shader_program bar_prog, bar_outline_prog;
protected:
	std::vector<std::vector<P2D> > samples;
	std::vector <std::vector<unsigned> > strips;
	B2D domain;
	axis_config axes[2];
	V2D extent;
	V3D axis_directions[2];
	P3D center_location;

	P3D transform_to_world(const P2D& domain_point) const;
	void set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i);
public:
	axis_config& ref_axis_config(unsigned ai);
	/// construct empty plot with default domain [0..1,0..1]
	plot1d();
	/// return number of axis
	unsigned get_nr_axes() const;
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
	/// return domain shown in plot
	const B2D& get_domain() { return domain; }
	/// reference domain shown in plot
	B2D& ref_domain() { return domain; }
	/// set the plot extend in 2D coordinates
	void set_extent(const V2D& new_extent);
	/// query the plot extend in 2D coordinates
	const V2D& get_extent() const;
	/// set the plot width to given value and if constrained == true the height, such that the aspect ration is the same as the aspect ratio of the domain
	void set_width(Crd new_width, bool constrained = true);
	/// set the plot height to given value and if constrained == true the width, such that the aspect ration is the same as the aspect ratio of the domain
	void set_height(Crd new_height, bool constrained = true);
	/// set the direction of x or y axis
	void set_axis_direction(unsigned ai, const V3D& new_axis_direction);
	/// place the origin of the plot in 3D to the given location
	void place_origin(const P3D& new_origin_location);
	/// place the plot extent center in 3D to the given location (this might can change the current origin location) 
	void place_center(const P3D& new_center_location);
	/// place a corner (0 .. lower left, 1 .. lower right, 2 .. upper left, 3 .. upper right) to a given 3D location ((this might can change the current origin / center location) 
	void place_corner(unsigned corner_index, const P3D& new_corner_location);
	/// return the current origin in 3D coordinates
	P3D get_origin() const;
	/// return the current plot center in 3D coordinates
	const P3D& get_center() const;
	/// return the i-th plot corner in 3D coordinates
	P3D get_corner(unsigned i) const;
	/// set the direction of x or y axis
	const V3D& get_axis_direction(unsigned ai) const;

	/**@name management of sub plots*/
	//@{
	/// add sub plot and return sub plot index 
	unsigned add_sub_plot(const std::string& name);
	/// delete the i-th sub plot
	void delete_sub_plot(unsigned i);
	/// return a reference to the plot1d configuration of the i-th plot
	plot1d_config& ref_sub_plot1d_config(unsigned i = 0);
	/// set the colors for all plot features of the i-th sub plot as variation of the given color
	void set_sub_plot_colors(unsigned i, const Clr& base_color);
	/// return the samples of the i-th sub plot
	std::vector<P2D>& ref_sub_plot_samples(unsigned i = 0);
	/// return the strip definition of the i-th sub plot
	std::vector<unsigned>& ref_sub_plot_strips(unsigned i = 0);
	//@}
	/// create the gui for the plot independent of the sub plots
	void create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p);
	/// create the gui for a configuration, overload to specialize for extended configs
	void create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);

	bool init(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void clear(cgv::render::context& ctx);
};

	}
}

#include <cgv/config/lib_end.h>