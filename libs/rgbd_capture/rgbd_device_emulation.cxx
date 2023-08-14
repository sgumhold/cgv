#include "rgbd_device_emulation.h"
#include "rgbd_input.h"

#include <cgv/utils/file.h>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;
using namespace chrono;

namespace rgbd {


	//seeks for the first stream_info file with one of the file extensions from exts
	//and writes the content to the stream_format object referenced by stream
	//return true on success false otherwise
	bool find_stream_info(const string &fn_dir ,const vector<string>& exts, stream_format &stream){
		for (const string ext : exts) {
			string fn_meta = fn_dir + "/stream_info." + ext;
			void* file = cgv::utils::file::find_first(fn_meta + '*');

			if (file != nullptr) {
				string fn = fn_dir + '/' + cgv::utils::file::find_name(file);
				size_t file_size = cgv::utils::file::find_size(file);
				if (!(file_size == sizeof(stream_format))) {
					cerr << "find_stream_info: file " << fn << " has the wrong size! expected: "
						<< sizeof(stream_format) << ",got: " << file_size;
					continue;
				}
				cgv::utils::file::read(fn, reinterpret_cast<char*>(&stream), sizeof(stream_format));
				return true;
			}
		}
		return false;
	}

	rgbd_emulation::rgbd_emulation(const std::string& fn):device_is_running(false)
	{
		path_name = fn;
		flags = idx = 0;
		//init frame timers
		last_color_frame_time = 0;
		last_depth_frame_time = 0;
		last_ir_frame_time = 0;
		last_mesh_frame_time = 0;

		//list of supported extensions
		static vector<string> color_exts = {"rgb", "bgr", "rgba", "bgra", "byr"};
		static vector<string> depth_exts = {"dep", "d_p"};
		static vector<string> ir_exts = {"ir"};
		static vector<string> mesh_exts = {"p_tri"};
		//query streams
		has_color_stream = find_stream_info(path_name, color_exts, color_stream);
		has_depth_stream = find_stream_info(path_name, depth_exts, depth_stream);
		has_ir_stream = find_stream_info(path_name, ir_exts, ir_stream);
		has_mesh_stream = find_stream_info(path_name, mesh_exts, mesh_stream);

		//find first frame file
		void* file = cgv::utils::file::find_first(path_name + "/kinect_*");
		
		if (file == nullptr) {
			cerr << "rgbd_emulation::rgbd_emulation: no frame files found for the prefix:" << fn << endl;
		}
		size_t file_count = 0;
		while(file != nullptr) {
			++file_count;
			file = cgv::utils::file::find_next(file);
		}
		number_of_files = file_count;
		
		//find camera parameters
		static emulator_parameters default_intrinsics;
		static bool initialized_default_parameters = false;
		if (!initialized_default_parameters){
			default_intrinsics.intrinsics = { 5.9421434211923247e+02, 5.9104053696870778e+02,
					3.3930780975300314e+02,2.4273913761751615e+02,0.0 };
			default_intrinsics.depth_scale = 1.0;
			initialized_default_parameters = true;
		}

		string data;
		string emulator_parameters_file_name = path_name + "/emulator_parameters";
		if (cgv::utils::file::exists(emulator_parameters_file_name) && cgv::utils::file::read(path_name + "/emulator_parameters", data, false)) {
			if (data.size() * sizeof(char) == sizeof(emulator_parameters)) {
				memcpy(&parameters, data.data(), sizeof(emulator_parameters));
			} else {
				cerr << "rgbd_emulation: " << path_name << "/emulator_parameters " << "has the wrong size!";
				parameters = default_intrinsics;
			}
		} else {
			parameters = default_intrinsics;
		}

	}

	bool rgbd_emulation::attach(const std::string& fn)
	{
		path_name = fn;
		flags = idx = 0;
		return true;
	}
	bool rgbd_emulation::is_attached() const
	{
		return !path_name.empty();
	}
	
	bool rgbd_emulation::detach()
	{
		path_name = "";
		return true;
	}
	
	bool rgbd_emulation::set_pitch(float y)
	{
		return true;
	}
	
	bool rgbd_emulation::has_IMU() const
	{
		return true;
	}
	const IMU_info& rgbd_emulation::get_IMU_info() const
	{
		static IMU_info info;
		info.has_angular_acceleration = true;
		info.has_linear_acceleration = true;
		info.has_time_stamp_support = false;
		return info;
	}
	bool rgbd_emulation::put_IMU_measurement(IMU_measurement& m, unsigned time_out) const
	{
		m.angular_velocity[0] = m.angular_velocity[1] = m.angular_velocity[2] = 0.0f;
		m.linear_acceleration[0] = m.linear_acceleration[1] = m.linear_acceleration[2] = 0.0f;
		m.time_stamp = 0;
		return true;
	}
	
	bool rgbd_emulation::check_input_stream_configuration(InputStreams is) const
	{
		unsigned streams_avaiable = IS_NONE, streams_in = is;

		if (has_color_stream) {
			streams_avaiable |= IS_COLOR;
		}
		if (has_depth_stream) {
			streams_avaiable |= IS_DEPTH;
		}
		if (has_ir_stream) {
			streams_avaiable |= IS_INFRARED;
		}
		if (has_mesh_stream) {
			streams_avaiable |= IS_MESH;
		}
		return (~(~streams_in | streams_avaiable)) == 0;
	}
	void rgbd_emulation::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if (has_color_stream && (is & IS_COLOR)) {
			stream_formats.push_back(color_stream);
		}
		if (has_depth_stream && (is & IS_DEPTH)) {
			stream_formats.push_back(depth_stream);
		}
		if (has_ir_stream && (is & IS_INFRARED)) {
			stream_formats.push_back(ir_stream);
		}
		if (has_mesh_stream && (is & IS_MESH)) {
			stream_formats.push_back(mesh_stream);
		}
	}
	bool rgbd_emulation::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;

		if (!check_input_stream_configuration(is)) {
			cerr << "invalid input streams configuration!\n";
			return false;
		}
		stream_formats.resize(0);
		query_stream_formats(is, stream_formats);

		device_is_running = true;
		return true;
	}
	bool rgbd_emulation::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (is_running())
			return true;

		for (auto stream : stream_formats) {
			if (has_color_stream && stream == color_stream) {
				continue;
			}
			if (has_depth_stream && stream == depth_stream) {
				continue;
			}
			if (has_ir_stream && stream == ir_stream) {
				continue;
			}
			if (has_mesh_stream && stream == mesh_stream) {
				continue;
			}
			return false;
		}

		device_is_running = true;
		return true;
	}
	bool rgbd_emulation::is_running() const
	{
		return device_is_running;
	}
	bool rgbd_emulation::stop_device()
	{
		device_is_running = false;
		return true;
	}
	unsigned rgbd_emulation::get_width(InputStreams is) const
	{
		if (!check_input_stream_configuration(is)) {
			cerr << "invalid input stream configuration";
			return 0;
		}
		
		if (is & IS_COLOR) {
			return static_cast<unsigned>(color_stream.width);
		}

		if (is & IS_DEPTH) {
			return static_cast<unsigned>(depth_stream.width);
		}

		if (is & IS_INFRARED) {
			return static_cast<unsigned>(ir_stream.width);
		}
		return 0;
	}
	unsigned rgbd_emulation::get_height(InputStreams is) const
	{
		if (!check_input_stream_configuration(is)) {
			cerr << "invalid input stream configuration";
			return 0;
		}

		if (is & IS_COLOR) {
			return static_cast<unsigned>(color_stream.height);
		}

		if (is & IS_DEPTH) {
			return static_cast<unsigned>(depth_stream.height);
		}

		if (is & IS_INFRARED) {
			return static_cast<unsigned>(ir_stream.height);
		}
		return 0;
	}

	bool rgbd_emulation::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		return get_frame_sync(is, frame, timeOut);
	}

	bool rgbd_emulation::get_frame_sync(InputStreams is, frame_type& frame, int timeOut)
	{	
		if (!is_attached()) {
			cerr << "rgbd_emulation::get_frame called on device that has not been attached" << endl;
			return false;
		}

		if (!is_running()) {
			cerr << "rgbd_emulation::get_frame called on device that is not running" << endl;
			return false;
		}

		//get the stream information
		stream_format* stream = nullptr;
		double* last_frame_time = nullptr;

		switch (is) {
		case IS_COLOR:
			stream = &color_stream;
			last_frame_time = &last_color_frame_time;
			break;
		case IS_DEPTH:
			stream = &depth_stream;
			last_frame_time = &last_depth_frame_time;
			break;
		case IS_INFRARED:
			stream = &ir_stream;
			last_frame_time = &last_ir_frame_time;
			break;
		case IS_MESH:
			stream = &mesh_stream;
			last_frame_time = &last_mesh_frame_time;
			break;
		}

		if (stream == nullptr) {
			cerr << "rgbd_emulation::get_frame: unsupported stream\n";
			return false;
		}

		double inv_fps = 1000.f / stream->fps;
		double current_frame_time = (double)chrono::duration_cast<milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
		
		//limit fps
		if (current_frame_time < *last_frame_time+inv_fps) {
			return false;
		}
		*last_frame_time = current_frame_time;

		//check index
		if (idx >= number_of_files) idx = 0;
		frame.frame_index = idx;

		//copy frame_format data from stream
		static_cast<frame_format&>(frame) = *stream;

		string fn = compose_file_name(path_name+"/kinect_", frame , idx);
		while (!cgv::utils::file::exists(fn)) {
			++idx;
			if (idx >= number_of_files) idx = 0;
			fn = compose_file_name(path_name + "/kinect_", frame, idx);
		}

		frame.frame_index = idx;
		if (is == IS_COLOR)
			next_warped_file_name = compose_file_name(path_name + "/warped_", frame, idx);
		
		frame.time = current_frame_time;
		

		//read file
		string data;
		if (!cgv::utils::file::read(fn, data, false)) {
			cerr << "rgbd_emulation: file not found: " << fn << '\n';
			return false;
		}

		frame.buffer_size = (unsigned)data.size();
		if (frame.frame_data.size() != frame.buffer_size) {
			frame.frame_data.resize(frame.buffer_size);
		}
		memcpy(&frame.frame_data.front(), &data.front(), data.size());
		idx++;
		return true;
	}

	void rgbd_emulation::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame,
		frame_type& warped_color_frame) const
	{
		if (next_warped_file_name.empty()) {
			std::cerr << "map_color_to_depth() no warped frames saved" << std::endl;
			return;
		}
		string data;
		if (!cgv::utils::file::read(next_warped_file_name, data, false)) {
			//std::cerr << "rgbd_emulation::map_color_to_depth() warped frame " << next_warped_file_name << " not found" << std::endl;
			return;
		}
		// prepare warped frame
		static_cast<frame_size&>(warped_color_frame) = depth_frame;
		warped_color_frame.pixel_format = color_frame.pixel_format;
		warped_color_frame.nr_bits_per_pixel = color_frame.nr_bits_per_pixel;
		warped_color_frame.compute_buffer_size();
		warped_color_frame.frame_data.resize(warped_color_frame.buffer_size);
		unsigned bytes_per_pixel = color_frame.nr_bits_per_pixel / 8;

		if (warped_color_frame.frame_data.size() != data.size()) {
			std::cerr << "rgbd_emulation::map_color_to_depth() frame size mismatch: expected " 
				<< warped_color_frame.frame_data.size() << ", but found " << data.size() << std::endl;
		}
		memcpy(&warped_color_frame.frame_data.front(), &data.front(), std::min(warped_color_frame.frame_data.size(), data.size()));
	}

	bool rgbd_emulation::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		// assuming kinect
		//depth /= 8;
		if (depth == 0)
			return false;
		
		double fx_d = 1.0 / parameters.intrinsics.fx;
		double fy_d = 1.0 / parameters.intrinsics.fy;
		double cx_d = parameters.intrinsics.cx;
		double cy_d = parameters.intrinsics.cy;
		double d = parameters.depth_scale*depth;
		point_ptr[0] = float((x - cx_d) * d * fx_d);
		point_ptr[1] = float((y - cy_d) * d * fy_d);
		point_ptr[2] = float(d);
		return true;
	}

}