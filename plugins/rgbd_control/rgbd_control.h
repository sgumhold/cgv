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

#include <string>

#include "lib_begin.h"

class rgbd_control : 
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
public:
	typedef cgv::math::fvec<float,3> Pnt;
	typedef cgv::math::fvec<float,2> Tex;
	typedef cgv::media::color<float> Clr;
	enum VisMode { VM_COLOR, VM_DEPTH, VM_POINTS };
	enum DeviceMode { DM_DETACHED, DM_PROTOCOL, DM_DEVICE };
	///
	rgbd_control();
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
	bool handle(cgv::gui::event& e);
	/// 
	void stream_help(std::ostream& os);
	///
	void create_gui();
protected:
	/// members for rgbd input
	rgbd::rgbd_input kin;
	std::string protocol_path;
	bool do_protocol;

	/// members for rgbd mouse
	rgbd::rgbd_mouse km;
	Tex mouse_pos;

	/// point cloud
	std::vector<Pnt> P;
	std::vector<Clr> C;

	/// processing parameters
	bool remap_color;
	bool flip[3];

	/// visualization parameters
	VisMode vis_mode;
	float color_scale;
	float depth_scale;
	float point_size;

	/// texture, shaders and display lists
	cgv::data::data_format color_fmt, depth_fmt;
	cgv::render::texture color, depth;

	/// one shader program for each visualization mode
	cgv::render::shader_program progs[3];
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
	unsigned nr_depth_frames, nr_color_frames;
	/// internal members used for configuration
	bool color_frame_changed, depth_frame_changed;
	bool attachment_changed;

	/// internal members used for data storage
	cgv::data::data_view color_data, depth_data;

protected:
	void construct_point_cloud();
	void timer_event(double t, double dt);
	void on_start_cb();
	void on_step_cb();
	void on_stop_cb();
	void on_save_cb();
	void on_save_point_cloud_cb();
	void on_load_cb();
	void on_device_select_cb();
	void on_pitch_cb();
};

#include <cgv/config/lib_end.h>