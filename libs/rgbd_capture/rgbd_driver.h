#pragma once

#include "rgbd_device.h"

#include "lib_begin.h"

namespace rgbd {

	/// interface for rgbd drivers (implement only by driver developers)
	class CGV_API rgbd_driver
	{
	public:
		/// virtual destructor
		virtual ~rgbd_driver();
		/// return the number of kinect_input devices found by driver
		virtual unsigned get_nr_devices() = 0;
		/// return the serial of the i-th kinect_input devices
		virtual std::string get_serial(int i) = 0;
		/// in case the rgbd device has a microphone or microphone array, return the id of the corresponding sound device (wasapi id under windows)
		virtual std::string get_audio_device_id(int i);
		/// create a rgbd device
		virtual rgbd_device* create_rgbd_device() = 0;
	};
}

#include <cgv/config/lib_end.h>