#pragma once

#include <vector>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/media/color.h>
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
struct CGV_API tick_config
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
	/// implement equality check
	bool operator == (const tick_config& tc) const;
	/// set tick config defaults
	tick_config(bool primary);
};

/// configuration information stored per domain axis
class CGV_API axis_config
{
private:
	/// minimum tick space value
	float min_tick_value;
	/// maximum tick space value
	float max_tick_value;
	///
	void update_tick_range();
	///
	float min_attribute_value_backup, max_attribute_value_backup;
protected:
	/// minimum attribute value
	float min_attribute_value;
	/// maximum attribute value
	float max_attribute_value;
	/// whether axis is drawn with logarithmic scale
	bool log_scale;
	/// minimum of logarithmic value in case that 0 is included in attribute range
	float log_minimum;
	/// maximum number of secondary ticks for auto adjustment of ticks on changes to attribute range
	unsigned auto_adjust_max_snd_ticks;
public:
	/// name of axis
	std::string name;
	/// read access to attrbribute minimum value
	float get_attribute_min() const { return min_attribute_value; }
	/// read access to attrbribute maximum value
	float get_attribute_max() const { return max_attribute_value; }
	/// read access to backup attribute range
	void put_backup_attribute_range(float& min_val, float& max_val) const;
	/// compute the extent in attribute space
	float get_attribute_extent() const { return get_attribute_max() - get_attribute_min(); }
	/// read access to log scale flag
	bool get_log_scale() const { return log_scale; }
	/// read access to minimum attribute value for robust log transformation
	float get_log_minimum() const { return log_minimum; }
	/// write access to attribute range
	void set_attribute_range(float _min, float _max);
	/// store current range in backup members
	void backup_attribute_range();
	/// store current range from backup members
	void restore_attribute_range();
	/// write access to attribute minimum value
	void set_attribute_minimum(float _min);
	/// write access to attribute maximum value
	void set_attribute_maximum(float _max);
	/// write access to log scale flag
	void set_log_scale(bool enabled);
	/// write access to minimum attribute value for robust log transformation
	void set_log_minimum(float _min);
	/// write access to log scale flag and minimum
	void set_log_config(bool enabled, float _min);
	/// extent in world space
	float extent;
	//! potential constraint for scaling of plot extent in world space
	/*! if 0, axis scaling is not constraint (default)
	    if > 0 ensure that world extents of all axes where constraint is > 0 are in same ratio as extent_scaling values */
	float extent_scaling;
	/// line width
	float line_width;
	/// color of axis
	rgb color;
	/// whether to show tick marks on both axes boundaries
	bool multi_axis_ticks;
	/// configuration of primary tickmarks
	tick_config primary_ticks;
	/// configuration of secondary tickmarks
	tick_config secondary_ticks;
	/// compare members relevant for drawing geometry precomputation for equality 
	bool operator == (const axis_config& ac) const;
	/// perform transformation from attribute space to tick mark space by applying log transformation if activated
	float tick_space_from_attribute_space(float value) const;
	///
	float window_space_from_tick_space(float value) const;
	///
	float tick_space_from_window_space(float value) const;
	/// 
	float attribute_space_from_tick_space(float value) const;
	/// convert from window to plot space
	float plot_space_from_window_space(float value) const;
	/// convert from plot to window space
	float window_space_from_plot_space(float value) const;
	/// convenience function
	float plot_space_from_attribute_space(float value) const;
	/// convenience function
	float attribute_space_from_plot_space(float value) const;
	/// set default values
	axis_config();
	/// adjust tick marks to attribute range based on given maximum number of secondary ticks
	void adjust_tick_marks_to_range(unsigned max_nr_secondary_ticks);
	/// create gui for axis
	void create_gui(cgv::base::base* bp, cgv::gui::provider& p);
};

	}
}

#include <cgv/config/lib_end.h>