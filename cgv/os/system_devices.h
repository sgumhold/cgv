#pragma once

#include <vector>
#include <string>
#include <cgv/utils/guid.h>

#include "lib_begin.h"

namespace cgv {
	namespace os {

/// <summary>
/// enumerate the system devices
/// </summary>
/// <param name="devices">vector of resulting device paths and container ids</param>
/// <param name="selector">device path prefix to restrict enumeration, i.e. USB, SWD, ...</param>
/// <param name="present">whether enumeration is restricted to devices currently present in system</param>
/// <returns>whether enumeration was possible, typically falls if \c selector parameter has invalid value</returns>
extern CGV_API bool enumerate_system_devices(std::vector<std::pair<std::string, cgv::utils::guid>>& devices, const std::string& selector = "", bool present = true);

	}
}
#include <cgv/config/lib_end.h>