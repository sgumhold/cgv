#include <cgv/base/base.h>
#include "pointcloud_lod_render_test.h"
#include <cgv_gl/gl/gl.h>
#include <cg_vr/vr_events.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/constants.h>

#include <random>
#include <chrono>
#include <numeric>

#include <concurrency.h>

using namespace std;
using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::utils;
using namespace cgv::render;
using namespace cgv::pointcloud;

namespace {
	static cgv::pointcloud::utility::WorkerPool pool(std::thread::hardware_concurrency() - 1);
}

pointcloud_lod_render_test::pointcloud_lod_render_test() {
	set_name("pointcloud_lod_render_test");

	source_prs.measure_point_size_in_pixel = false;
	source_prs.point_size = 1.00f;
	source_prs.blend_width_in_pixel = 1.0f;
	source_prs.blend_points = true;

	source_srs.measure_point_size_in_pixel = false;
	source_srs.point_size = 1.00f;
	source_srs.blend_width_in_pixel = 1.0f;
	source_srs.blend_points = true;
	source_srs.illumination_mode = cgv::render::IM_TWO_SIDED;

	rot_intensity = 0.2f;
	trans_intensity = 0.1;
	rcrs.radius = 0.001f;

	build_scene(5, 7, 3, 0.2f, 1.6f, 0.8f, table_height, 0.03f);

	//reflected members
	/*rebuild_ptrs.insert(&model_scale);
	rebuild_ptrs.insert(&model_position.x());
	rebuild_ptrs.insert(&model_position.y());
	rebuild_ptrs.insert(&model_position.z());
	rebuild_ptrs.insert(&model_rotation.x());
	rebuild_ptrs.insert(&model_rotation.y());
	rebuild_ptrs.insert(&model_rotation.z());*/
	rebuild_ptrs.insert(&put_on_table);
	//rebuild_ptrs.insert(&color_based_on_lod);
	//rebuild_ptrs.insert(&max_points);
	rebuild_ptrs.insert(&pointcloud_fit_table);
}


namespace {
	template<typename Point>
	void generate_lods_poisson(std::vector<Point>& input_buffer_data)
	{
		static constexpr int mean = 8;
		bool run_parralel = (input_buffer_data.size() > 10'000);

		if (run_parralel) {

			struct Task {
				Point* start;
				int num_points;
			};
			struct Tasks {
				std::atomic_int next_task = 0;
				std::vector<Task> task;
			} tasks;

			int64_t points_distributed = 0;
			int64_t points_total = input_buffer_data.size();
			constexpr int64_t batch_size = 500000;

			while (points_distributed < points_total) {
				int64_t batch = std::min(batch_size, points_total - points_distributed);
				tasks.task.push_back({ &input_buffer_data[points_distributed],(int)batch });
				points_distributed += batch;
			}

			pool.run([&input_buffer_data ,&tasks](int thread_id) {
				std::poisson_distribution<int> dist(mean);
				std::random_device rdev;

				while (true) {
					//fetch task
					int tid = tasks.next_task.fetch_add(1, std::memory_order_relaxed);
					if (tid < tasks.task.size()) {
						Task& task = tasks.task[tid];

						Point* end = task.start + task.num_points;
						for (Point* p = task.start; p < end; p++) {
							p->level() = std::min(2 * mean, std::max(0, mean - abs(dist(rdev) - mean)));
						}
					}
					else {
						return;
					}
				}
				});
		}
		else {
			std::poisson_distribution<int> dist(8);
			std::random_device rdev;

			for (auto& v : input_buffer_data) {
				v.level() = std::min(2 * mean, std::max(0, mean - abs(dist(rdev) - mean)));
			}
		}
	}
}


bool pointcloud_lod_render_test::self_reflect(cgv::reflect::reflection_handler & rh)
{
	return	
		rh.reflect_member("pointcloud_fit_table", pointcloud_fit_table) &&
		rh.reflect_member("max_points", max_points) &&
		rh.reflect_member("point_color_based_on_lod", color_based_on_lod) &&
		rh.reflect_member("model_scale", model_scale) &&
		rh.reflect_member("model_position_x", model_position.x()) &&
		rh.reflect_member("model_position_y", model_position.y()) &&
		rh.reflect_member("model_position_z", model_position.z()) &&
		rh.reflect_member("model_put_on_table", put_on_table) &&
		rh.reflect_member("model_rotation_x", model_rotation.x()) &&
		rh.reflect_member("model_rotation_y", model_rotation.y()) &&
		rh.reflect_member("model_rotation_z", model_rotation.z()) && 
		rh.reflect_member("show_environment", show_environment);
}

void pointcloud_lod_render_test::on_set(void * member_ptr)
{
	if (rebuild_ptrs.find(member_ptr) != rebuild_ptrs.end()) {
		renderer_out_of_date = true;
	}
	else if (member_ptr == &show_environment){
		clear_scene();
		build_scene(5, 7, 3, 0.2f, 1.6f, 0.8f, table_height, 0.03f);
	}
	else if (member_ptr == &color_based_on_lod) {
		recolor_point_cloud = true;
	}
	recreate_gui();
}

void pointcloud_lod_render_test::on_register()
{
}

void pointcloud_lod_render_test::unregister()
{
}

bool pointcloud_lod_render_test::init(cgv::render::context & ctx)
{
	cgv::gui::connect_vr_server(true);

	ctx.set_bg_clr_idx(3);
	ctx.set_bg_color(0, 0, 0, 0.9);
	cgv::render::view* view_ptr = find_view_as_node();
	
	if (view_ptr) {
		view_ptr->set_view_up_dir(vec3(0, 1, 0));
		view_ptr->set_focus(vec3(0, 0, 0));
		//view_ptr->set_eye_keep_view_angle(dvec3(0, 4, -4));
		// if the view points to a vr_view_interactor
		vr_view_interactor* vr_view_ptr = dynamic_cast<vr_view_interactor*>(view_ptr);
		if (vr_view_ptr) {
			// configure vr event processing
			vr_view_ptr->set_event_type_flags(
				cgv::gui::VREventTypeFlags(
					cgv::gui::VRE_KEY +
					cgv::gui::VRE_ONE_AXIS +
					cgv::gui::VRE_TWO_AXES +
					cgv::gui::VRE_TWO_AXES_GENERATES_DPAD +
					cgv::gui::VRE_POSE
				));
			vr_view_ptr->enable_vr_event_debugging(false);
			// configure vr rendering
			vr_view_ptr->draw_action_zone(false);
			vr_view_ptr->draw_vr_kits(true);
			vr_view_ptr->enable_blit_vr_views(true);
			vr_view_ptr->set_blit_vr_view_width(200);
		}
	}

	//cgv::render::ref_point_renderer(ctx, 1);
	cgv::render::ref_surfel_renderer(ctx, 1);
	cgv::render::ref_rounded_cone_renderer(ctx, 1);
	cgv::render::ref_box_renderer(ctx,1);
	cgv::render::ref_clod_point_renderer(ctx, 1);
	//cp_renderer.init(ctx);
	ctx.set_bg_color(0.7, 0.7, 0.8, 1.0);
	return true;
}


void pointcloud_lod_render_test::draw(cgv::render::context & ctx)
{
	//the renderer is accessible over a reference to a singelton but new instances are also possible
	cgv::render::clod_point_renderer& cp_renderer = ref_clod_point_renderer(ctx);
	{
		cgv::render::box_renderer& renderer = cgv::render::ref_box_renderer(ctx);
		renderer.set_render_style(style);
		renderer.set_box_array(ctx, boxes);
		renderer.set_color_array(ctx, box_colors);
		renderer.render(ctx, 0, boxes.size());
	}
	cp_renderer.set_render_style(cp_style);

	if (source_pc.get_nr_points() > 0) {
		if (renderer_out_of_date) {
			rgb color(1.0, 0.0, 0.0);
			//find weighted center
			vec3 centroid(0.f), pmin(std::numeric_limits<float>::infinity()), pmax(-std::numeric_limits<float>::infinity());
			float scale = 1.f;
			for (int i = 0; i < source_pc.get_nr_points(); ++i) {
				centroid += source_pc.pnt(i);
				pmin.x() = std::min(source_pc.pnt(i).x(), pmin.x()); pmin.y() = std::min(source_pc.pnt(i).y(), pmin.y());
				pmin.z() = std::min(source_pc.pnt(i).z(), pmin.z());
				pmax.x() = std::max(source_pc.pnt(i).x(), pmax.x()); pmax.y() = std::max(source_pc.pnt(i).y(), pmax.y());
				pmax.z() = std::max(source_pc.pnt(i).z(), pmax.z());
			}
			centroid /= source_pc.get_nr_points();
			vec3 ext = (pmax - pmin);

			if (pointcloud_fit_table) {
				scale = (1.0 / static_cast<double>(*std::max_element(ext.begin(), ext.end())));
			}
			//scale *= model_scale;

			{
				std::vector<LODPoint> V(source_pc.get_nr_points());

				vec3 position(0);
				if (put_on_table) {
					position.y() = table_height - (pmin.y() - centroid.y()) * scale;
				}

				//the octree_lod_generator expects the input points to be an array of structs, so we need to reshape the data
				for (int i = 0; i < source_pc.get_nr_points(); ++i) {
					if (put_on_table)
						V[i].position() = (source_pc.pnt(i) - centroid) * scale + position;
					else
						V[i].position() = (source_pc.pnt(i)) * scale + position;
					if (source_pc.has_colors()) {
						V[i].color() = source_pc.clr(i);
					}
					else {
						V[i].color() = rgb8(color);
					}
				}
				if ((LoDMode)lod_mode == LoDMode::OCTREE) {
					points_with_lod = std::move(lod_generator.generate_lods(V));
					//cp_renderer.set_points(&points_with_lod.data()->position, &points_with_lod.data()->colors, &points_with_lod.data()->level, points_with_lod.size(),sizeof(octree_lod_generator::Vertex));
				}
				else {
					generate_lods_poisson(V);
					points_with_lod = std::move(V);
				}
			}

		}
		
		if (renderer_out_of_date || recolor_point_cloud){
			//cp_renderer.generate_lods((cgv::render::LoDMode)lod_mode);

			if (color_based_on_lod) {
				std::vector<LODPoint> pnts = points_with_lod;
				int num_points = pnts.size();
				int max_lod = 0;
				for (int i = 0; i < source_pc.get_nr_points(); ++i) {
					max_lod = std::max((int)pnts[i].level(),max_lod);
				}

				std::vector<rgb8> col_lut;
				for (int lod = 0; lod <= max_lod; ++lod) {
					cgv::media::color<float, cgv::media::HLS> col;
					col.L() = 0.5f;
					col.S() = 1.f;
					col.H() = min_level_hue + (max_level_hue - min_level_hue) * ((float)lod / (float)max_lod);
					col_lut.push_back(col);
				}
				for (int i = 0; i < num_points; ++i) {
					pnts[i].color() = col_lut[pnts[i].level()];
				}
				cp_renderer.set_points(ctx, pnts.data(), pnts.size());
				//cp_renderer.set_points(ctx,&pnts.data()->position(), &pnts.data()->color(), &pnts.data()->level(), pnts.size(), sizeof(LODPoint));
			}
			else {
				cp_renderer.set_points(ctx, &points_with_lod.data()->position(), &points_with_lod.data()->color(), &points_with_lod.data()->level(), points_with_lod.size(), sizeof(LODPoint));
			}
			renderer_out_of_date = false;
			recolor_point_cloud = false;
		}

		// apply transform and draw
		dmat4 transform = cgv::math::translate4(model_position)* cgv::math::rotate4<float>(model_rotation) * cgv::math::scale4(model_scale, model_scale, model_scale);
		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(ctx.get_modelview_matrix()* transform);
		
		if (cp_renderer.enable(ctx))
			cp_renderer.draw(ctx, 0, std::min((size_t)source_pc.get_nr_points(),max_points));
		ctx.pop_modelview_matrix();
	}
	
	if (view_find_point_cloud) {
		find_pointcloud(ctx);
		view_find_point_cloud = false;
	}
}

void pointcloud_lod_render_test::find_pointcloud(cgv::render::context & ctx)
{
	cgv::render::view* view_ptr = find_view_as_node();
	if (view_ptr) {
		const point_cloud_types::Box& sb = source_pc.box();
		point_cloud_types::Box aabb(sb);

		view_ptr->set_focus(aabb.get_center());
		
		view_ptr->move(view_ptr->get_depth_of_focus()-1.0);
		
	}

	
}

void pointcloud_lod_render_test::clear(cgv::render::context & ctx)
{
	//cgv::render::ref_point_renderer(ctx, -1);
	cgv::render::ref_surfel_renderer(ctx, -1);
	cgv::render::ref_rounded_cone_renderer(ctx, -1);
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_clod_point_renderer(ctx, -1);
}

bool pointcloud_lod_render_test::handle(cgv::gui::event & e)
{
	if ((e.get_flags() & cgv::gui::EF_VR) == 0)
		return false;
	switch (e.get_kind()) {
		case cgv::gui::EID_KEY:
		{
			static const float angle = std::asin(1.f)/3.f;
			cgv::gui::vr_key_event& vrke = static_cast<cgv::gui::vr_key_event&>(e);
			if (vrke.get_action() == cgv::gui::KA_RELEASE) {
				return true;
			}
			switch (vrke.get_key()) {
			case vr::VR_DPAD_LEFT:
				rotate_pc_y(angle);
				break;
			case vr::VR_DPAD_RIGHT:
				rotate_pc_y(-angle);
				break;
			case vr::VR_DPAD_UP:
				rotate_pc_x(-angle);
				break;
			case vr::VR_DPAD_DOWN:
				rotate_pc_x(angle);
				break;
			}
			break;
		}
		return true;
	}
	return false;
}

void pointcloud_lod_render_test::stream_help(std::ostream & os)
{
}

void pointcloud_lod_render_test::create_gui()
{
	add_decorator("Point cloud", "heading", "level=1");
	connect_copy(add_button("load point cloud")->click, rebind(this, &pointcloud_lod_render_test::on_load_point_cloud_cb));
	connect_copy(add_button("clear point cloud")->click, rebind(this, &pointcloud_lod_render_test::on_clear_point_cloud_cb));
	connect_copy(add_button("build test point cloud")->click, rebind(this, &pointcloud_lod_render_test::build_test_object_32));
	//connect_copy(add_button("randomize position")->click, rebind(this, &pointcloud_lod_render_test::on_randomize_position_cb));
	//add_member_control(this, "rotation intensity", rot_intensity, "value_slider", "min=0.01;max=1.0;log=false;ticks=true");
	//add_member_control(this,"translation intensity", trans_intensity, "value_slider", "min=0.01;max=1.0;log=false;ticks=true");
	//connect_copy(add_button("find point cloud")->click, rebind(this, &pointcloud_lod_render_test::on_reg_find_point_cloud_cb));
	//connect_copy(add_button("rotate around x axis")->click, rebind(this, &pointcloud_lod_render_test::on_rotate_x_cb));
	//connect_copy(add_button("rotate around y axis")->click, rebind(this, &pointcloud_lod_render_test::on_rotate_y_cb));
	//connect_copy(add_button("rotate around z axis")->click, rebind(this, &pointcloud_lod_render_test::on_rotate_z_cb));
	add_member_control(this, "auto-scale pointcloud", pointcloud_fit_table, "toggle");
	add_member_control(this, "place pointcloud on table", put_on_table, "toggle");
	add_member_control(this, "show LODs", color_based_on_lod, "toggle");
	add_member_control(this, "show environment", show_environment, "toggle");
	add_member_control(this, "point limit", max_points, "value_slider", "min=0;max=1000000000;log=true;ticks=true");
	std::string mode_defs = "enums='random=2;octree=1'";
	connect_copy(add_control("lod generator", (DummyEnum&)lod_mode, "dropdown", mode_defs)->value_change, rebind(this, &pointcloud_lod_render_test::on_lod_mode_change));

	add_decorator("point cloud", "heading", "level=2");

	if (begin_tree_node("CLOD render style", cp_style, false)) {
		align("\a");
		add_gui("clod style", cp_style);
		align("\b");
		end_tree_node(cp_style);
	}

	if (begin_tree_node("Model positioning", gui_model_positioning, false)) {
		align("\a");
		add_member_control(this, "model scale", model_scale, "value_slider", "min=0.1;max=5.0;log=false;ticks=true");
		add_member_control(this, "model position x", model_position.x(), "value_slider", "min=-10.0;max=10.0;log=false;ticks=true");
		add_member_control(this, "model position y", model_position.y(), "value_slider", "min=-10.0;max=10.0;log=false;ticks=true");
		add_member_control(this, "model position z", model_position.z(), "value_slider", "min=-10.0;max=10.0;log=false;ticks=true");
		add_member_control(this, "model rotation x", model_rotation.x(), "value_slider", "min=0.0;max=360;log=false;ticks=true");
		add_member_control(this, "model rotation y", model_rotation.y(), "value_slider", "min=0.0;max=360;log=false;ticks=true");
		add_member_control(this, "model rotation z", model_rotation.z(), "value_slider", "min=0.0;max=360;log=false;ticks=true");
		align("\b");
		end_tree_node(cp_style);
	}
}


void pointcloud_lod_render_test::timer_event(double t, double dt)
{
	//post_redraw();
}

void pointcloud_lod_render_test::on_load_point_cloud_cb()
{
	std::string fn = cgv::gui::file_open_dialog("source point cloud(*.obj;*.pobj;*.ply;*.bpc;*.xyz;*.pct;*.points;*.wrl;*.apc;*.pnt;*.txt)", "Point cloud files:*.obj;*.pobj;*.ply;*.bpc;*.xyz;*.pct;*.points;*.wrl;*.apc;*.pnt;*.txt;");
	source_pc.read(fn);
	renderer_out_of_date = true;
	post_redraw();
}


void pointcloud_lod_render_test::on_clear_point_cloud_cb()
{
	source_pc.clear();
	crs_srs_pc.clear();
	renderer_out_of_date = true;
	post_redraw();
}

void pointcloud_lod_render_test::on_rotate_x_cb()
{
	const float PI_H = std::asin(1);
	quat rotation = quat(vec3(1.f, 0.f, 0.f), PI_H);
	source_pc.transform(rotation.get_matrix());
	renderer_out_of_date = true;
}

void pointcloud_lod_render_test::rotate_pc_x(const float angle)
{
	quat rotation = quat(vec3(1.f, 0.f, 0.f), angle);
	source_pc.transform(rotation.get_matrix());
	renderer_out_of_date = true;
}

void pointcloud_lod_render_test::rotate_pc_y(const float angle)
{
	quat rotation = quat(vec3(0.f, 1.f, 0.f), angle);
	source_pc.transform(rotation.get_matrix());
	renderer_out_of_date = true;
}

void pointcloud_lod_render_test::on_rotate_y_cb()
{
	const float PI_H = std::asin(1);
	quat rotation = quat(vec3(0.f, 1.f, 0.f), PI_H);
	source_pc.transform(rotation.get_matrix());
	renderer_out_of_date = true;
}

void pointcloud_lod_render_test::on_rotate_z_cb()
{
	const float PI_H = std::asin(1);
	quat rotation = quat(vec3(0.f, 0.f, 1.f), PI_H);
	source_pc.transform(rotation.get_matrix());
	renderer_out_of_date = true;
}

void pointcloud_lod_render_test::on_randomize_position_cb()
{
	uniform_real_distribution<float> angle_distribution(0.f, 3.142f);
	uniform_real_distribution<float> direction_distribution(0.f, 0.05f);
	random_device rng;
	float angle = rot_intensity*angle_distribution(rng);
	source_pc.rotate(cgv::math::quaternion<float>(normalize(vec3(direction_distribution(rng), direction_distribution(rng), direction_distribution(rng))), angle));
	source_pc.translate(trans_intensity*vec3(direction_distribution(rng), direction_distribution(rng), direction_distribution(rng)));
	renderer_out_of_date = true;
	post_redraw();
}

void pointcloud_lod_render_test::on_reg_find_point_cloud_cb()
{
	view_find_point_cloud = true;
	post_redraw();
}

void pointcloud_lod_render_test::on_point_cloud_style_cb()
{
	post_redraw();
}

void pointcloud_lod_render_test::on_lod_mode_change()
{
	renderer_out_of_date = true;
}

/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
void pointcloud_lod_render_test::construct_table(float tw, float td, float th, float tW) {
	// construct table
	rgb table_clr(0.3f, 0.2f, 0.0f);
	boxes.push_back(box3(
		vec3(-0.5f * tw - 2 * tW, th - tW, -0.5f * td - 2 * tW),
		vec3(0.5f * tw + 2 * tW, th, 0.5f * td + 2 * tW)));
	box_colors.push_back(table_clr);

	boxes.push_back(box3(vec3(-0.5f * tw, 0, -0.5f * td), vec3(-0.5f * tw - tW, th - tW, -0.5f * td - tW)));
	boxes.push_back(box3(vec3(-0.5f * tw, 0, 0.5f * td), vec3(-0.5f * tw - tW, th - tW, 0.5f * td + tW)));
	boxes.push_back(box3(vec3(0.5f * tw, 0, -0.5f * td), vec3(0.5f * tw + tW, th - tW, -0.5f * td - tW)));
	boxes.push_back(box3(vec3(0.5f * tw, 0, 0.5f * td), vec3(0.5f * tw + tW, th - tW, 0.5f * td + tW)));
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
}

/// construct boxes that represent a room of dimensions w,d,h and wall width W
void pointcloud_lod_render_test::construct_room(float w, float d, float h, float W, bool walls, bool ceiling) {	
	// construct floor
	boxes.push_back(box3(vec3(-0.5f * w, -W, -0.5f * d), vec3(0.5f * w, 0, 0.5f * d)));
	box_colors.push_back(rgb(0.2f, 0.2f, 0.2f));

	if (walls) {
		// construct walls
		boxes.push_back(box3(vec3(-0.5f * w, -W, -0.5f * d - W), vec3(0.5f * w, h, -0.5f * d)));
		box_colors.push_back(rgb(0.8f, 0.5f, 0.5f));
		boxes.push_back(box3(vec3(-0.5f * w, -W, 0.5f * d), vec3(0.5f * w, h, 0.5f * d + W)));
		box_colors.push_back(rgb(0.8f, 0.5f, 0.5f));

		boxes.push_back(box3(vec3(0.5f * w, -W, -0.5f * d - W), vec3(0.5f * w + W, h, 0.5f * d + W)));
		box_colors.push_back(rgb(0.5f, 0.8f, 0.5f));
	}
	if (ceiling) {
		// construct ceiling
		boxes.push_back(box3(vec3(-0.5f * w - W, h, -0.5f * d - W), vec3(0.5f * w + W, h + W, 0.5f * d + W)));
		box_colors.push_back(rgb(0.5f, 0.5f, 0.8f));
	}
}

void pointcloud_lod_render_test::construct_environment(float s, float ew, float ed, float w, float d, float h) {
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	unsigned n = unsigned(ew / s);
	unsigned m = unsigned(ed / s);
	float ox = 0.5f * float(n) * s;
	float oz = 0.5f * float(m) * s;
	for (unsigned i = 0; i < n; ++i) {
		float x = i * s - ox;
		for (unsigned j = 0; j < m; ++j) {
			float z = j * s - oz;
			if (fabsf(x) < 0.5f * w && fabsf(x + s) < 0.5f * w && fabsf(z) < 0.5f * d && fabsf(z + s) < 0.5f * d)
				continue;
			float h = 0.2f * (std::max(abs(x) - 0.5f * w, 0.0f) + std::max(abs(z) - 0.5f * d, 0.0f)) * distribution(generator) + 0.1f;
			boxes.push_back(box3(vec3(x, 0.0f, z), vec3(x + s, h, z + s)));
			constexpr float hue_radius = 0.3;
			constexpr float hue_center = 0.4;
			rgb color = cgv::media::color<float, cgv::media::HLS>(fmod(hue_center + hue_radius * distribution(generator), 1.f), 0.1f * distribution(generator) + 0.15f, 0.6f);
			box_colors.push_back(color);
		}
	}
}

/// construct a scene with a table
void pointcloud_lod_render_test::build_scene(float w, float d, float h, float W, float tw, float td, float th, float tW)
{
	construct_room(w, d, h, W, false, false);
	construct_table(tw, td, th, tW);
	if (show_environment)
		construct_environment(0.30f, 3 * w, 3 * d, w, d, h);
}

void pointcloud_lod_render_test::build_test_object_32()
{
	int grid_size = 64;
	source_pc = build_test_point_cloud(grid_size, grid_size, grid_size, grid_size, 1.0f);
	renderer_out_of_date = true;
}

void pointcloud_lod_render_test::clear_scene()
{
	boxes.clear();
	box_colors.clear();
}

#include "lib_begin.h"

extern CGV_API object_registration<pointcloud_lod_render_test> kc_or("");