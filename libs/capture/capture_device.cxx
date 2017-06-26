#include "capture_device.h"
#include "details/capture_device_impl.h"
#include "details/capture_driver.h"

namespace capture {


/// scan for supported capture devices with all registered drivers and return the number of found devices
unsigned scan_devices(bool scan_capabilities)
{

}

/// return the number of capture devices, if scan devices has not been called before, call it without scanning capabilities
unsigned get_nr_devices()
{

}

/// return the serial of the i-th capture device
const std::string& get_serial(int i)
{

}

/// return the capabilities of the i-th capture device
const device_capabilities& get_capabilities(int i)
{

}
//@}


/** the capture interface provides access to color and depth camera devices. This is independent of the device driver. 
    Different plugins can implement the driver and capture_device classes and seemlessly
	integrate into the capture class. */
class CGV_API capture_device
{
public:
	/**@name device attachment */
	//@{
	/// create a capature device object
	capture_device();
	/// destruct device object and automatically detach if necessary
	~capture_device();
	/// create a capture device object and attach to device with given serial
	capture_device(const std::string& serial);
	/// attach to the capture device of the given serial
	CaptureResult attach(const std::string& serial);
	/// return the serial of the device
	const std::string& get_serial() const;
	/// return whether device object is attached to a capture device
	bool is_attached() const;
	/// detach from serial (done automatically in constructor
	CaptureResult detach();
	/// return the current device status update values of all properties with automatic adjustment
	const device_status& get_status(bool with_updated_automatic_properties) const;
	//@}

	/**@name file io */
	//@{
	/// read a frame from file
	static bool read_frame(const std::string& file_name, void* data_ptr, unsigned frame_size);
	/// write a frame to a file
	static bool write_frame(const std::string& file_name, const void* data_ptr, unsigned frame_size);
	/// attach to a directory that contains saved frames
	bool attach_path(const std::string& path);
	/// enable protocolation of all frames acquired by the attached kinect_input device
	void enable_protocol(const std::string& path);
	/// disable protocolation
	void disable_protocol();
	//@}

	/**@name pan tilt roll and zoom control*/
	//@{
	/// set the pan value of a device that supports pan and return whether this was possible
	CaptureResult set_pan(float angle_in_degrees);
	/// set the tilt value of a device that supports tilt and return whether this was possible
	CaptureResult set_tilt(float angle_in_degrees);
	/// set the roll value of a device that supports roll and return whether this was possible
	CaptureResult set_roll(float angle_in_degrees);
	/// set the zoom value of a device that supports zoom and return whether this was possible
	CaptureResult set_zoom(float factor);
	/// Retrieve the next acceleration measurement. If no measurement is available wait up to time_out milliseconds. Return whether a measurement has been retrieved.
	CaptureResult measure_acceleration(accelerometer_measurement& m, unsigned time_out);
	//@}

	/**@name configuration of device */
	//@{
	/// activate or deactivate near field mode for depth measurement, return whether this was successful
	CaptureResult set_near_field_mode(bool on = true);

	CaptureResult set_auto_exposure(bool on = true);
	CaptureResult set_auto_white_balance(bool on = true);
	CaptureResult set_backlight_compensation(bool on = true);
	CaptureResult set_live_view(bool on = true);

	CaptureResult set_ae_mode_index(int index);
	CaptureResult set_focus_mode_index(int index);
	//@}


	/**@name property adjustment */
	//@{
	CaptureResult set_focus(float value);

	CaptureResult set_aperture(float value);
	CaptureResult set_exposure(float value);
	CaptureResult set_gain(float value);
	CaptureResult set_white_balance(float value);
	CaptureResult set_power_line_frequency_index(int index);

	CaptureResult set_brightness(float value);
	CaptureResult set_contrast(float value);
	CaptureResult set_hue(float value);
	CaptureResult set_sharpness(float value);
	CaptureResult set_gamma(float value);

	CaptureResult set_quality_index(int index);
	//@}

	/**@name image format selection */
	//@{
	CaptureResult set_color_image_format(const image_format& format);
	CaptureResult set_color_stream_format(const image_stream_format& format);
	CaptureResult set_depth_image_format(const image_format& format);
	CaptureResult set_depth_stream_format(const image_stream_format& format);
	CaptureResult set_infrared_image_format(const image_format& format);
	CaptureResult set_infrared_stream_format(const image_stream_format& format);
	//@}

	/**@name capturing functions */
	//@{
	/// prepare streaming of the specified input streams but do not yet start streaming, remember the capture processor
	CaptureResult prepare_streaming(InputStreams input_streams, capture_processor* cp = 0);
	/// trigger a previously prepared streaming
	CaptureResult trigger_streaming();

	/// start streaming of the specified input streams. For each frame call the process_frame method of the passed capture processor.
	CaptureResult start_streaming(InputStreams input_streams, capture_processor* cp = 0);
	/// stop streaming
	CaptureResult stop_streaming();

	/// prepare shooting a single image, adjust automatic properties like focus or white balance
	CaptureResult prepare_shooting(InputStreams input_streams, capture_processor* cp = 0);
	/// trigger a previously prepared shooting
	CaptureResult trigger_shooting();

	/// prepare and shoot a single frame and call the process_frame method of the capture processor as soon as frame is available
	CaptureResult shoot_image(InputStreams input_streams, capture_processor* cp = 0);

	/// prepare and shoot a sequence of nr_frames frames and call the process_frame method of the capture processor once for each captured frame
	CaptureResult shoot_image_sequence(InputStreams input_streams, unsigned nr_frames, capture_processor* fp = 0);
	//@}
protected:
	/// store attached serial
	std::string serial;
	/// pointer to capture device
	capture_device_impl* kd;
	/// pointer to driver that created the device
	capture_driver* cd;
	/// path where the frame protocol should be saved in order to create an emulation device
	std::string protocol_path;
	/// file index of protocol
	int protocol_idx;
	/// flags used to determine which frames have been saved to file for current index
	unsigned protocol_flags;
};

}

#include <cgv/config/lib_end.h>