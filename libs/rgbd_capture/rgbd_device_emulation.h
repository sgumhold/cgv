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
	void query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const;

	bool start_device(InputStreams is, std::vector<stream_format>& stream_formats);
	bool start_device(const std::vector<stream_format>& stream_formats);
	bool is_running() const;
	bool stop_device();
	
	unsigned get_width(InputStreams) const;
	unsigned get_height(InputStreams) const;

	bool get_frame(InputStreams is, frame_type& frame, int timeOut);
	void map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
		frame_type& warped_color_frame) const;
};

}