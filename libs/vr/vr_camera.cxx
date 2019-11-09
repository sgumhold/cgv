#include "vr_camera.h"

#include <iostream>
#include <string>

namespace vr {

	/// convert a camera error into an error string 
	std::string get_camera_error_string(CameraError ce)
	{
		switch (ce) {
		case CE_NO_ERROR: return "no error";
		case CE_NO_CAMERA_INTERFACE: return "no camera interface";
		case CE_NO_CAMERA_AVAILABLE: return "no camera available";
		case CE_NO_CAMERA_FIRMWARE: return "no camera firmware";
		case CE_ALREADY_INITIALIZED: return "already initialized";
		case CE_GET_FRAME_FAILED: return "failed to query frame";
		case CE_FAILED_TO_QUERY_FRAME_SIZE: return "failed to query frame size";
		case CE_UNKNOWN_FRAME_FORMAT: return "unknown frame format";
		case CE_ATTEMPT_TO_START_UNINITIALIZED: return "attempt to start uninitialized camera";
		case CE_START_FAILED: return "starting of camera failed";
		case CE_ATTEMPT_TO_STOP_NOT_STARTED_CAMERA: return "attempt to stop not started camera";
		case CE_ATTEMPT_TO_GET_FRAME_BUT_NOT_STARTED_CAMERA: return "attempt to get frame but camera not started";
		default: return "unknown error";
		}
	}

	vr_camera::vr_camera() : num_cameras(0u), state(CS_UNINITIALIZED), frame_format(CFF_RGBA), frame_split(CFS_NONE), frame_flipped(false)
	{
		last_error = CE_NO_ERROR;
	}

	/// return last error
	CameraError vr_camera::get_last_error() const
	{
		return last_error;
	}

	bool vr_camera::initialize()
	{
		if (state != CS_UNINITIALIZED) {
			last_error = CE_ALREADY_INITIALIZED;
			return false;
		}
		if (!initialize_impl())
			return false;
		state = CS_INITIALIZED;
		return true;
	}

	bool vr_camera::start() {
		if (state != CS_INITIALIZED) {
			last_error = CE_ATTEMPT_TO_START_UNINITIALIZED;
			return false;
		}
		if (!start_impl())
			return false;
		state = CS_STARTED;
		last_error = CE_NO_ERROR;
		return true;
	}

	bool vr_camera::stop() {
		if (state != CS_STARTED) {
			last_error = CE_ATTEMPT_TO_STOP_NOT_STARTED_CAMERA;
			return false;
		}
		if (!stop_impl())
			return false;
		state = CS_INITIALIZED;
		last_error = CE_NO_ERROR;
		return true;
	}

	/// check for a new frame, return false if not available. Otherwise copy frame to provided buffer that is resized if necessary
	bool vr_camera::get_frame(std::vector<uint8_t>& frame_data, uint32_t& width, uint32_t& height, bool undistorted, bool maximum_valid_rectangle)
	{
		if (state != CS_STARTED) {
			last_error = CE_ATTEMPT_TO_GET_FRAME_BUT_NOT_STARTED_CAMERA;
			return false;
		}
		if (!get_frame_impl(frame_data, width, height, undistorted, maximum_valid_rectangle))
			return false;
		last_error = CE_NO_ERROR;
		return true;
	}
	/// query id of shared opengl texture id
	bool vr_camera::get_gl_texture_id(uint32_t& tex_id, uint32_t& width, uint32_t& height, bool undistorted, float max_valid_texcoord_range[4])
	{
		if (state != CS_STARTED) {
			last_error = CE_ATTEMPT_TO_GET_FRAME_BUT_NOT_STARTED_CAMERA;
			return false;
		}
		if (!get_gl_texture_id_impl(tex_id, width, height, undistorted, max_valid_texcoord_range))
			return false;
		last_error = CE_NO_ERROR;
		return true;
	}

	CameraFrameFormat vr_camera::get_frame_format() const 
	{
		return frame_format; 
	}

	CameraFrameSplit vr_camera::get_frame_split() const
	{
		return frame_split;
	}

	CameraState vr_camera::get_state() const { return state; }

	uint8_t vr_camera::get_nr_cameras() const { return num_cameras; }

	bool vr_camera::is_frame_flipped() const
	{
		return frame_flipped;
	}

} // namespace vr