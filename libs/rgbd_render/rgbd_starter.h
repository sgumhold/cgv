#pragma once

#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <rgbd_capture/rgbd_input.h>

#include "lib_begin.h"

namespace rgbd {

extern CGV_API std::string get_device_enum(const std::string& prepend = "");

extern CGV_API std::string get_stream_format_enum(const std::vector<rgbd::stream_format>& sfs);

extern CGV_API std::string get_component_format(const rgbd::frame_format& ff);

class CGV_API rgbd_starter_base
{
protected:
	rgbd::rgbd_input rgbd_inp;
	rgbd::rgbd_calibration calib;
	int device_idx = -1;
	std::string prepend_device_enum = "detached=-1,";

	bool near_mode = true;
	float pitch = 0.0f;
	rgbd::MultiDeviceRole multi_device_role = rgbd::MDR_STANDALONE;

	std::vector<rgbd::stream_format> color_stream_formats;
	std::vector<rgbd::stream_format> depth_stream_formats;
	std::vector<rgbd::stream_format> ir_stream_formats;

	int color_stream_format_idx = -1;
	int depth_stream_format_idx = -1;
	int    ir_stream_format_idx = -1;

	bool attached = false;
	InputStreams is = IS_COLOR_AND_DEPTH;
	bool one_shot = false;
	bool is_running = false;

	unsigned nr_depth_frames = 0, nr_color_frames = 0, nr_infrared_frames = 0;
	bool color_frame_changed = false, depth_frame_changed = false, infrared_frame_changed = false;
	/// internal members used for data storage
	IMU_measurement imu;
	rgbd::frame_type color_frame, depth_frame, ir_frame;

	void update_stream_formats(cgv::gui::provider& p);
	// overload callbacks to deal with different events
	virtual void on_attach() {}
	virtual void on_start() {}
	virtual void on_new_frame(double t, InputStreams new_frames) {}
	virtual void on_stop() {}
public:
	void on_set_base(void* member_ptr, cgv::gui::provider& p);
	void timer_event_base(double t, double dt, cgv::gui::provider& p);
	void create_gui_base(cgv::base::base* bp, cgv::gui::provider& p);
};

template <class B>
class rgbd_starter : public B, public rgbd_starter_base, public cgv::gui::provider
{
	cgv::gui::trigger my_trigger;
public:
	typedef typename rgbd_starter<B> base_type;
	rgbd_starter() { 
		connect(my_trigger.shoot, this, &base_type::timer_event); 
		my_trigger.schedule_recuring(0.001);
		//	connect(cgv::gui::get_animation_trigger().shoot, this, &base_type::timer_event); 
	}
	bool start_first_device() {
		unsigned nr = rgbd_input::get_nr_devices();
		if (nr == 0)
			return false;
		device_idx = 0;
		on_set(&device_idx);
		is_running = true;
		on_set(&is_running);
		return is_running;
	}
	void on_set(void* member_ptr) { on_set_base(member_ptr, *this); update_member(member_ptr); }
	void timer_event(double t, double dt) { timer_event_base(t, dt, *this);  }
	void create_gui() {
		add_decorator("rgbd_starter", "heading", "level=1");
		create_gui_base(this, *this); 
	}
};

}
#include <cgv/config/lib_end.h>