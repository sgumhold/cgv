#include "gamepad_driver.h"

namespace gamepad {
	/// register a new driver
	void register_driver(gamepad_driver* gpd)
	{
		gpd->driver_index = unsigned(ref_drivers().size());
		ref_drivers().push_back(gpd);
		ref_driver_infos().resize(ref_driver_infos().size() + 1);
		ref_driver_infos().back().name = gpd->get_name();
		ref_driver_infos().back().enabled = true;
	}
	/// return registered drivers
	std::vector<gamepad_driver*>& ref_drivers()
	{
		static std::vector<gamepad_driver*> drivers;
		return drivers;
	}
	/// return reference to device info structures
	std::vector<device_info>& ref_device_infos()
	{
		static std::vector<device_info> device_infos;
		return device_infos;
	}
	/// return reference to device info structures
	std::vector<void*>& ref_device_handles()
	{
		static std::vector<void*> device_handles;
		return device_handles;
	}
	/// return information on the registered drivers
	std::vector<driver_info>& ref_driver_infos()
	{
		static std::vector<driver_info> driver_infos;
		return driver_infos;
	}
}