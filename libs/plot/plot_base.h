#pragma once

#include <vector>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/render/drawable.h>
#include <cgv/media/color.h>
#include <cgv/media/font/font.h>
#include <cgv/gui/provider.h>

#include "lib_begin.h"

namespace cgv {
	namespace plot {

/** common plot types */
struct CGV_API plot_types
{
	typedef float Crd;
	typedef cgv::media::color<float> Clr;

	typedef cgv::math::fvec<Crd,2> V2D;
	typedef cgv::math::fvec<Crd,2> P2D;
	typedef cgv::math::fvec<Crd,3> V3D;
	typedef cgv::math::fvec<Crd,3> P3D;

	typedef cgv::media::axis_aligned_box<Crd,2> B2D;
	typedef cgv::media::axis_aligned_box<Crd,3> B3D;
};

/** common plot configuration parameters */
struct CGV_API plot_base_config : public plot_types
{
	bool show_plot;

	bool show_points;
	float point_size;
	Clr point_color;
	
	bool show_sticks;
	float stick_width;
	Clr stick_color;

	bool show_bars;
	float bar_outline_width;
	float bar_percentual_width;
	Clr bar_color;
	Clr bar_outline_color;

	std::string name;

	plot_base_config();
	virtual ~plot_base_config();
};

enum TickType
{
	TT_NONE,
	TT_DASH,
	TT_LINE,
	TT_PLANE
};

struct tick_config : public plot_types
{
	TickType type;
	float    step;
};

struct axis_config : public plot_types
{
	tick_config ticks[2];
	bool        log_scale;
	axis_config();
};

/** base class for plot1d and plot2d */
class CGV_API plot_base : public cgv::render::drawable, public plot_types
{
protected:
	/// set uniforms for the i-th plot config
	virtual void set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i);

public:
	float plot_scale;

	bool show_axes;
	Clr axis_color;
	float axis_line_width;
	float tick_line_width[2];
	float tick_length[2];
	bool  label_ticks[2];

	static std::vector<const char*> font_names;
	static std::string font_name_enum_def;

	unsigned label_font_index;
	cgv::media::font::font_ptr label_font;
	cgv::media::font::font_face_ptr label_font_face;
	void ensure_font_names();
	void on_font_selection();
	void on_font_face_selection();
	float label_font_size;
	cgv::media::font::FontFaceAttributes label_ffa;

protected:
	std::vector<plot_base_config*> configs;
	virtual axis_config& ref_axis_config(unsigned ai) = 0;
public:
	/// construct with default parameters
	plot_base();
	/// return number of axis
	virtual unsigned get_nr_axes() const = 0;
	/// configure the label font
	void set_label_font(float font_size, cgv::media::font::FontFaceAttributes ffa = cgv::media::font::FFA_REGULAR, const std::string& font_name = "");
	/**@name management of sub plots*/
	//@{
	/// return current number of sub plots
	unsigned get_nr_sub_plots() const;
	/// add sub plot and return sub plot index 
	virtual unsigned add_sub_plot(const std::string& name) = 0;
	/// delete the i-th sub plot
	virtual void delete_sub_plot(unsigned i) = 0;
	/// return a reference to the plot base configuration of the i-th plot
	plot_base_config& ref_sub_plot_config(unsigned i);
	//@}

	/**@name gui support*/
	//@{
	/// create the gui for the plot independent of the sub plots
	virtual void create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p);
	/// create the gui for a configuration, overload to specialize for extended configs
	virtual void create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);
	/// create a gui for the plot with gui for all configs
	virtual void create_gui(cgv::base::base* bp, cgv::gui::provider& p);
	//@}
};

	}
}

#include <cgv/config/lib_end.h>