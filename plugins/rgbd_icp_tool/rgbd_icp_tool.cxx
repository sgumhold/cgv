#include <cgv/base/base.h>
#include "rgbd_icp_tool.h"
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/file_dialog.h>
#include <random>
#include <chrono>
#include <numeric>

using namespace std;
using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::utils;
using namespace cgv::render;

using namespace cgv::pointcloud;

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

	source_srs.measure_point_size_in_pixel = false;
	source_srs.point_size = 0.25f;
	source_srs.blend_width_in_pixel = 1.0f;
	source_srs.blend_points = true;
	source_srs.illumination_mode = cgv::render::IM_TWO_SIDED;

	target_srs.measure_point_size_in_pixel = false;
	target_srs.point_size = 0.25f;
	target_srs.blend_width_in_pixel = 1.0f;
	target_srs.blend_points = true;
	target_srs.illumination_mode = cgv::render::IM_TWO_SIDED;

	rot_intensity = 0.2f;
	trans_intensity = 0.1;
	lrs.line_width = 1.0f;
	rcrs.radius = 0.001f;

	icp_iterations = 50;
	icp_eps = 1e-8;

	goicp_distance_computation_mode = GoICP::DCM_DISTANCE_TRANSFORM;
	sicp_computation_mode = SICP::CM_POINT_TO_POINT;
}

bool rgbd_icp_tool::self_reflect(cgv::reflect::reflection_handler & rh)
{
	return rh.reflect_member("ply_path", ply_path);
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
	//cgv::render::ref_point_renderer(ctx, 1);
	cgv::render::ref_surfel_renderer(ctx, 1);
	cgv::render::ref_rounded_cone_renderer(ctx, 1);
	return true;
}

void draw_point_cloud(cgv::render::context & ctx, point_cloud & pc, surfel_render_style & srs, cgv::math::fvec<float,4> color) {
	ctx.push_modelview_matrix();
	if (pc.get_nr_points() > 0) {
		cgv::render::surfel_renderer& sr = ref_surfel_renderer(ctx);
		sr.set_render_style(srs);
		vector<point_cloud::Pnt> P(pc.get_nr_points());
		vector<point_cloud::Nml> N(pc.get_nr_points());
		for (int i = 0; i < pc.get_nr_points(); ++i) {
			P[i] = pc.pnt(i);
			if (pc.has_normals()) {
				N[i] = pc.nml(i);
			}
			else {
				N[i] = cgv::math::fvec<float, 3>(1, 0, 0);
			}
		}
		sr.set_position_array(ctx, P);
		sr.set_normal_array(ctx, N);
		vector<cgv::math::fvec<float, 4>> color(pc.get_nr_points(), color);
		sr.set_color_array(ctx, color);
		sr.render(ctx, 0, P.size());
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
		rcr.render(ctx, 0, P.size());

	}
	ctx.pop_modelview_matrix();
}

void rgbd_icp_tool::draw(cgv::render::context & ctx)
{
	ctx.push_modelview_matrix();
	draw_point_cloud(ctx, source_pc, source_srs,vec4(1.0,0.0,0.0,0.8));
	draw_point_cloud(ctx, target_pc, target_srs, vec4(0.0, 1.0, 0.0, 0.8));
	draw_correspondences(ctx, crs_srs_pc, crs_tgt_pc, rcrs, vec4(0.0, 1.0, 1.0, 0.8));
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
	//cgv::render::ref_point_renderer(ctx, -1);
	cgv::render::ref_surfel_renderer(ctx, -1);
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
	connect_copy(add_button("clear point cloud")->click, rebind(this, &rgbd_icp_tool::on_clear_point_cloud_cb));
	connect_copy(add_button("randomize source")->click, rebind(this, &rgbd_icp_tool::on_randomize_position_cb));
	add_member_control(this, "rotation intensity", rot_intensity, "value_slider", "min=0.01;max=1.0;log=false;ticks=true");
	add_member_control(this,"translation intensity", trans_intensity, "value_slider", "min=0.01;max=1.0;log=false;ticks=true");
	connect_copy(add_button("find point cloud")->click, rebind(this, &rgbd_icp_tool::on_reg_find_point_cloud_cb));
	connect_copy(add_button("ICP")->click, rebind(this, &rgbd_icp_tool::on_reg_ICP_cb));
	connect_copy(add_button("SICP")->click, rebind(this, &rgbd_icp_tool::on_reg_SICP_cb));
	connect_copy(add_button("GoICP")->click, rebind(this, &rgbd_icp_tool::on_reg_GoICP_cb));

	add_decorator("point cloud", "heading", "level=2");
	connect_copy(add_control("Point size", source_srs.point_size, "value_slider", "min=0.01;max=3.0;log=false;ticks=true")->value_change, rebind(this, &rgbd_icp_tool::on_point_cloud_style_cb));

	add_decorator("ICP", "heading", "level=2");
	add_member_control(this, "Max. iterations", icp_iterations, "value_slider", "min=50;max=1000;ticks=false");
	add_member_control(this, "ICP epsilon", icp_eps, "value_slider", "min=0.0000001;max=0.1;log=true;ticks=false");
	add_member_control(this, "Sampling Type", (DummyEnum&)icp_filter_type, "dropdown", "enums='Radom Sampling,Normal-space Sampling'");

	add_decorator("Go-ICP", "heading", "level=2");
	add_member_control(this, "Go-ICP MSE Threshold", goicp.mse_threshhold, "value_slider", "min=0.000001;max=1.0;log=true;ticks=false");
	add_member_control(this, "Distance Transform size", goicp.distance_transform_size, "value_slider", "min=50;max=1000;ticks=false");

	add_member_control(this, "Distance Computaion Mode", (DummyEnum&)goicp_distance_computation_mode, "dropdown", "enums='DISTANCE_TRANSFORM,ANN_TREE'");

	add_decorator("SICP", "heading", "level=2");
	add_member_control(this, "Max SICP runs", sicp.parameters.max_runs, "value_slider", "min=1;max=500;log=false;ticks=true");
	add_member_control(this, "outer loop", sicp.parameters.max_outer_loop, "value_slider", "min=1;max=100;log=false;ticks=true");
	add_member_control(this, "inner loop", sicp.parameters.max_inner_loop, "value_slider", "min=1;max=20;log=false;ticks=true");
	add_member_control(this, "mu", sicp.parameters.mu, "value_slider", "min=1;max=20;log=false;ticks=true");
	add_member_control(this, "max mu", sicp.parameters.max_mu, "value_slider", "min=1;max=100000;log=false;ticks=true");
	add_member_control(this, "use penalty", sicp.parameters.use_penalty, "toggle");
	add_member_control(this, "p", sicp.parameters.p, "value_slider", "min=0.1;max=1.0;log=false;ticks=true");
	add_member_control(this, "alpha", sicp.parameters.alpha, "value_slider", "min=1.05;max=2.0;log=false;ticks=true");
	add_member_control(this, "stop", sicp.parameters.stop, "value_slider", "min=0.00000001;max=0.001;log=true;ticks=false");
	add_member_control(this, "Computaion Mode", (DummyEnum&)sicp_computation_mode, "dropdown", "enums='DEFAULT,POINT_TO_POINT,POINT_TO_PLANE'");


	///rounded_cone_render
	if (begin_tree_node("cone render style", rcrs)) {
		align("\a");
		add_gui("render_style", rcrs);
		align("\b");
		end_tree_node(rcrs);
	}
	///surfel_render
	if (begin_tree_node("Surfel Rendering", source_srs, false)) {
		align("\a");
		add_gui("surfel_style", source_srs);
		align("\b");
		end_tree_node(source_srs);
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

void rgbd_icp_tool::on_clear_point_cloud_cb()
{
	source_pc.clear();
	target_pc.clear();
	crs_srs_pc.clear();
	crs_tgt_pc.clear();
	post_redraw();
}

void rgbd_icp_tool::on_randomize_position_cb()
{
	uniform_real_distribution<float> angle_distribution(0.f, 3.142f);
	uniform_real_distribution<float> direction_distribution(0.f, 0.05f);
	random_device rng;
	float angle = rot_intensity*angle_distribution(rng);
	source_pc.rotate(cgv::math::quaternion<float>(normalize(vec3(direction_distribution(rng), direction_distribution(rng), direction_distribution(rng))), angle));
	source_pc.translate(trans_intensity*vec3(direction_distribution(rng), direction_distribution(rng), direction_distribution(rng)));
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
	icp.set_eps(icp_eps);
	icp.set_num_random(100);

	icp.build_ann_tree();
	//icp.reg_icp(rotation, translation);
	icp.get_crspd(rotation, translation, crs_srs_pc, crs_tgt_pc);
	source_pc.rotate(cgv::math::quaternion<float>(rotation));
	source_pc.translate(translation);
	post_redraw();
}

void rgbd_icp_tool::on_reg_SICP_cb()
{
	sicp.set_source_cloud(source_pc);
	sicp.set_target_cloud(target_pc);
	vec3 translation,offset;
	mat3 rotation;
	sicp.register_point_cloud(sicp_computation_mode,rotation,translation);
	cout << "SICP rot:\n " << rotation << "SICP t:\n" << translation << '\n';
	vec3 mean = accumulate(&source_pc.pnt(0), &source_pc.pnt(0) + source_pc.get_nr_points(), vec3(0, 0, 0)) / ((float)source_pc.get_nr_points());
	//need to de-mean for rotation
	source_pc.translate(-mean);
	//do rotation
	source_pc.rotate(cgv::math::quaternion<float>(rotation));
	//do translation and reapply mean
	source_pc.translate(translation+mean);

	post_redraw();
}

void rgbd_icp_tool::on_reg_GoICP_cb()
{
	if (!(source_pc.get_nr_points() && target_pc.get_nr_points())) {
		return;
	}

	//prepare point clouds
	float cloud_scale;
	vec3 cloud_offset;
	point_cloud target_cloud, source_cloud;
	
	// scale input pointclouds to fit in a box with side length = 1.0 and move them to origin
	cgv::media::axis_aligned_box<float, 3> compound_aabb = source_pc.box();
	auto target_aabb = target_pc.box();
	compound_aabb.add_point(target_aabb.get_max_pnt());
	compound_aabb.add_point(target_aabb.get_min_pnt());
	cloud_scale = compound_aabb.get_extent()[compound_aabb.get_max_extent_coord_index()];

	target_cloud.resize(target_pc.get_nr_points());
	source_cloud.resize(source_pc.get_nr_points());

	// Move pointclouds so the center is in (0,0,0)
	vec3 center_sc = source_pc.box().get_center();
	vec3 center_ta = target_pc.box().get_center();

	for (int i = 0; i < source_cloud.get_nr_points(); ++i) {
		source_cloud.pnt(i) = (source_pc.pnt(i) - center_sc) / cloud_scale;
	}
	for (int i = 0; i < target_cloud.get_nr_points(); ++i) {
		target_cloud.pnt(i) = (target_pc.pnt(i) - center_ta) / cloud_scale;
	}

	// do pointcloud registration

	goicp.setDistanceComputationMode(goicp_distance_computation_mode);
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	// initialize point to target point cloud distance computation
	goicp.initializeDistanceComputation(target_cloud);
		std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
		std::cout << "initialized Distance Computation in " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << "[ms]\n";
		start = std::chrono::steady_clock::now();
	// initialize data structures related to the source cloud
	goicp.initializeRegistration(source_cloud);
		stop = std::chrono::steady_clock::now();
		std::cout << "initialized regsitration in " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << "[ms]\n";
	
		start = std::chrono::steady_clock::now();
	// run registration
	goicp.registerPointcloud();
		stop = std::chrono::steady_clock::now();
		std::cout << "finished regsitration in " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << "[ms]\n";

	goicp.clear();

	// apply optimal transformations and transform back to the original coordinate system

	//method 1, apply transformation after each other on every point
	/*
	source_pc.translate(-center_sc);
	source_pc.transform(goicp.optimal_rotation);
	source_pc.translate(goicp.optimal_translation * cloud_scale + center_ta);
	*/

	//method 2, build a composit transform and apply that to every point
	mat4 transform;
	transform.identity();

	transform.set_col(3, vec4(-center_sc,1.f));
	mat4 rot;
	rot.identity();
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			rot(i, j) = goicp.optimal_rotation(i, j);
		}
	}
	transform = rot * transform;
	transform.set_col(3, vec4(goicp.optimal_translation * cloud_scale + center_ta,0.f) + transform.col(3));

	source_pc.transform(transform);
	
	cout << "go-icp rotation: \n" << goicp.optimal_rotation << '\n' << "go-icp translation: \n" << goicp.optimal_translation << '\n' <<
			"target center: \n" << center_ta << '\n' << "source center: \n" << center_sc << '\n' <<
			"pointcloud rescale " << cloud_scale << '\n' <<
			"final composit transform: \n" << transform << '\n';
	post_redraw();
}

void rgbd_icp_tool::on_reg_find_point_cloud_cb()
{
	view_find_point_cloud = true;
	post_redraw();
}

void rgbd_icp_tool::on_point_cloud_style_cb()
{
	target_srs.point_size = source_srs.point_size;
	post_redraw();
}


#include "lib_begin.h"

extern CGV_API object_registration<rgbd_icp_tool> kc_or("");