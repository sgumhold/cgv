#pragma once
#include <type_traits>
#include <rgbd_device.h>

namespace rgbd_utils {
	
	template<unsigned S=3>
	struct frame_tuple{
		enum {
			F_COLOR = 0,
			F_DEPTH = 1,
			F_IR = 2,
		};

		rgbd::frame_type frame[S];
		bool read_frame[S];
		
		template<unsigned F>
		rgbd::frame_type get_frame() {
			read_frame[F] = true;
			return frame[F];
		}
	};


	template<unsigned S = 3>
	struct rgbd_camera_state {
		frame_tuple<S> frames;
		rgbd::IMU_measurement imu_data;
	};
}