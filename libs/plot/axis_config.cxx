#include "axis_config.h"

namespace cgv {
	namespace plot {

/// implement equality check
bool tick_config::operator == (const tick_config& tc) const
{
	return length == tc.length && step == tc.step && type == tc.type &&	label == tc.label && precision == tc.precision;
}

/// set tick config defaults
tick_config::tick_config(bool primary)
{
	step = primary ? 5.0f : 1.0f;
	type = primary ? TT_LINE : TT_DASH;
	line_width = primary ? 1.5f : 1.0f;
	length = primary ? 12.0f : 8.0f;
	label = primary;
	precision = -1;
}

void axis_config::update_tick_range() 
{
	min_tick_value = tick_space_from_attribute_space(min_attribute_value);
	max_tick_value = tick_space_from_attribute_space(max_attribute_value);
	if (auto_adjust_max_snd_ticks > 0)
		adjust_tick_marks_to_range(auto_adjust_max_snd_ticks);
}

void axis_config::set_attribute_range(float _min, float _max)
{
	min_attribute_value = _min;
	max_attribute_value = _max;
	update_tick_range();
}

/// store current range in backup members
void axis_config::backup_attribute_range()
{
	min_attribute_value_backup = min_attribute_value;
	max_attribute_value_backup = max_attribute_value;
}

/// read access to backup attribute range
void axis_config::put_backup_attribute_range(float& min_val, float& max_val) const
{
	min_val = min_attribute_value_backup;
	max_val = max_attribute_value_backup;
}

/// store current range from backup members
void axis_config::restore_attribute_range()
{
	set_attribute_range(min_attribute_value_backup, max_attribute_value_backup);
}

void axis_config::set_attribute_minimum(float _min)
{
	min_attribute_value = _min;
	update_tick_range();
}
void axis_config::set_attribute_maximum(float _max)
{
	max_attribute_value = _max;
	update_tick_range();
}
void axis_config::set_log_scale(bool enabled)
{
	log_scale = enabled;
	update_tick_range();
}
void axis_config::set_log_minimum(float _min) 
{
	log_minimum = _min; 
	update_tick_range(); 
}

/// write access to log scale flag and minimum
void axis_config::set_log_config(bool enabled, float _min)
{
	log_scale = enabled;
	log_minimum = _min;
	update_tick_range();
}

float axis_config::tick_space_from_attribute_space(float value) const
{
	if (!log_scale)
		return value;
	if (value > log_minimum)
		return log10(value);
	if (value < -log_minimum)
		return 2 * log10(log_minimum) - 2 - log10(-value);
	return log10(log_minimum) + value / log_minimum - 1;
}
float axis_config::attribute_space_from_tick_space(float value) const
{
	if (!log_scale)
		return value;
	if (value > log10(log_minimum))
		return pow(10.0f, value);
	if (value < log10(log_minimum) - 2)
		return -pow(10.0f, 2 * log10(log_minimum) - 2 - value);
	return log_minimum * (value + 1 - log10(log_minimum));
}
float axis_config::window_space_from_tick_space(float value) const
{
	float min_value = min_attribute_value;
	float max_value = max_attribute_value;
	if (log_scale) {
		min_value = tick_space_from_attribute_space(min_value);
		max_value = tick_space_from_attribute_space(max_value);
	}
	return (value - min_value) / (max_value - min_value);
}
float axis_config::tick_space_from_window_space(float value) const
{
	float min_value = min_attribute_value;
	float max_value = max_attribute_value;
	if (log_scale) {
		min_value = tick_space_from_attribute_space(min_value);
		max_value = tick_space_from_attribute_space(max_value);
	}
	return value* (max_value - min_value) + min_value;
}
float axis_config::plot_space_from_window_space(float value) const
{
	return extent * (value - 0.5f);
}
float axis_config::window_space_from_plot_space(float value) const
{
	return value / extent + 0.5f;
}
float axis_config::plot_space_from_attribute_space(float value) const
{
	return plot_space_from_window_space(window_space_from_tick_space(tick_space_from_attribute_space(value)));
}
float axis_config::attribute_space_from_plot_space(float value) const
{
	return attribute_space_from_tick_space(tick_space_from_window_space(window_space_from_plot_space(value)));
}

void test_axis_config()
{
	axis_config ac;
	ac.set_attribute_range(-2,10);
	ac.set_log_config(true, 0.01f);
	float l, v = 10;
	int i;
	for (i = 0; i < 16; ++i) {
		l = ac.tick_space_from_attribute_space(v);
		std::cout << v << " -> " << l << " -> " << ac.attribute_space_from_tick_space(l) << std::endl;
		v *= 0.5;
	}
	v = 0;
	l = ac.tick_space_from_attribute_space(v);
	std::cout << v << " -> " << l << " -> " << ac.attribute_space_from_tick_space(l) << std::endl;
	v = -2;
	for (i = 0; i < 16; ++i) {
		l = ac.tick_space_from_attribute_space(v);
		std::cout << v << " -> " << l << " -> " << ac.attribute_space_from_tick_space(l) << std::endl;
		v *= 0.5;
	}
}
bool axis_config::operator == (const axis_config& ac) const
{
	return log_scale == ac.log_scale && log_minimum == ac.log_minimum &&
		primary_ticks == ac.primary_ticks && secondary_ticks == ac.secondary_ticks;
}

axis_config::axis_config() : primary_ticks(true), secondary_ticks(false), color(0.3f,0.3f,0.3f)
{
	min_attribute_value = 0.0f;
	max_attribute_value = 1.0f;
	log_scale = false;
	log_minimum = 1e-10f;
	update_tick_range();
	extent = 1.0f;
	extent_scaling = 0.0f;
	line_width = 3.0f;
	auto_adjust_max_snd_ticks = 20;
	multi_axis_ticks = true;
}
/// adjust tick marks to attribute range based on given maximum number of secondary ticks
void axis_config::adjust_tick_marks_to_range(unsigned max_nr_secondary_ticks)
{
	float mn = tick_space_from_attribute_space(get_attribute_min());
	float mx = tick_space_from_attribute_space(get_attribute_max());
	float de = mx - mn;
	float reference_step = de / max_nr_secondary_ticks;
	float scale = (float)pow(10, -floor(log10(reference_step)));
	primary_ticks.step = 50.0f / scale;
	secondary_ticks.step = 10.0f / scale;
	static float magic_numbers[9] = {
		1.5f,  5.0f, 1.0f,
		3.5f, 10.0f, 2.0f,
		7.5f, 20.0f, 5.0f
	};
	for (unsigned i = 0; i < 9; i += 3)
		if (scale * reference_step < magic_numbers[i]) {
			primary_ticks.step = magic_numbers[i + 1] / scale;
			secondary_ticks.step = magic_numbers[i + 2] / scale;
			break;
		}
}

void axis_config::create_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	bool show = p.begin_tree_node(name, color, false, "level=3;options='w=148';align=' '");
	connect_copy(p.add_member_control(bp, "Log", log_scale, "toggle", "w=40")->value_change,
		cgv::signal::rebind(this, &axis_config::update_tick_range));
	if (show) {
		p.align("\a");
		p.add_member_control(bp, "Extent", extent, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		p.add_member_control(bp, "Extent Scaling", extent_scaling, "value_slider", "min=0;max=10;log=true;ticks=true");
		connect_copy(p.add_member_control(bp, "Attribute Min", min_attribute_value, "value_slider", "min=-10;max=10;log=true;ticks=true")->value_change,
			cgv::signal::rebind(this, &axis_config::update_tick_range));
		connect_copy(p.add_member_control(bp, "Attribute Max", max_attribute_value, "value_slider", "min=-10;max=10;log=true;ticks=true")->value_change,
			cgv::signal::rebind(this, &axis_config::update_tick_range));
		connect_copy(p.add_member_control(bp, "Log Minimum", log_minimum, "value")->value_change,
			cgv::signal::rebind(this, &axis_config::update_tick_range));
		p.add_member_control(bp, "Width", line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
		p.add_member_control(bp, "Color", color);
		connect_copy(p.add_member_control(bp, "Max 2nd Ticks", auto_adjust_max_snd_ticks, "value_slider", "min=0;max=50;log=true;ticks=true")->value_change,
			cgv::signal::rebind(this, &axis_config::update_tick_range));

		p.add_member_control(bp, "Multi Axis Ticks", multi_axis_ticks, "check");

		const char* tn[2] = { "Primary Tick", "Secondary Tick" };
		tick_config* tc[2] = { &primary_ticks, &secondary_ticks };
		for (unsigned ti = 0; ti < 2; ++ti) {
			bool vis = p.begin_tree_node(tn[ti], tc[ti]->label, false, "level=3;options='w=132';align=' '");
			p.add_member_control(bp, "Label", tc[ti]->label, "toggle", "w=60");
			if (vis) {
				p.align("\a");
				p.add_member_control(bp, "Type", tc[ti]->type, "dropdown", "enums='None,Dash,Line,Plane'");
				p.add_member_control(bp, "Step", tc[ti]->step, "value");
				p.add_member_control(bp, "Width", tc[ti]->line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
				p.add_member_control(bp, "Length", tc[ti]->length, "value_slider", "min=1;max=20;log=true;ticks=true");
				p.add_member_control(bp, "Precision", tc[ti]->precision, "value_slider", "min=-1;max=5;ticks=true");
				p.align("\b");
				p.end_tree_node(tc[ti]->label);
			}
		}
		p.align("\b");
		p.end_tree_node(color);
	}
}

	}
}
