#include <iostream>
#include <algorithm>
#include <future>
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
		const_cast<frame_type&>(frame).buffer_size = (unsigned) frame.frame_data.size();
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

string rgbd_input::get_audio_device_id(int id)
{
	for (unsigned i = 0; i < ref_driver_list().size(); ++i) {
		unsigned nr = ref_driver_list()[i]->get_nr_devices();
		if (id < (int)nr)
			return ref_driver_list()[i]->get_audio_device_id(id);
		id -= nr;
	}
	return string();
}

rgbd_input::rgbd_input()
{
	rgbd = 0;
	started = false;
	protocol_write_async = true;
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

/// check whether a multi-device role is supported
bool rgbd_input::is_supported(MultiDeviceRole mdr) const
{
	if (!is_attached())
		return true;
	return rgbd->is_supported(mdr);
}
/// configure device for a multi-device role and return whether this was successful (do this before starting)
bool rgbd_input::configure_role(MultiDeviceRole mdr)
{
	if (!is_attached())
		return false;
	return rgbd->configure_role(mdr);
}
MultiDeviceRole rgbd_input::get_role() const
{
	if (!is_attached())
		return MDR_STANDALONE;
	return rgbd->get_role();
}

bool rgbd_input::is_sync_supported() const
{
	if (!is_attached())
		return false;
	return rgbd->is_sync_supported();
}
bool rgbd_input::is_sync_in_connected() const
{
	if (!is_attached())
		return false;
	return rgbd->is_sync_in_connected();
}
bool rgbd_input::is_sync_out_connected() const
{
	if (!is_attached())
		return false;
	return rgbd->is_sync_out_connected();
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

void write_protocol_headers(vector<stream_format> streams,std::string path) {
	for (stream_format stream : streams) {
		cgv::utils::file::write(path + "/stream_info." + get_frame_extension(stream), reinterpret_cast<const char*>(&stream), sizeof(stream_format), false);
	}
}

void rgbd_input::enable_protocol(const std::string& path)
{
	protocol_path = path;
	protocol_idx  = 0;
	protocol_flags = 0;

	//write the metadata for every stream found
	if (is_started()) {
		write_protocol_headers(streams, path);
		//write camera parameters
		emulator_parameters parameters;
		if (rgbd->get_emulator_configuration(parameters)) {
			cgv::utils::file::write(path + "/emulator_parameters", reinterpret_cast<const char*>(&parameters), sizeof(emulator_parameters), false);
		}
	}
}

/// disable protocolation
void rgbd_input::disable_protocol()
{
	protocol_path = "";
	protocol_idx  = 0;
	protocol_flags = 0;
}

void rgbd::rgbd_input::clear_protocol(const string& path)
{
	cout << "rgbd::rgbd_input::clear_protocol: removing old protocol\n";
	cgv::utils::file::remove(path + "/emulator_parameters");
	static const char* exts[] = {
	"ir", "rgb", "bgr", "rgba", "bgra", "byr", "dep", "d_p", "p_tri"
	};
	for (const char* ext : exts) {
		void* file_handle = cgv::utils::file::find_first(path + "/kinect_*." + ext + "*");
		while (file_handle) {
			string fn = cgv::utils::file::find_name(file_handle);
			string file_path = path + "/" + fn;
			if (!cgv::utils::file::find_directory(file_handle)) {
				cout << "rgbd::rgbd_input::clear_protocol: removing " << file_path << "\n";
				cgv::utils::file::remove(file_path);
			}
			file_handle = cgv::utils::file::find_next(file_handle);
		}
		file_handle = cgv::utils::file::find_first(path + "/stream_info." + ext + "*");
		if (file_handle) {
			string fn = cgv::utils::file::find_name(file_handle);
			string file_path = path + "/" + fn;
			cout << "rgbd::rgbd_input::clear_protocol: removing " << file_path << "\n";
			cgv::utils::file::remove(file_path);
		}
	}
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
const std::vector<color_parameter_info>& rgbd_input::get_supported_color_control_parameter_infos() const
{
	static std::vector<color_parameter_info> I;
	if (!is_attached()) {
		cerr << "rgbd_input::get_supported_color_control_parameter_infos called on device that has not been attached" << endl;
		return I;
	}
	return rgbd->get_supported_color_control_parameter_infos();
}
std::pair<int32_t, bool> rgbd_input::get_color_control_parameter(ColorControlParameter ccp) const
{
	if (!is_attached()) {
		cerr << "rgbd_input::get_color_control_parameter called on device that has not been attached" << endl;
		return std::pair<int32_t, bool>(-1,false);
	}
	return rgbd->get_color_control_parameter(ccp);
}
bool rgbd_input::set_color_control_parameter(ColorControlParameter ccp, int32_t value, bool automatic_mode)
{
	if (!is_attached()) {
		cerr << "rgbd_input::start called on device that has not been attached" << endl;
		return false;
	}
	return rgbd->set_color_control_parameter(ccp, value, automatic_mode);
}

bool rgbd_input::start(InputStreams is, std::vector<stream_format>& stream_formats)
{
	if (!is_attached()) {
		cerr << "rgbd_input::start called on device that has not been attached" << endl;
		return false;
	}
	if (started)
		return true;
	started = rgbd->start_device(is, stream_formats);
	streams = stream_formats;
	if (!protocol_path.empty()) {
		write_protocol_headers(streams, protocol_path);
		//write camera parameters
		emulator_parameters parameters;
		if (rgbd->get_emulator_configuration(parameters)) {
			cgv::utils::file::write(protocol_path + "/emulator_parameters", reinterpret_cast<const char*>(&parameters), sizeof(emulator_parameters), false);
		}
	}
	return started;
}

/// query the calibration information and return whether this was successful
bool rgbd_input::query_calibration(rgbd_calibration& calib)
{
	if (!is_attached()) {
		cerr << "rgbd_input::query_calibration called on device that has not been attached" << endl;
		return false;
	}
	if (!is_started()) {
		cerr << "rgbd_input::query_calibration called on device that has not been started" << endl;
		return false;
	}
	rgbd->query_calibration(calib);
	return true;
}

bool rgbd_input::start(const std::vector<stream_format>& stream_formats)
{
	if (!is_attached()) {
		cerr << "rgbd_input::start called on device that has not been attached" << endl;
		return false;
	}
	if (started)
		return true;
	started = rgbd->start_device(stream_formats);
	streams = stream_formats;
	if (!protocol_path.empty()) {
		write_protocol_headers(streams, protocol_path);
	}
	return started;
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

bool write_protocol_frame(const std::string& fn, frame_type* frame_ptr)
{
	bool success = cgv::utils::file::write(fn, &frame_ptr->frame_data.front(), frame_ptr->frame_data.size(), false);
	delete frame_ptr;
	return success;
}

bool rgbd_input::write_protocol_frame_async(const std::string& fn, const frame_type& frame) const
{
	static future<bool> fu;
	if (frame.frame_data.size() == 0)
		return false;
	if (!protocol_write_async)
		return cgv::utils::file::write(fn, &frame.frame_data.front(), frame.frame_data.size(), false);

	if (fu.valid()) {
		fu.wait();
	}
	//fu = std::async(std::launch::async, cgv::utils::file::write, fn, &frame.frame_data.front(), frame.frame_data.size(), false);
	return true;
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
			string fn = compose_file_name(protocol_path + "/kinect_", frame, protocol_idx);
			if ((is & IS_COLOR) != 0) {
				next_warped_file_name = compose_file_name(protocol_path + "/warped_", frame, protocol_idx);
			}
			if (!write_protocol_frame_async(fn, frame))
				std::cerr << "rgbd_input::get_frame: could not protocol frame to " << fn << std::endl;
			else
				++protocol_idx;
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
	if (!next_warped_file_name.empty()) {
		if (!write_protocol_frame_async(next_warped_file_name, warped_color_frame))
			std::cerr << "rgbd_input::map_color_to_depth: could not protocol frame to " << next_warped_file_name << std::endl;
		next_warped_file_name.clear();
	}
}

/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
bool rgbd_input::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
{
	if (!is_attached()) {
		cerr << "rgbd_input::map_color_to_depth called on device that has not been attached" << endl;
		return false;
	}
	return rgbd->map_depth_to_point(x, y, depth, point_ptr);
}

}