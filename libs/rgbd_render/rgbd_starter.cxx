#include "rgbd_starter.h"
#include <cgv/utils/convert_string.h>
#include <cgv/utils/pointer_test.h>
#include <cgv/gui/dialog.h>

namespace rgbd {

std::string get_device_enum(const std::string& prepend)
{
	unsigned n = rgbd_input::get_nr_devices();
	std::string device_def = "enums='";
	device_def += prepend;
	for (unsigned i = 0; i < n; ++i) {
		if (i > 0)
			device_def += ",";
		device_def += rgbd_input::get_serial(i);
	}
	device_def += "'";
	return device_def;
}
std::string get_stream_format_enum(const std::vector<rgbd::stream_format>& sfs)
{
	std::string enum_def = "enums='default=-1";
	for (const auto& sf : sfs) {
		enum_def += ",";
		enum_def += cgv::utils::to_string(sf);
	}
	return enum_def + "'";
}
std::string get_component_format(const rgbd::frame_format& ff)
{
	std::string fmt_descr("uint");
	switch (ff.pixel_format) {
	case PF_I: // infrared
	case PF_DEPTH:
	case PF_DEPTH_AND_PLAYER:
	case PF_CONFIDENCE:
		fmt_descr += cgv::utils::to_string(ff.nr_bits_per_pixel) + "[L]";
		break;
	case PF_RGB:   // 24 or 32 bit rgb format with byte alignment
		fmt_descr += ff.nr_bits_per_pixel == 24 ? "8[R,G,B]" : "8[R,G,B,A]";
		break;
	case PF_BGR:   // 24 or 24 bit bgr format with byte alignment
		fmt_descr += ff.nr_bits_per_pixel == 24 ? "8[B,G,R]" : "8[B,G,R,A]";
		break;
	case PF_RGBA:  // 32 bit rgba format
		fmt_descr += "8[R,G,B,A]";
		break;
	case PF_BGRA:  // 32 bit bgra format
		fmt_descr += "8[B,G,R,A]";
		break;
	case PF_BAYER: // 8 bit raw bayer pattern values
		fmt_descr += "8[L]";
		break;
	}
	return fmt_descr;
}

void rgbd_starter_base::update_stream_formats(cgv::gui::provider& p)
{
	color_stream_formats.clear();
	rgbd_inp.query_stream_formats(IS_COLOR, color_stream_formats);
	if (p.find_control(color_stream_format_idx))
		p.find_control(color_stream_format_idx)->multi_set(get_stream_format_enum(color_stream_formats));
	depth_stream_formats.clear();
	rgbd_inp.query_stream_formats(IS_DEPTH, depth_stream_formats);
	if (p.find_control(depth_stream_format_idx))
		p.find_control(depth_stream_format_idx)->multi_set(get_stream_format_enum(depth_stream_formats));
	ir_stream_formats.clear();
	rgbd_inp.query_stream_formats(IS_INFRARED, ir_stream_formats);
	if (p.find_control(ir_stream_format_idx))
		p.find_control(ir_stream_format_idx)->multi_set(get_stream_format_enum(ir_stream_formats));
}
void rgbd_starter_base::on_set_base(void* member_ptr, cgv::gui::provider& p)
{
	cgv::utils::pointer_test pt(member_ptr);
	if (pt.is(is_running)) {
		if (is_running) {
			rgbd_inp.set_near_mode(near_mode);
			bool use_default = false;
			std::vector<stream_format> sfs;
			if ((is & IS_COLOR) == IS_COLOR)
				if (!(use_default = use_default || color_stream_format_idx == -1))
					sfs.push_back(color_stream_formats[color_stream_format_idx]);
			if ((is & IS_DEPTH) == IS_DEPTH)
				if (!(use_default = use_default || depth_stream_format_idx != -1))
					sfs.push_back(depth_stream_formats[depth_stream_format_idx]);
			if ((is & IS_INFRARED) == IS_INFRARED)
				if (!(use_default = use_default || ir_stream_format_idx != -1))
					sfs.push_back(ir_stream_formats[ir_stream_format_idx]);
			if (!use_default) {
				if (!rgbd_inp.start(sfs)) {
					cgv::gui::message("could not start kinect device");
					p.update_member(&(is_running = false));
					return;
				}
			}
			else {
				if (!rgbd_inp.start(InputStreams(is), sfs)) {
					cgv::gui::message("could not start kinect device");
					p.update_member(&(is_running = false));
					return;
				}
				else {
					const auto find_for_update = [this,&p](const auto& vec, const auto& sf, int& idx) {
						auto iter = std::find(vec.begin(), vec.end(), sf);
						if (iter != vec.end())
							p.update_member(&(idx = int(iter - vec.begin())));
					};
					for (const auto& sf : sfs) {
						find_for_update(color_stream_formats, sf, color_stream_format_idx);
						find_for_update(depth_stream_formats, sf, depth_stream_format_idx);
						find_for_update(ir_stream_formats, sf, ir_stream_format_idx);
						/*auto ci = std::find(color_stream_formats.begin(), color_stream_formats.end(), sf);
						if (ci != color_stream_formats.end())
							update_member(&(color_stream_format_idx = int(ci - color_stream_formats.begin())));
						auto di = std::find(depth_stream_formats.begin(), depth_stream_formats.end(), sf);
						if (di != depth_stream_formats.end()) {
							depth_stream_format_idx = int(di - depth_stream_formats.begin());
							update_member(&depth_stream_format_idx);
						}
						auto ii = std::find(ir_stream_formats.begin(), ir_stream_formats.end(), sf);
						if (ii != ir_stream_formats.end()) {
							ir_stream_format_idx = int(ii - ir_stream_formats.begin());
							update_member(&ir_stream_format_idx);
						}
						*/
					}
				}
			}
			rgbd_inp.query_calibration(calib);
			on_start();
		}
		else {
			rgbd_inp.stop();
			on_stop();
		}
	}
	if (pt.is(pitch)) {
		if (rgbd_inp.is_attached())
			rgbd_inp.set_pitch(pitch);
	}
	if (pt.is(device_idx)) {
		if (rgbd_inp.is_started())
			rgbd_inp.stop();
		is_running = false;
		rgbd_inp.detach();
		attached = false;
		if (device_idx >= 0) {
			unsigned nr = rgbd_input::get_nr_devices();
			if (device_idx < int(nr)) {
				if (rgbd_inp.attach(rgbd_input::get_serial(device_idx))) {
					attached = true;
					update_stream_formats(p);
					rgbd_inp.set_pitch(pitch);
					color_stream_format_idx = -1;
					depth_stream_format_idx = -1;
					ir_stream_format_idx = -1;
					p.update_member(&color_stream_format_idx);
					p.update_member(&depth_stream_format_idx);
					p.update_member(&ir_stream_format_idx);
					on_attach();
				}
			}
			else {
				device_idx = -1;
				p.update_member(&device_idx);
			}
		}
	}
}
void rgbd_starter_base::timer_event_base(double t, double dt, cgv::gui::provider& p)
{
	if (!rgbd_inp.is_started())
		return;
	if (rgbd_inp.put_IMU_measurement(imu, 0)) {
		for (unsigned i = 0; i < 3; ++i) {
			p.update_member(&imu.linear_acceleration[i]);
			p.update_member(&imu.angular_velocity[i]);
		}
	}
	bool new_frame;
	bool found_frame = false;
	do {
		new_frame = false;
		if ((is & IS_COLOR) == IS_COLOR && rgbd_inp.get_frame(IS_COLOR, color_frame, 0)) {
			color_frame_changed = new_frame = true;
			p.update_member(&(++nr_color_frames));
		}
		if ((is & IS_DEPTH) == IS_DEPTH && rgbd_inp.get_frame(IS_DEPTH, depth_frame, 0)) {
			depth_frame_changed = new_frame = true;
			p.update_member(&(++nr_depth_frames));
		}
		if ((is & IS_INFRARED) == IS_INFRARED && rgbd_inp.get_frame(IS_INFRARED, ir_frame, 0)) {
			infrared_frame_changed = new_frame = true;
			p.update_member(&(++nr_infrared_frames));
		}
		if (new_frame)
			found_frame = true;
	} while (new_frame);

	if (found_frame) {
		on_new_frame(t, InputStreams((color_frame_changed ? IS_COLOR : 0) +
			(depth_frame_changed ? IS_DEPTH : 0) + (infrared_frame_changed ? IS_INFRARED : 0)));
		if (is_running && one_shot)
			on_set_base(&(is_running = false), p);
	}
}
void rgbd_starter_base::create_gui_base(cgv::base::base* bp, cgv::gui::provider& p)
{
	std::string device_def = get_device_enum(prepend_device_enum);
	p.add_member_control(bp, "device", (cgv::type::DummyEnum&)device_idx, "dropdown", device_def);
	p.add_member_control(bp, "near_mode", near_mode, "toggle");
	p.add_view("nr_color_frames", nr_color_frames);
	p.add_view("nr_infrared_frames", nr_infrared_frames);
	p.add_view("nr_depth_frames", nr_depth_frames);
	if (p.begin_tree_node("Device", nr_color_frames, true, "level=2")) {
		p.align("\a");
		p.add_member_control(bp, "multi_device_role", multi_device_role, "dropdown", "enums='standalone,leader,follower'");
		p.add_gui("streams", is, "bit_field_control", "enums='color=1,depth=2,infred=8'");
		p.add_member_control(bp, "color_stream_format", (cgv::type::DummyEnum&)color_stream_format_idx, "dropdown", get_stream_format_enum(color_stream_formats));
		p.add_member_control(bp, "depth_stream_format", (cgv::type::DummyEnum&)depth_stream_format_idx, "dropdown", get_stream_format_enum(depth_stream_formats));
		p.add_member_control(bp, "ir_stream_format",    (cgv::type::DummyEnum&)ir_stream_format_idx, "dropdown", get_stream_format_enum(ir_stream_formats));
		p.add_member_control(bp, "one_shot", one_shot, "toggle");   
		p.add_member_control(bp, "is_running", is_running, "toggle", "true_label='stop';false_label='start'");   
		p.align("\b");
		p.end_tree_node(nr_color_frames);
	}
	if (p.begin_tree_node("Base and IMU", pitch, false, "level=2")) {
		p.align("\a");
		p.add_member_control(bp, "pitch", pitch, "value_slider", "min=-1;max=1;ticks=true");
		p.add_view("ax", imu.linear_acceleration[0]);
		p.add_view("ay", imu.linear_acceleration[1]);
		p.add_view("az", imu.linear_acceleration[2]);
		p.add_view("rx", imu.angular_velocity[0]);
		p.add_view("ry", imu.angular_velocity[1]);
		p.add_view("rz", imu.angular_velocity[2]);
		p.align("\b");
		p.end_tree_node(pitch);
	}
}

}

