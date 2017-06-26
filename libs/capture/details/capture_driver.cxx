#include "capture_driver.h"

namespace capture {

std::vector<capture_driver*>& capture_driver::ref_driver_list()
{
	static std::vector<capture_driver*> driver_list;
	return driver_list;
}
/// call to register a new driver
void capture_driver::register_driver(capture_driver* cd, const std::string& driver_name)
{
	ref_driver_list().push_back(cd);
}
/// call to unregister a driver - not implemented yet!
void capture_driver::unregister_driver(capture_driver* cd)
{
	auto i = std::find(ref_driver_list().begin(), ref_driver_list().end(), cd);
	if (i != ref_driver_list().end())
		ref_driver_list().erase(i);
}

/// virtual destructor
capture_driver::~capture_driver()
{

}

}
