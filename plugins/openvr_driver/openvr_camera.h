#pragma once

#include <memory>
#include <vector>
#include <chrono>

#include <vr/vr_camera.h>

#include "openvr.h"

#include "lib_begin.h"

///@ingroup VR
///@{

/**@file
  defines camera class for camera provided by openvr_kit
*/
///
namespace vr {
class CGV_API openvr_camera : public vr_camera {
public:
  openvr_camera(vr::IVRSystem *hmd);
  /// access to 3x4 matrix in column major format for transformation from camera (0 .. left, 1 .. right) to head coordinates
  bool put_camera_to_head_matrix(int camera_index, float* pose_matrix) const;
  /// access to 4x4 matrix in column major format for perspective transformation of camera (0..left, 1..right)
  bool put_projection_matrix(int camera_index, bool undistorted, float z_near, float z_far, float* projection_matrix) const;
  /// write the focal lengths in x- and y-direction to access to focal_length_2d_ptr[0|1] and the texture center to center_2d_ptr[0|1]
  bool put_camera_intrinsics(int camera_index, bool undistorted, float* focal_length_2d_ptr, float* center_2d_ptr) const;
protected:
  vr::IVRSystem *hmd;

private:
  vr::IVRTrackedCamera *tracked_camera;
  vr::TrackedCameraHandle_t tracked_camera_handle;
  uint32_t last_frame_sequence;
  vr::CameraVideoStreamFrameHeader_t current_frame_header;
  float left_camera_to_head[12];
  float right_camera_to_head[12];
  bool initialize_impl();
  bool start_impl();
  bool stop_impl();
  bool get_frame_impl(std::vector<uint8_t>& frame_data, uint32_t& width, uint32_t& height, bool undistorted, bool maximum_valid_rectangle);
  bool get_gl_texture_id_impl(uint32_t& tex_id, uint32_t& width, uint32_t& height, bool undistorted, float max_valid_texcoord_range[4]);
  bool query_intrinsics_impl(uint32_t camera_index, bool undistorted, float focal_lengths[2], float center[2]);
  bool query_projection_impl(uint32_t camera_index, bool undistorted, float z_near, float z_far, float projection_matrix[16]);
};

} // namespace vr
///@}

#include <cgv/config/lib_end.h>
