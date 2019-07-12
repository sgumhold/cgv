#include <iostream>
#include <algorithm>
#include "rgbd_input.h"
#include "rgbd_device_emulation.h"
#include <cgv/utils/file.h>
#include <cgv/utils/convert.h>

using namespace std;

namespace rgbd {


std::vector<rgbd_driver*>& rgbd_input::ref_driver_list()
{
	static std::vector<rgbd_driver*> driver_list;
	return driver_list;
}

void rgbd_input::register_driver(rgbd_driver* rgbd, const std::string& driver_name)
{
	ref_driver_list().push_back(rgbd);
}

void rgbd_input::unregister_driver(rgbd_driver* drv)
{
	std::vector<rgbd_driver*>::iterator i = std::find(ref_driver_list().begin(), ref_driver_list().end(), drv);
	if (i != ref_driver_list().end())
		ref_driver_list().erase(i);
}


bool rgbd_input::read_frame(const string& file_name, void* data_ptr, unsigned frame_size)
{
	if (!cgv::utils::file::exists(file_name))
		return false;
	string data;
	if (!cgv::utils::file::read(file_name, data, false))
		return false;
	if (data.size() != frame_size)
		return false;
	memcpy(data_ptr, data.c_str(), frame_size);
	return true;
}

bool rgbd_input::write_frame(const string& file_name,const void* data_ptr, unsigned frame_size)
{
	return cgv::utils::file::write(file_name, (const char*) data_ptr, frame_size, false);
}

unsigned rgbd_input::get_nr_devices()
{
	unsigned nr = 0;
	for (unsigned i = 0; i<ref_driver_list().size(); ++i)
		nr += ref_driver_list()[i]->get_nr_devices();
	return nr;
}

string rgbd_input::get_serial(int id)
{
	for (unsigned i = 0; i<ref_driver_list().size(); ++i) {
		unsigned nr = ref_driver_list()[i]->get_nr_devices();
		if (id < (int)nr)
			return ref_driver_list()[i]->get_serial(id);
		id -= nr;
	}
	return string();
}

rgbd_input::rgbd_input()
{
	rgbd = 0;
	started = false;
	protocol_idx = 0;
	protocol_flags = 0;
}

rgbd_input::~rgbd_input()
{
	if (started)
		stop();
	if (is_attached())
		detach();
}

rgbd_input::rgbd_input(const string& serial)
{
	rgbd = 0;
	started = false;
	attach(serial);
}

bool rgbd_input::attach(const string& serial)
{
	if (is_attached())
		detach();

	for (unsigned i = 0; i<ref_driver_list().size(); ++i) {
		rgbd = ref_driver_list()[i]->create_rgbd_device();
		if (rgbd->attach(serial))
			break;
		delete rgbd;
		rgbd = 0;
	}
	if (!rgbd)
		return false;

	this->serial = serial;
	return true;
}

const std::string& rgbd_input::get_serial() const
{
	return serial;
}

bool rgbd_input::attach_path(const string& path)
{
	if (is_attached())
		detach();
	rgbd = new rgbd_emulation(path);
	serial = path;
	return true;
}

bool rgbd_input::is_attached() const
{
	if (rgbd == 0)
		return false;
	return rgbd->is_attached();
}

bool rgbd_input::detach()
{
	if (!is_attached())
		return true;
	if (is_started())
		rgbd->stop_device();
	rgbd->detach();
	delete rgbd;
	rgbd = 0;
	return true;
}

void rgbd_input::enable_protocol(const std::string& path)
{
	protocol_path = path;
	protocol_idx  = 0;
	protocol_flags = 0;
}

/// disable protocolation
void rgbd_input::disable_protocol()
{
	protocol_path = "";
	protocol_idx  = 0;
	protocol_flags = 0;
}


bool rgbd_input::set_pitch(float y)
{
	if (!is_attached()) {
		cerr << "rgbd_input::set_pitch called on device that has not been attached to motor" << endl;
		return false;
	}
	return rgbd->set_pitch(y);
}

/// query the current measurement of the acceleration sensors 
bool rgbd_input::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
{
	if (!is_attached()) {
		cerr << "rgbd_input::put_acceleration_measurements called on device that has not been attached" << endl;
		return false;
	}
	return rgbd->put_IMU_measurement(m, time_out);
}

bool rgbd_input::check_input_stream_configuration(InputStreams is) const
{
	if (!is_attached()) {
		cerr << "rgbd_input::check_input_stream_configuration called on device that has not been attached" << endl;
		return false;
	}
	return rgbd->check_input_stream_configuration(is);
}

bool rgbd_input::start(InputStreams is)
{
	if (!is_attached()) {
		cerr << "rgbd_input::start called on device that has not been attached" << endl;
		return false;
	}
	if (started)
		return true;
	return started = rgbd->start_device(is);
}

bool rgbd_input::is_started() const
{
	return started;
}

bool rgbd_input::stop()
{
	if (!is_attached()) {
		cerr << "rgbd_input::stop called on device that has not been attached" << endl;
		return false;
	}
	if (is_started())
		started = !rgbd->stop_device();
	return !started;
}

unsigned rgbd_input::get_width() const
{
	if (!is_attached()) {
		cerr << "rgbd_input::get_width called on device that has not been attached" << endl;
		return 640;
	}
	return rgbd->get_width();
}

unsigned rgbd_input::get_height() const
{
	if (!is_attached()) {
		cerr << "rgbd_input::get_height called on device that has not been attached" << endl;
		return 480;
	}
	return rgbd->get_height();
}

bool rgbd_input::set_near_mode(bool on)
{
	if (!is_attached()) {
		cerr << "rgbd_input::set_near_mode called on device that has not been attached" << endl;
		return false;
	}
	return rgbd->set_near_mode(on);
}

unsigned rgbd_input::get_frame_size(FrameFormat ff) const
{
	unsigned entry_size = 4;
	switch (ff) {
	case FF_COLOR_RAW :   entry_size = 4; break;
	case FF_COLOR_RGB24 : entry_size = 3; break;
	case FF_DEPTH_RAW :   entry_size = 2; break;
	case FF_DEPTH_D8 :    entry_size = 1; break;
	case FF_DEPTH_D12 :   entry_size = 2; break;
	}
	return get_width()*get_height()*entry_size;
}

bool rgbd_input::get_frame(FrameFormat ff, void* data_ptr, int time_out)
{
	if (!is_attached()) {
		cerr << "rgbd_input::get_frame called on device that has not been attached" << endl;
		return false;
	}
	if (!is_started()) {
		cerr << "rgbd_input::get_frame called on attached device that has not been started" << endl;
		return false;
	}
	if (rgbd->get_frame(ff, data_ptr, time_out)) {
		if (!protocol_path.empty()) {
			unsigned flag = 2;
			if (ff < FF_DEPTH_RAW)
				flag = 1;
			if (protocol_flags & flag) {
				++protocol_idx;
				protocol_flags = flag;
			}
			else
				protocol_flags |= flag;
			string fn = compose_file_name(protocol_path, ff, protocol_idx);
			if (!cgv::utils::file::write(fn, (const char*) data_ptr, get_frame_size(ff), false))
				std::cerr << "rgbd_input::get_frame: could not protocol frame to " << fn << std::endl;
		}
		return true;
	}
	return false;
}
/// map a depth map to color pixel where color_pixel_data_ptr points to an array of short int pairs
void rgbd_input::map_depth_to_color_pixel(FrameFormat depth_ff, const void* depth_data_ptr, void* color_pixel_data_ptr) const
{
	if (!is_attached()) {
		cerr << "rgbd_input::map_depth_to_color_pixel called on device that has not been attached" << endl;
		return;
	}
	rgbd->map_depth_to_color_pixel(depth_ff, depth_data_ptr, color_pixel_data_ptr);
}

/// map a color frame to the image coordinates of the depth image
void rgbd_input::map_color_to_depth(FrameFormat depth_ff, const void* depth_data_ptr, FrameFormat color_ff, void* color_data_ptr) const
{
	if (!is_attached()) {
		cerr << "rgbd_input::map_color_to_depth called on device that has not been attached" << endl;
		return;
	}
	rgbd->map_color_to_depth(depth_ff, depth_data_ptr, color_ff, color_data_ptr);
}
/// map pixel coordinate and depth in given format to 3D point
bool rgbd_input::map_pixel_to_point(int x, int y, unsigned depth, FrameFormat depth_ff, float point[3])
{
	if (!is_attached()) {
		cerr << "rgbd_input::map_color_to_depth called on device that has not been attached" << endl;
		return false;
	}
	return rgbd->map_pixel_to_point(x, y, depth, depth_ff, point);
	return true;
}

}