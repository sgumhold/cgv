#pragma once

#include "plot_base.h"
#include <cgv/render/shader_program.h>
#include <libs/cgv_gl/box_renderer.h>
#include <cgv_gl/cone_renderer.h>

#include "lib_begin.h"

namespace cgv {
	namespace plot {

/** different lighting settings */
enum PlotFaceIllumination
{
	PFI_NONE,
	PFI_PER_FACE,
	PFI_PER_VERTEX
};

/** extend common plot configuration with parameters specific to 2d plot */
struct CGV_API plot3d_config : public plot_base_config
{
	/// whether to illustrate line orientation
	bool show_line_orientation;
	/// provide second dimension of bar extend
	mapped_size bar_percentual_depth;
	/// if samples per row > 0, the samples are interpreted as regular grid
	unsigned samples_per_row;
	/// whether to show faces
	bool show_surface;
	/// whether to turn on wireframe
	bool wireframe;
	/// color of faces
	rgb surface_color;
	/// how to illuminate the surface
	PlotFaceIllumination face_illumination;
	/// add tube and surface color
	void set_colors(const rgb& base_color);
	/// construct with default values
	plot3d_config(const std::string& _name);
};

/** The \c plot3d class draws 2d plots with potentially several sub plots of different plot configuration */
class CGV_API plot3d : public plot_base
{
	cgv::render::shader_program tick_label_prog;
	cgv::render::shader_program sphere_prog;
	cgv::render::shader_program box_prog;
	cgv::render::shader_program wirebox_prog;
	cgv::render::shader_program stick_prog;
	cgv::render::shader_program tube_prog;
	//cgv::render::shader_program surface_prog;
	cgv::render::box_render_style brs;
	cgv::render::cone_render_style rcrs;
	cgv::render::attribute_array_manager aam_domain;

	bool compute_sample_coordinate_interval(int i, int ai, float& samples_min, float& samples_max);
	void draw_domain(cgv::render::context& ctx);
	void draw_ticks(cgv::render::context& ctx);
	void draw_sub_plots(cgv::render::context& ctx);
protected:
	std::vector<std::vector<vec3> > samples;
	/// overloaded in derived classes to compute complete tick render information
	void compute_tick_render_information();
public:
	/// construct 3d plot with given number of additional attributes and default parameters
	plot3d(unsigned nr_attributes = 0);
	/**@name management of sub plots*/
	//@{
	/// add sub plot and return reference to samples
	unsigned add_sub_plot(const std::string& name);
	/// set the number of samples of the i-th sub plot to N
	void set_samples_per_row(unsigned i, unsigned N);
	/// return the number of samples per row
	unsigned get_samples_per_row(unsigned i) const;
	/// delete the i-th sub plot
	void delete_sub_plot(unsigned i);
	/// return a reference to the plot3d configuration of the i-th plot
	plot3d_config& ref_sub_plot3d_config(unsigned i = 0);
	/// return the samples of the i-th sub plot
	std::vector<vec3>& ref_sub_plot_samples(unsigned i = 0);
	//@}

	/// create the gui for a line subplot
	void create_line_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	/// create the gui for a bar subplot
	void create_bar_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	void create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);
	bool init(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void clear(cgv::render::context& ctx);
	void create_gui(cgv::base::base* bp, cgv::gui::provider& p);
};

	}
}
#include <cgv/config/lib_end.h>