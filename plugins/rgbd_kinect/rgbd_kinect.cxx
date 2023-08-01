#include <iostream>
#include "rgbd_kinect.h"
#include <Windows.h>
#include "NuiApi.h"
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

	bool extract_resolution(const frame_size& fs, NUI_IMAGE_RESOLUTION& resolution)
	{
		NUI_IMAGE_RESOLUTION ir = NUI_IMAGE_RESOLUTION_640x480;
		switch (fs.width) {
		case 80: 
			if (fs.height != 60)
				return false;
			ir = NUI_IMAGE_RESOLUTION_80x60; 
			break;
		case 320: 
			if (fs.height != 240)
				return false;
			ir = NUI_IMAGE_RESOLUTION_320x240;
			break;
		case 640: 
			if (fs.height != 480)
				return false;
			ir = NUI_IMAGE_RESOLUTION_640x480;
			break;
		case 1280: 
			if (fs.height != 960)
				return false;
			ir = NUI_IMAGE_RESOLUTION_1280x960;
			break;
		default:
			return false;
		}
		resolution = ir;
		return true;
	}
	/// create a detached kinect CLNUI device object
	rgbd_kinect::rgbd_kinect()
	{
		near_mode = false;
		camera = 0;
		handles = 0;
	}

	/// attach to the kinect device of the given serial
	bool rgbd_kinect::attach(const std::string& serial)
	{
		if (camera)
			detach();
		INuiSensor *pNuiSensor;
		HRESULT hr = NuiCreateSensorById(cgv::utils::str2wstr(serial).c_str(), &pNuiSensor);
		if (FAILED(hr))
			return false;
		camera = pNuiSensor;
		return is_attached();
	}

	/// return whether device object is attached to a kinect device
	bool rgbd_kinect::is_attached() const
	{
		return camera != 0;
	}
	/// detach from serial (done automatically in constructor
	bool rgbd_kinect::detach()
	{
		if (is_attached()) {
			if (is_running())
				stop_device();

			INuiSensor *pNuiSensor = (INuiSensor*)camera;
			pNuiSensor->NuiShutdown();
			pNuiSensor->Release();
			camera = 0;
		}
		return !camera;
	}

	/// return whether rgbd device has support for view finding actuator
	bool rgbd_kinect::has_view_finder() const
	{
		return true;
	}
	/// return a view finder info structure
	const view_finder_info& rgbd_kinect::get_view_finder_info() const
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
	bool rgbd_kinect::set_pitch(float y)
	{
		if (camera) {
			INuiSensor *pNuiSensor = (INuiSensor*)camera;
			HRESULT hr = pNuiSensor->NuiCameraElevationSetAngle((LONG)(y*NUI_CAMERA_ELEVATION_MAXIMUM));
			if (FAILED(hr))
				return false;
		}
		else {
			cerr << "rgbd_kinect::set_pitch called on device that has not been attached to camera" << endl;
			return false;
		}
		return true;
	}
	/// return the pitch position of the rgbd device in degrees with 0 as middle position
	float rgbd_kinect::get_pitch() const
	{
		if (camera) {
			INuiSensor *pNuiSensor = (INuiSensor*)camera;
			LONG y;
			HRESULT hr = pNuiSensor->NuiCameraElevationGetAngle(&y);
			if (FAILED(hr))
				return 0.0f;
			return (float)y / NUI_CAMERA_ELEVATION_MAXIMUM;
		}
		else {
			cerr << "rgbd_kinect::get_pitch called on device that has not been attached to camera" << endl;
			return false;
		}
		return true;
	}
	/// whether rgbd device has support for a near field depth mode
	bool rgbd_kinect::has_near_mode() const
	{
		return true;
	}
	/// return whether the near field depth mode is activated
	bool rgbd_kinect::get_near_mode() const
	{
		return near_mode;
	}
	bool rgbd_kinect::set_near_mode(bool on)
	{
		near_mode = on;
		if (is_running()) {
			INuiSensor *pNuiSensor = (INuiSensor*)camera;
			stream_handles* sh = (stream_handles*)handles;
			pNuiSensor->NuiImageStreamSetImageFrameFlags(sh->hDepthStreamHandle, near_mode ? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0);
		}
		return true;
	}

	/// check whether rgbd device has inertia measurement unit
	bool rgbd_kinect::has_IMU() const
	{
		return true;
	}
	/// return additional information on inertia measurement unit
	const IMU_info& rgbd_kinect::get_IMU_info() const
	{
		static IMU_info info;
		info.has_angular_acceleration = false;
		info.has_linear_acceleration = true;
		info.has_time_stamp_support = false;
		return info;
	}
	/// query the current measurement of the acceleration sensors within the given time_out in milliseconds; return whether a measurement has been retrieved
	bool rgbd_kinect::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
		if (camera) {
			INuiSensor *pNuiSensor = (INuiSensor*)camera;

			Vector4 reading;
			HRESULT hr = pNuiSensor->NuiAccelerometerGetCurrentReading(&reading);
			//pNuiSensor->NuiSetForceInfraredEmitterOff(true);
			if (FAILED(hr))
				return false;
			m.linear_acceleration[0] = reading.x;
			m.linear_acceleration[1] = reading.y;
			m.linear_acceleration[2] = reading.z;
			m.angular_velocity[0] = 0.0f;
			m.angular_velocity[1] = 0.0f;
			m.angular_velocity[2] = 0.0f;
			return true;
		}
		else {
			cerr << "rgbd_kinect::put_acceleration_measurements called on device that has not been attached to camera" << endl;
			return false;
		}
	}
	/// check whether the device supports the given combination of input streams
	bool rgbd_kinect::check_input_stream_configuration(InputStreams is) const
	{
		return true;
	}
	/// query the stream formats available for a given stream configuration
	void rgbd_kinect::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if ((is & IS_COLOR) != 0) {
			stream_formats.push_back(stream_format(640, 480, PF_BGR, 30));
			stream_formats.push_back(stream_format(1280, 960, PF_BGR, 12));
			stream_formats.push_back(stream_format(640, 480, PF_BAYER, 30, 8));
			stream_formats.push_back(stream_format(1280, 960, PF_BAYER, 12, 8));
		}
		if ((is & IS_INFRARED) != 0) {
			stream_formats.push_back(stream_format(640, 480, PF_I, 30, 16));
		}
		if ((is & IS_DEPTH) != 0) {
			if ((is & IS_PLAYER_INDEX) != 0) {
				stream_formats.push_back(stream_format(80, 60, PF_DEPTH_AND_PLAYER, 30, 16));
				stream_formats.push_back(stream_format(320, 240, PF_DEPTH_AND_PLAYER, 30, 16));
				stream_formats.push_back(stream_format(640, 480, PF_DEPTH_AND_PLAYER, 30, 16));
			}
			else {
				stream_formats.push_back(stream_format(80, 60, PF_DEPTH, 30, 16));
				stream_formats.push_back(stream_format(320, 240, PF_DEPTH, 30, 16));
				stream_formats.push_back(stream_format(640, 480, PF_DEPTH, 30, 16));
			}
		}
	}
	/// start the camera
	bool rgbd_kinect::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;

		if (camera) {
			// cast camera to NUISensor
			INuiSensor *pNuiSensor = (INuiSensor*)camera;

			// determine flags
			DWORD init_flags = 0;
			if ((is & IS_COLOR) != 0 || (is & IS_INFRARED) != 0)
				init_flags |= NUI_INITIALIZE_FLAG_USES_COLOR;
			if ((is & IS_DEPTH) != 0) {
				if ((is & IS_PLAYER_INDEX) != 0)
					init_flags |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
				else
					init_flags |= NUI_INITIALIZE_FLAG_USES_DEPTH;
			}

			// try to start the camera
			HRESULT hr = pNuiSensor->NuiInitialize(init_flags);
			if (FAILED(hr))
				return false;

			// determine the color stream
			stream_handles sh;
			if ((is & IS_COLOR) != 0) {
				hr = pNuiSensor->NuiImageStreamOpen(
					NUI_IMAGE_TYPE_COLOR, 
					NUI_IMAGE_RESOLUTION_640x480, 
					0, NUI_IMAGE_STREAM_FRAME_LIMIT_MAXIMUM, NULL, &sh.hColorStreamHandle);
				if (FAILED(hr))
					return false;
				stream_formats.push_back(color_format = stream_format(640, 480, PF_BGR, 30));
			}
			//
			if ((is & IS_INFRARED) != 0) {
				hr = pNuiSensor->NuiImageStreamOpen(
					NUI_IMAGE_TYPE_COLOR_INFRARED,
					NUI_IMAGE_RESOLUTION_640x480,
					0, NUI_IMAGE_STREAM_FRAME_LIMIT_MAXIMUM, NULL, &sh.hInfraredStreamHandle);
				if (FAILED(hr))
					return false;
				stream_formats.push_back(ir_format = stream_format(640, 480, PF_I, 30, 16));
			}
			// determine the depth stream potentially with player index
			if ((is & IS_DEPTH) != 0) {
				hr = pNuiSensor->NuiImageStreamOpen(
					(is & IS_PLAYER_INDEX) != 0 ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH, 
					NUI_IMAGE_RESOLUTION_640x480, 
					near_mode ? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0, 
					NUI_IMAGE_STREAM_FRAME_LIMIT_MAXIMUM, NULL, &sh.hDepthStreamHandle);
				if (FAILED(hr))
					return false;
				stream_formats.push_back(depth_format = stream_format(640, 480, (is & IS_PLAYER_INDEX) != 0 ? PF_DEPTH_AND_PLAYER : PF_DEPTH, 30, 16));
			}
			handles = new stream_handles(sh);
			return true;
		}
		else
			return false;
	}
	/// start the rgbd device with given stream formats 
	bool rgbd_kinect::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;

		if (camera) {
			// cast camera to NUISensor
			INuiSensor *pNuiSensor = (INuiSensor*)camera;

			// determine flags
			DWORD init_flags = 0;

			for (const auto& sf : stream_formats) {
				switch (sf.pixel_format) {
				case PF_I: 
				case PF_RGB:
				case PF_BGR:
				case PF_RGBA:
				case PF_BGRA:
				case PF_BAYER:
					init_flags |= NUI_INITIALIZE_FLAG_USES_COLOR;
					break;
				case PF_DEPTH:
					init_flags |= NUI_INITIALIZE_FLAG_USES_DEPTH;
					break;
				case PF_DEPTH_AND_PLAYER:
					init_flags |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
					break;
				}
			}
			// try to start the camera
			HRESULT hr = pNuiSensor->NuiInitialize(init_flags);
			if (FAILED(hr))
				return false;

			// determine the color stream
			stream_handles sh;
			InputStreams is_all = IS_NONE;

			for (const auto& sf : stream_formats) {
				DWORD frameFlags = 0;
				NUI_IMAGE_TYPE it;
				HANDLE* sh_ptr = 0;
				InputStreams is = IS_NONE;
				stream_format* sf_ptr = 0;
				switch (sf.pixel_format) {
				case PF_I:
					it = NUI_IMAGE_TYPE_COLOR_INFRARED; 
					sh_ptr = &sh.hInfraredStreamHandle;
					is = IS_INFRARED;
					sf_ptr = &ir_format;
					break;
				case PF_RGB:
				case PF_BGR:
				case PF_RGBA:
				case PF_BGRA:
					it = NUI_IMAGE_TYPE_COLOR;
					sh_ptr = &sh.hColorStreamHandle;
					is = IS_COLOR;
					sf_ptr = &color_format;
					break;
				case PF_BAYER:
					it = NUI_IMAGE_TYPE_COLOR_RAW_BAYER; 
					sh_ptr = &sh.hColorStreamHandle;
					is = IS_COLOR;
					sf_ptr = &color_format;
					break;
				case PF_DEPTH:
					it = NUI_IMAGE_TYPE_DEPTH;
					sh_ptr = &sh.hDepthStreamHandle;
					is = IS_DEPTH;
					sf_ptr = &depth_format;
					break;
				case PF_DEPTH_AND_PLAYER:
					it = NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX;
					sh_ptr = &sh.hDepthStreamHandle;
					is = IS_DEPTH_AND_PLAYER_INDEX;
					sf_ptr = &depth_format;
					break;
				}
				if ((is & IS_DEPTH) != 0 && near_mode)
					frameFlags |= NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE;

				if ((is & is_all) != 0) {
					std::cerr << "vector of stream formats references same stream several times" << std::endl;
				}
				else {
					is_all = InputStreams(is | is_all);
					NUI_IMAGE_RESOLUTION ir;
					if (!extract_resolution(sf, ir)) {
						std::cerr << "rgbd_kinect::start_device(): image resolution " << sf.width << "x" << sf.height << " not supported" << std::endl;
						return false;
					}
					hr = pNuiSensor->NuiImageStreamOpen(it, ir, frameFlags, NUI_IMAGE_STREAM_FRAME_LIMIT_MAXIMUM, NULL, sh_ptr);
					if (FAILED(hr))
						return false;
					*sf_ptr = sf;
				}
			}
			handles = new stream_handles(sh);
			return true;
		}
		else
			return false;
	}
	/// stop the camera
	bool rgbd_kinect::stop_device()
	{
		if (!is_running())
			return true;

		INuiSensor *pNuiSensor = (INuiSensor*)camera;
		pNuiSensor->NuiShutdown();

		stream_handles* sh = (stream_handles*)handles;
		delete sh;
		handles = NULL;
		return true;
	}

	/// return whether device has been started
	bool rgbd_kinect::is_running() const
	{
		return handles != 0;
	}

		/// query a frame of the given input stream
	bool rgbd_kinect::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		if (!is_attached()) {
			cerr << "rgbd_kinect::get_frame called on device that has not been attached" << endl;
			return false;
		}
		INuiSensor *pNuiSensor = (INuiSensor*)camera;

		if (!is_running()) {
			cerr << "rgbd_kinect::get_frame called on device that is not running" << endl;
			return false;
		}

		stream_handles* sh_ptr = (stream_handles*)handles;
		stream_format* sf_ptr = 0;
		HANDLE sh = NULL;
		switch (is) {
		case IS_COLOR:
			sh = sh_ptr->hColorStreamHandle;
			sf_ptr = &color_format;
			break;
		case IS_INFRARED:
			sh = sh_ptr->hInfraredStreamHandle;
			sf_ptr = &ir_format;
			break;
		case IS_DEPTH:
		case IS_DEPTH_AND_PLAYER_INDEX:
			sh = sh_ptr->hDepthStreamHandle;
			sf_ptr = &depth_format;
			break;
		default:
			std::cerr << "rgbd_kinect::get_frame(): combinations of input streams not allowed" << std::endl;
			return false;
		}
		if (sh == NULL) {
			std::cerr << "rgbd_kinect::get_frame(): query of not opened input streamcombinations of input streams not allowed" << std::endl;
			return false;
		}

		NUI_IMAGE_FRAME imageFrame;
		HRESULT hr = pNuiSensor->NuiImageStreamGetNextFrame(sh, timeOut, &imageFrame);
		if (FAILED(hr))
			return false;

		NUI_LOCKED_RECT LockedRect;
		hr = imageFrame.pFrameTexture->LockRect(0, &LockedRect, NULL, 0);
		if (FAILED(hr))
			return false;
		// copy frame format and ensure sufficient size
		static_cast<frame_format&>(frame) = *sf_ptr;
		frame.buffer_size = LockedRect.size;
		if (frame.frame_data.size() != frame.buffer_size)
			frame.frame_data.resize(frame.buffer_size);
		memcpy(&frame.frame_data.front(), LockedRect.pBits, LockedRect.size);
		hr = imageFrame.pFrameTexture->UnlockRect(0);
		hr = pNuiSensor->NuiImageStreamReleaseFrame(sh, &imageFrame);

		return true;
	}
	/// map a color frame to the image coordinates of the depth image
	void rgbd_kinect::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
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

		INuiSensor *pNuiSensor = (INuiSensor*)camera;

		static LONG* m_colorCoordinates = new LONG[depth_frame.width*depth_frame.height * 2];

		NUI_IMAGE_RESOLUTION color_resolution, depth_resolution;
		if (!extract_resolution(color_frame, color_resolution)) {
			std::cerr << "rgbd_kinect::map_color_to_depth(): color resolution " << color_frame.width << "x" << color_frame.height << " not supported" << std::endl;
			return;
		}
		if (!extract_resolution(depth_frame, depth_resolution)) {
			std::cerr << "rgbd_kinect::map_color_to_depth(): depth resolution " << depth_frame.width << "x" << depth_frame.height << " not supported" << std::endl;
			return;
		}
		// Get of x, y coordinates for color in depth space
		// This will allow us to later compensate for the differences in location, angle, etc between the depth and color cameras
		pNuiSensor->NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
			color_resolution,
			depth_resolution,
			depth_frame.width*depth_frame.height,
			const_cast<USHORT*>(reinterpret_cast<const USHORT*>(&depth_frame.frame_data.front())),
			depth_frame.width*depth_frame.height * 2,
			m_colorCoordinates
		);

		// loop over each row and column of the color
		unsigned i = 0;
		char* dest = &warped_color_frame.frame_data.front();
		for (int y = 0; y < depth_frame.height; ++y) {
			for (int x = 0; x < depth_frame.width; ++x) {
				// retrieve the depth to color mapping for the current depth pixel
				LONG color_x = m_colorCoordinates[i]; ++i;
				LONG color_y = m_colorCoordinates[i]; ++i;

				static char zeros[8] = { 0,0,0,0,0,0,0,0 };
				const char* src = zeros;
				// make sure the depth pixel maps to a valid point in color space
				if (color_x >= 0 && color_x < color_frame.width && color_y >= 0 && color_y < color_frame.height)
					src = &color_frame.frame_data.front() + bytes_per_pixel * (color_x + color_frame.width*color_y);
				std::copy(src, src + bytes_per_pixel, dest);
				dest += bytes_per_pixel;
			}
		}
	}
	
	/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
	bool rgbd_kinect::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
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
	rgbd_kinect_driver::rgbd_kinect_driver()
	{
	}
	/// destructor
	rgbd_kinect_driver::~rgbd_kinect_driver()
	{
	}
	/// return the number of kinect devices found by driver
	unsigned rgbd_kinect_driver::get_nr_devices()
	{
		HRESULT hr;
		int sensorCount = 0;
		hr = NuiGetSensorCount(&sensorCount);
		if (FAILED(hr))
		{
			return 0;
		}
		return sensorCount;
	}

	/// return the serial of the i-th kinect devices
	std::string rgbd_kinect_driver::get_serial(int i)
	{
		int n = get_nr_devices();
		if (i >= n)
			return "";

		HRESULT hr;
		INuiSensor *pNuiSensor;

		hr = NuiCreateSensorByIndex(i, &pNuiSensor);
		if (FAILED(hr))
			return "";

		BSTR id = pNuiSensor->NuiUniqueId();
		pNuiSensor->Release();
		return cgv::utils::wstr2str(std::wstring(id));
	}

	/// create a kinect device
	rgbd_device* rgbd_kinect_driver::create_rgbd_device()
	{
		return new rgbd_kinect();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_kinect_driver> rgbd_kinect_driver_registration("kinect_driver");