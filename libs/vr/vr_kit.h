#pragma once

#include "vr_state.h"
#include "vr_camera.h"
#include "vr_info.h"
#include "gl_vr_display.h"

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace vr {
	/// forward declaration of vr driver class
	class CGV_API vr_driver;

	/// configuration of a controller input axis
	struct CGV_API controller_input_config
	{
		//! all input values below dead_zone are clamped to 0.
		/*! For pad and stick inputs the length of the vector valued position is compared to dead_zone 
		    and the zero vector is compared inside of dead zone.*/
		float dead_zone;
		//! if precision is larger than zero, values are rounded to multiples of precision. 
		/*! If precision is 0.02 then the value 0.031 would be rounded to 0.04.*/
		float precision;
		//! if value gets larger than threshold a key press event is triggered if this is enabled.
		/*! In case input is pad or stick, a direction pad is emulated and the two thresholds are used to determine direction. */
		float threshold;
		/// initialize to defaults (dead_zone=precision=0;threshold=0.5)
		controller_input_config();
	};

	/**@name vr kit management */
	//@{
	//! a vr kit is composed of headset, two controllers, and two trackers, where all devices can be attached or detached
	/*! each vr_kit is uniquely specified by a handle that is represented as void* and
	can be access via get_handle(). Furthermore, each kit has a human readable
	name [get_name()].
	The corresponding driver is accessed through get_driver(). In case of an error, the error
	message can be accessed via get_last_error() of the base class gl_vr_display.

	query_state(vr_kit_state& state, int pose_query) gives access to the state of the kit and comes in 
	three variants distinguished by the pose_query parameter:
	- 0 ... query controller state only
	- 1 ... query most recent controller state and poses of hmd and controller
	- 2 ... wait for the optimal time to start a rendering process and query state and future poses

	Information on the currently attached devices can be queried with get_device_info().

	TODO: support for vibration based force feedback

	rendering to the hmd is done in current opengl context via init_fbos(), enable_fbo(eye),
	disable_fbo(eye) and submit_frame() (after rendering both eyes). The crg_vr_view plugin
	uses these functions and completely handles the rendering process. If you want to use
	the functions yourself, you need to ensure to make the opengl context current before calling
	them.
	In order to debug the vr views, the function blit_fbo(eye,x,y,w,h) allows to blit a vr view
	into the framebuffer currently bound to GL_DRAW_FRAMEBUFFER.

	In order to build the modelview and projection matrices for rendering, the functions
	put_eye_to_head_matrix(eye,float[12]) provides a 3x4 pose matrix for each eye.
	Multiplying to the hmd pose matrix and inverting the matrix product yields the modelview
	matrix. The projection matrix can be accessed per eye via
	put_projection_matrix(eye,z_near,z_far,float[16]).
	*/
	class CGV_API vr_kit : public gl_vr_display
	{
	protected:
		/// pointer to driver that created the vr kit
		vr_driver* driver;
		/// handle for internal use
		void* handle;
		/// pointer to camera
		vr_camera* camera;
		/// whether to skip driver calibration - defaults to false
		bool skip_calibration;
		/// name in case driver provides this information (not reliable)
		std::string name;
		/// store vr kit info to be filled and updated by driver implementations
		vr_kit_info info;
		/// store controller input configs per controller and input
		controller_input_config input_configs[4][5];
		/// destruct camera
		void destruct_camera();
		/// write access to the state of the tracking reference with given serial number
		vr_trackable_state& ref_tracking_reference_state(const std::string& serial_nummer);
		/// write access to tracking system info
		vr_tracking_system_info& ref_tracking_system_info();
		/// remove all reference states
		void clear_tracking_reference_states();
		/// mark all reference states as untracked
		void mark_tracking_references_as_untracked();
		/// derived kits implement this without caring about calibration; vr_kit::query_state() will apply driver calibration
		virtual bool query_state_impl(vr_kit_state& state, int pose_query) = 0;
		/// construct
		vr_kit(vr_driver* _driver, void* _handle, const std::string& _name, unsigned _width, unsigned _height, unsigned _nr_multi_samples = 4);
	public:
		/// return driver
		const vr_driver* get_driver() const;
		/// return handle of vr kit
		void* get_handle() const;
		/// return camera or nullptr if not available
		vr_camera* get_camera() const;
		/// return name of vr_kit
		const std::string& get_name() const;
		/// declare virtual destructor
		virtual ~vr_kit();
		/// return information on the currently attached devices
		const vr_kit_info& get_device_info() const;
		//! set the configuration of a controller input
		/*! A controller can have up to 5 inputs, which can be 1-axis throttels and triggers or
			2 axis touchpad (abbreviated as pad) and joysticks (abbreviated as stick).
			The input type can be checked in vr_kit_info queried with get_device_info().
			The provided deadzone, precision and threshold information is used by
			cgv::gui::vr_server for detection and creation of events. */
		virtual void set_controller_input_config(int controller_index, int input_index, const controller_input_config& cic);
		/// query the configuration of a controller input 
		const controller_input_config&  get_controller_input_config(int controller_index, int input_index) const;
		//! query current state of vr kit and return whether this was successful
		/*! \param state state is returned by writing it into passed reference
		    \param pose_query is 
				0 ... no poses are queried
				1 ... most current pose for controller is queried for example to get pose at button press in highest precision
				2 ... future pose for rendering next frame is queried for controllers and hmd
				add 4 to restrict query to left controller
				add 8 to restrict query to right controller
				add 12 to restrict query to both controllers 
		*/
		bool query_state(vr_kit_state& state, int pose_query = 2);
		/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
		virtual bool set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength) = 0;
		/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
		virtual void put_eye_to_head_matrix(int eye, float* pose_matrix) const = 0;
		//! access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right)
		/*! pose matrix is not needed for most vr kits and can be set to nullptr; only in case of wall based vr kits
		    the pose matrix needs to be specified*/
		virtual void put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix, const float* hmd_pose = 0) const = 0;
		/// access to 4x4 modelview transformation matrix of given eye in column major format, which is computed in default implementation from given 3x4 pose matrix and eye to head transformation
		virtual void put_world_to_eye_transform(int eye, const float* hmd_pose, float* modelview_matrix) const;
		/// submit the rendered stereo frame to the hmd
		virtual void submit_frame() = 0;
	};
}

///@}

#include <cgv/config/lib_end.h>
