#pragma once

#include <cgv/base/base.h>
#include <vector>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/render/drawable.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/shader_program.h>
#include <libs/cgv_gl/rectangle_renderer.h>
#include <cgv/media/color.h>
#include <cgv/media/color_scale.h>
#include <cgv/media/font/font.h>
#include <cgv/render/view.h>
#include <cgv/gui/provider.h>
#include "axis_config.h"

#include "lib_begin.h"

namespace cgv {
	namespace plot {

/// <summary>
/// enumeration of visual variables onto which additional attributes can be mapped
/// </summary>
enum VisualVariable
{
	VV_COLOR,
	VV_OPACITY,
	VV_SIZE
};

struct domain_config
{
	/// whether to show the coordinate axes including tickmarks and labels
	bool show_domain;
	/// whether to fill the domain
	bool fill;
	/// plot title
	std::string title;
	/// whether to show the plot title
	bool show_title;
	/// whether to show the sub plot names
	bool show_sub_plot_names;
	/// position of title
	vecn title_pos;
	/// color of the domain fill
	rgb color;
	/// color of the title
	rgba title_color;
	/// store size of virtual pixel based measurement
	float reference_size;
	/// store blend width in screen pixels used for antialiasing
	float blend_width_in_pixel;
	/// store a vector of axis configurations (2/3 for plot2/3d plus several attribute axes)
	std::vector<axis_config> axis_configs;
	/// store index of selected label font
	unsigned label_font_index;
	/// store selected label font size
	float label_font_size;
	/// store selected label font face attributes
	cgv::media::font::FontFaceAttributes label_ffa;
	/// store index of selected title font
	unsigned title_font_index;
	/// store selected title font size
	float title_font_size;
	/// store selected title font face attributes
	cgv::media::font::FontFaceAttributes title_ffa;
	/// set default values based on plot dimension and nr additional attribute axes
	domain_config(unsigned dim, unsigned nr_attrs);
};

/// different chart types
enum ChartType
{
	CT_POINT,
	CT_LINE_CHART,
	CT_BAR_CHART
};

struct mapped_rgb
{
	rgb color;
	int color_idx;
	mapped_rgb(const rgb& c = rgb(1,1,1)) : color(c), color_idx(-1) {}
};

struct mapped_opacity
{
	float opacity;
	int opacity_idx;
	mapped_opacity(float o = 1.0f) : opacity(o), opacity_idx(-1) {}
};

struct mapped_size
{
	float size;
	int size_idx;
	mapped_size(float s = 1.0f) : size(s), size_idx(-1) {}
};

struct mapped_rgba
{
	rgba color;
	int color_idx;
	int opacity_idx;
	mapped_rgba(const rgba& c = rgba(1, 1, 1, 1)) : color(c), color_idx(-1), opacity_idx(-1) {}
};

extern CGV_API void add_mapped_size_control(cgv::gui::provider& p, cgv::base::base* bp, const std::string& name, mapped_size& ms, std::string options = "");
extern CGV_API void add_mapped_rgb_control(cgv::gui::provider& p, cgv::base::base* bp, const std::string& name, mapped_rgb& ms);
extern CGV_API void add_mapped_rgba_control(cgv::gui::provider& p, cgv::base::base* bp, const std::string& name, mapped_rgba& ms);
extern CGV_API void add_mapped_opacity_control(cgv::gui::provider& p, cgv::base::base* bp, const std::string& name, mapped_opacity& ms);

enum SubPlotInfluence
{
	SPI_NONE = 0,
	SPI_POINT = 1,
	SPI_POINT_HALO = 2,
	SPI_LINE = 4,
	SPI_LINE_HALO = 8,
	SPI_STICK = 16,
	SPI_BAR = 32,
	SPI_BAR_OUTLINE = 64,
	SPI_ALL = 127
};

/** plot independent configuration parameters of one sub plot in a 2d or 3d plot */
struct CGV_API plot_base_config
{
	/// name of sub plot
	std::string name;

	/// offset into samples defaults to 0, if larger than end_sample vector is split into two parts
	size_t begin_sample;
	/// defaults to -1 and effectively is always the end of the sample vector
	size_t end_sample;

	/// whether to show sub plot
	bool show_plot;

	///  store bit field to define which sub plots are influenced by reference values
	SubPlotInfluence sub_plot_influence;
	/// reference color, when changed, all colors are adapted with set_colors()
	mapped_rgb ref_color;
	/// reference opacity, when changed, all opcities are adapted with set_opacity()
	mapped_opacity ref_opacity;
	/// reference size, when changed, all sizes are adapted with set_size()
	mapped_size ref_size;

	/// whether to show data points
	bool show_points;
	/// point size in pixels
	mapped_size point_size;

	/// point color
	mapped_rgba point_color;
	/// width of point halo in pixel
	mapped_size point_halo_width;
	/// color of point halo
	mapped_rgba point_halo_color;

	/// whether to connect data points with lines
	bool show_lines;
	/// line width
	mapped_size line_width;
	/// line color
	mapped_rgba line_color;
	/// width of line halo in pixel
	mapped_size line_halo_width;
	/// color of line halo
	mapped_rgba line_halo_color;

	/// whether to show straight lines to the bottom of the plot, which are called sticks
	bool show_sticks;
	/// extended stick information
	int stick_coordinate_index;
	/// base window position of stick
	float stick_base_window;
	/// line width of stick
	mapped_size stick_width;
	/// color of the stick line
	mapped_rgba stick_color;

	/// whether to show bars
	bool show_bars;
	/// extended bar information
	int bar_coordinate_index;
	///
	float bar_base_window;
	/// line width of bar outlines
	mapped_size bar_outline_width;
	/// percentual width of bar computed assuming a uniform y-sampling distance
	mapped_size bar_percentual_width;
	/// bar fill color
	mapped_rgba bar_color;
	/// bar outline color
	mapped_rgba bar_outline_color;

	/// set default values
	plot_base_config(const std::string& _name, unsigned dim);
	/// configure the sub plot to a specific chart type
	virtual void configure_chart(ChartType chart_type);
	/// set all colors from reference color
	virtual void set_colors(const rgb& base_color);
	virtual void set_color_indices(int idx);
	/// set all opacities from reference opacity
	virtual void set_opacities(float _opa);
	virtual void set_opacity_indices(int idx);
	/// set all sizes from reference size 
	virtual void set_sizes(float _size);
	virtual void set_size_indices(int idx);
	/// virtual constructor in order to allow to extend the configuration for derived classes
	virtual ~plot_base_config();
};

/// different attribute sources
enum AttributeSource
{
	AS_NONE,
	AS_SAMPLE_CONTAINER,
	AS_POINTER,
	AS_VBO
};

/// store source of a single plot attribute (one coordinate axis or one float attribute)
struct CGV_API attribute_source
{
	AttributeSource source;
	union {
		int sub_plot_index;                  // index of other subplot or -1 for current subplot
		const float* pointer;                      // pointer to external data
		const cgv::render::vertex_buffer* vbo_ptr; // pointer to vbo
	};
	size_t offset;                           // offset into vbo or coordinate axis into sample container 
	size_t count;
	size_t stride;                           // stride in all representations
	/// construct an empty attribute sources
	attribute_source();
	/// construct an attribute source referencing component ai of given sub plot
	attribute_source(int sub_plot_index, size_t ai, size_t _count, size_t _stride);
	/// constructor for source from external data
	attribute_source(const float* _pointer, size_t _count, size_t _stride);
	/// constructor for source from vbo
	attribute_source(const cgv::render::vertex_buffer* _vbo_ptr, size_t _offset, size_t _count, size_t _stride);
	/// copy constructor has no magic inside
	attribute_source(const attribute_source& as);
};

/// struct that manages attribute sources and corresponding gpu objects per subplot
struct CGV_API attribute_source_array
{
	bool samples_out_of_date;
	bool sources_out_of_date;
	size_t count;
	cgv::render::attribute_array_binding aab;
	cgv::render::vertex_buffer vbo;
	std::vector<attribute_source> attribute_sources;
	attribute_source_array();
};
/// define enum with composable legend components
enum LegendComponent
{
	LC_HIDDEN            =  0,
	LC_PRIMARY_COLOR     =  1,
	LC_SECONDARY_COLOR   =  2,
	LC_PRIMARY_OPACITY   =  4,
	LC_SECONDARY_OPACITY =  8,
	LC_PRIMARY_SIZE      = 16,
	LC_SECONDARY_SIZE    = 32,
	LC_ANY               = 63
};

struct sample_access
{
	virtual size_t size(unsigned i) const = 0;
	virtual float operator() (unsigned i, unsigned k, unsigned o) const = 0;
};

/** base class for plot2d and plot3d, which can have several sub plots each */
class CGV_API plot_base : public cgv::render::drawable, virtual public cgv::signal::tacker
{
	/**@name tick render information management */
	//@{
private:
	///
	cgv::render::shader_program legend_prog;
protected:
	cgv::render::view* view_ptr;
	/// render information stored per label
	struct label_info
	{
		vecn position;
		std::string label;
		cgv::render::TextAlignment align;
		float scale;
		label_info(const vecn& _position, const std::string& _label, cgv::render::TextAlignment _align)
			: position(_position), label(_label), align(_align), scale(1.0f) {}
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
	/// all tick labels 
	std::vector<label_info> tick_labels, legend_tick_labels;
	/// twice number of axis pairs with index of first tick label and number of tick labels for primary and secondary ticks
	std::vector<tick_batch_info> tick_batches, legend_tick_batches;
	/// depth offset of a single layer
	float layer_depth;

	/**@name font name handling*/
	//@{
	/// store a vector with all fonts on the system
	static std::vector<const char*> font_names;
	/// concatenate font names to enum definition for dropdown control
	static std::string font_name_enum_def;
public:
	/// ensure that font names have been enumerate
	void ensure_font_names();
protected:
	//@}

	/**@name configuration parameters of plot */
	//@{
	/// dimension of plot
	unsigned dim;
	/// number of additional attributes
	unsigned nr_attributes;
	/// domain configuration
	domain_config dom_cfg;
	/// pointer to currently used domain config
	domain_config* dom_cfg_ptr;
	/// store one configuration per sub plot
	std::vector<plot_base_config*> configs;
	//@}

	/**@name placement of plot*/
	//@{
	/// orientiation quaternion mapping from domain to world coordinates
	quat orientation;
	/// center location of domain in world coordinates
	vec3 center_location;
	///
	vec3 world_space_from_plot_space(const vecn& pnt_plot) const;
	/// transform from attribute space to world space
	vec3 transform_to_world(const vecn& pnt_attr) const;
	//@}
public:
	/**@name legend*/
	//@{
	/// whether to show legend
	LegendComponent legend_components;
	/// center location of legend in domain coordinates
	vec3 legend_location;
	/// width of legend
	vec2 legend_extent;
	/// coordinate direction along which to draw legend
	int legend_axis;
	/// color and opacity of legend
	rgba legend_color;
	//@}
	/**@name visual attribute mapping*/
	//@{
	/// handling of values that are out of range
	ivec4 out_of_range_mode;
	/// define maximum number of color mappings
	static const unsigned MAX_NR_COLOR_MAPPINGS = 2;
	/// index of attribute mapped to primary and secondary color
	int color_mapping[MAX_NR_COLOR_MAPPINGS];
	/// color scale indices of primary and secondary color mapping
	cgv::media::ColorScale color_scale_index[MAX_NR_COLOR_MAPPINGS];
	/// gamma adjustments for primary and secondary color mapping
	float color_scale_gamma[MAX_NR_COLOR_MAPPINGS];
	/// window space position of zero for primary and secondary color mapping
	float window_zero_position[MAX_NR_COLOR_MAPPINGS];
	
	/// define maximum number of opacity mappings
	static const unsigned MAX_NR_OPACITY_MAPPINGS = 2;
	/// index of attribute mapped to primary and secondary opacity
	int opacity_mapping[MAX_NR_OPACITY_MAPPINGS];
	/// gamma adjustments for primary and secondary opacity mapping
	float opacity_gamma[MAX_NR_OPACITY_MAPPINGS];
	/// flag whether opacity mapping is bipolar for primary and secondary opacity mapping
	bool  opacity_is_bipolar[MAX_NR_OPACITY_MAPPINGS];
	///  window space position of zero for primary and secondary opacity mapping
	float opacity_window_zero_position[MAX_NR_OPACITY_MAPPINGS];
	/// minimum opacity value for primary and secondary opacity mapping
	float opacity_min[MAX_NR_OPACITY_MAPPINGS];
	/// maximum opacity value for primary and secondary opacity mapping
	float opacity_max[MAX_NR_OPACITY_MAPPINGS];
	
	/// define maximum number of size mappings
	static const unsigned MAX_NR_SIZE_MAPPINGS = 2;
	/// index of attribute mapped to size
	int size_mapping[MAX_NR_SIZE_MAPPINGS];
	/// and independent gamma adjustments
	float size_gamma[MAX_NR_SIZE_MAPPINGS];
	/// min and max of mapped size
	float size_min[MAX_NR_SIZE_MAPPINGS], size_max[MAX_NR_SIZE_MAPPINGS];
	//@}
protected:
	/// store pointer to label font
	cgv::media::font::font_ptr label_font;
	/// store pointer to label font face
	cgv::media::font::font_face_ptr label_font_face;
	/// store pointer to title font
	cgv::media::font::font_ptr title_font;
	/// store pointer to title font face
	cgv::media::font::font_face_ptr title_font_face;
	/// vbo for legend drawing
	cgv::render::vertex_buffer vbo_legend;
	/// manage attributes for legend drawing
	cgv::render::attribute_array_binding aab_legend;
	/// attribute sources
	std::vector<attribute_source_array> attribute_source_arrays;
	///
	void on_legend_axis_change(cgv::gui::provider& p, cgv::gui::control<int>& ctrl);
	/// callback to change fonts
	void on_font_selection();
	/// callback to change font face
	void on_font_face_selection();
	/// extents used for drawing current 
	vec3 extent;
	/// prepare extents for drawing
	void prepare_extents();
	/// set the uniforms for plot configurations
	void set_plot_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog);
	/// set the uniforms for defining the mappings to visual variables
	void set_mapping_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog);
private:
	/// dimension independent implementation of attribute enabling
	size_t enable_attributes(cgv::render::context& ctx, int i, const sample_access& sa);
protected:
	/// render style of rectangles
	cgv::render::rectangle_render_style rrs, font_rrs;
	cgv::render::attribute_array_manager aam_legend, aam_legend_ticks, aam_title;
	///
	void draw_rectangles(cgv::render::context& ctx, cgv::render::attribute_array_manager& aam, 
		std::vector<box2>& R, std::vector<rgb>& C, std::vector<float>& D, size_t offset = 0);
	///
	void draw_tick_labels(cgv::render::context& ctx, cgv::render::attribute_array_manager& aam_ticks, 
		std::vector<label_info>& tick_labels, std::vector<tick_batch_info>& tick_batches, float depth);
	/// set vertex shader input attributes based on attribute source information
	size_t enable_attributes(cgv::render::context& ctx, int i, const std::vector<std::vector<vec2>>& samples);
	/// set vertex shader input attributes based on attribute source information
	size_t enable_attributes(cgv::render::context& ctx, int i, const std::vector<std::vector<vec3>>& samples);
	/// 
	void disable_attributes(cgv::render::context& ctx, int i);
	///
	void update_samples_out_of_date_flag();
	///
	virtual bool compute_sample_coordinate_interval(int i, int ai, float& samples_min, float& samples_max) = 0;
	///
	void draw_sub_plot_samples(int count, const plot_base_config& spc, bool strip = false);
	///
	void draw_title(cgv::render::context& ctx, vec2 pos, float depth, int si = -1);
	///
	void draw_legend(cgv::render::context& ctx, int layer_idx = 0, bool is_first = true, bool* multi_axis_modes = 0);
	///
	bool extract_tick_rectangles_and_tick_labels(
		std::vector<box2>& R, std::vector<rgb>& C, std::vector<float>& D,
		std::vector<label_info>& tick_labels, int ai, int ci, int ti, float he, 
		float z_plot, float plot_scale = 1.0f, vec2 plot_offset = vec2(0.0f,0.0f), float d = 0.0f, bool multi_axis = true);
	///
	void extract_legend_tick_rectangles_and_tick_labels(
		std::vector<box2>& R, std::vector<rgb>& C, std::vector<float>& D,
		std::vector<label_info>& tick_labels, std::vector<tick_batch_info>& tick_batches, float d, 
		bool clear_cache = false, bool is_first = true, bool* multi_axis_modes = 0);

public:
	/// construct from plot dimension and number of additional attributes with default parameters
	plot_base(unsigned dim, unsigned nr_attributes = 0);
	/// return nr dimensions of plot
	unsigned get_dim() const { return dim; }
	/// set the view ptr
	void set_view_ptr(cgv::render::view* _view_ptr);
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

	/**@name configure domain to world transform*/
	//@{
	/// return 2d domain shown in plot
	const box2 get_domain() const;
	/// return 3d domain shown in plot
	const box3 get_domain3() const;
	/// set the domain for 2d plots
	void set_domain(const box2& dom);
	/// set the domain for 3d plots
	void set_domain3(const box3& dom);
	/// set the plot extend in 2D coordinates
	void set_extent(const vecn& new_extent);
	/// query the plot extend in 2D coordinates
	vecn get_extent() const;
	//! set extent_scaling values for all axes
	/*! for all axes with extent_scaling > 0 (axes values default to 0) ensure that world extents 
	    are in same ratio as extent_scaling values */
	void set_extent_scaling(float x_scale, float y_scale, float z_scale = 0);
	/// set the plot width to given value and if constrained == true the height, such that the aspect ration is the same as the aspect ratio of the domain
	void set_width(float new_width, bool constrained = true);
	/// set the plot height to given value and if constrained == true the width, such that the aspect ration is the same as the aspect ratio of the domain
	void set_height(float new_height, bool constrained = true);
	/// set new orientation quaternion
	void set_orientation(const quat& _orientation);
	/// place the origin of the plot in 3D to the given location
	void place_origin(const vec3& new_origin_location);
	/// place the plot extent center in 3D to the given location (this might can change the current origin location) 
	void place_center(const vec3& new_center_location);
	/// place a corner (0 .. lower left, 1 .. lower right, 2 .. upper left, 3 .. upper right) to a given 3D location ((this might can change the current origin / center location) 
	void place_corner(unsigned corner_index, const vec3& new_corner_location);
	/// return the current origin in 3D coordinates
	vec3 get_origin() const;
	/// get current orientation quaternion
	const quat& get_orientation() const;
	/// return the current plot center in 3D coordinates
	const vec3& get_center() const;
	/// return the i-th plot corner in 3D coordinates
	vec3 get_corner(unsigned i) const;
	/// return true world direction of x, y or z axis
	const vec3 get_axis_direction(unsigned ai) const;
	//@}

	/**@name helper functions to adjust axes*/
	//@{
	/// adjust the domain with respect to \c ai th axis to the i-th subplot
	bool determine_axis_extent_from_subplot(unsigned ai, unsigned i, float& sample_min, float& sample_max);
	/// adjust the domain with respect to \c ai th axis to the visible or all data depending on last parameter
	void adjust_domain_axis_to_data(unsigned ai, bool adjust_min = true, bool adjust_max = true, bool only_visible = true);
	/// adjust selected axes of domain to the visible or all data depending on last parameter
	void adjust_domain_to_data(bool only_visible = true);
	/// extend domain such that given axis is included
	void include_axis_to_domain(unsigned ai);
	/// adjust tick marks of all axes based on maximum number of secondary ticks and domain min and max in coordinate of axis
	void adjust_tick_marks(unsigned max_nr_secondary_ticks = 20, bool adjust_to_attribute_ranges = true);
	/// adjust the extent such that it has same aspect ration as domain
	void adjust_extent_to_domain_aspect_ratio(int preserve_ai = 0);
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
	/// notify plot that samples of given subplot are out of date
	void set_samples_out_of_date(unsigned i);
	/// set the colors for all plot features of the i-th sub plot as variation of the given color
	void set_sub_plot_colors(unsigned i, const rgb& base_color);
	/// define a sub plot attribute ai from coordinate aj of the i-th internal sample container
	void set_sub_plot_attribute(unsigned i, unsigned ai, int subplot_index, size_t aj);
	/// define a sub plot attribute from an external pointer
	void set_sub_plot_attribute(unsigned i, unsigned ai, const float* _pointer, size_t count, size_t stride);
	/// define a sub plot attribute from a vbo (attribute must be stored in float type in vbo)
	void set_sub_plot_attribute(unsigned i, unsigned ai, const cgv::render::vertex_buffer* _vbo_ptr, size_t _offset, size_t _count, size_t _stride);
	//@}

	/// build legend prog and create aab
	bool init(cgv::render::context& ctx);
	/// destruct shader programs
	void clear(cgv::render::context& ctx);

	/**@name gui support*/
	//@{
protected:
	void update_ref_opacity(unsigned i, cgv::gui::provider& p);
	void update_ref_opacity_index(unsigned i, cgv::gui::provider& p);
	void update_ref_size(unsigned i, cgv::gui::provider& p);
	void update_ref_size_index(unsigned i, cgv::gui::provider& p);
	void update_ref_color(unsigned i, cgv::gui::provider& p);
	void update_ref_color_index(unsigned i, cgv::gui::provider& p);

public:
	/// create the gui for the plot without gui for sub plots
	virtual void create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p);
	/// create the gui for base subplot settings
	virtual void create_base_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);
	/// create the gui for a point subplot
	virtual void create_point_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	/// create the gui for a line subplot
	virtual void create_line_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	/// create the gui for a stick subplot
	virtual void create_stick_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	/// create the gui for a bar subplot
	virtual void create_bar_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc);
	/// create the gui for a subplot configuration
	virtual void create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i);
	/// create a gui for the plot with gui for all configs
	virtual void create_gui(cgv::base::base* bp, cgv::gui::provider& p);
	//@}
};

	}
}

#include <cgv/config/lib_end.h>