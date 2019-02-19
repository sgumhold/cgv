#pragma once

#include "vr_event.h"

#include <vector>

#include "lib_begin.h"

namespace vr {
	/// forward declaration of vr driver class
	class CGV_API vr_driver;
	/**@name vr device management */
	//@{
	/// information provided per vr device
	class CGV_API vr_kit
	{
	protected:
		/// pointer to driver that created the vr kit
		vr_driver* driver;
		/// device handle for internal use
		void* device_handle;
		/// name in case driver provides this information (not reliable)
		std::string name;
		/// whether force feedback is supported
		bool force_feedback_support;
		/// whether it is wireless
		bool wireless;
		/// store last error here
		std::string last_error;
		/// construct
		vr_kit(vr_driver* _driver, void* _handle, const std::string& _name, bool _ffb_support, bool _wireless);
	public:
		/// return driver
		const vr_driver* get_driver() const;
		/// return device handle
		void* get_device_handle() const;
		/// return name of vr_kit
		const std::string& get_name() const;
		/// return last error of vr_kit
		const std::string& get_last_error() const;
		/// return whether vr_kit is wireless
		bool is_wireless() const;
		/// return whether controllers support force feedback
		bool has_force_feedback() const;
		/// declare virtual destructor
		virtual ~vr_kit();
		//! query current state of vr kit and return whether this was successful
		/*! if pose_query is 
			0 ... no poses are queried
			1 ... most current pose for controller is queried for example to get pose at button press in highest precision
			2 ... future pose for rendering next frame is queried for controllers and hmd*/
		virtual bool query_state(vr_kit_state& state, int pose_query = 2) = 0;
		//! NOT IMPLEMENTED retrieve next key event from given device, return false if device's event queue is empty
		/*!Typically you use this function in the following way:
			while (query_key_event(i,key,action)) { process(key,action); } */
		/// virtual bool query_key_event(VRKeys& key, KeyAction& action) = 0;
		/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
		virtual bool set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength) = 0;
		/// initialize render targets and framebuffer objects in current opengl context
		virtual bool init_fbos() = 0;
		/// check whether fbos have been initialized
		virtual bool fbos_initialized() const = 0;
		/// destruct render targets and framebuffer objects in current opengl context
		virtual void destruct_fbos() = 0;
		/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
		virtual void put_eye_to_head_matrix(int eye, float* pose_matrix) = 0;
		/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right)
		virtual void put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix) = 0;
		/// enable the framebuffer object of given eye (0..left, 1..right) 
		virtual void enable_fbo(int eye) = 0;
		/// disable the framebuffer object of given eye (0..left, 1..right)
		virtual void disable_fbo(int eye) = 0;
		/// submit the rendered stereo frame to the hmd
		virtual void submit_frame() = 0;
	};
}

#include <cgv/config/lib_end.h>
