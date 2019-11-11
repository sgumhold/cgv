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

protected:
  vr::IVRSystem *hmd;

private:
  vr::IVRTrackedCamera *tracked_camera;
  vr::TrackedCameraHandle_t tracked_camera_handle;
  uint32_t last_frame_sequence;
  vr::CameraVideoStreamFrameHeader_t current_frame_header;

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
