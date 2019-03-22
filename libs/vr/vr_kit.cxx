#include "vr_kit.h"

namespace vr {

	/// construct
	vr_kit::vr_kit(vr_driver* _driver, void* _handle, const std::string& _name, bool _ffb_support, bool _wireless) :
		driver(_driver), device_handle(_handle), name(_name), force_feedback_support(_ffb_support), wireless(_wireless) {}
	/// declare virtual destructor
	vr_kit::~vr_kit()
	{
	}
	/// return driver
	const vr_driver* vr_kit::get_driver() const { return driver; }
	/// return device handle
	void* vr_kit::get_device_handle() const { return device_handle; }
	/// return name of vr_kit
	const std::string& vr_kit::get_name() const { return name; }
	/// return last error of vr_kit
	const std::string& vr_kit::get_last_error() const { return last_error; }
	/// return whether vr_kit is wireless
	bool vr_kit::is_wireless() const { return wireless; }
	/// return whether controllers support force feedback
	bool vr_kit::has_force_feedback() const { return force_feedback_support; }
}


