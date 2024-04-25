#include "rgbd_driver.h"

namespace rgbd {

/// virtual destructor
rgbd_driver::~rgbd_driver()
{
}
std::string rgbd_driver::get_audio_device_id(int)
{
	return "";
}

}
