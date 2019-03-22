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
	/// add line color setting
	void set_colors(const rgb& base_color);
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
	bool compute_sample_coordinate_interval(int i, int ai, float& samples_min, float& samples_max);

	/// store 2d samples for data series
	std::vector<std::vector<vec2> > samples;
	/// allow to split series into connected strips that are represented by the number of contained samples
	std::vector <std::vector<unsigned> > strips;
public:
	/// construct empty plot with default domain [0..1,0..1]
	plot2d();

	/**@name management of sub plots*/
	//@{
	/// add sub plot and return sub plot index 
	unsigned add_sub_plot(const std::string& name);
	/// delete the i-th sub plot
	void delete_sub_plot(unsigned i);
	/// return a reference to the plot1d configuration of the i-th plot
	plot2d_config& ref_sub_plot2d_config(unsigned i = 0);
	/// return the samples of the i-th sub plot
	std::vector<vec2>& ref_sub_plot_samples(unsigned i = 0);
	/// return the strip definition of the i-th sub plot
	std::vector<unsigned>& ref_sub_plot_strips(unsigned i = 0);
	//@}

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