#pragma once

#include <vr/gl_vr_display.h>
#include "openvr.h"

#include <vector>

#include "lib_begin.h"

namespace vr {
	/**@name vr device management */
	//@{
	/// information provided per vr device
	class CGV_API openvr_kit : public gl_vr_display
	{
	protected:
		vr::IVRSystem* get_hmd();
	public:
		/// construct
		openvr_kit(unsigned _width, unsigned _height, vr_driver* _driver, vr::IVRSystem* _hmd, const std::string& _name, bool _ffb_support, bool _wireless);
		/// declare virtual destructor
		~openvr_kit();
		/// for each controller provide information on throttles and sticks and how they map to state axes
		const std::vector<std::pair<int, int> >& get_controller_throttles_and_sticks(int controller_index) const;
		/// for each controller provide information on throttles' and sticks' deadzone and precision values
		const std::vector<std::pair<float, float> >& get_controller_throttles_and_sticks_deadzone_and_precision(int controller_index) const;
		/// query current state of vr kit and return whether this was successful
		bool query_state(vr_kit_state& state, int pose_query);
		/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
		bool set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength);
		/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
		void put_eye_to_head_matrix(int eye, float* pose_matrix);
		/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right) 
		void put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix);
		/// initialize render targets and framebuffer objects in current opengl context
		bool init_fbos();
		/// submit the rendered stereo frame to the hmd
		void submit_frame();
	};
}

#include <cgv/config/lib_end.h>
