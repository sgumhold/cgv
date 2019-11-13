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
	/// access to 3x4 matrix in column major format for transformation from camera (0 .. left, 1 .. right) to head coordinates
	bool openvr_camera::put_camera_to_head_matrix(int camera_index, float* pose_matrix) const
	{
		if (camera_index > 0 && get_nr_cameras() == 1) {
			last_error = "attempt to query camera to head matrix for secondary camera on mono camera system";
			return false;
		}
		const float* begin = camera_index == 0 ? left_camera_to_head : right_camera_to_head;
		std::copy(begin, begin + 12, pose_matrix);
		return true;
	}
	/// access to 4x4 matrix in column major format for perspective transformation of camera (0..left, 1..right)
	bool openvr_camera::put_projection_matrix(int camera_index, bool undistorted, float z_near, float z_far, float* projection_matrix) const
	{
		if (state == CS_UNINITIALIZED) {
			last_error = "attempt to inspect openvr camera that is not initialized; first call the initialize function";
			return false;
		}
		vr::EVRTrackedCameraFrameType frame_type = undistorted ? vr::VRTrackedCameraFrameType_Undistorted : vr::VRTrackedCameraFrameType_Distorted;
		vr::HmdMatrix44_t projection;
		vr::EVRTrackedCameraError camera_error = tracked_camera->GetCameraProjection(vr::k_unTrackedDeviceIndex_Hmd, camera_index, frame_type, z_near, z_far, &projection);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to query projection matrix from openvr camera ";
			last_error += '0' + camera_index;
			last_error += ": ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		put_hmatrix(projection, &projection_matrix[0]);
		return true;
	}
	/// write the focal lengths in x- and y-direction to access to focal_length_2d_ptr[0|1] and the texture center to center_2d_ptr[0|1]
	bool openvr_camera::put_camera_intrinsics(int camera_index, bool undistorted, float* focal_length_2d_ptr, float* center_2d_ptr) const
	{
		if (state == CS_UNINITIALIZED) {
			last_error = "attempt to inspect openvr camera that is not initialized; first call the initialize function";
			return false;
		}
		vr::EVRTrackedCameraFrameType frame_type = undistorted ? vr::VRTrackedCameraFrameType_Undistorted : vr::VRTrackedCameraFrameType_Distorted;
		vr::HmdVector2_t foc, ctr;
		vr::EVRTrackedCameraError camera_error = tracked_camera->GetCameraIntrinsics(vr::k_unTrackedDeviceIndex_Hmd, camera_index, frame_type, &foc, &ctr);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to query camera intrinsics from openvr camera ";
			last_error += '0' + camera_index;
			last_error += ": ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		focal_length_2d_ptr[0] = foc.v[0];
		focal_length_2d_ptr[1] = foc.v[1];
		center_2d_ptr[0] = ctr.v[0];
		center_2d_ptr[1] = ctr.v[1];
		return true;
	}
	bool openvr_camera::initialize_impl()
	{
		// get interface
		tracked_camera = vr::VRTrackedCamera();
		if (!tracked_camera) {
			last_error = "no openvr camera interface available";
			return false;
		}
		// check for camera
		bool has_camera = false;
		vr::EVRTrackedCameraError camera_error =
			tracked_camera->HasCamera(vr::k_unTrackedDeviceIndex_Hmd, &has_camera);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to query whether hmd has openvr camera: ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		if (!has_camera) {
			last_error = "no openvr camera available";
			return false;
		}
		// query number of cameras
		vr::ETrackedPropertyError property_error;
		num_cameras = hmd->GetInt32TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, Prop_NumCameras_Int32, &property_error);
		if (property_error != vr::TrackedProp_Success) {
			last_error = "failed to query number of openvr camera property: ";
			hmd->GetPropErrorNameFromEnum(property_error);
			return false;
		}
		// query camera to head matrices
		if (get_nr_cameras() == 1) {
			HmdMatrix34_t M = hmd->GetMatrix34TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, Prop_CameraToHeadTransform_Matrix34, &property_error);
			if (property_error != vr::TrackedProp_Success) {
				last_error = "failed to query camera to head transformation of openvr mono camera: ";
				last_error += hmd->GetPropErrorNameFromEnum(property_error);
				return false;
			}
			put_pose_matrix(M, left_camera_to_head);
			put_pose_matrix(M, right_camera_to_head);
		}
		else {
			// this would be the correct implementation

			HmdMatrix34_t M[2];
			uint32_t cnt = hmd->GetArrayTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, Prop_CameraToHeadTransforms_Matrix34_Array,
				k_unHmdMatrix34PropertyTag, M, 2 * sizeof(HmdMatrix34_t), &property_error);
			if (property_error != vr::TrackedProp_Success) {
				last_error = "failed to query camera to head transformations of openvr stereo camera: ";
				last_error += hmd->GetPropErrorNameFromEnum(property_error);
				return false;
			}
			put_pose_matrix(M[0], left_camera_to_head);
			put_pose_matrix(M[1], right_camera_to_head);

			/*
			std::cout << "left camera to head:";
			int i;
			for (i = 0; i < 12; ++i) {
				if (i % 3 == 0)
					std::cout << std::endl;
				std::cout << " " << left_camera_to_head[i];
			}
			std::cout << "\n" << std::endl;

			std::cout << "right camera to head:";
			for (i = 0; i < 12; ++i) {
				if (i % 3 == 0)
					std::cout << std::endl;
				std::cout << " " << right_camera_to_head[i];
			}
			std::cout << "\n" << std::endl;
			*/
			// but sometime this does not work, so we query the eye to head transformations instead
			if (right_camera_to_head[0] < 0.8f || right_camera_to_head[4] < 0.8f || right_camera_to_head[8] < 0.8f) {
				put_pose_matrix(hmd->GetEyeToHeadTransform(Eye_Left), left_camera_to_head);
				put_pose_matrix(hmd->GetEyeToHeadTransform(Eye_Right), right_camera_to_head);
				/*
				std::cout << "left eye to head:";
				for (i = 0; i < 12; ++i) {
					if (i % 3 == 0)
						std::cout << std::endl;
					std::cout << " " << left_camera_to_head[i];
				}
				std::cout << "\n" << std::endl;
				std::cout << "right eye to head:";
				for (i = 0; i < 12; ++i) {
					if (i % 3 == 0)
						std::cout << std::endl;
					std::cout << " " << right_camera_to_head[i];
				}
				std::cout << "\n" << std::endl;
				*/
			}
		}
		// query frame layout
		int32_t frame_layout = hmd->GetInt32TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, Prop_CameraFrameLayout_Int32, &property_error);
		if (property_error != vr::TrackedProp_Success) {
			last_error = "failed to query frame layout of openvr camera property: ";
			hmd->GetPropErrorNameFromEnum(property_error);
			return false;
		}
		if ((frame_layout & vr::EVRTrackedCameraFrameLayout_Mono) != 0)
			frame_split = CFS_NONE;
		else if ((frame_layout & vr::EVRTrackedCameraFrameLayout_Stereo) != 0) {
			if ((frame_layout & vr::EVRTrackedCameraFrameLayout_VerticalLayout) != 0)
				frame_split = CFS_VERTICAL;
			else if ((frame_layout & vr::EVRTrackedCameraFrameLayout_HorizontalLayout) != 0)
				frame_split = CFS_HORIZONTAL;
			else {
				last_error = "unknown stereo frame layout of openvr camera";
				return false;
			}
		}
		else {
			last_error = "unknown frame layout of openvr camera";
			return false;
		}
		if ( (num_cameras == 1 && frame_split != CFS_NONE) ||
			 (num_cameras == 2 && frame_split == CFS_NONE) ) {
			last_error = "number of cameras are inconsistent with frame layout";
			return false;
		}
		// query stream format does not work
		/*
		int32_t stream_format = hmd->GetInt32TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, Prop_CameraStreamFormat_Int32, &property_error);
		if (property_error != vr::TrackedProp_Success) {
			last_error = "failed to query stream format of openvr camera property: ";
			hmd->GetPropErrorNameFromEnum(property_error);
			return false;
		}
		switch (stream_format) {
		case CVS_FORMAT_UNKNOWN:
			last_error = "unkown stream format of openvr camera";
			return false;
		case vr::CVS_FORMAT_RGB24:		// 24 bits per pixel
			frame_format = CFF_RGBA;
			break;
		case vr::CVS_FORMAT_RAW10:		// 10 bits per pixel
		case vr::CVS_FORMAT_NV12:		// 12 bits per pixel
		case vr::CVS_FORMAT_NV12_2:		// 12 bits per pixel, 2x height
		case vr::CVS_FORMAT_YUYV16:		// 16 bits per pixel
		case vr::CVS_FORMAT_BAYER16BG:   // 16 bits per pixel, 10-bit BG-format Bayer, see https://docs.opencv.org/3.1.0/de/d25/imgproc_color_conversions.html
		case vr::CVS_FORMAT_MJPEG:       // variable-sized MJPEG Open DML format, see https://www.loc.gov/preservation/digital/formats/fdd/fdd000063.shtml
			last_error = "unsupported frame format of openvr camera";
			return false;
		}
		*/
		return true;
	}

	bool openvr_camera::start_impl() 
	{ 
		// start video streaming service
		last_frame_sequence = -1;
		tracked_camera->AcquireVideoStreamingService(vr::k_unTrackedDeviceIndex_Hmd, &tracked_camera_handle);
		if (tracked_camera_handle == INVALID_TRACKED_CAMERA_HANDLE) {
			last_error = "Failed to start openvr camera, ensure that openvr camera is enabled in steamvr";
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
		vr::EVRTrackedCameraError camera_error = tracked_camera->GetCameraFrameSize(vr::k_unTrackedDeviceIndex_Hmd, distortion_type, &width, &height, &frame_size);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to query openvr camera frame size: ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		// reallocate buffer of frame size changed
		if (frame_size != frame_data.size()) {
			switch (frame_format) {
			case CameraFrameFormat::CFF_RGBA:
				frame_data.resize(frame_size);
				break;
			default:
				last_error = "unknown frame format of openvr camera";
				return false;
			}
			frame_data.shrink_to_fit();
		}
		// get frame header to query frame index
		vr::CameraVideoStreamFrameHeader_t frame_header;
		camera_error = tracked_camera->GetVideoStreamFrameBuffer(tracked_camera_handle, distortion_type, 0, 0, &frame_header, sizeof(frame_header));
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to get video stream frame header from openvr camera: ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		// check whether new frame is available
		if (frame_header.nFrameSequence == last_frame_sequence) {
			return false;
		}
		// retrieve new frame
		camera_error = tracked_camera->GetVideoStreamFrameBuffer(tracked_camera_handle, distortion_type, frame_data.data(), frame_size, &frame_header, sizeof(frame_header));
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to get frame from openvr camera: ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		last_frame_sequence = frame_header.nFrameSequence;
		return true;
	}
	bool openvr_camera::get_gl_texture_id_impl(uint32_t& tex_id, uint32_t& width, uint32_t& height, bool undistorted, float max_valid_texcoord_range[4])
	{
		vr::EVRTrackedCameraFrameType distortion_type = undistorted ? vr::VRTrackedCameraFrameType_MaximumUndistorted : vr::VRTrackedCameraFrameType_Distorted;
		// query texture size and max valid texcoord range
		vr::VRTextureBounds_t bounds;
		vr::EVRTrackedCameraError camera_error = tracked_camera->GetVideoStreamTextureSize(vr::k_unTrackedDeviceIndex_Hmd, distortion_type, &bounds, &width, &height);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to query texture size from openvr camera: ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		max_valid_texcoord_range[0] = bounds.uMin;
		max_valid_texcoord_range[1] = bounds.vMin;
		max_valid_texcoord_range[2] = bounds.uMax;
		max_valid_texcoord_range[3] = bounds.vMax;
		// get frame header
		vr::CameraVideoStreamFrameHeader_t frame_header;
		camera_error = tracked_camera->GetVideoStreamFrameBuffer(tracked_camera_handle, distortion_type, 0, 0, &frame_header, sizeof(frame_header));
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to get video stream frame header from openvr camera: ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		// query texture id
		camera_error = tracked_camera->GetVideoStreamTextureGL(tracked_camera_handle, distortion_type, &tex_id, &frame_header, sizeof(frame_header));
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed to get gl texture id of shared texture from openvr camera: ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		return true;
	}

	bool openvr_camera::stop_impl() 
	{
		vr::EVRTrackedCameraError camera_error = tracked_camera->ReleaseVideoStreamingService(tracked_camera_handle);
		if (camera_error != vr::VRTrackedCameraError_None) {
			last_error = "failed stop openvr camera: ";
			last_error += tracked_camera->GetCameraErrorNameFromEnum(camera_error);
			return false;
		}
		tracked_camera_handle = INVALID_TRACKED_CAMERA_HANDLE;
		return true;
	}
} // namespace vr