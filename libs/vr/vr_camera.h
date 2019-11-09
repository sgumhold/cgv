#pragma once

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

/// different error codes
enum CameraError
{
	CE_NO_ERROR,
	CE_NO_CAMERA_INTERFACE,
	CE_NO_CAMERA_AVAILABLE,
	CE_NO_CAMERA_FIRMWARE,
	CE_ALREADY_INITIALIZED,
	CE_FAILED_TO_QUERY_FRAME_SIZE,
	CE_UNKNOWN_FRAME_FORMAT,
	CE_ATTEMPT_TO_START_UNINITIALIZED,
	CE_START_FAILED,
	CE_GET_FRAME_FAILED,
	CE_ATTEMPT_TO_STOP_NOT_STARTED_CAMERA,
	CE_ATTEMPT_TO_GET_FRAME_BUT_NOT_STARTED_CAMERA
};

/// convert a camera error into an error string 
extern CGV_API std::string get_camera_error_string(CameraError ce);

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
	/// store last error
	CameraError last_error;
public:
	/**@name camera inspection and control*/
	//@{
	/// return number of cameras in the headset (1 for mono and 2 for stereo)
	uint8_t get_nr_cameras() const;
	/// query the camera intrinsics 
	bool query_intrinsics(float focal_lengths[2], float center[2], uint32_t camera_index = 0);
	/// query the camera projection matrix for given z_near and z_far values; matrix is encoded in column major order
	bool query_projection(float z_near, float z_far, float projection_matrix[16], uint32_t camera_index = 0);
	/// start streaming of frames
	bool start();
	/// stop streaming of frames
	bool stop();
	/// return the camera state
	CameraState get_state() const;
	/// return last error
	CameraError get_last_error() const;
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
private:
  CameraState state;
  virtual bool initialize_impl() = 0;
  virtual bool start_impl() = 0;
  virtual bool stop_impl() = 0;
  virtual bool get_frame_impl(std::vector<uint8_t>& frame_data, uint32_t& width, uint32_t& height, bool undistorted, bool maximum_valid_rectangle) = 0;
  virtual bool get_gl_texture_id_impl(uint32_t& tex_id, uint32_t& width, uint32_t& height, bool undistorted, float max_valid_texcoord_range[4]) = 0;
  virtual bool query_intrinsics_impl(float focal_lengths[2], float center[2], uint32_t camera_index) = 0;
  virtual bool query_projection_impl(float z_near, float z_far, float projection_matrix[16], uint32_t camera_index) = 0;
};

} // namespace vr
///@}

#include <cgv/config/lib_end.h>
