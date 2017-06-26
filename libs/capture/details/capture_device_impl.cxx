#include "capture_device_impl.h"

namespace capture {

	/// attach instance to device based on serial
	capture_device_impl::~capture_device_impl()
	{
	}
	/// return whether device object is attached to a device
	bool capture_device_impl::is_attached() const
	{
		return false;
	}
	/// set the pan value of a device that supports pan and return whether this was possible
	CaptureResult capture_device_impl::set_pan(float angle_in_degrees) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_tilt(float angle_in_degrees) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_roll(float angle_in_degrees) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_zoom(float factor) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_near_field_mode(bool on) { return CR_PROPERTY_UNSUPPORTED; }

	CaptureResult capture_device_impl::set_auto_exposure(bool on) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_auto_white_balance(bool on) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_backlight_compensation(bool on) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_live_view(bool on) { return CR_PROPERTY_UNSUPPORTED; }

	CaptureResult capture_device_impl::set_ae_mode_index(int index) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_focus_mode_index(int index) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_focus(float value) { return CR_PROPERTY_UNSUPPORTED; }

	CaptureResult capture_device_impl::set_aperture(float value) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_exposure(float value) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_gain(float value) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_white_balance(float value) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_power_line_frequency_index(int index) { return CR_PROPERTY_UNSUPPORTED; }

	CaptureResult capture_device_impl::set_brightness(float value) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_contrast(float value) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_hue(float value) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_sharpness(float value) { return CR_PROPERTY_UNSUPPORTED; }
	CaptureResult capture_device_impl::set_gamma(float value) { return CR_PROPERTY_UNSUPPORTED; }

	CaptureResult capture_device_impl::set_quality_index(int index) { return CR_PROPERTY_UNSUPPORTED; }

	CaptureResult capture_device_impl::set_color_image_format(const image_format& format) { return CR_FAILURE; }
	CaptureResult capture_device_impl::set_color_stream_format(const image_stream_format& format) { return CR_FAILURE; }
	CaptureResult capture_device_impl::set_depth_image_format(const image_format& format) { return CR_FAILURE; }
	CaptureResult capture_device_impl::set_depth_stream_format(const image_stream_format& format) { return CR_FAILURE; }
	CaptureResult capture_device_impl::set_infrared_image_format(const image_format& format) { return CR_FAILURE; }
	CaptureResult capture_device_impl::set_infrared_stream_format(const image_stream_format& format) { return CR_FAILURE; }

	/// Retrieve the next acceleration measurement. If no measurement is available wait up to time_out milliseconds. Return whether a measurement has been retrieved.
	CaptureResult capture_device_impl::measure_acceleration(accelerometer_measurement& m, unsigned time_out) { return CR_FAILURE;  }

	/// prepare streaming of the specified input streams but do not yet start streaming, remember the capture processor
	CaptureResult capture_device_impl::prepare_streaming(InputStreams input_streams, capture_processor* cp) { return CR_FAILURE; }
	/// trigger a previously prepared streaming
	CaptureResult capture_device_impl::trigger_streaming() { return CR_FAILURE; }

	/// start streaming of the specified input streams. For each frame call the process_frame method of the passed capture processor.
	CaptureResult capture_device_impl::start_streaming(InputStreams input_streams, capture_processor* cp) { return CR_FAILURE; }
	/// stop streaming
	CaptureResult capture_device_impl::stop_streaming() { return CR_FAILURE; }

	/// prepare shooting a single image, adjust automatic properties like focus or white balance
	CaptureResult capture_device_impl::prepare_shooting(InputStreams input_streams, capture_processor* cp) { return CR_FAILURE; }
	/// trigger a previously prepared shooting
	CaptureResult capture_device_impl::trigger_shooting() { return CR_FAILURE; }

	/// prepare and shoot a single frame and call the process_frame method of the capture processor as soon as frame is available
	CaptureResult capture_device_impl::shoot_image(InputStreams input_streams, capture_processor* cp) { return CR_FAILURE; }

	/// prepare and shoot a sequence of nr_frames frames and call the process_frame method of the capture processor once for each captured frame
	CaptureResult capture_device_impl::shoot_image_sequence(InputStreams input_streams, unsigned nr_frames, capture_processor* fp) { return CR_FAILURE; }
}
