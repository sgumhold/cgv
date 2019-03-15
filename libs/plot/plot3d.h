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
struct CGV_API plot3d_config : public plot_base_config
{
	unsigned samples_per_row;
	bool show_faces;
	rgb face_color;

	PlotFaceIllumination face_illumination;

	plot3d_config(const std::string& _name);
};

/** The \c plot3d class draws 2d plots with potentially several sub plots of different plot configuration */
class CGV_API plot3d : public plot_base
{
	cgv::render::shader_program point_prog;
	cgv::render::shader_program line_prog;
	cgv::render::shader_program bar_prog;
	cgv::render::shader_program face_prog;

	void set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i);
protected:
	std::vector<std::vector<vec3> > samples;
	box3 domain;

	vec3 extent;
	vec3 axis_directions[2];
	vec3 center_location;

	axis_config axes[3];

public:
	/// construct empty plot with default domain [0..1,0..1,0..1]
	plot3d();
	/// adjust domain to data
	void adjust_domain_to_data(bool include_xy_plane = true);
	/// reference the shown domain
	box3& ref_domain() { return domain; }
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
	/// return a reference to the plot3d configuration of the i-th plot
	plot3d_config& ref_sub_plot3d_config(unsigned i = 0);
	/// return the samples of the i-th sub plot
	std::vector<vec3>& ref_sub_plot_samples(unsigned i = 0);
	//@}

	void create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);
	bool init(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void clear(cgv::render::context& ctx);
};

	}
}
#include <cgv/config/lib_end.h>