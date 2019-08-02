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


bool rgbd_input::read_frame(const string& file_name, frame_type& frame)
{
	if (!cgv::utils::file::exists(file_name))
		return false;

	string data;
	if (!cgv::utils::file::read(file_name, data, false))
		return false;
	static_cast<frame_info&>(frame) = reinterpret_cast<frame_info&>(data.front());
	if (data.size() != frame.buffer_size + sizeof(frame_info))
		return false;
	frame.frame_data.resize(frame.buffer_size);
	memcpy(&frame.frame_data.front(), &data.at(sizeof(frame_info)), frame.buffer_size);
	return true;
}

bool rgbd_input::write_frame(const string& file_name,const frame_type& frame)
{
	// ensure buffer size set
	if (frame.buffer_size != frame.frame_data.size()) {
		std::cerr << "UPS frame buffer size not set correctly" << std::endl;
		const_cast<frame_type&>(frame).buffer_size = frame.frame_data.size();
	}
	return 
		cgv::utils::file::write(file_name, reinterpret_cast<const char*>(&frame), sizeof(frame_info), false) &&
		cgv::utils::file::append(file_name, &frame.frame_data.front(), frame.frame_data.size());
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
/// query the stream formats available for a given stream configuration
void rgbd_input::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
{
	if (!is_attached()) {
		cerr << "rgbd_input::query_stream_formats called on device that has not been attached" << endl;
		return;
	}
	rgbd->query_stream_formats(is, stream_formats);
}

bool rgbd_input::start(InputStreams is, std::vector<stream_format>& stream_formats)
{
	if (!is_attached()) {
		cerr << "rgbd_input::start called on device that has not been attached" << endl;
		return false;
	}
	if (started)
		return true;
	return started = rgbd->start_device(is, stream_formats);
}

bool rgbd_input::start(const std::vector<stream_format>& stream_formats)
{
	if (!is_attached()) {
		cerr << "rgbd_input::start called on device that has not been attached" << endl;
		return false;
	}
	if (started)
		return true;
	return started = rgbd->start_device(stream_formats);
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

bool rgbd_input::set_near_mode(bool on)
{
	if (!is_attached()) {
		cerr << "rgbd_input::set_near_mode called on device that has not been attached" << endl;
		return false;
	}
	return rgbd->set_near_mode(on);
}


bool rgbd_input::get_frame(InputStreams is, frame_type& frame, int timeOut)
{
	if (!is_attached()) {
		cerr << "rgbd_input::get_frame called on device that has not been attached" << endl;
		return false;
	}
	if (!is_started()) {
		cerr << "rgbd_input::get_frame called on attached device that has not been started" << endl;
		return false;
	}
	if (rgbd->get_frame(is, frame, timeOut)) {
		if (!protocol_path.empty()) {
			string fn = compose_file_name(protocol_path, frame, frame.frame_index);
			if (!cgv::utils::file::write(fn, &frame.frame_data.front(), frame.frame_data.size(), false))
				std::cerr << "rgbd_input::get_frame: could not protocol frame to " << fn << std::endl;
		}
		return true;
	}
	return false;
}

/// map a color frame to the image coordinates of the depth image
void rgbd_input::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
	frame_type& warped_color_frame) const
{
	if (!is_attached()) {
		cerr << "rgbd_input::map_color_to_depth called on device that has not been attached" << endl;
		return;
	}
	rgbd->map_color_to_depth(depth_frame, color_frame, warped_color_frame);
}

}