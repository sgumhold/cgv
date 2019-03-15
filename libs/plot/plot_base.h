#pragma once

#include <cgv/base/base.h>
#include <vector>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/render/drawable.h>
#include <cgv/media/color.h>
#include <cgv/media/font/font.h>
#include <cgv/gui/provider.h>

#include "lib_begin.h"

namespace cgv {
	namespace plot {

		/// different tickmark types
enum TickType
{
	TT_NONE, //! no tick at all
	TT_DASH, //! short line at the axis
	TT_LINE, //! line that spans the domain
	TT_PLANE //! used for 3D plots only
};

/// tickmark configuration of one tickmark type
struct tick_config
{
	/// type of tick
	TickType type;
	/// step width between two ticks along axis
	float    step;
	/// line width
	float    line_width;
	/// tick length relative to domain extent
	float    length;
	/// whether to show text labels at tick
	bool     label;
	/// number of digits after decimal point, defaults to -1 which gives adaptive precision
	int      precision;
	/// set tick config defaults
	tick_config(bool primary);
};

/// configuration information stored per domain axis
struct axis_config 
{
	/// whether axis is drawn with logarithmic scale
	bool log_scale;
	/// line width
	float line_width;
	/// color of axis
	cgv::render::render_types::rgb color;
	/// configuration of primary tickmarks
	tick_config primary_ticks;
	/// configuration of secondary tickmarks
	tick_config secondary_ticks;
	/// set default values
	axis_config();
};

struct domain_config
{
	/// whether to show the coordinate axes including tickmarks and labels
	bool show_domain;
	/// whether to fill the domain
	bool fill;
	/// color of the domain fill
	cgv::render::render_types::rgb color;
	/// store a vector of axis configurations (2/3 for plot2/3d)
	std::vector<axis_config> axis_configs;
	/// store index of selected font
	unsigned label_font_index;
	/// store selected font size
	float label_font_size;
	/// store selected font face attributes
	cgv::media::font::FontFaceAttributes label_ffa;
	/// set default values
	domain_config(unsigned nr_axes);
};

/// different chart types
enum ChartType
{
	CT_POINT,
	CT_LINE_CHART,
	CT_BAR_CHART
};

/** plot independent configuration parameters of one sub plot in a 2d or 3d plot */
struct CGV_API plot_base_config : public cgv::render::render_types
{
	/// name of sub plot
	std::string name;

	/// whether to show plot, ac
	bool show_plot;

	/// whether to show data points
	bool show_points;
	/// point size in pixels
	float point_size;
	/// point color
	rgb point_color;
	
	/// whether to show straight lines to the bottom of the plot, which are called sticks
	bool show_sticks;
	/// line width of stick
	float stick_width;
	/// color of the stick line
	rgb stick_color;

	/// whether to show bars
	bool show_bars;
	/// line width of bar outlines
	float bar_outline_width;
	/// percentual width of bar computed assuming a uniform y-sampling distance
	float bar_percentual_width;
	/// bar fill color
	rgb bar_color;
	/// bar outline color
	rgb bar_outline_color;

	/// set default values
	plot_base_config(const std::string& _name);
	/// configure the sub plot to a specific chart type
	virtual void configure_chart(ChartType chart_type);
	/// virtual constructor in order to allow to extend the configuration for derived classes
	virtual ~plot_base_config();
};


/** base class for plot2d and plot3d, which can have several sub plots each */
class CGV_API plot_base : public cgv::render::drawable
{
	/**@name tick render information management */
	//@{
private:
	/// domain configuration of last time that tick render information has been computed, initialized with 0 number of axis_configs to ensure tick render information computation for first time 
	domain_config last_dom_cfg;
protected:
	/// render information stored per label
	struct label_info
	{
		vec2 position;
		std::string label;
		cgv::render::TextAlignment align;
		label_info(const vec2& _position, const std::string& _label, cgv::render::TextAlignment _align)
			: position(_position), label(_label), align(_align) {}
	};
	/// 
	struct tick_batch_info
	{
		/// indices of coordinate axis used for definition of 2d points
		int ai, aj;
		///
		bool primary;
		/// index of first tick vertex in batch
		unsigned first_vertex;
		///
		unsigned vertex_count;
		///
		unsigned first_label;
		///
		unsigned label_count;
		///
		tick_batch_info(int _ai, int _aj, bool _primary, unsigned _first_vertex = 0, unsigned _first_label = 0);
	};
	/// all vertex locations of tick lines
	std::vector<vec2> tick_vertices;
	/// all tick labels 
	std::vector<label_info> tick_labels;
	/// twice number of axis pairs with index of first tick label and number of tick labels for primary and secondary ticks
	std::vector<tick_batch_info> tick_batches;
	/// check whether tick information has to be updated
	bool tick_render_information_outofdate() const;
	/// ensure that tick render information is current
	void ensure_tick_render_information();
	/// overloaded in derived classes to compute complete tick render information
	virtual void compute_tick_render_information() = 0;
	/// used in implementation of compute_tick_render_information() in derived class to collect for given axis combination the primary and secondary tick render information batches
	void collect_tick_geometry(int ai, int aj, const float* dom_min_pnt, const float* dom_max_pnt, const float* extent);

	/**@name font name handleing*/
	//@{
	/// store a vector with all fonts on the system
	static std::vector<const char*> font_names;
	/// concatenate font names to enum definition for dropdown control
	static std::string font_name_enum_def;
	/// ensure that font names have been enumerate
	void ensure_font_names();
	//@}

	/**@name configuration parameters of plot */
	//@{
	/// domain configuration
	domain_config dom_cfg;
	/// pointer to currently used domain config
	domain_config* dom_cfg_ptr;

	/// store one configuration per sub plot
	std::vector<plot_base_config*> configs;
	//@}

	/// store pointer to current font
	cgv::media::font::font_ptr label_font;
	/// store pointer to current font face
	cgv::media::font::font_face_ptr label_font_face;

	/// callback to change fonts
	void on_font_selection();
	/// callback to change font face
	void on_font_face_selection();
	/// set the uniforms for the i-th sub plot, overloaded by derived classes to set uniforms of derived configuration classes
	virtual void set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i = -1);
public:
	/// construct with default parameters
	plot_base(unsigned nr_axes);
	/**@name management of domain configuration */
	//@{
	/// configure the label font
	void set_label_font(float font_size, cgv::media::font::FontFaceAttributes ffa = cgv::media::font::FFA_REGULAR, const std::string& font_name = "");
	/// return const pointer to domain configuration
	const domain_config* get_domain_config_ptr() const;
	/// return pointer to domain configuration
	domain_config* get_domain_config_ptr();
	/// set the domain configuration to an external configuration in order to synch several plots, if set to null, the internal domain config is used again
	void set_domain_config_ptr(domain_config* _new_ptr);
	//@}

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


	/// ensure tick computation
	void init_frame(cgv::render::context& ctx);

	/**@name gui support*/
	//@{
	/// create the gui for the plot without gui for sub plots
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