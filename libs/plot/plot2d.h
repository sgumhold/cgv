#pragma once

#include "plot_base.h"
//#include "mark2d_provider.h"
#include <cgv/render/shader_program.h>

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
	/// list of styles for provider based marks
	//std::vector<std::pair<mark2d_provider*, mark_style*>> marks;
};

/** The \c plot2d class draws 2d plots with potentially several sub plots of different chart types */
class CGV_API plot2d : public plot_base
{
protected:
	cgv::render::shader_program line_prog;
	cgv::render::shader_program point_prog;
	cgv::render::shader_program rectangle_prog;
	///
	bool draw_point_plot(cgv::render::context& ctx, int si, int layer_idx);
	bool draw_line_plot(cgv::render::context& ctx, int si, int layer_idx);
	bool draw_stick_plot(cgv::render::context& ctx, int si, int layer_idx);
	void configure_bar_plot(cgv::render::context& ctx);
	bool draw_bar_plot(cgv::render::context& ctx, int si, int layer_idx);
	int draw_sub_plots_jointly(cgv::render::context& ctx, int layer_idx);
	//bool extract_tick_rectangles_and_tick_labels(std::vector<box2>& R, std::vector<rgb>& C, std::vector<float>& D,
	//	std::vector<label_info>& tick_labels, int ai, int ti, float he, float z_plot = std::numeric_limits<float>::quiet_NaN());
	void extract_domain_rectangles(std::vector<box2>& R, std::vector<rgb>& C, std::vector<float>& D);
	void extract_domain_tick_rectangles_and_tick_labels(std::vector<box2>& R, std::vector<rgb>& C, std::vector<float>& D, std::vector<label_info>& tick_labels, std::vector<tick_batch_info>& tick_batches);
	void draw_domain(cgv::render::context& ctx, int si = -1, bool no_fill = false);
protected:

	bool compute_sample_coordinate_interval(int i, int ai, float& samples_min, float& samples_max);
	/// store 2d samples for data series
	std::vector<std::vector<vec2> > samples;
	/// allow to split series into connected strips that are represented by the number of contained samples
	std::vector <std::vector<unsigned> > strips;
	/// attribute managers for domain rectangles and domain tick labels
	cgv::render::attribute_array_manager aam_domain, aam_domain_tick_labels;
public:
	bool disable_depth_mask;
	/// whether to manage separate axes for each sub plot
	bool* multi_axis_modes;
	/// offset in between sub plots in x, y and z direction
	vec3 sub_plot_delta;
	/// construct 2D plot with given number of additional attributes and default parameters
	plot2d(const std::string& title, unsigned nr_attributes = 0);
	///prevent copy and assignment for now until properly implemented
	plot2d& operator=(const plot2d&) = delete;
	plot2d(const plot2d&) = delete;
	///
	~plot2d();
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