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
#include <cgv_gl/sphere_renderer.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <libs/point_cloud/gl_point_cloud_drawable.h>
#include <rgbd_input.h>
#include <random>
#include <future>

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
enum InteractionState
{
	IS_NONE,
	IS_OVER,
	IS_GRAB
};

/// the plugin class vr_rgbd inherits like other plugins from node, drawable and provider
class vr_rgbd :
	public cgv::base::node,
	public gl_point_cloud_drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
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
	/// intermediate points
	std::vector<vec3> P;
	/// intermediate colors
	std::vector<rgb> C;
	///
	std::future<size_t> future_handle;
	/// 
	bool rgbd_started;
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
	std::vector<rgb>  intersection_colors;
	std::vector<int>  intersection_box_indices;
	std::vector<int>  intersection_controller_indices;

	// state of current interaction with boxes for each controller
	InteractionState state[4];

	// render style for interaction
	cgv::render::sphere_render_style srs;
	cgv::render::box_render_style movable_style;

	// compute intersection points of controller ray with movable boxes
	void compute_intersections(const vec3& origin, const vec3& direction, int ci, const rgb& color)
	{
		for (size_t i = 0; i < movable_boxes.size(); ++i) {
			vec3 origin_box_i = origin - movable_box_translations[i];
			movable_box_rotations[i].inverse_rotate(origin_box_i);
			vec3 direction_box_i = direction;
			movable_box_rotations[i].inverse_rotate(direction_box_i);
			float t_result;
			vec3  p_result;
			vec3  n_result;
			if (cgv::media::ray_axis_aligned_box_intersection(
				origin_box_i, direction_box_i,
				movable_boxes[i],
				t_result, p_result, n_result, 0.000001f)) {

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
	void build_scene(float w, float d, float h, float W,
		float tw, float td, float th, float tW)
	{
		construct_room(w, d, h, W, false, false);
		construct_table(tw, td, th, tW);
		construct_environment(0.2f, 3 * w, 3 * d, h, w, d, h);
		construct_movable_boxes(tw, td, th, tW, 20);
	}
	/// generate a random point cloud
	void generate_point_cloud()
	{
		std::default_random_engine r;
		std::uniform_real_distribution<float> d(0.0f,1.0f);
		vec3 S(0.0f, 2.0f, 0.0f);
		vec3 V(1.0f, 0, 0);
		vec3 U(0.0f, 1.0f, 0);
		vec3 X = cross(V, U);
		float aspect = 1.333f;
		float tan_2 = 0.3f;
		//pc.create_normals();
		pc.create_colors();
		for (int i = 0; i < 10000; ++i) {
			float x = 2 * d(r) - 1;
			float y = 2 * d(r) - 1;
			float z = d(r) + 1;
			vec3 p = x * aspect * tan_2 * z * X + y * tan_2 * z * U + z*V;
			size_t pi = pc.add_point(S+p);
			//pc.nml(pi) = -normalize(p);
			pc.clr(pi) = point_cloud_types::Clr(point_cloud_types::float_to_color_component(d(r)),0,0);
		}
		show_point_begin = 0;
		show_point_end = pc.get_nr_points();
		show_point_step = 1;
	}
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
		build_scene(5, 7, 3, 0.2f, 1.6f, 0.8f, 0.9f, 0.03f);
		//generate_point_cloud();
		pc.create_colors();
		show_point_begin = 0;
		show_point_end = 0;
		show_point_step = 1;
		vr_view_ptr = 0;
		ray_length = 2;
		connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_rgbd::on_device_change);

		srs.radius = 0.005f;
		show_nmls = false;
		state[0] = state[1] = state[2] = state[3] = IS_NONE;
		rgbd_started = false;
		record_frame = false;
		record_all_frames = false;

		connect(cgv::gui::get_animation_trigger().shoot, this, &vr_rgbd::timer_event);
	}
	size_t construct_point_cloud()
	{

		P.clear();
		C.clear();
		const unsigned short* depths = reinterpret_cast<const unsigned short*>(&depth_frame_2.frame_data.front());
		const unsigned char* colors = reinterpret_cast<const unsigned char*>(&color_frame_2.frame_data.front());
	
		rgbd_inp.map_color_to_depth(depth_frame_2, color_frame_2, warped_color_frame_2);
		colors = reinterpret_cast<const unsigned char*>(&warped_color_frame_2.frame_data.front());

		int i = 0;
		float s = 1.0f / 255;
		for (int y = 0; y < depth_frame_2.height; ++y)
			for (int x = 0; x < depth_frame_2.width; ++x) {
				vec3 p;
				if (rgbd_inp.map_depth_to_point(x, y, depths[i], &p[0])) {
					// flipping y to make it the same direction as in pixel y coordinate
					p[1] = -p[1];
					P.push_back(p);
					C.push_back(rgba8(colors[4 * i + 2], colors[4 * i + 1], colors[4 * i], 255));
				}
				++i;
			}
		return P.size();
	}
	void timer_event(double t, double dt)
	{
		// in case a point cloud is being constructed
		if (future_handle.valid()) {
			// check for termination of thread
			if (future_handle.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				size_t N = future_handle.get();
				// copy computed point cloud
				if (!pc.has_components()) {
					pc.create_components();
					pc.add_component();
				}
				if (record_frame || record_all_frames) {
					pc.add_component();
					record_frame = false;
					update_member(&record_frame);
				}
				else {
					pc.clear_component(pc.get_nr_components() - 1);
				}
				for (size_t i = 0; i < P.size(); ++i) {
					size_t pi = pc.add_point(P[i]);
					pc.clr(i) = point_cloud_types::Clr(
						float_to_color_component(C[i].R()),
						float_to_color_component(C[i].G()),
						float_to_color_component(C[i].B())
					);
				}
				show_point_end = pc.get_nr_points();
				update_member(&show_point_end);
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
						color_frame_2 = color_frame;
						depth_frame_2 = depth_frame;
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
	void configure_subsample_controls()
	{
		if (find_control(show_point_begin)) {
			find_control(show_point_begin)->set("max", show_point_end);
			find_control(show_point_end)->set("min", show_point_begin);
			find_control(show_point_end)->set("max", pc.get_nr_points());
		}
	}

	void create_gui()
	{
		add_decorator("vr_rgbd", "heading", "level=2");
		add_member_control(this, "rgbd_started", rgbd_started, "check");
		add_member_control(this, "record_frame", record_frame, "check");
		add_member_control(this, "record_all_frames", record_all_frames, "check");

		add_member_control(this, "ray_length", ray_length, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		if (begin_tree_node("point cloud", pc)) {
			align("\a");
			bool show = begin_tree_node("points", show_points, false, "level=3;w=100;align=' '");
			add_member_control(this, "show", show_points, "toggle", "w=50");
			if (show) {
				align("\a");
				if (begin_tree_node("subsample", show_point_step, false, "level=3")) {
					align("\a");
					add_member_control(this, "show step", show_point_step, "value_slider", "min=1;max=20;log=true;ticks=true");
					add_decorator("range control", "heading", "level=3");
					add_member_control(this, "begin", show_point_begin, "value_slider", "min=0;max=10;ticks=true");
					add_member_control(this, "end", show_point_end, "value_slider", "min=0;max=10;ticks=true");
					add_decorator("window control", "heading", "level=3");
					configure_subsample_controls();
					align("\b");
					end_tree_node(show_point_step);
				}
				align("\b");
				add_member_control(this, "sort_points", sort_points, "check");
				add_gui("surfel_style", surfel_style);
				end_tree_node(show_points);
			}
			show = begin_tree_node("components", pc.components, false, "level=3;w=100;align=' '");
			add_member_control(this, "show", surfel_style.use_group_color, "toggle", "w=50");
			if (show) {
				align("\a");
				if (begin_tree_node("component colors", pc.component_colors, false)) {
					align("\a");
					for (unsigned i = 0; i < pc.component_colors.size(); ++i) {
						add_member_control(this, std::string("C") + cgv::utils::to_string(i), pc.component_colors[i]);
					}
					align("\b");
					end_tree_node(pc.component_colors);
				}
				if (begin_tree_node("group transformations", pc.component_translations, false)) {
					align("\a");
					for (unsigned i = 0; i < pc.component_translations.size(); ++i) {
						add_gui(std::string("T") + cgv::utils::to_string(i), pc.component_translations[i]);
						add_gui(std::string("Q") + cgv::utils::to_string(i), pc.component_rotations[i], "direction");
					}
					align("\b");
					end_tree_node(pc.component_translations);
				}
				align("\b");
				end_tree_node(pc.components);
			}
			show = begin_tree_node("box", show_box, false, "level=3;w=100;align=' '");
			add_member_control(this, "show", show_box, "toggle", "w=50");
			if (show) {
				add_member_control(this, "show", show_boxes, "toggle", "w=50");
				add_gui("color", box_color);
				add_gui("box_style", box_style);
				add_gui("box_wire_style", box_wire_style);
				end_tree_node(show_box);
			}
			align("\b");
			end_tree_node(pc);
		}
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
	void on_set(void* member_ptr)
	{
		if (member_ptr == &rgbd_started && rgbd_started != rgbd_inp.is_started()) {
			if (rgbd_started)
				start_rgbd();
			else
				stop_rgbd();
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
		case cgv::gui::EID_POSE:
			cgv::gui::vr_pose_event& vrpe = static_cast<cgv::gui::vr_pose_event&>(e);
			// check for controller pose events
			int ci = vrpe.get_trackable_index();
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
						movable_box_translations[bi] = 
							rotation * (movable_box_translations[bi] - last_pos) + pos;
						// update orientation with rotation, note that quaternions
						// need to be multiplied in oposite order. In case of matrices
						// one would write box_orientation_matrix *= rotation
						movable_box_rotations[bi] = quat(rotation) * movable_box_rotations[bi];
						// update intersection points
						intersection_points[i] = rotation * (intersection_points[i] - last_pos) + pos;
					}
				}
				else {// not grab
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
					else
						if (state[ci] == IS_NONE)
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
		if (!gl_point_cloud_drawable::init(ctx))
			return false;

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
				vr_view_ptr->set_event_type_flags(
					cgv::gui::VREventTypeFlags(
						cgv::gui::VRE_KEY +
						cgv::gui::VRE_THROTTLE +
						cgv::gui::VRE_STICK +
						cgv::gui::VRE_STICK_KEY +
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
		cgv::render::ref_box_renderer(ctx, 1);
		cgv::render::ref_sphere_renderer(ctx, 1);
		return true;
	}
	void clear(cgv::render::context& ctx)
	{
		gl_point_cloud_drawable::clear(ctx);
		cgv::render::ref_box_renderer(ctx, -1);
		cgv::render::ref_sphere_renderer(ctx, -1);
	}
	void draw(cgv::render::context& ctx)
	{
		gl_point_cloud_drawable::draw(ctx);
		
		if (vr_view_ptr) {
			std::vector<vec3> P;
			std::vector<rgb> C;
			const vr::vr_kit_state* state_ptr = vr_view_ptr->get_current_vr_state();
			if (state_ptr) {
				for (int ci = 0; ci < 4; ++ci) if (state_ptr->controller[ci].status == vr::VRS_TRACKED) {
					vec3 ray_origin, ray_direction;
					state_ptr->controller[ci].put_ray(&ray_origin(0), &ray_direction(0));
					P.push_back(ray_origin);
					P.push_back(ray_origin + ray_length * ray_direction);
					rgb c(float(1 - ci), 0.5f*(int)state[ci], float(ci));
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
	}
};

/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
void vr_rgbd::construct_table(float tw, float td, float th, float tW)
{
	// construct table
	rgb table_clr(0.3f, 0.2f, 0.0f);
	boxes.push_back(box3(
		vec3(-0.5f*tw - 2*tW, th, -0.5f*td - 2*tW), 
		vec3( 0.5f*tw + 2*tW, th + tW, 0.5f*td + 2*tW)));
	box_colors.push_back(table_clr);

	boxes.push_back(box3(vec3(-0.5f*tw, 0, -0.5f*td), vec3(-0.5f*tw - tW, th, -0.5f*td - tW)));
	boxes.push_back(box3(vec3(-0.5f*tw, 0, 0.5f*td), vec3(-0.5f*tw - tW, th, 0.5f*td + tW)));
	boxes.push_back(box3(vec3(0.5f*tw, 0, -0.5f*td), vec3(0.5f*tw + tW, th, -0.5f*td - tW)));
	boxes.push_back(box3(vec3(0.5f*tw, 0, 0.5f*td), vec3(0.5f*tw + tW, th, 0.5f*td + tW)));
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
	box_colors.push_back(table_clr);
}
/// construct boxes that represent a room of dimensions w,d,h and wall width W
void vr_rgbd::construct_room(float w, float d, float h, float W, bool walls, bool ceiling)
{
	// construct floor
	boxes.push_back(box3(vec3(-0.5f*w, -W, -0.5f*d), vec3(0.5f*w, 0, 0.5f*d)));
	box_colors.push_back(rgb(0.2f, 0.2f, 0.2f));

	if (walls) {
		// construct walls
		boxes.push_back(box3(vec3(-0.5f*w, -W, -0.5f*d - W), vec3(0.5f*w, h, -0.5f*d)));
		box_colors.push_back(rgb(0.8f, 0.5f, 0.5f));
		boxes.push_back(box3(vec3(-0.5f*w, -W, 0.5f*d), vec3(0.5f*w, h, 0.5f*d + W)));
		box_colors.push_back(rgb(0.8f, 0.5f, 0.5f));

		boxes.push_back(box3(vec3(0.5f*w, -W, -0.5f*d - W), vec3(0.5f*w + W, h, 0.5f*d + W)));
		box_colors.push_back(rgb(0.5f, 0.8f, 0.5f));
	}
	if (ceiling) {
		// construct ceiling
		boxes.push_back(box3(vec3(-0.5f*w - W, h, -0.5f*d - W), vec3(0.5f*w + W, h + W, 0.5f*d + W)));
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
		float x = i * s - 0.5f*ew;
		for (unsigned j = 0; j < m; ++j) {
			float z = j * s - 0.5f*ed;
			if ( (x + s > -0.5f*w && x < 0.5f*w) && (z + s > -0.5f*d && z < 0.5f*d) )
				continue;
			float h = 0.2f*(std::max(abs(x)-0.5f*w,0.0f)+std::max(abs(z)-0.5f*d,0.0f))*distribution(generator)+0.1f;
			boxes.push_back(box3(vec3(x, 0, z), vec3(x+s, h, z+s)));
			box_colors.push_back(
				rgb(0.3f*distribution(generator)+0.3f, 
					0.3f*distribution(generator)+0.2f, 
					0.2f*distribution(generator)+0.1f));
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
		extent *= std::min(tw, td)*0.2f;

		vec3 center(-0.5f*tw + x * tw, th + tW, -0.5f*td + y * td);
		movable_boxes.push_back(box3(-0.5f*extent, 0.5f*extent));
		movable_box_colors.push_back(rgb(distribution(generator), distribution(generator), distribution(generator)));
		movable_box_translations.push_back(center);
		quat rot(signed_distribution(generator), signed_distribution(generator), signed_distribution(generator), signed_distribution(generator));
		rot.normalize();
		movable_box_rotations.push_back(rot);
	}
}

#include <cgv/base/register.h>

cgv::base::object_registration<vr_rgbd> vr_rgbd_reg("");

///@}