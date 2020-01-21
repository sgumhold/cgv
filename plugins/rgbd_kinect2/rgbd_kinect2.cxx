#include <iostream>
#include "rgbd_kinect2.h"
#include <Windows.h>
#include <Kinect.h>
#include <Ole2.h>
#include <cgv/utils/convert.h>

using namespace std;

struct stream_handles
{
	HANDLE hColorStreamHandle;
	HANDLE hInfraredStreamHandle;
	HANDLE hDepthStreamHandle;
	stream_handles() : hColorStreamHandle(NULL), hInfraredStreamHandle(NULL), hDepthStreamHandle(NULL) {}
};

namespace rgbd {

	/// create a detached kinect CLNUI device object
	rgbd_kinect2::rgbd_kinect2()
	{
		near_mode = false;
		camera = 0;
		handles = 0;
	}

	/// attach to the kinect device of the given serial
	bool rgbd_kinect2::attach(const std::string& serial)
	{
		if (is_attached()) {
			detach();
		}

		if (FAILED(GetDefaultKinectSensor(&camera))) {
			return false;
		}
		return true;
	}

	/// return whether device object is attached to a kinect device
	bool rgbd_kinect2::is_attached() const
	{
		return camera != 0;
	}
	/// detach from serial
	bool rgbd_kinect2::detach()
	{
		camera->Release();
		camera = nullptr;
		return true;
	}

	/// return whether rgbd device has support for view finding actuator
	bool rgbd_kinect2::has_view_finder() const
	{
		return true;
	}
	/// return a view finder info structure
	const view_finder_info& rgbd_kinect2::get_view_finder_info() const
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

	/// set the pitch position of the kinect base, range of values is [-1, 1]
	bool rgbd_kinect2::set_pitch(float y)
	{
		return true;
	}
	/// return the pitch position of the rgbd device in degrees with 0 as middle position
	float rgbd_kinect2::get_pitch() const
	{
		return false;
	}
	/// whether rgbd device has support for a near field depth mode
	bool rgbd_kinect2::has_near_mode() const
	{
		return true;
	}
	/// return whether the near field depth mode is activated
	bool rgbd_kinect2::get_near_mode() const
	{
		return near_mode;
	}
	bool rgbd_kinect2::set_near_mode(bool on)
	{
		return false;
	}

	/// check whether rgbd device has inertia measurement unit
	bool rgbd_kinect2::has_IMU() const
	{
		return true;
	}
	/// return additional information on inertia measurement unit
	const IMU_info& rgbd_kinect2::get_IMU_info() const
	{
		static IMU_info info;
		info.has_angular_acceleration = false;
		info.has_linear_acceleration = true;
		info.has_time_stamp_support = false;
		return info;
	}
	/// query the current measurement of the acceleration sensors within the given time_out in milliseconds; return whether a measurement has been retrieved
	bool rgbd_kinect2::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
		return false;
	}
	/// check whether the device supports the given combination of input streams
	bool rgbd_kinect2::check_input_stream_configuration(InputStreams is) const
	{
		return true;
	}
	/// query the stream formats available for a given stream configuration
	void rgbd_kinect2::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if ((is & IS_COLOR) != 0) {
			stream_formats.push_back(stream_format(1920, 1080, PF_BGR, 30, 24));
		}
		if ((is & IS_INFRARED) != 0) {
			stream_formats.push_back(stream_format(1920, 1080, PF_I, 30, 8));
		}
		if ((is & IS_DEPTH) != 0) {
			stream_formats.push_back(stream_format(1920, 1080, PF_DEPTH, 30, 16));
		}
	}
	/// start the camera
	bool rgbd_kinect2::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{

		return false;
	}
	/// start the rgbd device with given stream formats 
	bool rgbd_kinect2::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;

		camera->get_CoordinateMapper(&mapper);
		camera->Open();
		camera->OpenMultiSourceFrameReader(
			FrameSourceTypes::FrameSourceTypes_Depth
			| FrameSourceTypes::FrameSourceTypes_Color
			| FrameSourceTypes::FrameSourceTypes_Body,
			&reader);

		return false;
	}
	/// stop the camera
	bool rgbd_kinect2::stop_device()
	{
		return false;
	}

	/// return whether device has been started
	bool rgbd_kinect2::is_running() const
	{
		return handles != 0;
	}

		/// query a frame of the given input stream
	bool rgbd_kinect2::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		if (!is_attached()) {
			cerr << "rgbd_kinect2::get_frame called on device that has not been attached" << endl;
			return false;
		}
		

		if (!is_running()) {
			cerr << "rgbd_kinect2::get_frame called on device that is not running" << endl;
			return false;
		}

		return true;
	}
	/// map a color frame to the image coordinates of the depth image
	void rgbd_kinect2::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
		frame_type& warped_color_frame) const
	{
		
	}
	
	/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
	bool rgbd_kinect2::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
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

	/// construct CLNUI driver
	rgbd_kinect2_driver::rgbd_kinect2_driver()
	{
	}
	/// destructor
	rgbd_kinect2_driver::~rgbd_kinect2_driver()
	{
	}
	/// return the number of kinect devices found by driver
	unsigned rgbd_kinect2_driver::get_nr_devices()
	{
		IKinectSensor* sensor;

		if (FAILED(GetDefaultKinectSensor(&sensor))) {
			return 0;
		}
		sensor->Release();
		return 1;
	}

	/// return the serial of the i-th kinect devices
	std::string rgbd_kinect2_driver::get_serial(int i)
	{
		int n = get_nr_devices();
		if (i >= n)
			return "";

		IKinectSensor* sensor;

		if (FAILED(GetDefaultKinectSensor(&sensor))) {
			return "";
		}
		
		sensor->Open();
		WCHAR id[256] = L"";
		HRESULT res = sensor->get_UniqueKinectId(256, id);
		sensor->Close();
		sensor->Release();
	
		//return cgv::utils::wstr2str(std::wstring(id));
		return "default-kinect2";
	}

	/// create a kinect device
	rgbd_device* rgbd_kinect2_driver::create_rgbd_device()
	{
		return new rgbd_kinect2();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_kinect2_driver> rgbd_kinect2_driver_registration("kinect2_driver");