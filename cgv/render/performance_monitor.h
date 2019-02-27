#pragma once

#include <deque>
#include <vector>
#include <cgv/utils/stopwatch.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/media/color.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/// different items that can be shown in the long bar
enum PerformanceMonitoringBar {
	PMB_MIN,
	PMB_MAX,
	PMB_CUR,
	PMB_AVG
};

struct CGV_API performance_measurement
{
	double time;
	int task_id;
	bool start;
	performance_measurement(double _time, int _task_id, bool _start) : time(_time), task_id(_task_id), start(_start) {}
};

struct CGV_API performance_task
{
	std::string name;
	cgv::media::color<float> col;
	performance_task(std::string _name, cgv::media::color<float> _col) : name(_name), col(_col) {}
};

/** This class allows to monitor the performance of a set of tasks that are repeatedly 
    executed over time. Each repetition is called a frame. The performance monitor 
	supports storage of performance information in a file and prepares everything for
	rendering performance measurements, where the actually rendering is implemented in
	libs/cgv_gl/gl/gl_performance_monitor. */
class CGV_API performance_monitor
{
public:
	typedef cgv::math::fvec<int,2> Pos;
	typedef cgv::media::axis_aligned_box<int,2> Rec;
	typedef cgv::media::color<float> Col;
	typedef std::vector<performance_measurement> frame_data;
private:
	bool frame_finished;
	unsigned frame_id;
protected:
	bool enabled;
	double fps_alpha;
	double fps;
	Rec placement;
	int nr_display_cycles;
	int bar_line_width;
	Col plot_color;
	std::string file_name;
	std::vector<performance_task> tasks;
	std::deque<frame_data> data;
	std::vector<PerformanceMonitoringBar> bar_config;

	cgv::utils::stopwatch watch;
	float time_scale;
	std::vector<Pos> positions;
	std::vector<Col> colors;

	void compute_colors(const frame_data& fdata);
	void compute_positions(int x, int y, int dx, int dy, const frame_data& fdata);
	frame_data& current_frame();
	void add_measurement(const performance_measurement& pm);

public:
	/// construct performance monitor with standard configuration
	performance_monitor();
	/**@name  file io configuration */
	//@{
	/// 
	void set_file_name(const std::string& _file_name);
	///
	const std::string& get_file_name() const { return file_name; }
	/// enable performance monitoring
	void enable();
	/// disable performance monitoring
	void disable();
	/// return whether performance monitoring is enabled
	bool is_enabled() const { return enabled; }
	//@}

	/**@name handling of tasks and frames*/
	//@{
	/// initialize the list of tasks to one for the frame and one for each default rendering pass
	void init_tasks();
	/// add a new to me monitored item
	int add_task(const std::string& name, const Col& col);
	/// return the size of the buffer to store frame data
	unsigned get_buffer_size() const { return placement.get_extent()(0); }
	/// start performance measurement of a new frame 
	void start_frame();
	/// return the current frame id
	unsigned get_frame_id() const { return frame_id; }
	/// add a measurement for starting given task
	void start_task(int task_id);
	/// add a measurement for finishing given task
	void finish_task(int task_id);
	/// finish measurement of a frame, if this is not called by hand, it is called automatically in the next call to start_frame.
	void finish_frame();
	//@}

	/**@name drawing configuration */
	//@{
	/// removes all items of the bar config and hides the bar
	void clear_bar();
	/// add a bar item to the bar config
	void add_bar_item(PerformanceMonitoringBar item);
	/// place the performance monitor on screen in pixel coordinates
	void set_placement(const Rec& rectangle);
	/// return placement of performance monitor
	Rec get_placement() const { return placement; }
	/// set the number of display cycles to by drawn for the performance monitor
	void set_nr_display_cycles(unsigned _nr_cycles);
	/// return the number of display cycles to by drawn for the performance monitor
	unsigned get_nr_display_cycles() const { return nr_display_cycles; }
	//@}
};

	}
}

#include <cgv/config/lib_end.h>