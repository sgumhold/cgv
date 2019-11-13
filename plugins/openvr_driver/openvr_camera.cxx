#include "openvr_camera.h"
#include "openvr_kit.h"
#include <iostream>
#include <string>

namespace vr {

	openvr_camera::openvr_camera(vr::IVRSystem *hmd) : hmd(hmd), tracked_camera(nullptr), tracked_camera_handle(INVALID_TRACKED_CAMERA_HANDLE)
	{
		frame_format = CameraFrameFormat::CFF_RGBA;
		frame_split = CameraFrameSplit::CFS_HORIZONTAL;
		frame_flipped = true;
	}
	bool openvr_camera::query_intrinsics_impl(uint32_t camera_index, bool undistorted, float focal_lengths[2], float center[2])
	{
		vr::EVRTrackedCameraFrameType distortion_type = undistorted ? vr::VRTrackedCameraFrameType_Undistorted : vr::VRTrackedCameraFrameType_Distorted;
		vr::HmdVector2_t foc, ctr;
		vr::EVRTrackedCameraError camera_error = tracked_camera->GetCameraIntrinsics(vr::k_unTrackedDeviceIndex_Hmd, camera_index, distortion_type, &foc, &ctr);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = CE_NO_CAMERA_AVAILABLE;
			std::string error_name = tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			// add break point on next line to get textual description of error
			return false;
		}
		focal_lengths[0] = foc.v[0];
		focal_lengths[1] = foc.v[1];
		center[0] = ctr.v[0];
		center[1] = ctr.v[1];
		return true;
	}
	bool openvr_camera::query_projection_impl(uint32_t camera_index, bool undistorted, float z_near, float z_far, float projection_matrix[16])
	{
		vr::EVRTrackedCameraFrameType distortion_type = undistorted ? vr::VRTrackedCameraFrameType_Undistorted : vr::VRTrackedCameraFrameType_Distorted;
		vr::HmdMatrix44_t projection;
		vr::EVRTrackedCameraError camera_error = tracked_camera->GetCameraProjection(vr::k_unTrackedDeviceIndex_Hmd, camera_index, distortion_type, z_near, z_far, &projection);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = CE_NO_CAMERA_AVAILABLE;
			std::string error_name = tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			// add break point on next line to get textual description of error
			return false;
		}
		put_hmatrix(projection, &projection_matrix[0]);
		return true;
	}
	bool openvr_camera::initialize_impl()
	{
		tracked_camera = vr::VRTrackedCamera();
		if (!tracked_camera) {
			last_error = CE_NO_CAMERA_INTERFACE;
			return false;
		}
		bool has_camera = false;
		vr::EVRTrackedCameraError camera_error =
			tracked_camera->HasCamera(vr::k_unTrackedDeviceIndex_Hmd, &has_camera);
		if (camera_error != vr::VRTrackedCameraError_None || !has_camera) {
			last_error = CE_NO_CAMERA_AVAILABLE;
			std::string error_name = tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			// add break point on next line to get textual description of error
			return false;
		}

		/*Accessing the FW description is just a further check to ensure camera
		communication is valid as expected.*/
		vr::ETrackedPropertyError propertyError;
		char buffer[128];
		hmd->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,
			vr::Prop_CameraFirmwareDescription_String,
			buffer, sizeof(buffer), &propertyError);
		if (propertyError != vr::TrackedProp_Success) {
			last_error = CE_NO_CAMERA_FIRMWARE;
			return false;
		}
		return true;
	}

	bool openvr_camera::start_impl() 
	{ 
		// start video streaming service
		last_frame_sequence = -1;
		tracked_camera->AcquireVideoStreamingService(vr::k_unTrackedDeviceIndex_Hmd, &tracked_camera_handle);
		if (tracked_camera_handle == INVALID_TRACKED_CAMERA_HANDLE) {
			last_error = CE_START_FAILED;
			return false;
		}
		return true;
	}
	bool openvr_camera::get_frame_impl(std::vector<uint8_t>& frame_data, uint32_t& width, uint32_t& height, bool undistorted, bool maximum_valid_rectangle)
	{
		vr::EVRTrackedCameraFrameType distortion_type = vr::VRTrackedCameraFrameType_Distorted;
		if (undistorted)
			distortion_type = maximum_valid_rectangle ? vr::VRTrackedCameraFrameType_MaximumUndistorted : vr::VRTrackedCameraFrameType_Undistorted;

		// allocate for camera frame buffer requirements
		uint32_t frame_size = 0;
		if (tracked_camera->GetCameraFrameSize(vr::k_unTrackedDeviceIndex_Hmd, distortion_type, &width, &height, &frame_size) != vr::VRTrackedCameraError_None)
		{
			last_error = CE_FAILED_TO_QUERY_FRAME_SIZE;
			return false;
		}
		// reallocate buffer of frame size changed
		if (frame_size != frame_data.size()) {
			switch (frame_format) {
			case CameraFrameFormat::CFF_RGBA:
				frame_data.resize(frame_size);
				break;
			default:
				last_error = CE_UNKNOWN_FRAME_FORMAT;
				return false;
			}
			frame_data.shrink_to_fit();
		}
		// get frame header to query frame index
		vr::CameraVideoStreamFrameHeader_t frame_header;
		vr::EVRTrackedCameraError camera_error =
			tracked_camera->GetVideoStreamFrameBuffer(
				tracked_camera_handle, distortion_type,
				nullptr, 0, &frame_header, sizeof(frame_header));
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = CE_GET_FRAME_FAILED;
			return false;
		}
		// check whether new frame is available
		if (frame_header.nFrameSequence == last_frame_sequence) {
			return false;
		}
		// retrieve new frame
		camera_error = tracked_camera->GetVideoStreamFrameBuffer(
			tracked_camera_handle, distortion_type,
			frame_data.data(), frame_size, &frame_header, sizeof(frame_header));
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = CE_GET_FRAME_FAILED;
			return false;
		}
		last_frame_sequence = frame_header.nFrameSequence;
		return true;
	}
	bool openvr_camera::get_gl_texture_id_impl(uint32_t& tex_id, uint32_t& width, uint32_t& height, bool undistorted, float max_valid_texcoord_range[4])
	{
		vr::EVRTrackedCameraFrameType distortion_type = undistorted ? vr::VRTrackedCameraFrameType_Undistorted : vr::VRTrackedCameraFrameType_Distorted;
		// get frame header to query frame index
		vr::CameraVideoStreamFrameHeader_t frame_header;
		vr::EVRTrackedCameraError camera_error =
			tracked_camera->GetVideoStreamFrameBuffer(
				tracked_camera_handle, distortion_type,
				nullptr, 0, &frame_header, sizeof(frame_header));
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = CE_GET_FRAME_FAILED;
			return false;
		}
		// query texture size and max valid texcoord range
		vr::VRTextureBounds_t bounds;
		camera_error = tracked_camera->GetVideoStreamTextureSize(vr::k_unTrackedDeviceIndex_Hmd, distortion_type, &bounds, &width, &height);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = CE_GET_FRAME_FAILED;
			return false;
		}
		max_valid_texcoord_range[0] = bounds.uMin;
		max_valid_texcoord_range[1] = bounds.vMin;
		max_valid_texcoord_range[2] = bounds.uMax;
		max_valid_texcoord_range[3] = bounds.vMax;

		// query texture id
		camera_error = tracked_camera->GetVideoStreamTextureGL(tracked_camera_handle, distortion_type, &tex_id, &frame_header, sizeof(frame_header));
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = CE_GET_FRAME_FAILED;
			return false;
		}
		return true;
	}

	bool openvr_camera::stop_impl() 
	{
		tracked_camera->ReleaseVideoStreamingService(tracked_camera_handle);
		tracked_camera_handle = INVALID_TRACKED_CAMERA_HANDLE;
		return true;
	}
} // namespace vr