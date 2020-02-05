#include "vr_camera.h"

#include <iostream>
#include <string>

namespace vr {

	vr_camera::vr_camera() : num_cameras(0u), state(CS_UNINITIALIZED), frame_format(CFF_RGBA), frame_split(CFS_NONE), frame_flipped(false)
	{
	}

	/// return last error
	const std::string& vr_camera::get_last_error() const
	{
		return last_error;
	}

	bool vr_camera::initialize()
	{
		if (state != CS_UNINITIALIZED) {
			last_error = "attempt to initialize already initialized vr camera";
			return false;
		}
		if (!initialize_impl())
			return false;
		state = CS_INITIALIZED;
		last_error.clear();
		return true;
	}

	bool vr_camera::start() {
		if (state == CS_UNINITIALIZED) {
			last_error = "attempt to start not initialized vr camera, please initialize first";
			return false;
		}
		if (state == CS_STARTED) {
			last_error = "attempt to start already started vr camera";
			return false;
		}
		if (!start_impl())
			return false;
		state = CS_STARTED;
		last_error.clear();
		return true;
	}

	bool vr_camera::stop() {
		if (state != CS_STARTED) {
			last_error = "attempt to stop camera that was not started, please start camera first";
			return false;
		}
		if (!stop_impl())
			return false;
		state = CS_INITIALIZED;
		last_error.clear();
		return true;
	}

	/// check for a new frame, return false if not available. Otherwise copy frame to provided buffer that is resized if necessary
	bool vr_camera::get_frame(std::vector<uint8_t>& frame_data, uint32_t& width, uint32_t& height, bool undistorted, bool maximum_valid_rectangle)
	{
		if (state != CS_STARTED) {
			last_error = "attempt to get frame from vr camera that was not started, please start camera first";
			return false;
		}
		if (!get_frame_impl(frame_data, width, height, undistorted, maximum_valid_rectangle))
			return false;
		last_error.clear();
		return true;
	}
	/// query id of shared opengl texture id
	bool vr_camera::get_gl_texture_id(uint32_t& tex_id, uint32_t& width, uint32_t& height, bool undistorted, float max_valid_texcoord_range[4])
	{
		if (state != CS_STARTED) {
			last_error = "attempt to get gl texture id from vr camera that was not started, please start camera first";
			return false;
		}
		if (!get_gl_texture_id_impl(tex_id, width, height, undistorted, max_valid_texcoord_range))
			return false;
		last_error.clear();
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

	CameraState vr_camera::get_state() const 
	{
		return state; 
	}

	uint8_t vr_camera::get_nr_cameras() const 
	{
		return num_cameras; 
	}

	bool vr_camera::is_frame_flipped() const
	{
		return frame_flipped;
	}

} // namespace vr