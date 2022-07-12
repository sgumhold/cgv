#pragma once

#include "rgbd_driver.h"

#include "lib_begin.h"

namespace rgbd {

/** interface to provided access to rgbd devices. This is independent of device driver. 
    Different plugins can implement the rgbd_driver and rgbd_device classes and seemlessly
	integrate into the rgbd_input class. */
class CGV_API rgbd_input
{
public:
	/**@name driver registration */
	//@{
protected:
	/// internal function to provide a list of drivers
	static std::vector<rgbd_driver*>& ref_driver_list();
public:
	/// call to register a new driver
	static void register_driver(rgbd_driver* drv, const std::string& driver_name);
	/// call to unregister a driver - not implemented yet!
	static void unregister_driver(rgbd_driver* drv);
	//@}

	/**@name enumeration of devices*/
	//@{
	/// return the number of rgbd devices
	static unsigned get_nr_devices();
	/// return the serial of the i-th rgbd devices
	static std::string get_serial(int i);
	//@}

	/**@name device attachment */
	//@{
	/// create a rgbd input device object
	rgbd_input();
	/// destruct rgbd device object and automatically detach if necessary
	~rgbd_input();
	/// create a rgbd input device object and attach to device with given serial
	rgbd_input(const std::string& serial);
	/// attach to the rgbd input device of the given serial
	bool attach(const std::string& serial);
	/// return the serial of the device
	const std::string& get_serial() const;
	/// return whether device object is attached to a rgbd input device
	bool is_attached() const;
	/// detach from serial (done automatically in destructor)
	bool detach();
	//@}

	/**@name file io */
	//@{
	/// read a frame from file
	static bool read_frame(const std::string& file_name, frame_type& frame);
	/// write a frame to a file
	static bool write_frame(const std::string& file_name, const frame_type& frame);
	/// attach to a directory that contains saved frames
	bool attach_path(const std::string& path);
	/// enable protocolation of all frames acquired by the attached rgbd input device
	void enable_protocol(const std::string& path);
	/// disable protocolation
	void disable_protocol();
	/// delete recorded protocol
	void clear_protocol(const std::string& path);
	//@}

	/**@name base control*/
	//@{
	/// set the pitch position of the kinect_input base, range of values is [-1, 1]
	bool set_pitch(float y);
	/// query the current measurement of the acceleration sensors within the given time_out in milliseconds; return whether a measurement has been retrieved
	bool put_IMU_measurement(IMU_measurement& m, unsigned time_out) const;
	//@}

	/**@name camera control*/
	//@{
	/// check whether the device supports the given combination of input streams
	bool check_input_stream_configuration(InputStreams is) const;
	/// query the stream formats available for a given stream configuration
	void query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const;
	///
	bool set_near_mode(bool on = true);
	/// start the rgbd input with standard stream formats returned in second parameter
	bool start(InputStreams, std::vector<stream_format>& stream_formats);
	/// start the rgbd input with given stream formats 
	bool start(const std::vector<stream_format>& stream_formats);
	/// check whether device is started
	bool is_started() const;
	/// stop the rgbd input device
	bool stop();
	/// query a frame of the given input stream
	bool get_frame(InputStreams is, frame_type& frame, int timeOut);
	/// map a color frame to the image coordinates of the depth image
	void map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame, frame_type& warped_color_frame) const;
	//! map pixel coordinates and depth to 3d point
	/*! return whether depth was valid
	    point_ptr needs to provide space for 3 floats
		resulting point coordinates are measured in meters with
		- x pointing to the right
		- y to the top, and
		- z in forward direction
		Careful: the corresponding coordinate system is left handed!
	*/
	bool map_depth_to_point(int x, int y, int depth, float* point_ptr) const;
protected:
	/// store whether camera has been started
	bool started;
	/// store attached serial
	std::string serial;
	/// pointer to rgbd input device
	rgbd_device* rgbd;
	/// path where the frame protocol should be saved in order to create an emulation device
	std::string protocol_path;
	/// file index of protocol
	int protocol_idx;
	/// flags used to determine which frames have been saved to file for current index
	unsigned protocol_flags;
public:
	/// whether to write protocol frames asynchronously
	bool protocol_write_async;
protected:
	/// store filename for protocol of warped frames
	mutable std::string next_warped_file_name;
	/// helper function to write protocol frame asynchronously
	bool write_protocol_frame_async(const std::string& fn, const frame_type& frame) const;
	/// cached stream formats
	std::vector<stream_format> streams;
};

/// helper template to register a driver
template <class T>
struct driver_registration
{
	driver_registration(const std::string& driver_name)
	{
		rgbd_input::register_driver(new T, driver_name);
	}
};

}

#include <cgv/config/lib_end.h>