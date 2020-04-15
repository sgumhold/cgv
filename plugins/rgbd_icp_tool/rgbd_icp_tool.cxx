#include <cgv/base/base.h>
#include "rgbd_icp_tool.h"
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/file_dialog.h>
#include <ICP.h>
#include <GoICP.h>
#include <random>

using namespace std;
using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::utils;
using namespace cgv::render;

rgbd_icp_tool::rgbd_icp_tool() {
	set_name("rgbd_icp_tool");

	prs.measure_point_size_in_pixel = false;
	prs.point_size = 0.1f;
	prs.blend_width_in_pixel = 0.0f;
	prs.blend_points = false;
}

bool rgbd_icp_tool::self_reflect(cgv::reflect::reflection_handler & rh)
{
	return
		rh.reflect_member("ply_path", ply_path);
}

void rgbd_icp_tool::on_set(void * member_ptr)
{
}

void rgbd_icp_tool::on_register()
{
}

void rgbd_icp_tool::unregister()
{
}

bool rgbd_icp_tool::init(cgv::render::context & ctx)
{
	ctx.set_bg_clr_idx(3);
	cgv::render::view* view_ptr = find_view_as_node();
	if (view_ptr) {
		view_ptr->set_view_up_dir(vec3(0, -1, 0));
		view_ptr->set_focus(vec3(0, 0, 0));
	}
	cgv::render::ref_point_renderer(ctx, 1);
	return true;
}

void rgbd_icp_tool::draw(cgv::render::context & ctx)
{
	ctx.push_modelview_matrix();
	if (source_pc.get_nr_points() > 0) {
		cgv::render::point_renderer& pr = ref_point_renderer(ctx);
		pr.set_render_style(prs);
		vector<point_cloud::Pnt> P(source_pc.get_nr_points());
		for (int i = 0; i < source_pc.get_nr_points();++i) {
			P[i] = source_pc.transformed_pnt(i);
		}
		pr.set_position_array(ctx, P);
		if (pr.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, (GLsizei)P.size());
			pr.disable(ctx);
		}
	}
	ctx.pop_modelview_matrix();
}

void rgbd_icp_tool::clear(cgv::render::context & ctx)
{
	cgv::render::ref_point_renderer(ctx, -1);
}

bool rgbd_icp_tool::handle(cgv::gui::event & e)
{
	return false;
}

void rgbd_icp_tool::stream_help(std::ostream & os)
{
}

void rgbd_icp_tool::create_gui()
{
	add_decorator("rgbd", "heading", "level=1");
	connect_copy(add_button("load source point cloud")->click, rebind(this, &rgbd_icp_tool::on_load_source_point_cloud_cb));
	connect_copy(add_button("load target point cloud")->click, rebind(this, &rgbd_icp_tool::on_load_target_point_cloud_cb));
	connect_copy(add_button("randomize source")->click, rebind(this, &rgbd_icp_tool::on_randomize_position_cb));
	
}

void rgbd_icp_tool::timer_event(double t, double dt)
{
}

void rgbd_icp_tool::on_load_source_point_cloud_cb()
{
	std::string fn = cgv::gui::file_open_dialog("source point cloud", "Point cloud files (obj,ply):*.obj;*.ply");
	source_pc.read(fn);
	post_redraw();
}

void rgbd_icp_tool::on_load_target_point_cloud_cb()
{
	std::string fn = cgv::gui::file_open_dialog("source point cloud", "OBJ files (obj,ply):*.obj;*.ply");
	target_pc.read(fn);
	post_redraw();
}

void rgbd_icp_tool::on_randomize_position_cb()
{
	uniform_real_distribution<float> angle_distribution(0.f, 3.142f);
	uniform_real_distribution<float> direction_distribution(0.f, 1.f);
	random_device rng;
	float angle = angle_distribution(rng);
	source_pc.rotate(cgv::math::quaternion<float>(normalize(vec3(direction_distribution(rng), direction_distribution(rng), direction_distribution(rng))), angle));
	source_pc.translate(vec3(direction_distribution(rng), direction_distribution(rng), direction_distribution(rng)));
	post_redraw();
}

void rgbd_icp_tool::on_step_cb()
{
	
}

#include "lib_begin.h"
#include "rgbd_icp_tool.h"

extern CGV_API object_registration<rgbd_icp_tool> kc_or("");