#pragma once

#include <rgbd_input.h>

#include "lib_begin.h"

#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <k4a/k4a.hpp>
#include <k4a/k4atypes.h>
#define INVALID INT32_MIN

namespace rgbd {


	typedef struct _pinhole_t
	{
		float px;
		float py;
		float fx;
		float fy;

		int width;
		int height;
	} pinhole_t;

	typedef struct _coordinate_t
	{
		int x;
		int y;
		float weight[4];
	} coordinate_t;

	typedef enum
	{
		INTERPOLATION_NEARESTNEIGHBOR, /**< Nearest neighbor interpolation */
		INTERPOLATION_BILINEAR,        /**< Bilinear interpolation */
		INTERPOLATION_BILINEAR_DEPTH   /**< Bilinear interpolation with invalidation when neighbor contain invalid
													 data with value 0 */
	} interpolation_t;

	/// interface for kinect devices provided by a driver (only to be used by driver implementors)
	class CGV_API rgbd_kinect_azure : public rgbd_device
	{
	public:
		/// create a detached kinect CLNUI device object
		rgbd_kinect_azure();
		~rgbd_kinect_azure();
		std::string get_audio_device() const;
		/// attach to the kinect device of the given serial
		bool attach(const std::string& serial);
		/// return whether device object is attached to a kinect device
		bool is_attached() const;
		/// detach from serial (done automatically in constructor
		bool detach();

		/// check whether a multi-device role is supported
		bool is_supported(MultiDeviceRole mdr) const;
		/// configure device for a multi-device role and return whether this was successful (do this before starting)
		bool configure_role(MultiDeviceRole mdr);
		/// whether device supports external synchronization
		bool is_sync_supported() const;
		/// return whether syncronization input jack is connected
		bool is_sync_in_connected() const;
		/// return whether syncronization output jack is connected
		bool is_sync_out_connected() const;

		/// get range, step and default value of color control parameter
		const std::vector<color_parameter_info>& get_supported_color_control_parameter_infos() const;
		/// query color control value and whether its adjustment is in automatic mode
		std::pair<int32_t, bool> get_color_control_parameter(ColorControlParameter ccp) const;
		/// set a color control value and automatic mode and return whether successful
		bool set_color_control_parameter(ColorControlParameter ccp, int32_t value, bool automatic_mode);

		/// check whether rgbd device has inertia measurement unit
		bool has_IMU() const;
		/// return additional information on inertia measurement unit
		const IMU_info& get_IMU_info() const;
		/// query the current measurement of the acceleration sensors within the given time_out in milliseconds; return whether a measurement has been retrieved
		bool put_IMU_measurement(IMU_measurement& m, unsigned time_out) const;
		/// check whether the device supports the given combination of input streams
		bool check_input_stream_configuration(InputStreams is) const;
		/// query the stream formats available for a given stream configuration
		void query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const;
		/// start the camera
		bool start_device(InputStreams is, std::vector<stream_format>& stream_formats);
		/// start the rgbd device with given stream formats 
		virtual bool start_device(const std::vector<stream_format>& stream_formats);
		///
		bool query_calibration(rgbd_calibration& calib);
		/// stop the camera
		bool stop_device();
		/// return whether device has been started
		bool is_running() const;
		/// query a frame of the given input stream
		bool get_frame(InputStreams is, frame_type& frame, int timeOut);
		/// map a color frame to the image coordinates of the depth image
		void map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
			frame_type& warped_color_frame) const;
		/// map a depth value together with pixel indices to a 3D point with coordinates in meters; point_ptr needs to provide space for 3 floats
		bool map_depth_to_point(int x, int y, int depth, float* point_ptr) const;


		void invert_2x2(const float J[2 * 2], float Jinv[2 * 2]) const; 
		bool transformation_iterative_unproject(const float* uv, float* xy, bool valid, unsigned int max_passes) const;
		bool transformation_project_internal(const k4a_calibration_camera_t* camera_calibration_tt, const float xy[2],
											 float uv[2], bool valid, float J_xy[2 * 2]) const;
		bool transformation_unproject_internal(const k4a_calibration_camera_t* camera_calibration, const float uv[2],
											   float xy[2], bool valid) const;

		virtual bool get_emulator_configuration(emulator_parameters& parameters) const override;
		// undistort
		void compute_xy_range(const k4a_calibration_t* calibration,
			const k4a_calibration_type_t camera,
			const int width,
			const int height,
			float& x_min,
			float& x_max,
			float& y_min,
			float& y_max);
		pinhole_t create_pinhole_from_xy_range(const k4a_calibration_t* calibration, const k4a_calibration_type_t camera);
		void create_undistortion_lut(const k4a_calibration_t* calibration,
			const k4a_calibration_type_t camera,
			const pinhole_t* pinhole,
			k4a_image_t lut,
			interpolation_t type);
		void remap(const k4a_image_t src, const k4a_image_t lut, k4a_image_t dst, interpolation_t type);

	private:
		mutable std::vector<color_parameter_info> ccp_infos;
		std::unique_ptr<k4a::error> capture_thread_device_error;
		std::atomic<bool> capture_thread_has_new_messages = false;
		bool capture_thread_broken = false;

		mutable k4a::device device;
		std::string device_serial;
		std::atomic<bool> device_started;

		stream_format color_format, ir_format, depth_format;
		bool near_mode;

		std::unique_ptr<std::thread> capture_thread;
		
		mutable std::mutex capture_lock;
		volatile bool has_new_color_frame, has_new_depth_frame, has_new_ir_frame;
		unsigned frame_cache_size = 4, current_read_color_frame = 0, current_write_color_frame = 0, current_read_depth_frame = 0, current_write_depth_frame = 0, current_read_ir_frame = 0, current_write_ir_frame = 0;
		std::vector<std::unique_ptr<rgbd::frame_type>> color_frames, depth_frames, ir_frames;
		//rgbd::frame_type *color_frame =nullptr, *depth_frame=nullptr, *ir_frame=nullptr;
		bool imu_enabled;
		mutable IMU_measurement imu_data;
		mutable volatile bool has_new_IMU_data;
		k4a::calibration camera_calibration;
		k4a::transformation camera_transform;
		k4a_calibration_intrinsic_parameters_t* intrinsics;
		k4a_calibration_intrinsics_t* intrinsics_t;
		k4a_calibration_camera_t* camera_calibration_t;
		k4a_calibration_t* calibration_t;

		int returnCode = 1;
		k4a_device_t device_t = NULL;
		const int32_t TIMEOUT_IN_MS = 1000;
		k4a_capture_t capt = NULL;
		std::string file_name;
		uint32_t device_count = 0;
		k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
		k4a_image_t depth_image = NULL;
		k4a_image_t lut = NULL;
		k4a_image_t undistorted = NULL;
		interpolation_t interpolation_type = INTERPOLATION_NEARESTNEIGHBOR;
		//interpolation_t interpolation_type = INTERPOLATION_BILINEAR_DEPTH;
		//interpolation_t interpolation_type = INTERPOLATION_BILINEAR;
		pinhole_t pinhole;
	protected:
		void capture(int is);
		void check_errors();
		bool recover_from_errors();
	};

	/// interface for kinect drivers (implement only as driver implementor)
	class CGV_API rgbd_kinect_azure_driver : public rgbd_driver
	{
	public:
		/// construct driver
		rgbd_kinect_azure_driver();
		/// destructor
		~rgbd_kinect_azure_driver();
		/// return the number of kinect devices found by driver
		unsigned get_nr_devices();
		/// return the serial of the i-th kinect devices
		std::string get_serial(int i);
		/// in case the rgbd device has a microphone or microphone array, return the id of the corresponding sound device (wsapi id under windows)
		std::string get_audio_device_id(int i);
		/// create a kinect device
		rgbd_device* create_rgbd_device();
	};
}
#include <cgv/config/lib_end.h>