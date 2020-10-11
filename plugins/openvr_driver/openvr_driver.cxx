#include <cassert>
#include <vr/vr_driver.h>
#include "openvr_kit.h"
#include <cgv/type/standard_types.h>
#include <cgv/utils/options.h>
#include "openvr.h"
#include <iostream>
using namespace vr;

namespace vr {
	// encode user index in handle
	void* get_handle(int user_index)
	{
		void* handle = 0;
		(int&)handle = user_index;
		return handle;
	}

	// decode user index from handle
	int get_user_index(void* handle)
	{
		int user_index = (int&)handle;
		assert(user_index >= 0 && user_index < 4);
		return user_index;
	}
}

std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}


struct openvr_driver : public vr_driver
{
	vr::IVRSystem *hmd_ptr;
	std::string driver_name;
	std::string kit_name;
	std::string last_error;
	bool installed;
	/// construct driver
	openvr_driver(const std::string& options)
	{
		installed = false;
		if (cgv::utils::has_option("NO_OPENVR"))
			return;
			
		// init SteamVR Runtime
		vr::EVRInitError error = vr::VRInitError_None;
		hmd_ptr = vr::VR_Init(&error, vr::VRApplication_Scene);
		if (error != vr::VRInitError_None)
			hmd_ptr = vr::VR_Init(&error, vr::VRApplication_Other);

		if (error != vr::VRInitError_None) {
			hmd_ptr = NULL;
			last_error  = "Unable to init VR runtime: ";
			last_error += vr::VR_GetVRInitErrorAsEnglishDescription(error);
			return;
		}

		installed = true;
		// query name of driver and device
		driver_name = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
		kit_name = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
	}
	~openvr_driver()
	{
		if (hmd_ptr) {
			vr::VR_Shutdown();
			hmd_ptr = NULL;
		}

	}
	/// return whether driver is installed
	bool is_installed() const
	{
		return installed;
	}
	/// return name of driver
	std::string get_driver_name() const
	{
		return driver_name.empty() ? "OpenVR Driver" : driver_name;
	}
	/// scan all connected vr kits and return a vector with their ids
	std::vector<void*> scan_vr_kits()
	{
		std::vector<void*> handles;
		if (!is_installed())
			return handles;
		if (VR_IsHmdPresent() && hmd_ptr) {
			bool do_register = true;
			vr_kit* kit = get_vr_kit(hmd_ptr);
			if (!kit) {
				uint32_t width, height;
				hmd_ptr->GetRecommendedRenderTargetSize(&width, &height);
				kit = new openvr_kit(width, height, this, hmd_ptr, kit_name);
				vr_kit_state state;
				kit->query_state(state, 2);
				register_vr_kit(hmd_ptr, kit);
			}
			handles.push_back(hmd_ptr);
		}
		return handles;
	}
	/// scan all connected vr kits and return a vector with their ids
	vr_kit* replace_by_index(int& index, vr_kit* new_kit_ptr)
	{
		std::vector<void*> handles;
		if (!is_installed())
			return 0;
		if (VR_IsHmdPresent() && hmd_ptr) {
			bool do_register = true;
			vr_kit* kit = get_vr_kit(hmd_ptr);
			if (!kit) {
				uint32_t width, height;
				hmd_ptr->GetRecommendedRenderTargetSize(&width, &height);
				kit = new openvr_kit(width, height, this, hmd_ptr, kit_name);
				register_vr_kit(hmd_ptr, kit);
			}
			if (index == 0) {
				replace_vr_kit(hmd_ptr, new_kit_ptr);
				return kit;
			}
			else
				--index;
		}
		return 0;
	}
	/// scan all connected vr kits and return a vector with their ids
	bool replace_by_pointer(vr_kit* old_kit_ptr, vr_kit* new_kit_ptr)
	{
		std::vector<void*> handles;
		if (!is_installed())
			return false;
		if (VR_IsHmdPresent() && hmd_ptr) {
			bool do_register = true;
			vr_kit* kit = get_vr_kit(hmd_ptr);
			if (!kit) {
				uint32_t width, height;
				hmd_ptr->GetRecommendedRenderTargetSize(&width, &height);
				kit = new openvr_kit(width, height, this, hmd_ptr, kit_name);
				register_vr_kit(hmd_ptr, kit);
			}
			if (kit == old_kit_ptr) {
				replace_vr_kit(hmd_ptr, new_kit_ptr);
				return true;
			}
		}
		return false;
	}
	/// put a 3d up direction into passed array
	void put_up_direction(float* up_dir) const
	{
		up_dir[0] = 0;
		up_dir[1] = 1.0f;
		up_dir[2] = 0;
	}
	/// return the floor level relativ to the world origin
	float get_floor_level() const
	{
		std::vector<float> boundary;
		put_action_zone_bounary(boundary);
		size_t n = boundary.size() / 3;
		float y = 0.0f;
		for (size_t i = 0; i < n; ++i)
			y += boundary[3 * i + 1];
		y /= (float)n;
		return y;
	}
	/// return height of action zone
	float get_action_zone_height() const
	{
		return 2.5f;
	}
	/// return a vector of floor points defining the action zone boundary as a closed polygon
	void put_action_zone_bounary(std::vector<float>& boundary) const
	{
		if (!installed)
			return;

		auto* chap = vr::VRChaperone();
		if (!chap)
			return;

		vr::HmdQuad_t rect;
		chap->GetPlayAreaRect(&rect);

		boundary.resize(12);
		for (unsigned i = 0; i < 4; ++i) {
			boundary[3 * i] = rect.vCorners[i].v[0];
			boundary[3 * i + 1] = rect.vCorners[i].v[1];
			boundary[3 * i + 2] = rect.vCorners[i].v[2];
		}
	}

};

driver_registry<openvr_driver> openvr_driver_registry("openvr_driver");
