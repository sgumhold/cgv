#include "vr_state.h"

namespace vr {
	/// standard constructor for initialization of members
	vr_trackable_state::vr_trackable_state()
	{
		status = VRS_DETACHED;
		pose[0] = pose[4] = pose[8] = 1;
		pose[1] = pose[2] = pose[3] = pose[5] = pose[6] = pose[7] = 0;
		pose[9] = pose[10] = pose[11] = 0;
	}
	/// standard constructor for initialization of members
	vr_controller_state::vr_controller_state()
	{
		time_stamp = 0;
		button_flags = 0;
		for (unsigned i = 0; i < max_nr_controller_axes; ++i)
			axes[i] = 0;
		vibration[0] = vibration[1] = 0;
	}		/// place the 3d ray origin and the 3d ray direction into the given arrays
	void vr_controller_state::put_ray(float* ray_origin, float* ray_direction) const
	{
		ray_origin[0] = pose[9];
		ray_origin[1] = pose[10];
		ray_origin[2] = pose[11];
		ray_direction[0] = -pose[6];
		ray_direction[1] = -pose[7];
		ray_direction[2] = -pose[8];
	}


	vr_kit_state::vr_kit_state()
	{

	}
	/// check for equality
	bool vr_kit_state::operator == (const vr_kit_state& state) const
	{
		return hmd == state.hmd &&
			controller[0] == state.controller[0] &&
			controller[1] == state.controller[1];
	}
	///
	bool vr_trackable_state::operator == (const vr_trackable_state& state) const
	{
		return status == state.status &&
			pose[0] == state.pose[0] &&
			pose[1] == state.pose[1] &&
			pose[2] == state.pose[2] &&
			pose[3] == state.pose[3] &&
			pose[4] == state.pose[4] &&
			pose[5] == state.pose[5] &&
			pose[6] == state.pose[6] &&
			pose[7] == state.pose[7] &&
			pose[8] == state.pose[8] &&
			pose[9] == state.pose[9] &&
			pose[10] == state.pose[10] &&
			pose[11] == state.pose[11];
	}
	///
	bool vr_controller_state::operator == (const vr_controller_state& state) const
	{
		return
			vr_trackable_state::operator==(state) &&
			button_flags == state.button_flags &&
			axes[0] == state.axes[0] &&
			axes[1] == state.axes[1] &&
			axes[2] == state.axes[2] &&
			axes[3] == state.axes[3] &&
			axes[4] == state.axes[4] &&
			axes[5] == state.axes[5] &&
			axes[6] == state.axes[6] &&
			axes[7] == state.axes[7] &&
			vibration[0] == state.vibration[0] &&
			vibration[1] == state.vibration[1];
	}
	/// convert key to string
	std::string get_key_string(unsigned short key)
	{
		static const char* vr_key_names[] = {
		"VR_UNKNOWN",
		"VR_SYSTEM",          
		"VR_MENU",            
		"VR_GRIP",            
		"VR_DPAD_DOWN_LEFT",  
		"VR_DPAD_DOWN",       
		"VR_DPAD_DOWN_RIGHT", 
		"VR_DPAD_LEFT",       
		"VR_DPAD_RIGHT",      
		"VR_DPAD_UP_LEFT",    
		"VR_DPAD_UP",         
		"VR_DPAD_UP_RIGHT",   
		"VR_A",               
		"VR_INPUT0_TOUCH",    
		"VR_INPUT0",          
		"VR_INPUT1_TOUCH",    
		"VR_INPUT1",          
		"VR_INPUT2_TOUCH",    
		"VR_INPUT2",          
		"VR_INPUT3_TOUCH",    
		"VR_INPUT3",          
		"VR_INPUT4_TOUCH",    
		"VR_INPUT4",
		"VR_PROXIMITY"
		};
		int index = key - (int)VR_UNKNOWN;
		return index < 24 ? vr_key_names[index] : "VR_UNKNOWN";
	}
	/// convert flags to string
	std::string get_state_flag_string(VRButtonStateFlags flags)
	{
		static const char* flag_names[] = {
		"SYSTEM",
		"MENU",
		"GRIP",
		"DPAD_LEFT",
		"DPAD_RIGHT",
		"DPAD_DOWN",
		"DPAD_UP",
		"A",
		"INPUT0_TOUCH",
		"INPUT0",
		"INPUT1_TOUCH",
		"INPUT1",
		"INPUT2_TOUCH",
		"INPUT2",
		"INPUT3_TOUCH",
		"INPUT3",
		"INPUT4_TOUCH",
		"INPUT4",
		"PROXIMITY"
		};
		static const VRButtonStateFlags flag_values[] = { 
			VRF_SYSTEM       ,
			VRF_MENU         ,
			VRF_GRIP         ,
			VRF_DPAD_LEFT    ,
			VRF_DPAD_RIGHT   ,
			VRF_DPAD_DOWN    ,
			VRF_DPAD_UP      ,
			VRF_A            ,
			VRF_INPUT0_TOUCH ,
			VRF_INPUT0       ,
			VRF_INPUT1_TOUCH ,
			VRF_INPUT1       ,
			VRF_INPUT2_TOUCH ,
			VRF_INPUT2       ,
			VRF_INPUT3_TOUCH ,
			VRF_INPUT3       ,
			VRF_INPUT4_TOUCH ,
			VRF_INPUT4       ,
			VRF_PROXIMITY
		};
		std::string result;
		for (unsigned i = 0; i < 19; ++i)
			if ((flags & flag_values[i]) != 0) {
				if (result.empty())
					result = flag_names[i];
				else
					result += std::string("+") + flag_names[i];
			}
		return result;

	}
	/// convert flags to string
	std::string get_status_string(VRStatus status)
	{
		switch (status) {
		case VRS_DETACHED: return "detached";
		case VRS_ATTACHED: return "attached";
		case VRS_TRACKED: return "tracked";
		default: return "unknown status";
		}
	}
}