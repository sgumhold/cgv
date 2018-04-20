#pragma once

#include "capture_device_impl.h"

#include "../lib_begin.h"

namespace capture {

class capture_device;

/// interface for capture drivers (implement only as driver implementor)
class CGV_API capture_driver
{
protected:
	friend class capture_device;
	/**@name driver registration */
	//@{
	/// internal function to provide a list of drivers
	static std::vector<capture_driver*>& ref_driver_list();
	/// call to register a new driver
	static void register_driver(capture_driver* cd, const std::string& driver_name);
	/// call to unregister a driver - not implemented yet!
	static void unregister_driver(capture_driver* cd);
	//@}
	/// virtual destructor
	virtual ~capture_driver();
	/// return the driver name
	virtual const std::string& get_name() const = 0;
	/// scan for supported capture devices and return the number of devices
	virtual unsigned scan_devices(bool scan_capabilities = false) = 0;
	/// return the number of found capture devices, scan for devices if this has not been explicitly called before
	virtual unsigned get_nr_devices() = 0;
	/// return the serial of the i-th capture devices
	virtual std::string get_serial(int i) = 0;
	/// return the capabilities of the i-th capture device
	virtual const device_capabilities& get_capabilities(int i) const = 0;
	/// create a capture device implementation
	virtual capture_device_impl* create_device() = 0;
};

/// helper template to register a driver
template <class T>
struct driver_registration
{
	driver_registration(const std::string& driver_name)
	{
		capture_driver::register_driver(new T, driver_name);
	}
};

}

#include <cgv/config/lib_end.h>