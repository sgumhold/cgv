#include <iostream>
#include "rgbd_kinect_azure.h"
#include <cgv/utils/convert.h>


using namespace std;

namespace rgbd {
	/// create a detached kinect device object
	rgbd_kinect_azure::rgbd_kinect_azure()
	{

	}

	/// attach to the kinect device of the given serial
	bool rgbd_kinect_azure::attach(const std::string& serial)
	{
		for (int i = 0; i < k4a::device::get_installed_count();++i) {
			k4a::device device;
			string dev_serial;
			try {
				device = k4a::device::open(i);
				dev_serial = device.get_serialnum(),
				device.close();
			}
			catch (runtime_error e) {
				cerr << "rgbd_kinect_azure_driver: " << e.what() << '\n';
			}

			if (dev_serial == serial) {
				this->device_serial = serial;
				return true;
			}
		}
		return false;
	}

	/// return whether device object is attached to a kinect device
	bool rgbd_kinect_azure::is_attached() const
	{
		return false;
	}
	/// detach from serial (done automatically in constructor
	bool rgbd_kinect_azure::detach()
	{
		return false;
	}

	/// return whether rgbd device has support for view finding actuator
	bool rgbd_kinect_azure::has_view_finder() const
	{
		return false;
	}
	/// return a view finder info structure, copy from rgbd_kinect.cxx
	const view_finder_info& rgbd_kinect_azure::get_view_finder_info() const
	{
		static view_finder_info info;
		info.degrees_of_freedom = 1;
		info.min_angle[0] = -27.0f;
		info.max_angle[0] =  27.0f;
		info.min_angle[1] = 0.0f;
		info.max_angle[1] = 0.0f;
		info.min_angle[2] = 0.0f;
		info.max_angle[2] = 0.0f;
		return info;
	}

	/// whether rgbd device has support for a near field depth mode
	bool rgbd_kinect_azure::has_near_mode() const
	{
		return true;
	}
	/// return whether the near field depth mode is activated
	bool rgbd_kinect_azure::get_near_mode() const
	{
		return near_mode;
	}
	bool rgbd_kinect_azure::set_near_mode(bool on)
	{
		return false;
	}

	/// check whether rgbd device has inertia measurement unit
	bool rgbd_kinect_azure::has_IMU() const
	{
		return true;
	}
	/// return additional information on inertia measurement unit, copy from rgbd_kinect.cxx
	const IMU_info& rgbd_kinect_azure::get_IMU_info() const
	{
		static IMU_info info;
		info.has_angular_acceleration = false;
		info.has_linear_acceleration = true;
		info.has_time_stamp_support = false;
		return info;
	}
	/// query the current measurement of the acceleration sensors within the given time_out in milliseconds; return whether a measurement has been retrieved
	bool rgbd_kinect_azure::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
		return false;
	}
	/// check whether the device supports the given combination of input streams
	bool rgbd_kinect_azure::check_input_stream_configuration(InputStreams is) const
	{
		return false;
	}
	/// query the stream formats available for a given stream configuration
	void rgbd_kinect_azure::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if ((is & IS_COLOR) != 0) {

		}
		if ((is & IS_INFRARED) != 0) {
			
		}
		if ((is & IS_DEPTH) != 0) {
			if ((is & IS_PLAYER_INDEX) != 0) {
				
			}
			else {

			}
		}
	}
	/// start the camera
	bool rgbd_kinect_azure::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;
		return false;
	}
	/// start the rgbd device with given stream formats 
	bool rgbd_kinect_azure::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;
		return false;
	}
	/// stop the camera
	bool rgbd_kinect_azure::stop_device()
	{
		if (!is_running())
			return true;
		return false;
	}

	/// return whether device has been started
	bool rgbd_kinect_azure::is_running() const
	{
		return false;
	}

		/// query a frame of the given input stream
	bool rgbd_kinect_azure::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		if (!is_attached()) {
			cerr << "rgbd_kinect::get_frame called on device that has not been attached" << endl;
			return false;
		}

		if (!is_running()) {
			cerr << "rgbd_kinect::get_frame called on device that is not running" << endl;
			return false;
		}

		return false;
	}
	/// map a color frame to the image coordinates of the depth image
	void rgbd_kinect_azure::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
		frame_type& warped_color_frame) const
	{
		if (!is_attached()) {
			cerr << "rgbd_kinect::map_color_to_depth called on device that has not been attached" << endl;
			return;
		}
		// prepare warped frame
		static_cast<frame_size&>(warped_color_frame) = depth_frame;
		warped_color_frame.pixel_format = color_frame.pixel_format;
		warped_color_frame.nr_bits_per_pixel = color_frame.nr_bits_per_pixel;
		warped_color_frame.compute_buffer_size();
		warped_color_frame.frame_data.resize(warped_color_frame.buffer_size);
		unsigned bytes_per_pixel = color_frame.nr_bits_per_pixel / 8;
	}
	
	/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
	bool rgbd_kinect_azure::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		//data from rgbd_kinect.cxx
		depth /= 8;
		if (depth == 0)
			return false;

		static const double fx_d = 1.0 / 5.9421434211923247e+02;
		static const double fy_d = 1.0 / 5.9104053696870778e+02;
		static const double cx_d = 3.3930780975300314e+02;
		static const double cy_d = 2.4273913761751615e+02;
		double d = 0.001 * depth;
		point_ptr[0] = float((x - cx_d) * d * fx_d);
		point_ptr[1] = float((y - cy_d) * d * fy_d);
		point_ptr[2] = float(d);
		return true;
	}

	/// construct driver
	rgbd_kinect_azure_driver::rgbd_kinect_azure_driver()
	{
		cout << "hellloooooooooooo--------------------------";
	}
	/// destructor
	rgbd_kinect_azure_driver::~rgbd_kinect_azure_driver()
	{
	}
	/// return the number of kinect devices found by driver
	unsigned rgbd_kinect_azure_driver::get_nr_devices()
	{
		return k4a::device::get_installed_count();
	}

	/// return the serial of the i-th kinect devices
	std::string rgbd_kinect_azure_driver::get_serial(int i)
	{
		k4a::device device;

		try {
			device = k4a::device::open(i);
			string serial = device.get_serialnum();
			device.close();
			return serial;
		}
		catch (runtime_error e) {
			cerr << "rgbd_kinect_azure_driver: " << e.what() << '\n';
		}
		return string();
	}

	/// create a kinect device
	rgbd_device* rgbd_kinect_azure_driver::create_rgbd_device()
	{
		return new rgbd_kinect_azure();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_kinect_azure_driver> rgbd_kinect_azure_driver_registration("kinect_azure_driver");