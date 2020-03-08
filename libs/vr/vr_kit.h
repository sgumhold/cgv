#pragma once

#include "vr_state.h"
#include "vr_camera.h"

#include <vector>
#include <string>

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace vr {
	/// forward declaration of vr driver class
	class CGV_API vr_driver;
	/**@name vr kit management */
	//@{
	//! information provided per vr device
	/*! each vr_kit is uniquely specified by a device handle that is represented as void* and
	can be access via get_device_handle(). Furthermore, each kit has a human readable
	name [get_name()].
	The corresponding driver is access through get_driver(). In case of an error, the error
	message can be accessed via get_last_error().

	query_state(vr_kit_state& state, int pose_query) gives access to the state of the kit and comes in 
	three variants distinguished by the pose_query parameter:
	- 0 ... query controller state only
	- 1 ... query most recent controller state and poses of hmd and controller
	- 2 ... wait for the optimal time to start a rendering process and query state and future poses

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
	class CGV_API vr_kit
	{
	protected:
		/// pointer to driver that created the vr kit
		vr_driver* driver;
		/// device handle for internal use
		void* device_handle;
		/// pointer to camera
		vr_camera* camera;
		/// name in case driver provides this information (not reliable)
		std::string name;
		/// whether force feedback is supported
		bool force_feedback_support;
		/// whether it is wireless
		bool wireless;
		/// store last error here
		std::string last_error;
		/// destruct camera
		void destruct_camera();
		/// 
		vr_trackable_state& ref_reference_state(const std::string& serial_nummer);
		/// remove all reference states
		void clear_reference_states();
		/// mark all reference states as untracked
		void mark_references_as_untracked();
		/// derived kits implement this without caring about calibration; vr_kit::query_state() will apply driver calibration
		virtual bool query_state_impl(vr_kit_state& state, int pose_query) = 0;
		/// construct
		vr_kit(vr_driver* _driver, void* _handle, const std::string& _name, bool _ffb_support, bool _wireless);
	public:
		/// return driver
		const vr_driver* get_driver() const;
		/// return device handle
		void* get_device_handle() const;
		/// return camera or nullptr if not available
		vr_camera* get_camera() const;
		/// return name of vr_kit
		const std::string& get_name() const;
		/// return last error of vr_kit
		const std::string& get_last_error() const;
		/// return whether vr_kit is wireless
		bool is_wireless() const;
		/// return whether controllers support force feedback
		bool has_force_feedback() const;
		/// return whether device has camera
		bool has_camera() const;
		/// declare virtual destructor
		virtual ~vr_kit();
		//! for each controller provide information on throttles and sticks and how they map to state axes
		/*! each entry of the returned vector is either
		    - a throttle if second pair entry is -1 mapped to axis with index in first pair entry
			- or a stick with the two pair entries being indices of x and y axes */
		virtual const std::vector<std::pair<int, int> >& get_controller_throttles_and_sticks(int controller_index) const = 0;
		//! for each controller provide information on throttles' and sticks' deadzone and precision values
		/*! The pairs in the returned vector correspond to the pairs in the vector returned by get_controller_throttles_and_sticks().
		    For each throttle or stick two float values are provided:
			- the first value is the deadzone of the throttle or stick, i.e. a deadzone of 0.1 means that all stick positions p
			  with |p| < 0.1 are treated as (0,0). For throttles all values below 0.1 are treated as 0. For deadzone values 
			  of 0, no deadzone is generated.
			- the second value is the precision as a floating point number. I.e. for a precision od 0.1 the coordinates
			  of a stick position and the throttle values can only assume multiples of 0.1 values. This is achieved by
			  rounding the axis values to the next multiple of the precision value. If the precision value 
			  is 0, no rounding is done and the raw axis values are passed on.
			The deadzone and precision information is used by the cgv::gui::vr_server for detection and creation of
			events. The values stored in the events are deadzone corrected and rounded to the precision of the throttles
			and sticks. The cgv::gui::vr_server allows to replace the deadzone and precision values of a device with 
			two user defined vectors of float pairs (which can be identical) through the function 
			cgv::gui::vr_server::provide_controller_throttles_and_sticks_deadzone_and_precision(). */
		virtual const std::vector<std::pair<float, float> >& get_controller_throttles_and_sticks_deadzone_and_precision(int controller_index) const = 0;
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
		//! NOT IMPLEMENTED retrieve next key event from given device, return false if device's event queue is empty
		/*!Typically you use this function in the following way:
			while (query_key_event(i,key,action)) { process(key,action); } */
		/// virtual bool query_key_event(VRKeys& key, KeyAction& action) = 0;
		/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
		virtual bool set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength) = 0;
		/// return width in pixel of view
		virtual int get_width() const = 0;
		/// return height in pixel of view
		virtual int get_height() const = 0;
		/// allow to set a different size; in case fbos have been initialized before, destruct_fbos() and init_fbos() has to be called with the gl context of this vr kit being current
		virtual void set_size(int new_width, int new_height) = 0;
		/// initialize render targets and framebuffer objects in current opengl context
		virtual bool init_fbos() = 0;
		/// initialize render targets and framebuffer objects in current opengl context
		virtual bool blit_fbo(int eye, int x, int y, int w, int h) = 0;
		/// check whether fbos have been initialized
		virtual bool fbos_initialized() const = 0;
		/// destruct render targets and framebuffer objects in current opengl context
		virtual void destruct_fbos() = 0;
		/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
		virtual void put_eye_to_head_matrix(int eye, float* pose_matrix) const = 0;
		//! access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right)
		/*! pose matrix is not needed for most vr kits and can be set to nullptr; only in case of wall based vr kits
		    the pose matrix needs to be specified*/
		virtual void put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix, const float* hmd_pose = 0) const = 0;
		/// access to 4x4 modelview transformation matrix of given eye in column major format, which is computed in default implementation from given 3x4 pose matrix and eye to head transformation
		virtual void put_world_to_eye_transform(int eye, const float* hmd_pose, float* modelview_matrix) const;
		/// enable the framebuffer object of given eye (0..left, 1..right) 
		virtual void enable_fbo(int eye) = 0;
		/// disable the framebuffer object of given eye (0..left, 1..right)
		virtual void disable_fbo(int eye) = 0;
		/// bind texture of given eye to current texture unit
		virtual void bind_texture(int eye) = 0;
		/// submit the rendered stereo frame to the hmd
		virtual void submit_frame() = 0;
	};
}

///@}

#include <cgv/config/lib_end.h>
