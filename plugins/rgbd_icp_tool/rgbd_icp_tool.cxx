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

	source_prs.measure_point_size_in_pixel = false;
	source_prs.point_size = 0.25f;
	source_prs.blend_width_in_pixel = 1.0f;
	source_prs.blend_points = true;

	target_prs.measure_point_size_in_pixel = false;
	target_prs.point_size = 0.25f;
	target_prs.blend_width_in_pixel = 1.0f;
	target_prs.blend_points = true;

	lrs.line_width = 1.0f;
	//rcrs.radius = 0.001f;

	icp_iterations = 50;
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
	ctx.set_bg_color(0, 0, 0, 0.9);
	cgv::render::view* view_ptr = find_view_as_node();
	if (view_ptr) {
		view_ptr->set_view_up_dir(vec3(0, -1, 0));
		view_ptr->set_focus(vec3(0, 0, 0));
	}
	cgv::render::ref_point_renderer(ctx, 1);
	cgv::render::ref_rounded_cone_renderer(ctx, 1);
	return true;
}

void draw_point_cloud(cgv::render::context & ctx, point_cloud & pc, point_render_style & prs, cgv::math::fvec<float,4> color) {
	ctx.push_modelview_matrix();
	if (pc.get_nr_points() > 0) {
		cgv::render::point_renderer& pr = ref_point_renderer(ctx);
		pr.set_render_style(prs);
		vector<point_cloud::Pnt> P(pc.get_nr_points());
		for (int i = 0; i < pc.get_nr_points(); ++i) {
			P[i] = pc.pnt(i);
		}
		pr.set_position_array(ctx, P);
		vector<cgv::math::fvec<float, 4>> color(pc.get_nr_points(), color);
		pr.set_color_array(ctx, color);
		if (pr.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, (GLsizei)P.size());
			pr.disable(ctx);
		}
	}
	ctx.pop_modelview_matrix();
}

void draw_correspondences(cgv::render::context& ctx, point_cloud& crspd_src, point_cloud& crspd_tgt, cgv::render::rounded_cone_render_style& rcrs, cgv::math::fvec<float, 4> color) {
	ctx.push_modelview_matrix();
	if (crspd_src.get_nr_points() > 0) {
		vector<point_cloud::Pnt> P;
		//add start and end point of each correspondence in world coordinates to points
		for (int i = 0; i < crspd_src.get_nr_points(); ++i) {
			P.push_back(crspd_src.pnt(i));
			P.push_back(crspd_tgt.pnt(i));
		}
		cgv::render::rounded_cone_renderer& rcr = ref_rounded_cone_renderer(ctx);
		rcr.set_render_style(rcrs);
		rcr.set_position_array(ctx, P);
		//rcr.set_radius_array(ctx, radii);
		vector<cgv::math::fvec<float, 4>> color(P.size(), color);
		rcr.set_color_array(ctx, color);
		std::cout << "tct: " << rcr.validate_and_enable(ctx) << std::endl;
		if (rcr.validate_and_enable(ctx)) {
			glDrawArrays(GL_LINES, 0, (GLsizei)P.size());
			rcr.disable(ctx);
		}
	}
	ctx.pop_modelview_matrix();
}

void rgbd_icp_tool::draw(cgv::render::context & ctx)
{
	ctx.push_modelview_matrix();
	draw_point_cloud(ctx, source_pc, source_prs,vec4(1.0,0.0,0.0,0.8));
	draw_point_cloud(ctx, target_pc, target_prs, vec4(0.0, 1.0, 0.0, 0.8));
	draw_correspondences(ctx, crs_srs_pc, crs_tgt_pc, rcrs, vec4(0.0, 0.0, 1.0, 0.8));
	ctx.pop_modelview_matrix();

	if (view_find_point_cloud) {
		find_pointcloud(ctx);
		view_find_point_cloud = false;
	}
}

void rgbd_icp_tool::find_pointcloud(cgv::render::context & ctx)
{
	cgv::render::view* view_ptr = find_view_as_node();
	if (view_ptr) {
		const point_cloud_types::Box& sb = source_pc.box();
		const point_cloud_types::Box& tb = target_pc.box();
		point_cloud_types::Box aabb(sb);
		aabb.add_point(tb.get_min_pnt());
		aabb.add_point(tb.get_max_pnt());

		view_ptr->set_focus(aabb.get_center());
		
		view_ptr->move(view_ptr->get_depth_of_focus()-1.0);
		
	}

	
}

void rgbd_icp_tool::clear(cgv::render::context & ctx)
{
	cgv::render::ref_point_renderer(ctx, -1);
	cgv::render::ref_rounded_cone_renderer(ctx, -1);
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
	add_decorator("Point cloud registration", "heading", "level=1");
	connect_copy(add_button("load source point cloud")->click, rebind(this, &rgbd_icp_tool::on_load_source_point_cloud_cb));
	connect_copy(add_button("load target point cloud")->click, rebind(this, &rgbd_icp_tool::on_load_target_point_cloud_cb));
	connect_copy(add_button("randomize source")->click, rebind(this, &rgbd_icp_tool::on_randomize_position_cb));
	connect_copy(add_button("find point cloud")->click, rebind(this, &rgbd_icp_tool::on_reg_find_point_cloud_cb));
	connect_copy(add_button("ICP")->click, rebind(this, &rgbd_icp_tool::on_reg_ICP_cb));
	connect_copy(add_button("SICP")->click, rebind(this, &rgbd_icp_tool::on_reg_SICP_cb));
	connect_copy(add_button("GoICP")->click, rebind(this, &rgbd_icp_tool::on_reg_GoICP_cb));

	//add_member_control(this, "ICP epsilon", icp_eps, "value_slider", "min=0.0000001;max=0.1;log=true;ticks=true");
	add_decorator("point cloud", "heading", "level=2");
	connect_copy(add_control("Point size", source_prs.point_size, "value_slider", "min=0.01;max=3.0;log=false;ticks=true")->value_change, rebind(this, &rgbd_icp_tool::on_point_cloud_style_cb));

	add_decorator("ICP", "heading", "level=2");
	add_member_control(this, "Max. iterations", icp_iterations, "value_slider", "min=50;max=1000;ticks=false");

	add_decorator("Go-ICP", "heading", "level=2");
	add_member_control(this, "Go-ICP MSE Threshold", goicp.mse_threshhold, "value_slider", "min=0.000001;max=1.0;log=true;ticks=false");
	add_member_control(this, "Distance Transform size", goicp.distance_transform_size, "value_slider", "min=50;max=1000;ticks=false");

	///rounded_cone_render
	if (begin_tree_node("cone render style", rcrs)) {
		align("\a");
		add_gui("render_style", rcrs);
		align("\b");
		end_tree_node(rcrs);
	}
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
	uniform_real_distribution<float> direction_distribution(0.f, 0.05f);
	random_device rng;
	float angle = angle_distribution(rng);
	source_pc.rotate(cgv::math::quaternion<float>(normalize(vec3(direction_distribution(rng), direction_distribution(rng), direction_distribution(rng))), angle));
	source_pc.translate(vec3(direction_distribution(rng), direction_distribution(rng), direction_distribution(rng)));
	post_redraw();
}

void rgbd_icp_tool::on_reg_ICP_cb()
{
	if (!(source_pc.get_nr_points() && target_pc.get_nr_points())){
		return;
	}
	
	icp.set_source_cloud(source_pc);
	icp.set_target_cloud(target_pc);
	icp.set_iterations(icp_iterations);
	icp.set_eps(0.0000001f);

	point_cloud_types::Mat rotation;
	rotation.identity();
	point_cloud_types::Dir translation;
	translation.zeros();
	icp.set_source_cloud(source_pc);
	icp.set_target_cloud(target_pc);
	icp.set_iterations(2);
	icp.set_eps(1e-6);
	icp.set_num_random(100);

	icp.initialize();
	//icp.reg_icp(rotation, translation);
	icp.get_crspd(rotation, translation, crs_srs_pc, crs_tgt_pc);
	source_pc.rotate(cgv::math::quaternion<float>(rotation));
	source_pc.translate(translation);
	post_redraw();
}

void rgbd_icp_tool::on_reg_SICP_cb()
{

}

void rgbd_icp_tool::on_reg_GoICP_cb()
{
	if (!(source_pc.get_nr_points() && target_pc.get_nr_points())) {
		return;
	}
	goicp.set_source_cloud(source_pc);
	goicp.set_target_cloud(target_pc);
	goicp.register_pointcloud();
	source_pc.rotate(cgv::math::quaternion<float>(goicp.optimal_rotation));
	source_pc.translate(goicp.optimal_translation);
	cout << "rotation: \n" << goicp.optimal_rotation << '\n' << "translation: \n" << goicp.optimal_translation << '\n';
	post_redraw();
}

void rgbd_icp_tool::on_reg_find_point_cloud_cb()
{
	view_find_point_cloud = true;
	post_redraw();
}

void rgbd_icp_tool::on_point_cloud_style_cb()
{
	target_prs.point_size = source_prs.point_size;
	post_redraw();
}


#include "lib_begin.h"

extern CGV_API object_registration<rgbd_icp_tool> kc_or("");