#pragma once

#include "../capture_result.h"
#include "../device_capabilities.h"
#include "../device_status.h"
#include "../capture_processor.h"

#include "../lib_begin.h"

namespace capture {

/// interface for capture devices provided by a driver (only to be used by driver implementors)
class CGV_API capture_device_impl
{
public:
	/// virtual destructor calls detach if necessary
	virtual ~capture_device_impl();
	/// attach instance to device based on serial
	virtual CaptureResult attach(const std::string& serial) = 0;
	/// return whether device object is attached to a device
	virtual bool is_attached() const;
	/// detach from device (done automatically in destructor)
	virtual CaptureResult detach() = 0;

	/// return capabilities, only valid after attachment to device
	virtual const device_capabilities& get_capabilities() const = 0;
	/// return the serial of the device, only valid after attachment to device
	virtual const std::string& get_serial() const = 0;

	/// return the current device status update values of all properties with automatic adjustment
	virtual const device_status& get_status(bool with_updated_automatic_properties) const = 0;

	/// set the pan value of a device that supports pan and return whether this was possible
	virtual CaptureResult set_pan(float angle_in_degrees);
	/// set the tilt value of a device that supports tilt and return whether this was possible
	virtual CaptureResult set_tilt(float angle_in_degrees);
	/// set the roll value of a device that supports roll and return whether this was possible
	virtual CaptureResult set_roll(float angle_in_degrees);
	/// set the zoom value of a device that supports zoom and return whether this was possible
	virtual CaptureResult set_zoom(float factor);
	/// activate or deactivate near field mode for depth measurement, return whether this was successful
	virtual CaptureResult set_near_field_mode(bool on = true);

	virtual CaptureResult set_auto_exposure(bool on = true);
	virtual CaptureResult set_auto_white_balance(bool on = true);
	virtual CaptureResult set_backlight_compensation(bool on = true);
	virtual CaptureResult set_live_view(bool on = true);

	virtual CaptureResult set_ae_mode_index(int index);
	virtual CaptureResult set_focus_mode_index(int index);
	virtual CaptureResult set_focus(float value);

	virtual CaptureResult set_aperture(float value);
	virtual CaptureResult set_exposure(float value);
	virtual CaptureResult set_gain(float value);
	virtual CaptureResult set_white_balance(float value);
	virtual CaptureResult set_power_line_frequency_index(int index);

	virtual CaptureResult set_brightness(float value);
	virtual CaptureResult set_contrast(float value);
	virtual CaptureResult set_hue(float value);
	virtual CaptureResult set_sharpness(float value);
	virtual CaptureResult set_gamma(float value);

	virtual CaptureResult set_quality_index(int index);

	virtual CaptureResult set_color_image_format(const image_format& format);
	virtual CaptureResult set_color_stream_format(const image_stream_format& format);
	virtual CaptureResult set_depth_image_format(const image_format& format);
	virtual CaptureResult set_depth_stream_format(const image_stream_format& format);
	virtual CaptureResult set_infrared_image_format(const image_format& format);
	virtual CaptureResult set_infrared_stream_format(const image_stream_format& format);

	/// Retrieve the next acceleration measurement. If no measurement is available wait up to time_out milliseconds. Return whether a measurement has been retrieved.
	virtual CaptureResult measure_acceleration(accelerometer_measurement& m, unsigned time_out);

	/// prepare streaming of the specified input streams but do not yet start streaming, remember the capture processor
	virtual CaptureResult prepare_streaming(InputStreams input_streams, capture_processor* cp = 0);
	/// trigger a previously prepared streaming
	virtual CaptureResult trigger_streaming();

	/// start streaming of the specified input streams. For each frame call the process_frame method of the passed capture processor.
	virtual CaptureResult start_streaming(InputStreams input_streams, capture_processor* cp = 0);
	/// stop streaming
	virtual CaptureResult stop_streaming();

	/// prepare shooting a single image, adjust automatic properties like focus or white balance
	virtual CaptureResult prepare_shooting(InputStreams input_streams, capture_processor* cp = 0);
	/// trigger a previously prepared shooting
	virtual CaptureResult trigger_shooting();

	/// prepare and shoot a single frame and call the process_frame method of the capture processor as soon as frame is available
	virtual CaptureResult shoot_image(InputStreams input_streams, capture_processor* cp = 0);

	/// prepare and shoot a sequence of nr_frames frames and call the process_frame method of the capture processor once for each captured frame
	virtual CaptureResult shoot_image_sequence(InputStreams input_streams, unsigned nr_frames, capture_processor* fp = 0);
};

}

#include <cgv/config/lib_end.h>