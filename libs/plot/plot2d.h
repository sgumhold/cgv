#pragma once

#include "plot_base.h"
#include <cgv/render/shader_program.h>

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
struct CGV_API plot2d_config : public plot_base_config
{
	unsigned N;

	bool show_faces;
	Clr face_color;

	PlotFaceIllumination face_illumination;

	plot2d_config();
};

/** The \c plot2d class draws 2d plots with potentially several sub plots of different plot configuration */
class CGV_API plot2d : public plot_base
{
	cgv::render::shader_program point_prog;
	cgv::render::shader_program line_prog;
	cgv::render::shader_program bar_prog;
	cgv::render::shader_program face_prog;

	void set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i);
protected:
	std::vector<std::vector<P3D> > samples;
	B3D domain;

public:
	/// construct empty plot with default domain [0..1,0..1,0..1]
	plot2d();
	/// adjust domain to data
	void adjust_domain_to_data(bool include_xy_plane = true);
	/// reference the shown domain
	B3D& ref_domain() { return domain; }
	/**@name management of sub plots*/
	//@{
	/// add sub plot and return reference to samples
	unsigned add_sub_plot(const std::string& name);
	/// set the number of samples of the i-th sub plot to N
	void set_samples_per_row(unsigned N, unsigned i = 0);
	/// return the number of samples per row
	unsigned get_samples_per_row(unsigned i) const;
	/// delete the i-th sub plot
	void delete_sub_plot(unsigned i);
	/// return a reference to the plot2d configuration of the i-th plot
	plot2d_config& ref_sub_plot2d_config(unsigned i = 0);
	/// return the samples of the i-th sub plot
	std::vector<P3D>& ref_sub_plot_samples(unsigned i = 0);
	//@}

	void create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);
	bool init(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void clear(cgv::render::context& ctx);
};

	}
}
#include <cgv/config/lib_end.h>