namespace rgbd_utils {
	class kinect_azure{
			rgbd::frame_type color_frame, depth_frame, ir_frame;
			bool read_color_frame, read_depth_frame, read_ir_frame;
	};
}