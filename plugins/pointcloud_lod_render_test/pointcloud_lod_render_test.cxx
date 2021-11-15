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
	
	//glCheckError from https://learnopengl.com/In-Practice/Debugging
	GLenum glCheckError_(const char* file, int line)
	{
		GLenum errorCode;
		while ((errorCode = glGetError()) != GL_NO_ERROR)
		{
			std::string error;
			switch (errorCode)
			{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			}
			std::cout << error << " | " << file << " (" << line << ")" << std::endl;
		}
		return errorCode;
	}
	#define glCheckError() glCheckError_(__FILE__, __LINE__)
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
	srs.radius = 0.12f;
	ars.length_scale = 0.05f;

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

	color_based_on_lod = false;
	show_environment = true;
	pointcloud_fit_table = false;
	put_on_table = false;

	for (int i =0;i<num_culling_protection_zones;++i){
		culling_protection_zone_positions[i] = vec3(0.f);
		culling_protection_zone_radii[i] = 0.f;
	}
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
	update_member(member_ptr);
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
		vr_view_ptr = dynamic_cast<vr_view_interactor*>(view_ptr);
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
	
	cgv::render::ref_surfel_renderer(ctx, 1);
	cgv::render::ref_cone_renderer(ctx, 1);
	cgv::render::ref_box_renderer(ctx,1);
	cgv::render::ref_clod_point_renderer(ctx, 1);
	cgv::render::ref_sphere_renderer(ctx, 1);
	cgv::render::ref_arrow_renderer(ctx, 1);

	ctx.set_bg_color(0.7, 0.7, 0.8, 1.0);

	//build custom shader progs
	if (!custom_draw_prog.is_created()) {
		custom_draw_prog.build_program(ctx, "clod_point_labels.glpr", true);
	}

	if (!labeling_prog.is_created()) {
		labeling_prog.create(ctx);
		labeling_prog.attach_file(ctx, "point_labeler.glcs", cgv::render::ST_COMPUTE);
#ifndef NDEBUG
		if (labeling_prog.last_error.size() > 0) {
			std::cerr << labeling_prog.last_error << '\n';
			labeling_prog.last_error = "";
		}
#endif // #ifdef NDEBUG
		labeling_prog.link(ctx);
	}

	glGenBuffers(1, &point_label_buffer);

	return true;
}

void pointcloud_lod_render_test::init_frame(cgv::render::context& ctx)
{
	if (init_label_buffer && point_labels.size() > 0) {
		assert(point_label_buffer != 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, point_label_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, point_labels.size() * sizeof(GLint), point_labels.data(), GL_STATIC_READ);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glCheckError();
		init_label_buffer = false;
	}

	//run queued actions which require an opengl context
	if (!queued_actions.empty()) {
		cgv::render::clod_point_renderer& cp_renderer = ref_clod_point_renderer(ctx);
		GLuint reduced_points = cp_renderer.get_reduced_points();
		GLuint point_indices = cp_renderer.get_index_buffer();
		GLuint num_points = cp_renderer.num_reduced_points();

		for (auto& action : queued_actions) {
			label_points(ctx, action.label, action.position, action.radius, reduced_points, point_indices, num_points);
		}
		queued_actions.clear();
	}
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
	
	for (int i = 0; i < num_culling_protection_zones; ++i) {
		vec4 cps = culling_protection_zone_positions[i].lift();
		mat4 view = ctx.get_modelview_matrix();
		cps = view * cps;
		//cp_renderer.set_protection_zone(vec3(cps.x(),cps.y(),cps.z()), culling_protection_zone_radii[i], i);
	}

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
		model_transform = cgv::math::translate4(model_position)* cgv::math::rotate4<float>(model_rotation) * cgv::math::scale4(model_scale, model_scale, model_scale);
		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(ctx.get_modelview_matrix()* model_transform);
		

		if (use_label_prog) {
			cp_renderer.set_prog(custom_draw_prog);
		}

		if (cp_renderer.enable(ctx)) {
			cp_renderer.reduce_points(ctx, 0, /*std::min((size_t)source_pc.get_nr_points(), max_points),*/ (size_t)source_pc.get_nr_points());
			/*GLint* labels = static_cast<GLint*>(glMapNamedBufferRange(
				point_label_buffer, 0, source_pc.get_nr_points()*sizeof(GLint), GL_MAP_READ_BIT));

			
			glCheckError();
			glUnmapNamedBuffer(point_label_buffer);*/
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, point_label_buffer);
			glCheckError();
			cp_renderer.draw_points(ctx);
			glCheckError();
			//cp_renderer.draw(ctx, 0, std::min((size_t)source_pc.get_nr_points(), max_points));
			cp_renderer.disable(ctx);
			glCheckError();
		}
			
		ctx.pop_modelview_matrix();
	}

	cgv::render::sphere_renderer& s_renderer = ref_sphere_renderer(ctx);
	if (!coordinate_c.empty()) {
		s_renderer.set_render_style(srs);
		s_renderer.set_position_array(ctx, &coordinate_c.front().position, coordinate_c.size(), sizeof(vertex));
		s_renderer.set_radius_array(ctx, &coordinate_c.front().radius, coordinate_c.size(), sizeof(vertex));
		s_renderer.set_color_array(ctx, &coordinate_c.front().color, coordinate_c.size(), sizeof(vertex));
		s_renderer.render(ctx, 0, (GLsizei)coordinate_c.size());
	}

	cgv::render::arrow_renderer& a_renderer = ref_arrow_renderer(ctx);
	if (!points.empty()) {
		a_renderer.set_render_style(ars);
		a_renderer.set_position_array(ctx, points);
		a_renderer.set_color_array(ctx, colors);
		a_renderer.set_direction_array(ctx, normals);
		a_renderer.render(ctx, 0, points.size());
	}

	if (!forward_points.empty()) {
		a_renderer.set_render_style(ars);
		a_renderer.set_position_array(ctx, forward_points);
		a_renderer.set_color_array(ctx, forward_colors);
		a_renderer.set_direction_array(ctx, forward_normals);
		a_renderer.render(ctx, 0, forward_points.size());
	}
	
	if (view_find_point_cloud) {
		find_pointcloud(ctx);
		view_find_point_cloud = false;
	}

	// draw vr stuff
	
	if (vr_view_ptr) {
		std::array<vec3,4> P;
		std::array<float,4> R;
		std::array<rgb,4> C;
		size_t array_index = 0;
		constexpr float ray_length = 1.2;
		const vr::vr_kit_state* state_ptr = vr_view_ptr->get_current_vr_state();
		if (state_ptr) {
			for (int ci = 0; ci < 2; ++ci) if (state_ptr->controller[ci].status == vr::VRS_TRACKED) {
				vec3 ray_origin, ray_direction;
				state_ptr->controller[ci].put_ray(&ray_origin(0), &ray_direction(0));
				P[array_index] = ray_origin;
				R[array_index] = 0.002f;
				P[array_index+1] = ray_origin + ray_length * ray_direction;
				R[array_index+1] = 0.003f;
				rgb c(float(1 - ci), 0.5f, float(ci));
				C[array_index] = c;
				C[array_index+1] = c;
				array_index += 2;
			}
		}
		if (P.size() > 0) {
			auto& cr = cgv::render::ref_cone_renderer(ctx);
			cr.set_render_style(cone_style);
			cr.set_position_array(ctx, P.data(), array_index);
			cr.set_color_array(ctx, C.data(), array_index);
			cr.set_radius_array(ctx, R.data(), array_index);
			cr.render(ctx, 0, P.size());
		}
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
	custom_draw_prog.destruct(ctx);
	glDeleteBuffers(1, &point_label_buffer);

	cgv::render::ref_surfel_renderer(ctx, -1);
	cgv::render::ref_cone_renderer(ctx, -1);
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_clod_point_renderer(ctx, -1);
	cgv::render::ref_sphere_renderer(ctx, -1);
	cgv::render::ref_arrow_renderer(ctx, -1);
}

bool pointcloud_lod_render_test::handle(cgv::gui::event & e)
{
	if ((e.get_flags() & cgv::gui::EF_VR) == 0)
		return false;
	switch (e.get_kind()) {
		case cgv::gui::EID_POSE:
		{
			cgv::gui::vr_pose_event& vrpe = static_cast<cgv::gui::vr_pose_event&>(e);


			// check for controller pose events
			int ci = vrpe.get_trackable_index();

			if (ci < 0 || ci >= 2) {
				return true;
			}
			culling_protection_zone_positions[ci] = vrpe.get_position();
			if (ci != -1) {
				points.clear();
				colors.clear();
				normals.clear();
				pos = vrpe.get_position();
				ori = vrpe.get_orientation();
				points.push_back(pos);
				colors.push_back(rgb(0.0f, 1.0f, 0.0f));
				normals.push_back(ori * vec3(0.0f, 0.0f, -1.0f));
			}
		}
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
				//rotate_pc_x(-angle);
				c_pos = vr_view_ptr->get_tracking_origin();
				std::cout << "pos: " << c_pos << std::endl;

				forward_points.push_back(c_pos);
				forward_colors.push_back(rgb(0.0, 0.0, 1.0));
				forward_normals.push_back(vec3(0.0f, 0.0f, pos.z()) - c_pos);
				p.position = ori * vec3(0.0f, 0.0f, -1.0f) * 0.1f + c_pos;
				p.color = rgb(1.0, 0.0, 0.0);
				p.radius = 0.15f;
				vr_view_ptr->set_tracking_origin(p.position);
				coordinate_c.push_back(p);
				break;
			case vr::VR_DPAD_DOWN:
				//rotate_pc_x(angle);
				c_pos = vr_view_ptr->get_tracking_origin();
				std::cout << "pos: " << c_pos << std::endl;

				forward_points.push_back(c_pos);
				forward_colors.push_back(rgb(0.0, 0.0, 1.0));
				forward_normals.push_back(vec3(0.0f, 0.0f, pos.z()) - c_pos);
				p.position = ori * vec3(0.0f, 0.0f, 1.0f) * 0.1f + c_pos;
				p.color = rgb(1.0, 0.0, 0.0);
				p.radius = 0.15f;
				vr_view_ptr->set_tracking_origin(p.position);
				coordinate_c.push_back(p);
				break;
			case vr::VR_GRIP: {
				// hide points by adding a relabeling action to the queue
				int ci = vrke.get_controller_index();
				if (ci < 0 || ci >= 2)
					return true;
				point_labeling_intend action;
				action.label = (int)point_label::DELETED;
				action.position = culling_protection_zone_positions[ci];
				action.radius = culling_protection_zone_radii[ci];
				queued_actions.push_back(action);
				break;
				}

			}
			break;
		}
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
	add_member_control(this, "use label shader", use_label_prog, "toggle");
	connect_copy(add_button("random assign deleted label")->click, rebind(this, &pointcloud_lod_render_test::on_random_labels));
	std::string mode_defs = "enums='random=2;octree=1'";
	connect_copy(add_control("lod generator", (DummyEnum&)lod_mode, "dropdown", mode_defs)->value_change, rebind(this, &pointcloud_lod_render_test::on_lod_mode_change));

	add_decorator("point cloud", "heading", "level=2");

	if (begin_tree_node("CLOD render style", cp_style, false)) {
		align("\a");
		add_gui("clod style", cp_style);
		align("\b");
		end_tree_node(cp_style);
	}

	if (begin_tree_node("cone style", cone_style, false)) {
		align("\a");
		add_gui("cone style", cone_style);
		align("\b");
		end_tree_node(cone_style);
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

	if (begin_tree_node("point cloud cleaning[grip]", gui_culling_protection_zone, false)) {
		add_member_control(this, "radius controller 1", culling_protection_zone_radii[0], "value_slider", "min=0.0;max=10.0;log=false;ticks=true");
		add_member_control(this, "radius controller 2", culling_protection_zone_radii[1], "value_slider", "min=0.0;max=10.0;log=false;ticks=true");
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
	point_labels.resize(source_pc.get_nr_points(), (GLint)point_label::VISIBLE);
	init_label_buffer = true;
	renderer_out_of_date = true;
	post_redraw();
}


void pointcloud_lod_render_test::on_clear_point_cloud_cb()
{
	source_pc.clear();
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

void pointcloud_lod_render_test::on_random_labels()
{
	for (auto& label : point_labels) {
		label = label & (rand() & 1);
	}
	init_label_buffer = true;
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


void pointcloud_lod_render_test::label_points(cgv::render::context& ctx, GLint label, const vec3 position, const float radius,
	GLuint reduced_points, GLuint reduced_points_indices, const unsigned num_reduced_points)
{
	if (num_reduced_points == 0) {
		return;
	}
	glGetError();

	// shader layout constants
	constexpr int points_pos = 1, index_pos = 2, labels_pos = 6;
	mat4 model_transform_f = model_transform;
	labeling_prog.set_uniform(ctx, "sphere_position", position,true);
	labeling_prog.set_uniform(ctx, "sphere_radius", radius,true);
	labeling_prog.set_uniform(ctx, "point_label", label,true);
	labeling_prog.set_uniform(ctx, "batch_size", (GLint)num_reduced_points,true);
	labeling_prog.set_uniform(ctx, "batch_offset", (GLint)0,true);
	labeling_prog.set_uniform(ctx, "model_transform", model_transform_f,true);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, points_pos, reduced_points);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index_pos, reduced_points_indices);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, labels_pos, point_label_buffer);

	labeling_prog.enable(ctx);

	// run computation
	glDispatchCompute((num_reduced_points / 128) + 1, 1, 1);
	
	// synchronize
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	labeling_prog.disable(ctx);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//copy labels for debugging
	/*GLint* indices = static_cast<GLint*>(glMapNamedBufferRange(reduced_points_indices, 0, num_reduced_points*sizeof(int), GL_MAP_READ_BIT));
	GLint* labels = static_cast<GLint*>(glMapNamedBufferRange(point_label_buffer, 0, point_labels.size()*sizeof(int), GL_MAP_READ_BIT));
	memcpy(point_labels.data(), labels, point_labels.size() * sizeof(int));
	std::vector<int> reduced_point_indices_v; reduced_point_indices_v.resize(num_reduced_points);
	memcpy(reduced_point_indices_v.data(), indices, reduced_point_indices_v.size() * sizeof(int));
	auto first_zero = std::find(point_labels.rbegin(), point_labels.rend(), 0);
	glUnmapNamedBuffer(point_label_buffer);
	glUnmapNamedBuffer(reduced_points_indices);
	*/
}

void pointcloud_lod_render_test::clear_scene()
{
	boxes.clear();
	box_colors.clear();
}

#include "lib_begin.h"

extern CGV_API object_registration<pointcloud_lod_render_test> kc_or("");