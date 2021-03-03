#pragma once

#include "plot_base.h"
#include <cgv/render/shader_program.h>
#include <libs/cgv_gl/rectangle_renderer.h>

#include "lib_begin.h"

namespace cgv {
	namespace plot {

/** extend common plot configuration with parameters specific to 2d plot */
struct CGV_API plot2d_config : public plot_base_config
{
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
	cgv::render::shader_program point_prog, stick_prog;
	cgv::render::shader_program rectangle_prog;
	///
	void draw_sub_plots(cgv::render::context& ctx);
	void draw_domain(cgv::render::context& ctx);
	void draw_tick_labels(cgv::render::context& ctx);
protected:
	bool disable_depth_mask;

	bool compute_sample_coordinate_interval(int i, int ai, float& samples_min, float& samples_max);
	/// store 2d samples for data series
	std::vector<std::vector<vec2> > samples;
	/// allow to split series into connected strips that are represented by the number of contained samples
	std::vector <std::vector<unsigned> > strips;
	///
	float layer_depth;
	/// render style of rectangles
	cgv::render::rectangle_render_style rrs;
public:
	/// construct 2D plot with given number of additional attributes and default parameters
	plot2d(unsigned nr_attributes = 0);

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

	/// construct shader programs
	bool init(cgv::render::context& ctx);
	/// draw plot
	void draw(cgv::render::context& ctx);
	/// destruct shader programs
	void clear(cgv::render::context& ctx);

	/// create the gui for a point subplot
	void create_point_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	/// create the gui for a stick subplot
	void create_stick_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	/// create the gui for a bar subplot
	void create_bar_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	///
	void create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);
	void create_gui(cgv::base::base* bp, cgv::gui::provider& p);
};

	}
}

#include <cgv/config/lib_end.h>