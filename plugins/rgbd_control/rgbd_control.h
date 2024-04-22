#pragma once

#include <rgbd_input.h>
#include <rgbd_mouse.h>

#include <cgv/base/node.h>
#include <cgv/math/fvec.h>
#include <cgv/media/color.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/data/data_view.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/texture.h>
#include <cgv_gl/point_renderer.h>
#include <libs/point_cloud/point_cloud_provider.h>
#include <libs/point_cloud/point_cloud.h>

#include <string>
#include <mutex>
#include <future>

#include "lib_begin.h"

using namespace std;

class rgbd_control : 
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider,
	public cgv::pointcloud::point_cloud_provider
{
public:
	using vec2 = cgv::vec2;
	using vec3 = cgv::vec3;
	using dvec2 = cgv::dvec2;
	using dvec3 = cgv::dvec3;
	using ivec3 = cgv::ivec3;
	using rgba8 = cgv::rgba8;

	enum VisMode { VM_COLOR, VM_DEPTH, VM_INFRARED, VM_WARPED };
	enum DeviceMode { DM_DETACHED, DM_PROTOCOL, DM_DEVICE };
	///
	rgbd_control(bool no_interactor);
	/// overload to return the type name of this object. By default the type interface is queried over get_type.
	std::string get_type_name() const { return "rgbd_control"; }
	///
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	///
	void on_set(void* member_ptr);
	/// scan rgbd devices
	void on_register();
	/// turn off rgbd device
	void unregister();

	/// adjust view
	bool init(cgv::render::context& ctx);
	/// load new textures to gpu
	void init_frame(cgv::render::context& ctx);
	/// overload to draw the content of this drawable
	void draw(cgv::render::context& ctx);
	///
	void clear(cgv::render::context& ctx);
	/// 
	bool handle(cgv::gui::event& e);
	/// 
	void stream_help(std::ostream& os);
	///
	void create_gui();
	
	bool show_grayscale;

	bool do_bilateral_filter;

	bool no_interactor = false;
  protected:
	///
	point_cloud get_point_cloud() override;
	
	cgv::signal::signal<>& new_point_cloud_ready() override;

  protected:
	void update_texture_from_frame(cgv::render::context& ctx, cgv::render::texture& tex, const rgbd::frame_type& frame, bool recreate, bool replace);
	///
	void convert_to_grayscale(const rgbd::frame_type& color_frame, rgbd::frame_type& gray_frame);

	void bilateral_filter(const rgbd::frame_type& color_frame, rgbd::frame_type& bf_frame, int d, double sigma_clr, double sigma_space, int border_type);

	void value_compute(const char& current_pixel, const char& center_pixel, char& output);
	/// members for rgbd input
	rgbd::rgbd_input rgbd_inp;
	std::string record_path;
	rgbd::MultiDeviceRole multi_device_role = rgbd::MDR_STANDALONE;
	bool do_recording;
	bool stream_color;
	bool stream_depth;
	bool stream_infrared;
	bool stream_mesh;
	std::vector<rgbd::stream_format> color_stream_formats;
	std::vector<rgbd::stream_format> depth_stream_formats;
	std::vector<rgbd::stream_format> ir_stream_formats;
	int color_stream_format_idx;
	int depth_stream_format_idx;
	int ir_stream_format_idx;
	bool also_save_pc = false;

	void rgbd_control::update_stream_formats();
	/// members for rgbd mouse
	rgbd::rgbd_mouse km;
	vec2 mouse_pos;

	/// raw point cloud
	std::vector<vec3> P, P2;
	std::vector<rgba8> C, C2;

	/// mesh
	std::vector<cgv::ivec3> M_TRIANGLES;
	std::vector<cgv::vec3> M_POINTS;
	std::vector<cgv::vec2> M_UV;

	/// processing parameters
	bool remap_color;
	bool flip[3];

	/// visualization parameters
	VisMode vis_mode;
	float color_scale;
	float depth_scale;
	float infrared_scale;
	vec2 depth_range;
	cgv::render::point_render_style prs;

	/// texture, shaders and display lists
	cgv::data::data_format color_fmt, depth_fmt, infrared_fmt;
	cgv::render::texture color, depth, infrared, warped_color;

	/// one shader program for each visualization mode
	cgv::render::shader_program rgbd_prog;

  private:
	/// internal members used to create gui
	DeviceMode device_mode;
	bool near_mode;
	int device_idx;
	float pitch;
	float x, y, z;
	float aspect;
	bool stopped;
	bool step_only;
	unsigned nr_depth_frames, nr_color_frames, nr_infrared_frames,nr_mesh_frames;
	/// internal members used for configuration
	bool color_frame_changed, depth_frame_changed, infrared_frame_changed,mesh_frame_changed;
	bool color_attachment_changed, depth_attachment_changed, infrared_attachment_changed;

	/// internal members used for data storage
	rgbd::frame_type color_frame, depth_frame, ir_frame, warped_color_frame,mesh_frame;
	rgbd::frame_type color_frame_2, depth_frame_2, ir_frame_2, warped_color_frame_2, gray_frame_2, bilateral_filter_frame_2;

	std::future<size_t> future_handle;
	size_t construct_point_cloud();
	size_t construct_point_cloud_cgv();
	bool cgv_reconstruct;
	bool use_distortion_map;
	void compute_distortion_map();
	std::vector<vec2> distortion_map;
	rgbd::rgbd_calibration calib;
	std::string calib_directory;
	std::string calib_file_name = "calib.json";
	bool prepend_serial_to_calib_file_name = true;
	bool acquire_next;
	bool always_acquire_next;
	bool visualisation_enabled;
	cgv::signal::signal<> new_point_cloud_sig;

  protected:
	void timer_event(double t, double dt);
	void on_start_cb();
	void on_step_cb();
	void on_stop_cb();
	void on_save_cb();
	void on_save_point_cloud_cb();
	void on_load_cb();
	void on_device_select_cb();
	void on_pitch_cb();
	void on_clear_protocol_cb();
};

#include <cgv/config/lib_end.h>