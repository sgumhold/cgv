#include "system_devices.h"
#include <cgv/utils/convert.h>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#include <initguid.h>
#include <setupapi.h>
#pragma comment (lib, "Setupapi.lib")
#include <devpkey.h>
#endif

namespace cgv {
	namespace os {

bool enumerate_system_devices(std::vector<std::pair<std::string, cgv::utils::guid>>& devices, const std::string& selector, bool present)
{
#ifdef WIN32
	// prepare device enumaration
	const TCHAR* enumerator = 0;
#ifdef UNICODE
	std::wstring selector_w;
#endif
	if (!selector.empty()) {
#ifdef UNICODE
		selector_w = cgv::utils::str2wstr(selector);
		enumerator = selector_w.c_str();
#else
		enumerator = selector.c_str();
#endif
	}
	HDEVINFO deviceInfo = SetupDiGetClassDevs(NULL, enumerator, NULL, present ? (DIGCF_PRESENT | DIGCF_ALLCLASSES) : DIGCF_ALLCLASSES);
	if (deviceInfo == INVALID_HANDLE_VALUE) {
		std::cerr << "Error: " << GetLastError() << std::endl;
		return false;
	}
	// Enumerate through all devices in the system
	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	DWORD memberIndex = 0;
	while (SetupDiEnumDeviceInfo(deviceInfo, memberIndex, &deviceInfoData)) {
		memberIndex++;
		TCHAR devicePathBuffer[500];
		DWORD devicePathLength = 0;
		if (!SetupDiGetDeviceInstanceId(deviceInfo, &deviceInfoData, devicePathBuffer, sizeof(devicePathBuffer), &devicePathLength))
			continue;
#ifdef UNICODE
		std::string device_path = cgv::utils::wstr2str(std::wstring(devicePathBuffer, devicePathLength));
#else
		std::string device_path(devicePathBuffer, devicePathLength);
#endif
		cgv::utils::guid container_id;
		DEVPROPTYPE propType;
		if (!SetupDiGetDevicePropertyW(deviceInfo, &deviceInfoData, &DEVPKEY_Device_ContainerId, &propType, reinterpret_cast<BYTE*>(&container_id), sizeof(GUID), nullptr, 0))
			std::cerr << "Error: " << GetLastError() << std::endl;
		devices.push_back({ device_path,container_id });
	}
	SetupDiDestroyDeviceInfoList(deviceInfo);
	return true;
#else
	std::cerr << "cgv::os::enumerate_system_devices() only implemented on Windows" << std::endl;
    return false;
#endif
}

	}
}
