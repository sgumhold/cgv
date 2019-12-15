#include "rgbd_device.h"
#include <chrono>

using namespace std;

namespace rgbd {

class rgbd_emulation : public rgbd_device
{
public:
	string path_name;
	mutable string next_warped_file_name;
	unsigned idx;
	unsigned flags;


	/*fn : filename prefix of the files used to create an instance of the emulator
		   fn needs to be a path e.g D:\kinect\kinect_ where D:\kinect\ is the directory 
		   and kinect_ the prefix used for the files.*/
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
	bool map_depth_to_point(int x, int y, int depth, float* point_ptr) const;

private:
	stream_format color_stream, depth_stream, ir_stream;
	bool has_color_stream, has_depth_stream, has_ir_stream;
	bool device_is_running;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_color_frame_time;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_depth_frame_time;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_ir_frame_time;
	size_t number_of_files;
};

}