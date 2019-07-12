#include "rgbd_device.h"

using namespace std;

namespace rgbd {

class rgbd_emulation : public rgbd_device
{
public:
	string file_name;
	unsigned idx;
	unsigned flags;

	rgbd_emulation(const std::string& fn);

	bool attach(const std::string& fn);
	bool is_attached() const;
	bool detach();

	bool set_pitch(float y);
	
	bool has_IMU() const; 
	const IMU_info& get_IMU_info() const;
	bool put_IMU_measurement(IMU_measurement& m, unsigned time_out) const;
	
	bool check_input_stream_configuration(InputStreams is) const;
	
	bool start_device(InputStreams is);
	bool is_running() const;
	bool stop_device();
	
	unsigned get_width(InputStreams) const;
	unsigned get_height(InputStreams) const;

	bool get_frame(FrameFormat ff, void* data_ptr, int timeOut);
	void map_depth_to_color_pixel(FrameFormat depth_ff, const void* depth_data_ptr, void* color_pixel_data_ptr) const;
	void map_color_to_depth(FrameFormat depth_ff, const void* depth_data_ptr, FrameFormat color_ff, void* color_data_ptr) const;
	bool map_pixel_to_point(int x, int y, unsigned depth, FrameFormat depth_ff, float point[3]);
};

}