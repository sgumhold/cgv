#pragma once

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

#include <point_cloud.h>
#include <GoICP.h>
#include <ICP.h>

#include <string>
#include <mutex>
#include <future>

#include "lib_begin.h"

class rgbd_icp_tool :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
public:
	rgbd_icp_tool();

	/// overload to return the type name of this object. By default the type interface is queried over get_type.
	std::string get_type_name() const { return "rgbd_icp_tool"; }

	bool self_reflect(cgv::reflect::reflection_handler& rh);

	void on_set(void* member_ptr);
	
	void on_register();
	
	void unregister();

	/// adjust view
	bool init(cgv::render::context& ctx);
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

protected:
	void timer_event(double t, double dt);

	void on_load_source_point_cloud_cb();
	void on_load_target_point_cloud_cb();

	void on_randomize_position_cb();
	void on_step_cb();
private:
	ICP icp;
	GoICP goicp;
	std::string ply_path;
	point_cloud source_pc, target_pc;
	cgv::render::point_render_style prs;

};

#include <cgv/config/lib_end.h>