#include "performance_monitor.h"
#include <cgv/utils/file.h>
#include <stack>
#include <stdio.h>

namespace cgv {
	namespace render {

performance_monitor::frame_data& performance_monitor::current_frame()	
{
	return data.back(); 
}

void performance_monitor::add_measurement(const performance_measurement& pm)
{
	current_frame().push_back(pm); 
}

performance_monitor::performance_monitor() : plot_color(0.3f,1,1)
{
	fps = -1;
	fps_alpha = 0.1;
	time_scale = 60;
	enabled = true;
	frame_finished = true;
	placement.ref_min_pnt().set(10,10);
	placement.ref_max_pnt().set(310,110);
	nr_display_cycles = 2;
	bar_line_width = 5;
	frame_id = 0;
	init_tasks();
	bar_config.push_back(PMB_MAX);
	bar_config.push_back(PMB_CUR);
	bar_config.push_back(PMB_MIN);
}

/// enable performance monitoring
void performance_monitor::enable() 
{
	enabled = true;
}
/// disable performance monitoring
void performance_monitor::disable() 
{
	enabled = false; 
}


void performance_monitor::set_file_name(const std::string& _file_name) 
{
	file_name = _file_name; 
}


/// initialize the list of tasks to one for the frame and one for each default rendering pass
void performance_monitor::init_tasks()
{
	tasks.clear();
	add_task("frame", Col(0.5f,0.5f,0.5f));
	add_task("main", Col(0.8f,0.2f,0.2f));
	add_task("stereo", Col(0.2f,0.2f,0.8f));
	add_task("shadow_map", Col(0.1f, 0.1f, 0.1f));
	add_task("shadow_volume", Col(0.2f, 0.2f, 0.2f));
	add_task("opaque_surface", Col(1,1,0));
	add_task("transparent_surfaces", Col(0.9f,0.7f,0.5f));
	add_task("pick", Col(0, 0, 1));
	add_task("user", Col(0, 1, 0));
}

/// removes all items of the bar config and hides the bar
void performance_monitor::clear_bar() 
{
	bar_config.clear(); 
}
/// add a bar item to the bar config
void performance_monitor::add_bar_item(PerformanceMonitoringBar item)
{
	bar_config.push_back(item);
}

/// place the performance monitor on screen in pixel coordinates
void performance_monitor::set_placement(const Rec& rectangle) 
{
	placement = rectangle;
}

/// set the number of display cycles to by drawn for the performance monitor
void performance_monitor::set_nr_display_cycles(unsigned _nr_cycles) 
{
	nr_display_cycles = _nr_cycles; 
}

/// add a new to me monitored item
int performance_monitor::add_task(const std::string& name, const cgv::media::color<float>& col)
{
	performance_task pt(name,col);
	tasks.push_back(pt);
	return (int)tasks.size() - 1;
}
/// start performance measurement of a new frame 
void performance_monitor::start_frame()
{
	if (!enabled)
		return;
	if (!frame_finished)
		finish_frame();

	while (data.size() >= get_buffer_size())
		data.pop_front();
	data.push_back(frame_data());
	watch.restart();
	performance_measurement pm(watch.get_elapsed_time(), 0, true);
	add_measurement(pm);
	frame_finished = false;
	++frame_id;
}

///
void performance_monitor::start_task(int task_id)
{
	if (!enabled)
		return;
	performance_measurement pm(watch.get_elapsed_time(), task_id, true);
	add_measurement(pm);
}

void performance_monitor::finish_task(int task_id)
{
	if (!enabled)
		return;
	performance_measurement pm(watch.get_elapsed_time(), task_id, false);
	add_measurement(pm);
}
/// finish measurement of a frame, if this is not called by hand, it is called automatically in the next call to start_frame.
void performance_monitor::finish_frame()
{
	if (!enabled)
		return;
	const char* start_or_finish[] = { "start", "finish" };
	performance_measurement pm(watch.get_elapsed_time(), 0, false);
	add_measurement(pm);
	double new_fps = 1.0 / (data.back().back().time - data.back().front().time);
	if (fps < 0)
		fps = new_fps;
	else
		fps = fps_alpha * new_fps + (1.0 - fps_alpha)*fps;
	frame_finished = true;
	if (file_name.empty())
		return;
	bool need_header = cgv::utils::file::exists(file_name);
	FILE* fp = fopen(file_name.c_str(), "wa");
	if (!fp)
		return;
	int i;
	if (need_header) {
		const frame_data& cf = current_frame();
		for (i=0; i<(int)cf.size(); ++i) {
			const performance_measurement& pm = cf[i];
			fprintf(fp, i==0?"%s %s":",%s %s", start_or_finish[pm.start ? 0 : 1], tasks[pm.task_id].name.c_str());
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "%d", frame_id);
	for (i=0; i<(int)current_frame().size(); ++i) 
		fprintf(fp, ", %f", current_frame()[i].time);
	fprintf(fp, "\n");
	fclose(fp);
}

void performance_monitor::compute_colors(const frame_data& fdata)
{
	colors.resize(2*fdata.size()-2);
	std::stack<int> task_stack;
	task_stack.push(0);
	for (unsigned t=0; t<fdata.size()-1; ++t) {
		int task_id = fdata[t].task_id;
		if (fdata[t].start) {
			colors[2*t+1] = colors[2*t] = tasks[task_id].col;
			task_stack.push(task_id);
		}
		else {
			task_stack.pop();
			colors[2*t+1] = colors[2*t] = tasks[task_stack.top()].col;
		}
	}
}

void performance_monitor::compute_positions(int x0, int y0, int dx, int dy, const frame_data& fdata)
{
	positions.resize(2*fdata.size()-2);
	double scale_x = time_scale*dx;
	double scale_y = time_scale*dy;
	int x = x0, y = y0;
	for (unsigned t=0; t < fdata.size()-1; ++t) {
		positions[2*t].set(x,y);
		x = x0 + (int)(fdata[t+1].time*scale_x+0.5);
		y = y0 + (int)(fdata[t+1].time*scale_y+0.5);
		positions[2*t+1].set(x,y);
	}
}

	}
}