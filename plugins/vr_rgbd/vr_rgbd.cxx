#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/trigger.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/options.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/point_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <rgbd_input.h>
#include <random>
#include <future>
#include <iostream>
#include <chrono>
#include <point_cloud/point_cloud.h>
#include <point_cloud/ICP.h>
#include <point_cloud/SICP.h>
#include <cg_vr/vr_events.h>
#include <vr/vr_state.h>
#include <vr/vr_kit.h>
#include <vr/vr_driver.h>
#include <cgv/defines/quote.h>
#include <numeric>

///@ingroup VR
///@{

/**@file
   example plugin for vr usage
*/

// these are the vr specific headers
#include <vr/vr_driver.h>
#include <cg_vr/vr_server.h>
#include <vr_view_interactor.h>
#include "intersection.h"

// different interaction states for the controllers
enum InteractionState { IS_NONE, IS_OVER, IS_GRAB };

struct vertex
{
	cgv::vec3 point;
	cgv::rgba8 color;
};

/// the plugin class vr_rgbd inherits like other plugins from node, drawable and provider
class vr_rgbd : public cgv::base::node,
				public cgv::render::drawable,
				public cgv::gui::event_handler,
				public cgv::gui::provider
{
  public:
	  using vec3 = cgv::vec3;
	  using quat = cgv::quat;
	  using mat3 = cgv::mat3;
	  using box3 = cgv::box3;
	  using rgb = cgv::rgb;
	  using rgba8 = cgv::rgba8;
	  using dvec3 = cgv::dvec3;
  protected:
	/// internal members used for data storage
	rgbd::frame_type color_frame, depth_frame, warped_color_frame;
	rgbd::frame_type color_frame_2, depth_frame_2, ir_frame_2, warped_color_frame_2;
	///
	size_t nr_depth_frames, nr_color_frames;
	///
	bool record_frame;
	///
	bool record_all_frames;
	///
	bool record_key_frames;
	bool clear_all_frames;
	bool in_calibration;
	bool zoom_in;
	bool zoom_out;
	bool save_pointcloud;
	int key_frame_step;
	bool is_save_imgs;
	int imgs_counter;
	/// intermediate point cloud and to be rendered point cloud
	std::vector<vertex> intermediate_pc, current_pc;
	/// list of recorded point clouds
	std::vector<std::vector<vertex>> recorded_pcs;
	/// translations of recorded point clouds
	std::vector<quat> rotations;
	/// rotations of recorded point clouds
	std::vector<vec3> translations;
	/// rendering style for points
	cgv::render::point_render_style point_style;
	/// counter of storing point cloud

	/// counter of pc
	int counter_pc;
	/// registration
	bool registration_started;

	/// merged point cloud 
	

	int rgbd_controller_index;
	/// current pose of the controller
	mat3 controller_orientation;
	vec3 controller_position;
	/// pose of controller when last point cloud was acquire; this is used for contruction of point cloud in parallel
	/// thread
	mat3 controller_orientation_pc;
	vec3 controller_position_pc;
	/// current calibration pose mapping from rgbd coordinates to controller coordinates
	mat3 rgbd_2_controller_orientation;
	vec3 rgbd_2_controller_position;
	/// calibration pose mapping from rgbd coordinates to controller coordinates stored at the time when freezing the
	/// point cloud for calibration
	mat3 rgbd_2_controller_orientation_start_calib;
	vec3 rgbd_2_controller_position_start_calib;
	///
	bool show_points;
	unsigned max_nr_shown_recorded_pcs;
	bool trigger_is_pressed;
	float recording_fps;
	///
	std::future<size_t> future_handle;
	///

	/// path to be set for pc files
	std::string pc_file_path;
	///
	bool rgbd_started;
	std::string rgbd_protocol_path;
	///
	rgbd::rgbd_input rgbd_inp;
	// store the scene as colored boxes
	std::vector<box3> boxes;
	std::vector<rgb> box_colors;

	// rendering style for boxes
	cgv::render::box_render_style style;

	// length of to be rendered rays
	float ray_length;

	// keep reference to vr_view_interactor
	vr_view_interactor* vr_view_ptr;

	// store the movable boxes
	std::vector<box3> movable_boxes;
	std::vector<rgb> movable_box_colors;
	std::vector<vec3> movable_box_translations;
	std::vector<quat> movable_box_rotations;

	// intersection points
	std::vector<vec3> intersection_points;
	std::vector<rgb> intersection_colors;
	std::vector<int> intersection_box_indices;
	std::vector<int> intersection_controller_indices;

	// camera pose points
	std::vector<vec3> cam_pose_points;
	std::vector<rgb> cam_pose_colors;
	std::vector<rgb> cam_cord_x_clr;
	std::vector<rgb> cam_cord_y_clr;
	std::vector<rgb> cam_cord_z_clr;
	std::vector<mat3> cam_pose_ori;
	std::vector<int> cam_pose_box_indices;
	std::vector<int> cam_pose_controller_indices;

	std::vector<vec3> cam_cord_x;
	std::vector<vec3> cam_cord_y;
	std::vector<vec3> cam_cord_z;

	// state of current interaction with boxes for each controller
	InteractionState state[4];

	// render style for interaction
	cgv::render::sphere_render_style srs;
	cgv::render::box_render_style movable_style;
	cgv::render::arrow_render_style ars;
	// declare aam for arrow renderer
	cgv::render::attribute_array_manager a_manager;
	// sicp;
	cgv::pointcloud::SICP::ComputationMode sicp_computation_mode;
	cgv::pointcloud::SICP sicp;

	point_cloud merged_pc;

	// compute intersection points of controller ray with movable boxes
	void compute_intersections(const vec3& origin, const vec3& direction, int ci, const rgb& color)
	{
		for (size_t i = 0; i < movable_boxes.size(); ++i) {
			vec3 origin_box_i = origin - movable_box_translations[i];
			movable_box_rotations[i].inverse_rotate(origin_box_i);
			vec3 direction_box_i = direction;
			movable_box_rotations[i].inverse_rotate(direction_box_i);
			float t_result;
			vec3 p_result;
			vec3 n_result;
			if (cgv::media::ray_axis_aligned_box_intersection(origin_box_i, direction_box_i, movable_boxes[i], t_result,
															  p_result, n_result, 0.000001f))
			{

				// transform result back to world coordinates
				movable_box_rotations[i].rotate(p_result);
				p_result += movable_box_translations[i];
				movable_box_rotations[i].rotate(n_result);

				// store intersection information
				intersection_points.push_back(p_result);
				intersection_colors.push_back(color);
				intersection_box_indices.push_back((int)i);
				intersection_controller_indices.push_back(ci);
			}
		}
	}

	/// register on device change events
	void on_device_change(void* kit_handle, bool attach)
	{
		post_recreate_gui();
	}
	/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
	void construct_table(float tw, float td, float th, float tW);
	/// construct boxes that represent a room of dimensions w,d,h and wall width W
	void construct_room(float w, float d, float h, float W, bool walls, bool ceiling);
	/// construct boxes for environment
	void construct_environment(float s, float ew, float ed, float eh, float w, float d, float h);
	/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
	void construct_movable_boxes(float tw, float td, float th, float tW, size_t nr);
	/// construct a scene with a table
	void build_scene(float w, float d, float h, float W, float tw, float td, float th, float tW)
	{
		construct_room(w, d, h, W, false, false);
		construct_table(tw, td, th, tW);
		construct_environment(0.2f, 3 * w, 3 * d, h, w, d, h);
		construct_movable_boxes(tw, td, th, tW, 20);
	}
	/// generate a random point cloud
	void generate_point_cloud(std::vector<vertex>& pc)
	{
		std::default_random_engine r;
		std::uniform_real_distribution<float> d(0.0f, 1.0f);
		vec3 S(0.0f, 2.0f, 0.0f);
		vec3 V(1.0f, 0, 0);
		vec3 U(0.0f, 1.0f, 0);
		vec3 X = cross(V, U);
		float aspect = 1.333f;
		float tan_2 = 0.3f;
		for (int i = 0; i < 10000; ++i) {
			float x = 2 * d(r) - 1;
			float y = 2 * d(r) - 1;
			float z = d(r) + 1;
			vec3 p = x * aspect * tan_2 * z * X + y * tan_2 * z * U + z * V;
			rgba8 c((cgv::type::uint8_type)(255 * d(r)), 0, 0);
			vertex v;
			v.point = S + p;
			v.color = c;
			pc.push_back(v);
		}
	}
	///merge all recorded pcs
	void on_merge_all_pcs_cb();
	/// start the rgbd device
	void start_rgbd()
	{
		if (!rgbd_inp.is_attached()) {
			if (rgbd::rgbd_input::get_nr_devices() == 0)
				return;
			if (!rgbd_inp.attach(rgbd::rgbd_input::get_serial(0)))
				return;
		}
		rgbd_inp.set_near_mode(true);
		std::vector<rgbd::stream_format> stream_formats;
		rgbd_started = rgbd_inp.start(rgbd::IS_COLOR_AND_DEPTH, stream_formats);
		update_member(&rgbd_started);
	}
	/// stop rgbd device
	void stop_rgbd()
	{
		if (!rgbd_inp.is_started())
			return;
		rgbd_started = rgbd_inp.stop();
		update_member(&rgbd_started);
	}

  public:
	vr_rgbd()
	{
		set_name("vr_rgbd");
		rgbd_controller_index = 0;
		controller_orientation.identity();
		controller_position = vec3(0, 1.5f, 0);
		in_calibration = false;
		zoom_in = false;
		zoom_out = false;
		save_pointcloud = true;
		registration_started = false;
		rgbd_2_controller_orientation.identity();
		rgbd_2_controller_orientation.set_col(0, vec3(-1, 0, 0));
		rgbd_2_controller_orientation.set_col(1, vec3(0, -0.7071f, 0.7071f));
		rgbd_2_controller_orientation.set_col(2, vec3(0, 0.7071f, 0.7071f));
		rgbd_2_controller_position.zeros();

		rgbd_2_controller_orientation_start_calib.identity();
		rgbd_2_controller_position_start_calib.zeros();

		build_scene(5, 7, 3, 0.2f, 1.6f, 0.8f, 0.9f, 0.03f);
		// icp_pc->read("C:/Users/ltf/Desktop/test/monkey.obj");
		// pc2vertex(*icp_pc, current_pc);
		// generate_point_cloud(current_pc);
		vr_view_ptr = 0;
		ray_length = 2;
		connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_rgbd::on_device_change);

		srs.radius = 0.005f;
		ars.length_scale = 0.03f;
		state[0] = state[1] = state[2] = state[3] = IS_NONE;
		rgbd_started = false;
		record_frame = false;
		record_all_frames = false;
		record_key_frames = false;
		clear_all_frames = false;
		trigger_is_pressed = false;
		recording_fps = 5;
		show_points = true;
		point_style.point_size = 2;
		point_style.blend_points = false;
		point_style.blend_width_in_pixel = 0;
		max_nr_shown_recorded_pcs = 20;
		counter_pc = 0;

		is_save_imgs = false;
		imgs_counter = 0;

		//pc_file_path = QUOTE_SYMBOL_VALUE(INPUT_DIR) " / .. / data";
		pc_file_path = "D://data//";
		connect(cgv::gui::get_animation_trigger().shoot, this, &vr_rgbd::timer_event);
	}
	size_t construct_point_cloud()
	{
		intermediate_pc.clear();
		const unsigned short* depths = reinterpret_cast<const unsigned short*>(&depth_frame_2.frame_data.front());
		const unsigned char* colors = reinterpret_cast<const unsigned char*>(&color_frame_2.frame_data.front());

		rgbd_inp.map_color_to_depth(depth_frame_2, color_frame_2, warped_color_frame_2);
		colors = reinterpret_cast<const unsigned char*>(&warped_color_frame_2.frame_data.front());

		if (is_save_imgs) {
			std::string fn = "D:\\rgbd_imgs\\";
			std::string fnc = fn + std::to_string(imgs_counter) + ".rgb";
			std::string fnd = fn + std::to_string(imgs_counter) + ".dep";

			depth_frame_2.write(fnd);
			warped_color_frame_2.write(fnc);
			imgs_counter++;
		}

		int i = 0;
		for (int y = 0; y < depth_frame_2.height; ++y)
			for (int x = 0; x < depth_frame_2.width; ++x) {
				vec3 p;
				if (rgbd_inp.map_depth_to_point(x, y, depths[i], &p[0])) {
					// flipping y to make it the same direction as in pixel y coordinate
					p = -p;
					p = rgbd_2_controller_orientation * p + rgbd_2_controller_position;
					p = controller_orientation_pc * p + controller_position_pc;
					rgba8 c(colors[4 * i + 2], colors[4 * i + 1], colors[4 * i], 255);
					vertex v;
					// filter points without color for 32 bit formats
					static const rgba8 filter_color = rgba8(0, 0, 0, 255);
					if (!(c == filter_color)) {
						v.color = c;
						v.point = p;
					}
					// v.point = p;
					// v.color = c;
					intermediate_pc.push_back(v);
				}
				++i;
			}
		return intermediate_pc.size();
	}
	rgbd::frame_type read_rgb_frame() // should be a thread
	{
		return color_frame;
	}
	rgbd::frame_type read_depth_frame()
	{
		return depth_frame;
	}
	/// cast vertex to point_cloud
	void copy_pointcloud(const std::vector<vertex> input, point_cloud& output)
	{
		//if(!output.has_colors()) {
			//output.has_clrs = true;
			for (unsigned int i = 0; i < input.size(); i++) {
				point_cloud_types::Pnt temp;
				temp[0] = input.at(i).point[0];
				temp[1] = input.at(i).point[1];
				temp[2] = input.at(i).point[2];
				point_cloud_types::Clr tempcolor;
				tempcolor[0] = input.at(i).color[0];
				tempcolor[1] = input.at(i).color[1];
				tempcolor[2] = input.at(i).color[2];
				output.P.push_back(temp);
				output.C.push_back(tempcolor);
			}
		//}
		
	}
	/// cast point_cloud to vertex
	void pc2vertex(const point_cloud& input, std::vector<vertex>& output)
	{
		for (unsigned int i = 0; i < input.get_nr_points(); i++) {
			vertex temp;
			temp.point[0] = input.pnt(i).x();
			temp.point[1] = input.pnt(i).y();
			temp.point[2] = input.pnt(i).z();
			/*temp.color[0] = input.clr(i)[0];
			temp.color[1] = input.clr(i)[1];
			temp.color[2] = input.clr(i)[2];*/
			temp.color[0] = 0.5;
			temp.color[1] = 0.0;
			temp.color[2] = 0.0;
			output.push_back(temp);
		}
	}
	/// here should be const point cloud
	void write_pcs_to_disk(int i)
	{
		if (!intermediate_pc.empty()) {
			// define point cloud type, wirte to disk
			point_cloud* pc_save = new point_cloud();
			pc_save->has_clrs = true;
			copy_pointcloud(intermediate_pc, *pc_save);
			/// pathname
			std::string filename = pc_file_path + std::to_string(i) + ".obj";
			pc_save->write(filename);
		}
	}
	size_t read_pc_queue(const std::string filename, std::string content)
	{
		cgv::utils::file::read(filename, content, false);
		// read pcs from disk
	}
	void registrationPointCloud()
	{
		cgv::pointcloud::ICP* icp = new cgv::pointcloud::ICP();
		if (recorded_pcs.size() >= 1) {
			cgv::math::fmat<float, 3, 3> r;
			cgv::math::fvec<float, 3> t;
			r.identity();
			t.zeros();
			point_cloud* sourcePC = new point_cloud();
			point_cloud* sourcecopy = new point_cloud();
			point_cloud* targetPC = new point_cloud();
			sourcePC->resize(intermediate_pc.size());
			targetPC->resize(recorded_pcs.front().size());
			sourcecopy->resize(intermediate_pc.size());
			copy_pointcloud(recorded_pcs.front(), *targetPC);
			copy_pointcloud(intermediate_pc, *sourcePC);
			icp->set_source_cloud(*sourcePC);
			icp->set_target_cloud(*targetPC);
			icp->set_iterations(5);
			icp->set_eps(1e-10);
			icp->reg_icp(r, t);
			for (int i = 0; i < sourcePC->get_nr_points(); i++) {
				sourcePC->pnt(i) = r * sourcePC->pnt(i) + t;
			}
			intermediate_pc.clear();
			pc2vertex(*sourcePC, intermediate_pc);
			std::cout << "size: " << recorded_pcs.size() << " " << intermediate_pc.size() << std::endl;
		}
	}

	void generate_rdm_pc(point_cloud& pc1, point_cloud& pc2)
	{
		mat3 rotate_m;
		rotate_m.identity();
		double theta = M_PI / 8; // The angle of rotation in radians
		rotate_m.set_col(0, vec3(std::cos(theta), -sin(theta), 0));
		rotate_m.set_col(1, vec3(sin(theta), std::cos(theta), 0));
		rotate_m.set_col(2, vec3(0, 0, 1));
		for (int i = 0; i < 10000; i++) {
			point_cloud_types::Pnt origin;
			origin.zeros();
			origin.x() = 1024 * rand() / (RAND_MAX + 1.0f);
			origin.y() = 1024 * rand() / (RAND_MAX + 1.0f);
			origin.z() = 1024 * rand() / (RAND_MAX + 1.0f);
			pc1.pnt(i) = origin;
			origin = rotate_m * origin;
			pc2.pnt(i) = origin + vec3(0.0, 0.4, 0.4);
		}
	}

	void test_icp()
	{
		cgv::pointcloud::ICP* icp = new cgv::pointcloud::ICP();
		cgv::math::fmat<float, 3, 3> r;
		cgv::math::fvec<float, 3> t;
		r.identity();
		t.zeros();
		point_cloud* sourcePC = new point_cloud();
		point_cloud* targetPC = new point_cloud();
		sourcePC->resize(10000);
		targetPC->resize(10000);
		generate_rdm_pc(*sourcePC, *targetPC);
		icp->set_source_cloud(*sourcePC);
		icp->set_target_cloud(*targetPC);
		// icp->set_source_cloud(*targetPC);
		// icp->set_target_cloud(*sourcePC);
		icp->set_iterations(5);
		icp->set_num_random(3);
		icp->set_eps(1e-10);
		icp->reg_icp(r, t);
	}

	void construct_TSDtree()
	{
		// using pc queue to construct the TSDtree
	}
	bool record_this_frame(double t)
	{
		if (!(record_frame || record_all_frames || trigger_is_pressed))
			return false;
		static double last_recording_time = -1;
		if (t - last_recording_time < 1.0 / recording_fps)
			return false;
		last_recording_time = t;
		return true;
	}

	void timer_event(double t, double dt)
	{
		// in case a point cloud is being constructed
		if (future_handle.valid()) {
			// check for termination of thread
			if (future_handle.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				size_t N = future_handle.get();
				// copy computed point cloud
				if (record_this_frame(t)) {
					if (registration_started && !recorded_pcs.empty()) {
						// registrationPointCloud();
						// test_icp();
						on_reg_SICP_cb();
						registration_started = !registration_started;
						update_member(&registration_started);
					}
					recorded_pcs.push_back(intermediate_pc);
					if (save_pointcloud) {
						counter_pc++;
						write_pcs_to_disk(counter_pc);
					}
					current_pc.clear();
					record_frame = false;
					update_member(&record_frame);
				}
				else if (clear_all_frames) {
					recorded_pcs.clear();
					clear_all_frames = false;
					update_member(&clear_all_frames);
				}
				else
					current_pc = intermediate_pc;
				post_redraw();
			}
		}
		if (rgbd_inp.is_started()) {
			if (rgbd_inp.is_started()) {
				bool new_frame;
				bool found_frame = false;
				bool depth_frame_changed = false;
				bool color_frame_changed = false;
				do {
					new_frame = false;
					bool new_color_frame_changed = rgbd_inp.get_frame(rgbd::IS_COLOR, color_frame, 0);
					// TODO add ORB feature extraction and estimate camera pose
					if (new_color_frame_changed) {
						++nr_color_frames;
						color_frame_changed = new_color_frame_changed;
						new_frame = true;
						update_member(&nr_color_frames);
					}
					bool new_depth_frame_changed = rgbd_inp.get_frame(rgbd::IS_DEPTH, depth_frame, 0);
					if (new_depth_frame_changed) {
						++nr_depth_frames;
						depth_frame_changed = new_depth_frame_changed;
						new_frame = true;
						update_member(&nr_depth_frames);
					}
					if (new_frame)
						found_frame = true;
				} while (new_frame);
				if (found_frame)
					post_redraw();
				if (color_frame.is_allocated() && depth_frame.is_allocated() &&
					(color_frame_changed || depth_frame_changed)) {

					if (!future_handle.valid()) {
						if (!in_calibration) {
							color_frame_2 = color_frame;
							depth_frame_2 = depth_frame;
						}
						if (zoom_out && !zoom_in) {
							controller_orientation_pc = controller_orientation * 2;
							controller_position_pc = controller_position;
						}
						else if (zoom_in && !zoom_out) {
							controller_orientation_pc = controller_orientation * 0.5;
							controller_position_pc = controller_position;
						}
						else {
							controller_orientation_pc = controller_orientation;
							controller_position_pc = controller_position;
						}
						future_handle = std::async(&vr_rgbd::construct_point_cloud, this);
					}
				}
			}
		}
	}

	std::string get_type_name() const
	{
		return "vr_rgbd";
	}
	void create_gui()
	{
		add_decorator("vr_rgbd", "heading", "level=2");
		add_gui("rgbd_protocol_path", rgbd_protocol_path, "directory", "w=150");
		add_member_control(this, "rgbd_started", rgbd_started, "check");
		add_member_control(this, "record_frame", record_frame, "check");
		add_member_control(this, "record_all_frames", record_all_frames, "check");
		add_member_control(this, "clear_all_frames", clear_all_frames, "check");
		add_member_control(this, "trigger_is_pressed", trigger_is_pressed, "check");
		add_member_control(this, "recording_fps", recording_fps, "value_slider", "min=1;max=30;ticks=true;log=true");
		add_member_control(this, "in_calibration", in_calibration, "check");
		add_member_control(this, "zoom_in", zoom_in, "check");
		add_member_control(this, "zoom_out", zoom_out, "check");
		add_member_control(this, "save_pc", save_pointcloud, "check");
		add_member_control(this, "register_pc", registration_started, "check");
		add_member_control(this, "is_save_imgs", is_save_imgs, "check");
		//connect_copy(add_button("SICP")->click, rebind(this, &vr_rgbd::on_reg_SICP_cb));
		connect_copy(add_button("merge_all_pcs")->click, rebind(this, &vr_rgbd::on_merge_all_pcs_cb));
		add_member_control(this, "rgbd_controller_index", rgbd_controller_index, "value_slider",
						   "min=0;max=3;ticks=true");

		add_member_control(this, "ray_length", ray_length, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		bool show = begin_tree_node("points", show_points, true, "w=100;align=' '");
		add_member_control(this, "show", show_points, "toggle", "w=50");
		if (show) {
			align("\a");
			add_member_control(this, "max_nr_shown_recorded_pcs", max_nr_shown_recorded_pcs, "value_slider",
							   "min=0;max=100;log=true;ticks=true");
			// add_member_control(this, "sort_points", sort_points, "check");
			if (begin_tree_node("point style", point_style)) {
				align("\a");
				add_gui("point_style", point_style);
				align("\b");
				end_tree_node(point_style);
			}
			align("\b");
			end_tree_node(show_points);
		}

		add_decorator("SICP", "heading", "level=2");
		add_member_control(this, "Max SICP runs", sicp.parameters.max_runs, "value_slider",
						   "min=1;max=500;log=false;ticks=true");
		add_member_control(this, "outer loop", sicp.parameters.max_outer_loop, "value_slider",
						   "min=1;max=100;log=false;ticks=true");
		add_member_control(this, "inner loop", sicp.parameters.max_inner_loop, "value_slider",
						   "min=1;max=20;log=false;ticks=true");
		add_member_control(this, "mu", sicp.parameters.mu, "value_slider", "min=1;max=20;log=false;ticks=true");
		add_member_control(this, "max mu", sicp.parameters.max_mu, "value_slider",
						   "min=1;max=100000;log=false;ticks=true");
		add_member_control(this, "use penalty", sicp.parameters.use_penalty, "toggle");
		add_member_control(this, "p", sicp.parameters.p, "value_slider", "min=0.1;max=1.0;log=false;ticks=true");
		add_member_control(this, "alpha", sicp.parameters.alpha, "value_slider",
						   "min=1.05;max=2.0;log=false;ticks=true");
		add_member_control(this, "stop", sicp.parameters.stop, "value_slider",
						   "min=0.00000001;max=0.001;log=true;ticks=false");
		add_member_control(this, "Computaion Mode", (cgv::type::DummyEnum&)sicp_computation_mode, "dropdown",
						   "enums='DEFAULT,POINT_TO_POINT,POINT_TO_PLANE'");

		if (begin_tree_node("box style", style)) {
			align("\a");
			add_gui("box style", style);
			align("\b");
			end_tree_node(style);
		}
		if (begin_tree_node("movable box style", movable_style)) {
			align("\a");
			add_gui("movable box style", movable_style);
			align("\b");
			end_tree_node(movable_style);
		}
		if (begin_tree_node("intersections", srs)) {
			align("\a");
			add_gui("sphere style", srs);
			align("\b");
			end_tree_node(srs);
		}
	}
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return rh.reflect_member("rgbd_controller_index", rgbd_controller_index) &&
			   rh.reflect_member("zoom_in", zoom_in) && rh.reflect_member("zoom_out", zoom_out) &&
			   rh.reflect_member("save_pc", save_pointcloud) &&
			   rh.reflect_member("register_pc", registration_started) &&
			   rh.reflect_member("recording_fps", recording_fps) && rh.reflect_member("ray_length", ray_length) &&
			   rh.reflect_member("record_frame", record_frame) &&
			   rh.reflect_member("record_all_frames", record_all_frames) &&
			   rh.reflect_member("clear_all_frames", clear_all_frames) &&
			   rh.reflect_member("rgbd_started", rgbd_started) && 
			   rh.reflect_member("is_save_imgs", is_save_imgs) &&
			   rh.reflect_member("rgbd_protocol_path", rgbd_protocol_path);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &rgbd_started && rgbd_started != rgbd_inp.is_started()) {
			if (rgbd_started)
				start_rgbd();
			else
				stop_rgbd();
		}
		if (member_ptr == &rgbd_protocol_path) {
			rgbd_inp.stop();
			rgbd_inp.detach();
			rgbd_inp.attach_path(rgbd_protocol_path);
			if (rgbd_started)
				start_rgbd();
		}
		update_member(member_ptr);
		post_redraw();
	}
	void stream_help(std::ostream& os)
	{
		os << "vr_rgbd: no shortcuts defined" << std::endl;
	}
	bool handle(cgv::gui::event& e)
	{
		// check if vr event flag is not set and don't process events in this case
		if ((e.get_flags() & cgv::gui::EF_VR) == 0)
			return false;
		// check event id
		switch (e.get_kind()) {
		case cgv::gui::EID_KEY: {
			cgv::gui::vr_key_event& vrke = static_cast<cgv::gui::vr_key_event&>(e);
			int ci = vrke.get_controller_index();
			if (ci == 0 && vrke.get_key() == vr::VR_DPAD_DOWN) {
				switch (vrke.get_action()) {
				case cgv::gui::KA_PRESS:
					rgbd_2_controller_orientation_start_calib = controller_orientation; // V^0 = V
					rgbd_2_controller_position_start_calib = controller_position;		// r^0 = r
					in_calibration = true;
					update_member(&in_calibration);
					break;
				case cgv::gui::KA_RELEASE:
					rgbd_2_controller_orientation = transpose(rgbd_2_controller_orientation_start_calib) *
													controller_orientation * rgbd_2_controller_orientation;
					rgbd_2_controller_position =
						  transpose(rgbd_2_controller_orientation_start_calib) *
						  ((controller_orientation * rgbd_2_controller_position + controller_position) -
						   rgbd_2_controller_position_start_calib);
					in_calibration = false;
					update_member(&in_calibration);
					break;
				}
			}
			if (ci == 0 && vrke.get_key() == vr::VR_DPAD_LEFT) {
				switch (vrke.get_action()) {
				case cgv::gui::KA_PRESS:
					zoom_in = true;
					update_member(&zoom_in);
					break;
				case cgv::gui::KA_RELEASE:
					zoom_in = false;
					update_member(&zoom_in);
					break;
				}
			}
			if (ci == 0 && vrke.get_key() == vr::VR_DPAD_RIGHT) {
				switch (vrke.get_action()) {
				case cgv::gui::KA_PRESS:
					zoom_out = true;
					update_member(&zoom_out);
					break;
				case cgv::gui::KA_RELEASE:
					zoom_out = false;
					update_member(&zoom_out);
					break;
				}
			}
			if (ci == 0 && vrke.get_key() == vr::VR_MENU) {
				switch (vrke.get_action()) {
				case cgv::gui::KA_PRESS:
					clear_all_frames = true;
					update_member(&clear_all_frames);
					break;
				case cgv::gui::KA_RELEASE:
					clear_all_frames = false;
					update_member(&clear_all_frames);
					break;
				}
			}
			return true;
		}
		case cgv::gui::EID_THROTTLE: {
			cgv::gui::vr_throttle_event& vrte = static_cast<cgv::gui::vr_throttle_event&>(e);
			if ((vrte.get_last_value() > 0.8f) != (vrte.get_value() > 0.8f)) {
				trigger_is_pressed = (vrte.get_value() > 0.5f);
				cam_pose_points.push_back(controller_position);
				cam_pose_ori.push_back(controller_orientation);
				cam_cord_x.push_back(cam_pose_ori.back().col(0));
				cam_cord_y.push_back(cam_pose_ori.back().col(1));
				cam_cord_z.push_back(cam_pose_ori.back().col(2));
				cam_pose_colors.push_back(rgb(1.0, 0.0, 0.0));
				cam_cord_x_clr.push_back(rgb(1.0, 0.0, 0.0));
				cam_cord_y_clr.push_back(rgb(0.0, 1.0, 0.0));
				cam_cord_z_clr.push_back(rgb(0.0, 0.0, 1.0));
				update_member(&trigger_is_pressed);
			}
			break;
		}
		case cgv::gui::EID_POSE:
			cgv::gui::vr_pose_event& vrpe = static_cast<cgv::gui::vr_pose_event&>(e);
			// check for controller pose events
			int ci = vrpe.get_trackable_index();
			if (ci == rgbd_controller_index) {
				controller_orientation = vrpe.get_orientation();
				controller_position = vrpe.get_position();
			}
			if (ci != -1) {
				if (state[ci] == IS_GRAB) {
					// in grab mode apply relative transformation to grabbed boxes

					// get previous and current controller position
					vec3 last_pos = vrpe.get_last_position();
					vec3 pos = vrpe.get_position();
					// get rotation from previous to current orientation
					// this is the current orientation matrix times the
					// inverse (or transpose) of last orientation matrix:
					// vrpe.get_orientation()*transpose(vrpe.get_last_orientation())
					mat3 rotation = vrpe.get_rotation_matrix();
					// iterate intersection points of current controller
					for (size_t i = 0; i < intersection_points.size(); ++i) {
						if (intersection_controller_indices[i] != ci)
							continue;
						// extract box index
						unsigned bi = intersection_box_indices[i];
						// update translation with position change and rotation
						movable_box_translations[bi] = rotation * (movable_box_translations[bi] - last_pos) + pos;
						// update orientation with rotation, note that quaternions
						// need to be multiplied in oposite order. In case of matrices
						// one would write box_orientation_matrix *= rotation
						movable_box_rotations[bi] = quat(rotation) * movable_box_rotations[bi];
						// update intersection points
						intersection_points[i] = rotation * (intersection_points[i] - last_pos) + pos;
					}
				}
				else { // not grab
					// clear intersections of current controller
					size_t i = 0;
					while (i < intersection_points.size()) {
						if (intersection_controller_indices[i] == ci) {
							intersection_points.erase(intersection_points.begin() + i);
							intersection_colors.erase(intersection_colors.begin() + i);
							intersection_box_indices.erase(intersection_box_indices.begin() + i);
							intersection_controller_indices.erase(intersection_controller_indices.begin() + i);
						}
						else
							++i;
					}

					// compute intersections
					vec3 origin, direction;
					vrpe.get_state().controller[ci].put_ray(&origin(0), &direction(0));
					compute_intersections(origin, direction, ci, ci == 0 ? rgb(1, 0, 0) : rgb(0, 0, 1));

					// update state based on whether we have found at least
					// one intersection with controller ray
					if (intersection_points.size() == i)
						state[ci] = IS_NONE;
					else if (state[ci] == IS_NONE)
						state[ci] = IS_OVER;
				}
				post_redraw();
			}
			return true;
		}
		return false;
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_point_renderer(ctx, 1);

		if (!cgv::utils::has_option("NO_OPENVR"))
			ctx.set_gamma(1.0f);
		cgv::gui::connect_vr_server(true);

		auto view_ptr = find_view_as_node();
		if (view_ptr) {
			view_ptr->set_eye_keep_view_angle(dvec3(0, 4, -4));
			// if the view points to a vr_view_interactor
			vr_view_ptr = dynamic_cast<vr_view_interactor*>(view_ptr);
			if (vr_view_ptr) {
				// configure vr event processing
				vr_view_ptr->set_event_type_flags(cgv::gui::VREventTypeFlags(
					  cgv::gui::VRE_KEY + cgv::gui::VRE_ONE_AXIS + cgv::gui::VRE_ONE_AXIS_GENERATES_KEY +
					  cgv::gui::VRE_TWO_AXES + cgv::gui::VRE_TWO_AXES_GENERATES_DPAD + cgv::gui::VRE_POSE));
				vr_view_ptr->enable_vr_event_debugging(false);
				// configure vr rendering
				vr_view_ptr->draw_action_zone(false);
				vr_view_ptr->draw_vr_kits(true);
				vr_view_ptr->enable_blit_vr_views(true);
				vr_view_ptr->set_blit_vr_view_width(200);
			}
		}
		cgv::render::ref_box_renderer(ctx, 1);
		cgv::render::ref_sphere_renderer(ctx, 1);
		cgv::render::ref_arrow_renderer(ctx, 1);
		return true;
	}
	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_point_renderer(ctx, -1);
		cgv::render::ref_box_renderer(ctx, -1);
		cgv::render::ref_sphere_renderer(ctx, -1);
		cgv::render::ref_arrow_renderer(ctx, -1);
	}
	void draw_pc(cgv::render::context& ctx, const std::vector<vertex>& pc)
	{
		if (pc.empty())
			return;
		auto& pr = cgv::render::ref_point_renderer(ctx);
		pr.set_position_array(ctx, &pc.front().point, pc.size(), sizeof(vertex));
		pr.set_color_array(ctx, &pc.front().color, pc.size(), sizeof(vertex));
		if (pr.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, (GLsizei)pc.size());
			pr.disable(ctx);
		}
	}

	void draw(cgv::render::context& ctx)
	{
		if (show_points) {
			auto& pr = cgv::render::ref_point_renderer(ctx);
			pr.set_render_style(point_style);
			pr.set_y_view_angle((float)vr_view_ptr->get_y_view_angle());
			draw_pc(ctx, current_pc);

			size_t begin = 0;
			size_t end = recorded_pcs.size();
			if (end > max_nr_shown_recorded_pcs)
				begin = end - max_nr_shown_recorded_pcs;

			for (size_t i = begin; i < end; ++i)
				draw_pc(ctx, recorded_pcs[i]);
		}
		if (vr_view_ptr) {
			std::vector<vec3> P;
			std::vector<rgb> C;
			const vr::vr_kit_state* state_ptr = vr_view_ptr->get_current_vr_state();
			if (state_ptr) {
				for (int ci = 0; ci < 2; ++ci)
					if (state_ptr->controller[ci].status == vr::VRS_TRACKED) {
						vec3 ray_origin, ray_direction;
						state_ptr->controller[ci].put_ray(&ray_origin(0), &ray_direction(0));
						P.push_back(ray_origin);
						P.push_back(ray_origin + ray_length * ray_direction);
						rgb c(float(1 - ci), 0.5f * (int)state[ci], float(ci));
						C.push_back(c);
						C.push_back(c);
					}
			}
			if (P.size() > 0) {
				cgv::render::shader_program& prog = ctx.ref_default_shader_program();
				int pi = prog.get_position_index();
				int ci = prog.get_color_index();
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, pi, P);
				cgv::render::attribute_array_binding::enable_global_array(ctx, pi);
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, ci, C);
				cgv::render::attribute_array_binding::enable_global_array(ctx, ci);
				glLineWidth(3);
				prog.enable(ctx);
				glDrawArrays(GL_LINES, 0, (GLsizei)P.size());
				prog.disable(ctx);
				cgv::render::attribute_array_binding::disable_global_array(ctx, pi);
				cgv::render::attribute_array_binding::disable_global_array(ctx, ci);
				glLineWidth(1);
			}
		}
		// draw static boxes
		cgv::render::box_renderer& renderer = cgv::render::ref_box_renderer(ctx);
		renderer.set_render_style(style);
		renderer.set_box_array(ctx, boxes);
		renderer.set_color_array(ctx, box_colors);
		if (renderer.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, (GLsizei)boxes.size());
		}
		renderer.disable(ctx);

		// draw dynamic boxes
		renderer.set_render_style(movable_style);
		renderer.set_box_array(ctx, movable_boxes);
		renderer.set_color_array(ctx, movable_box_colors);
		renderer.set_translation_array(ctx, movable_box_translations);
		renderer.set_rotation_array(ctx, movable_box_rotations);
		if (renderer.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, (GLsizei)movable_boxes.size());
		}
		renderer.disable(ctx);

		// draw intersection points
		if (!intersection_points.empty()) {
			auto& sr = cgv::render::ref_sphere_renderer(ctx);
			sr.set_position_array(ctx, intersection_points);
			sr.set_color_array(ctx, intersection_colors);
			sr.set_render_style(srs);
			if (sr.validate_and_enable(ctx)) {
				glDrawArrays(GL_POINTS, 0, (GLsizei)intersection_points.size());
				sr.disable(ctx);
			}
		}

		// draw pose of camera
		if (!cam_pose_points.empty()) {
			auto& sr = cgv::render::ref_sphere_renderer(ctx);
			sr.set_position_array(ctx, cam_pose_points);
			sr.set_color_array(ctx, cam_pose_colors);
			sr.set_render_style(srs);
			if (sr.validate_and_enable(ctx)) {
				glDrawArrays(GL_POINTS, 0, (GLsizei)cam_pose_colors.size());
				sr.disable(ctx);
			}
		}
		// draw coordinate of camera
		if (!cam_pose_points.empty()) {
			cgv::render::arrow_renderer& a_renderer = cgv::render::ref_arrow_renderer(ctx);
			a_renderer.set_render_style(ars);
			a_renderer.set_position_array(ctx, cam_pose_points);
			a_renderer.set_color_array(ctx, cam_cord_x_clr);
			a_renderer.set_direction_array(ctx, cam_cord_x);
			a_renderer.render(ctx, 0, cam_pose_points.size());
		}
		if (!cam_pose_points.empty()) {
			cgv::render::arrow_renderer& a_renderer = cgv::render::ref_arrow_renderer(ctx);
			a_renderer.set_render_style(ars);
			a_renderer.set_position_array(ctx, cam_pose_points);
			a_renderer.set_color_array(ctx, cam_cord_y_clr);
			a_renderer.set_direction_array(ctx, cam_cord_y);
			a_renderer.render(ctx, 0, cam_pose_points.size());
		}
		if (!cam_pose_points.empty()) {
			cgv::render::arrow_renderer& a_renderer = cgv::render::ref_arrow_renderer(ctx);
			a_renderer.set_render_style(ars);
			a_renderer.set_position_array(ctx, cam_pose_points);
			a_renderer.set_color_array(ctx, cam_cord_z_clr);
			a_renderer.set_direction_array(ctx, cam_cord_z);
			a_renderer.render(ctx, 0, cam_pose_points.size());
		}
	}
	void on_reg_SICP_cb()
	{
		point_cloud source_pc, target_pc;
		// last frame in the recorded_pcs
		copy_pointcloud(recorded_pcs.front(), target_pc);
		// current frame
		copy_pointcloud(intermediate_pc, source_pc);
		if (!target_pc.has_normals()) {
			static constexpr int k = 15;
			ann_tree neighborhood;
			neighborhood.build(target_pc);
			neighbor_graph graph;
			graph.build(target_pc.get_nr_points(), k, neighborhood);

			normal_estimator normi = normal_estimator(target_pc, graph);
			normi.compute_plane_bilateral_weighted_normals(true);
		}
		if (!source_pc.has_normals()) {
			static constexpr int k = 15;
			ann_tree neighborhood;
			neighborhood.build(source_pc);
			neighbor_graph graph;
			graph.build(source_pc.get_nr_points(), k, neighborhood);

			normal_estimator normi = normal_estimator(source_pc, graph);
			normi.compute_plane_bilateral_weighted_normals(true);
		}
		if (source_pc.has_normals() && target_pc.has_normals()) {
			std::cout << "both of them have normals" << std::endl;
		}
		sicp.set_source_cloud(source_pc);
		sicp.set_target_cloud(target_pc);
		vec3 translation, offset;
		mat3 rotation;
		sicp.parameters.max_runs = 20;
		sicp.parameters.p = 0.4f;
		//sicp.register_point_cloud(cgv::pointcloud::SICP::CM_POINT_TO_POINT, rotation, translation);
		sicp.register_point_cloud(cgv::pointcloud::SICP::CM_POINT_TO_PLANE, rotation, translation);
		std::cout << "SICP rot:\n " << rotation << "SICP t:\n" << translation << '\n';
		vec3 mean = std::accumulate(&source_pc.pnt(0), &source_pc.pnt(0) + source_pc.get_nr_points(), vec3(0, 0, 0)) /
					((float)source_pc.get_nr_points());
		// need to de-mean for rotation
		source_pc.translate(-mean);
		// do rotation
		source_pc.rotate(cgv::math::quaternion<float>(rotation));
		// do translation and reapply mean
		source_pc.translate(translation + mean);

		post_redraw();
	}
};

/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
void vr_rgbd::construct_table(float tw, float td, float th, float tW)
{
	// construct table
	rgb table_clr(0.3f, 0.2f, 0.0f);
	boxes.push_back(box3(vec3(-0.5f * tw - 2 * tW, th, -0.5f * td - 2 * tW),
						 vec3(0.5f * tw + 2 * tW, th + tW, 0.5f * td + 2 * tW)));
	box_colors.push_back(table_clr);

	boxes.push_back(box3(vec3(-0.5f * tw, 0, -0.5f * td), vec3(-0.5f * tw - tW, th, -0.5f * td - tW)));
	boxes.push_back(box3(vec3(-0.5f * tw, 0, 0.5f * td), vec3(-0.5f * tw - tW, th, 0.5f * td + tW)));
	boxes.push_back(box3(vec3(0.5f * tw, 0, -0.5f * td), vec3(0.5f * tw + tW, th, -0.5f * td - tW)));
	boxes.push_back(box3(vec3(0.5f * tw, 0, 0.5f * td), vec3(0.5f * tw + tW, th, 0.5f * td + tW)));
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
}
/// construct boxes that represent a room of dimensions w,d,h and wall width W
void vr_rgbd::construct_room(float w, float d, float h, float W, bool walls, bool ceiling)
{
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

#include <random>

/// construct boxes for environment
void vr_rgbd::construct_environment(float s, float ew, float ed, float eh, float w, float d, float h)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	unsigned n = unsigned(ew / s);
	unsigned m = unsigned(ed / s);
	for (unsigned i = 0; i < n; ++i) {
		float x = i * s - 0.5f * ew;
		for (unsigned j = 0; j < m; ++j) {
			float z = j * s - 0.5f * ed;
			if ((x + s > -0.5f * w && x < 0.5f * w) && (z + s > -0.5f * d && z < 0.5f * d))
				continue;
			float h = 0.2f * (std::max(abs(x) - 0.5f * w, 0.0f) + std::max(abs(z) - 0.5f * d, 0.0f)) *
							distribution(generator) +
					  0.1f;
			boxes.push_back(box3(vec3(x, 0, z), vec3(x + s, h, z + s)));
			box_colors.push_back(rgb(0.3f * distribution(generator) + 0.3f, 0.3f * distribution(generator) + 0.2f,
									 0.2f * distribution(generator) + 0.1f));
		}
	}
}

/// construct boxes that can be moved around
void vr_rgbd::construct_movable_boxes(float tw, float td, float th, float tW, size_t nr)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	std::uniform_real_distribution<float> signed_distribution(-1, 1);
	for (size_t i = 0; i < nr; ++i) {
		float x = distribution(generator);
		float y = distribution(generator);
		vec3 extent(distribution(generator), distribution(generator), distribution(generator));
		extent += 0.1f;
		extent *= std::min(tw, td) * 0.2f;

		vec3 center(-0.5f * tw + x * tw, th + tW, -0.5f * td + y * td);
		movable_boxes.push_back(box3(-0.5f * extent, 0.5f * extent));
		movable_box_colors.push_back(rgb(distribution(generator), distribution(generator), distribution(generator)));
		movable_box_translations.push_back(center);
		quat rot(signed_distribution(generator), signed_distribution(generator), signed_distribution(generator),
				 signed_distribution(generator));
		rot.normalize();
		movable_box_rotations.push_back(rot);
	}
}

void vr_rgbd::on_merge_all_pcs_cb()
{
	if(!recorded_pcs.empty()) {
		for (unsigned int i = 0; i < recorded_pcs.size();++i) {
			point_cloud temp;
			copy_pointcloud(recorded_pcs.at(i), temp);
			merged_pc.append(temp);
		}
		std::cout << "start writing to files" << std::endl;
		merged_pc.write("D:\\data\\merged_pc.obj");
		std::cout << "merge finish" << std::endl;
	}
}

#include <cgv/base/register.h>

cgv::base::object_registration<vr_rgbd> vr_rgbd_reg("");

///@}