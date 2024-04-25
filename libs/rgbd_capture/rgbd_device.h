#pragma once

#include "frame.h"
#include "rgbd_calibration.h"
#include <cgv/math/fvec.h>
#include <cgv/math/camera.h>

#include "lib_begin.h"

namespace rgbd {
	struct camera_intrinsics {
		double fx, fy; //focal length, width in pixel
		double cx, cy; //principal point;
		double sk; //skew
		unsigned image_width, image_height;
	};
	///struct for representing parameters used in the device emulator
	struct emulator_parameters {
		camera_intrinsics intrinsics;
		double depth_scale;
	};

	/// helper object for parsing dynamic sized frames of type PF_POINTS_AND_TRIANGLES. 
	struct CGV_API mesh_data_view {
		typedef cgv::math::fvec<float, 3>  Point;
		typedef cgv::math::fvec<float, 2>  TextureCoord;
		typedef cgv::math::fvec<uint32_t, 3>  Triangle;
		//pointers to continous sequences of objects
		Point* points;
		Triangle* triangles;
		TextureCoord *uv;
		//number of objects
		size_t points_size, triangles_size, uv_size;

		mesh_data_view(char* data, const size_t size);

		bool parse_data(char* data, const size_t size);
		
		inline bool is_valid() {
			return points != nullptr;
		}
	};
	//! different roles to support synchronized multi-rgbd-device setting
	/** One role per rgbd device or rgbd input. Only kinect azur driver supports leader
	    and follower role. As with the kinect Azure viewer one can start rgbd_control 
		plugin once per attached device and configure leader role for the device that 
		is used as master and has synch cable plugged into the Synch Out plug only, and 
		the follower role for all other devices. All followers need to be started first
		and actually stream data as soons as the leader has been started. */
	enum MultiDeviceRole
	{
		MDR_STANDALONE, /// device used without synchronization
		MDR_LEADER,     /// device that sends synchronization signals
		MDR_FOLLOWER    /// device that retrieves synchronization signals
	};

	/// different color camera control parameters
	enum ColorControlParameter {
		CCP_EXPOSURE,
		CCP_BRIGHTNESS,
		CCP_CONTRAST,
		CCP_SATURATION,
		CCP_SHARPNESS,
		CCP_WHITEBALANCE,
		CCP_BACKLIGHT_COMPENSATION,
		CCP_GAIN,
		CCP_POWERLINE_FREQUENCY
	};
	/// information about a color control parameter
	struct color_parameter_info
	{
		std::string name;
		ColorControlParameter parameter_id;
		bool supports_automatic_adjustment, default_automatic_adjustment;
		int32_t min_value, max_value, value_step, default_value;
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
		IS_MESH = 64,

		IS_COLOR_AND_DEPTH = IS_COLOR | IS_DEPTH,
		IS_DEPTH_AND_PLAYER_INDEX = IS_DEPTH | IS_PLAYER_INDEX,
		
		IS_ALL = 127
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
		/// angular velocity around x, y, and z axes in degrees/s
		float angular_velocity[3];
		/// time stamp in milliseconds. Only present if supported by accelerometer
		unsigned long long time_stamp;
		/// additional time stamp for angular measurements
		unsigned long long angular_time_stamp;
	};
	/// return the preferred extension for a given frame format
	extern CGV_API std::string get_frame_extension(const frame_format& ff);
	/// extent file name by frame index and format based extension
	extern CGV_API std::string compose_file_name(const std::string& file_name, const frame_format& ff, unsigned idx);

	/// interface for rgbd devices provided by a driver (this class is used by driver implementors)
	class CGV_API rgbd_device
	{
	protected:
		/// store role for starting the device
		MultiDeviceRole multi_device_role = MDR_STANDALONE;
	public:
		/// virtual destructor
		virtual ~rgbd_device();
		/// in case the rgbd device has a microphone or microphone array, return the id of the corresponding sound device (wasapi id under windows)
		virtual std::string get_audio_device() const;
		/// attach to the kinect device of the given serial
		virtual bool attach(const std::string& serial) = 0;
		/// return whether device object is attached to a kinect_input device
		virtual bool is_attached() const = 0;
		/// detach from serial (done automatically in constructor
		virtual bool detach() = 0;

		/// check whether a multi-device role is supported
		virtual bool is_supported(MultiDeviceRole mdr) const;
		/// configure device for a multi-device role and return whether this was successful (do this before starting)
		virtual bool configure_role(MultiDeviceRole mdr);
		/// return the multi-device role of the device
		MultiDeviceRole get_role() const { return multi_device_role; }
		/// whether device supports external synchronization
		virtual bool is_sync_supported() const { return false; }
		/// return whether syncronization input jack is connected
		virtual bool is_sync_in_connected() const { return false; }
		/// return whether syncronization output jack is connected
		virtual bool is_sync_out_connected() const { return false; }

		/// return information about the support color control parameters
		virtual const std::vector<color_parameter_info>& get_supported_color_control_parameter_infos() const;
		/// query color control value and whether its adjustment is in automatic mode
		virtual std::pair<int32_t, bool> get_color_control_parameter(ColorControlParameter ccp) const;
		/// set a color control value and automatic mode and return whether successful
		virtual bool set_color_control_parameter(ColorControlParameter ccp, int32_t value, bool automatic_mode);

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
		/// query the stream formats available for a given stream configuration
		virtual void query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const = 0;
		/// start the rgbd device with standard stream formats returned in second parameter
		virtual bool start_device(InputStreams is, std::vector<stream_format>& stream_formats) = 0;
		/// query current calibration information (device must be started) and return whether this was successful
		virtual bool query_calibration(rgbd_calibration& calib);
		/// start the rgbd device with given stream formats 
		virtual bool start_device(const std::vector<stream_format>& stream_formats) = 0;
		/// stop the rgbd device
		virtual bool stop_device() = 0;
		/// return whether device has been started
		virtual bool is_running() const = 0;
		/// query a frame of the given input stream
		virtual bool get_frame(InputStreams is, frame_type& frame, int timeOut) = 0;
		/// map a color frame to the image coordinates of the depth image
		virtual void map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame, 
			frame_type& warped_color_frame) const = 0;
		/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
		virtual bool map_depth_to_point(int x, int y, int depth, float* point_ptr) const = 0;
		/// get the camera parameters
		virtual bool get_emulator_configuration(emulator_parameters& parameters) const;
	};
}

#include <cgv/config/lib_end.h>