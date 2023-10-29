#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "lib_begin.h"

///@ingroup VR
///@{

/**@file
  defines camera class for camera provided by vr_kit
*/
///
namespace vr {

/// different status values for a vr camera
enum CameraState {
  CS_UNINITIALIZED,
  CS_INITIALIZED,
  CS_STARTED
};

/// currently only a single frame format supported
enum CameraFrameFormat { 
	CFF_RGBA = 0 
};

/// in case of stereo cameras a frame contains images of both eyes either split vertically (left right) or horizontally (top bottom)
enum CameraFrameSplit { 
	CFS_NONE = 0,  // no split indicates mono camera
	CFS_VERTICAL,  // left right split
	CFS_HORIZONTAL // top bottom split
};

/// interface for mono or stereo cameras in VR headsets
class CGV_API vr_camera 
{
protected:
	/// give vr kit access to camera constructor
	friend class vr_kit;
	/// construct camera object
	vr_camera();
	/// give vr driver access to camera initialization and destruction
	friend class vr_driver;
	/// initialization is typically called  the c
	bool initialize();
	/// destruct camera
	virtual ~vr_camera() = default;
	/// store last error as a string to be as flexible as possible
	mutable std::string last_error;
public:
	/**@name camera inspection and control*/
	//@{
	/// return number of cameras in the headset (1 for mono and 2 for stereo)
	uint8_t get_nr_cameras() const;
	/// access to 3x4 matrix in column major format for transformation from camera (0 .. left, 1 .. right) to head coordinates
	virtual bool put_camera_to_head_matrix(int camera_index, float* pose_matrix) const = 0;
	/// access to 4x4 matrix in column major format for perspective transformation of camera (0..left, 1..right)
	virtual bool put_projection_matrix(int camera_index, bool undistorted, float z_near, float z_far, float* projection_matrix) const = 0;
	/// write the focal lengths in x- and y-direction to access to focal_length_2d_ptr[0|1] and the texture center to center_2d_ptr[0|1]
	virtual bool put_camera_intrinsics(int camera_index, bool undistorted, float* focal_length_2d_ptr, float* center_2d_ptr) const = 0;
	/// start streaming of frames
	bool start();
	/// stop streaming of frames
	bool stop();
	/// return the camera state
	CameraState get_state() const;
	/// check for error
	bool has_error() const { return !last_error.empty(); }
	/// return last error, if no error has occured, the function returns an empty string
	const std::string& get_last_error() const;
	//@}

	/**@name access to frames*/
	//@{
	/// query pixel format
	CameraFrameFormat get_frame_format() const;
	/// query stereo frame layout
	CameraFrameSplit get_frame_split() const;
	/// query whether frame row order is from top to bottom
	bool is_frame_flipped() const;
	/// check for a new frame, return false if not available. Otherwise copy frame to provided buffer that is resized if necessary
	bool get_frame(std::vector<uint8_t>& frame_data, uint32_t& width, uint32_t& height, bool undistorted, bool maximum_valid_rectangle);
	/// query id of shared opengl texture id
	bool get_gl_texture_id(uint32_t& tex_id, uint32_t& width, uint32_t& height, bool undistorted, float max_valid_texcoord_range[4]);
	//@}
protected:
  uint8_t num_cameras;
  CameraFrameFormat frame_format;
  CameraFrameSplit frame_split;
  bool frame_flipped;
  CameraState state;
private:
  virtual bool initialize_impl() = 0;
  virtual bool start_impl() = 0;
  virtual bool stop_impl() = 0;
  virtual bool get_frame_impl(std::vector<uint8_t>& frame_data, uint32_t& width, uint32_t& height, bool undistorted, bool maximum_valid_rectangle) = 0;
  virtual bool get_gl_texture_id_impl(uint32_t& tex_id, uint32_t& width, uint32_t& height, bool undistorted, float max_valid_texcoord_range[4]) = 0;
};

} // namespace vr
///@}

#include <cgv/config/lib_end.h>
