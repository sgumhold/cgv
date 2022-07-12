#pragma once

#include <string>
#include <vector>
#include <cgv/math/fvec.h>

#include "lib_begin.h"

namespace rgbd {

	// deprecated enum now split into pixel format and frame  different frame formats
	/*
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
		FF_DEPTH_RGB32,
		// infrared comes single 16 bit channel
		FF_INFRARED
	};
	*/


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
	/// frame size in pixels
	struct frame_size
	{
		/// width of frame in pixel
		int width;
		/// height of frame in pixel 
		int height;
	};

	/// format of individual pixels
	enum PixelFormat {
		PF_I, // infrared

		/* TODO: add color formats in other color spaces like YUV */

		PF_RGB,   // 24 or 32 bit rgb format with byte alignment
		PF_BGR,   // 24 or 24 bit bgr format with byte alignment
		PF_RGBA,  // 32 bit rgba format
		PF_BGRA,  // 32 bit brga format
		PF_BAYER, // 8 bit per pixel, raw bayer pattern values

		PF_DEPTH,
		PF_DEPTH_AND_PLAYER,
		PF_POINTS_AND_TRIANGLES,
		PF_CONFIDENCE
	};

	/// format of a frame
	struct CGV_API frame_format : public frame_size
	{
		/// format of pixels
		PixelFormat pixel_format;
		// total number of bits per pixel
		unsigned nr_bits_per_pixel; 
		/// return number of bytes per pixel (ceil(nr_bits_per_pixel/8))
		unsigned get_nr_bytes_per_pixel() const;
		/// buffer size; returns width*height*get_nr_bytes_per_pixel()
		unsigned buffer_size;
		/// standard computation of the buffer size member
		void compute_buffer_size();
	};

	/// struct to store single frame
	struct frame_info : public frame_format
	{
		///
		unsigned frame_index;
		/// 
		double time;
	};
	/// struct to store single frame
	struct CGV_API frame_type: public frame_info
	{
		/// vector with RAW frame data 
		std::vector<char> frame_data;
		/// check whether frame data is allocated
		bool is_allocated() const;
		/// write to file
		bool write(const std::string& fn) const;
		/// read from file
		bool read(const std::string& fn);
	};

	/// steam format adds frames per second
	struct CGV_API stream_format : public frame_format
	{
		stream_format(int w = 640, int h = 480, PixelFormat pf = PF_RGB, float fps = 30, unsigned _nr_bits = 32, unsigned _buffer_size = -1);
		bool operator == (const stream_format& sf) const;
		float fps;
	};

	extern CGV_API std::ostream& operator << (std::ostream& os, const frame_size& fs);
	extern CGV_API std::ostream& operator << (std::ostream& os, const frame_format& ff);
	extern CGV_API std::ostream& operator << (std::ostream& os, const stream_format& sf);

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
		/// angular acceleration around x, y, and z axes in degrees/s²
		float angular_acceleration[3];
		/// time stamp in milliseconds. Only present if supported by accelerometer
		unsigned long long time_stamp;
	};
	/// return the preferred extension for a given frame format
	extern CGV_API std::string get_frame_extension(const frame_format& ff);
	/// extent file name by frame index and format based extension
	extern CGV_API std::string compose_file_name(const std::string& file_name, const frame_format& ff, unsigned idx);

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
		/// query the stream formats available for a given stream configuration
		virtual void query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const = 0;
		/// start the rgbd device with standard stream formats returned in second parameter
		virtual bool start_device(InputStreams is, std::vector<stream_format>& stream_formats) = 0;
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