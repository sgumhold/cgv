#pragma once

#include <string>
#include <vector>

#include "lib_begin.h"

namespace rgbd {

	/// different frame formats
	enum FrameFormat {
		FF_COLOR_RAW,
		FF_COLOR_RGB24,
		FF_COLOR_RGB32,
		// raw depth values
		FF_DEPTH_RAW,
		// depthCorrected8 = (256*2048) / (2048-depthRaw), range [0,255]
		FF_DEPTH_D8,
		// depthCorrected12 = (2048*2048) / (2048-depthRaw), range [0, 2047]
		FF_DEPTH_D12,
		FF_DEPTH_RGB32
	};


	struct image_size
	{
		/// width of the image
		int width;
		/// height of the image
		int height;
	};

	enum PixelFormat {
		PF_INFRARED,
		PF_LONG_INFRARED,
		PF_Y8,
		PF_Y16,
		PF_YUV,
		PF_YUV2,
		PF_NV12,
		PF_RGB,
		PF_BGR,
		PF_RGBA,
		PF_BGRA,
		PF_BAYER,
		PF_DEPTH8,
		PF_DEPTH16,
		PF_DEPTH_CONFIDENCE,
		PF_RGBD32,
	};

	struct image_format : public image_size
	{
		PixelFormat pixel_format;
	};

	struct frame_info : public image_format
	{
		/// The pointer to RAW data in the user-specified format
		void* data_ptr;
		/// Buffer size (normally width*height*bits_per_pixel)
		int buffer_size;
		/// 
		long long time_stemp;
		///
		unsigned frame_index;
	};

	/// different input streams
	enum InputStreams {
		IS_NONE = 0,
		IS_COLOR = 1,
		IS_DEPTH = 2,
		IS_DEPTH_CONFIDENCE = 4,
		IS_INFRARED = 8,
		IS_PLAYER_INDEX = 16,
		IS_SKELETON = 32,

		IS_COLOR_AND_DEPTH = IS_COLOR | IS_DEPTH,
		IS_DEPTH_AND_PLAYER_INDEX = IS_DEPTH | IS_PLAYER_INDEX,

		IS_ALL = 63
	};

	/// info on view finder capabilities
	struct view_finder_info
	{
		/// number of degrees of freedom that can be adjusted
		unsigned degrees_of_freedom;
		/// up to three minima of adjustable angles in degrees
		float min_angle[3];
		/// up to three maximum of adjustable angles in degrees
		float max_angle[3];
	};

	/// info in inertia measurement unit
	struct IMU_info
	{
		/// return whether measurement supports linear acceleration measurements
		bool has_linear_acceleration;
		/// return whether measurement supports angular acceleration measurements
		bool has_angular_acceleration;
		/// return whether measurement supports time stamp
		bool has_time_stamp_support;
	};

	/// struct that represents a measurement from an inertia measurement unit
	struct IMU_measurement
	{
		/// linear acceleration along x, y, and z axes in m/s²
		float linear_acceleration[3];
		/// angular acceleration around x, y, and z axes in degrees/s²
		float angular_acceleration[3];
		/// time stamp in milliseconds. Only present if supported by accelerometer
		unsigned long long time_stamp;
	};
	/// return the preferred extension for a given frame format
	extern CGV_API const char* get_frame_extension(FrameFormat ff);
	/// extent file name by frame index and format based extension
	extern CGV_API std::string compose_file_name(const std::string& file_name, FrameFormat ff, unsigned idx);

	/// interface for rgbd devices provided by a driver (this class is used by driver implementors)
	class CGV_API rgbd_device
	{
	public:
		/// virtual destructor
		virtual ~rgbd_device();
		/// attach to the kinect device of the given serial
		virtual bool attach(const std::string& serial) = 0;
		/// return whether device object is attached to a kinect_input device
		virtual bool is_attached() const = 0;
		/// detach from serial (done automatically in constructor
		virtual bool detach() = 0;

		/// return whether rgbd device has support for view finding actuator
		virtual bool has_view_finder() const;
		/// return a view finder info structure
		virtual const view_finder_info& get_view_finder_info() const;
		/// set the pitch position of the rgbd device in degrees with 0 as middle position, return whether this was successful
		virtual bool set_pitch(float y);
		/// return the pitch position of the rgbd device in degrees with 0 as middle position
		virtual float get_pitch() const;

		/// whether rgbd device has support for a near field depth mode
		virtual bool has_near_mode() const;
		/// return whether the near field depth mode is activated
		virtual bool get_near_mode() const;
		/// activate or deactivate near field depth mode, return whether this was successful
		virtual bool set_near_mode(bool on = true);

		/// check whether rgbd device has inertia measurement unit
		virtual bool has_IMU() const;
		/// return additional information on inertia measurement unit
		virtual const IMU_info& get_IMU_info() const;
		/// query the current measurement of the acceleration sensors within the given time_out in milliseconds; return whether a measurement has been retrieved
		virtual bool put_IMU_measurement(IMU_measurement& m, unsigned time_out) const;

		/// check whether the device supports the given combination of input streams
		virtual bool check_input_stream_configuration(InputStreams is) const = 0;
		/// start the rgbd device
		virtual bool start_device(InputStreams is) = 0;
		/// stop the rgbd device
		virtual bool stop_device() = 0;
		/// return whether device has been started
		virtual bool is_running() const = 0;
		/// return the image width of the frames
		virtual unsigned get_width(InputStreams is = IS_DEPTH) const = 0;
		/// return the image width of the frames
		virtual unsigned get_height(InputStreams is = IS_DEPTH) const = 0;
		/// compute the frame size of the given format
		virtual unsigned get_entry_size(FrameFormat ff) const;
		/// compute the frame size of the given format
		virtual unsigned get_frame_size(FrameFormat ff) const;
		/// query a frame in the given format from color or depth camera
		virtual bool get_frame(FrameFormat ff, void* data_ptr, int timeOut) = 0;
		/// map a color frame to the image coordinates of the depth image
		virtual void map_color_to_depth(FrameFormat depth_ff, const void* depth_data_ptr, FrameFormat color_ff, void* color_data_ptr) const = 0;
	};
}

#include <cgv/config/lib_end.h>