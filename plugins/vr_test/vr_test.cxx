#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/box_renderer.h>
#include <cg_gamepad/gamepad_server.h>

///@ingroup VR
///@{

/**@file
   example plugin for vr usage
*/

// these are the vr specific headers
#include <vr/vr_driver.h>
#include <cg_vr/vr_server.h>
#include <vr_view_interactor.h>

/// the plugin class vr_test inherits like other plugins from node, drawable and provider
class vr_test :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
protected:
	// store the scene as colored boxes
	std::vector<box3> boxes;
	std::vector<rgb> box_colors;

	// rendering style and renderer
	cgv::render::box_render_style style;

	// keep deadzone and precision vector for left controller
	cgv::gui::vr_server::vec_flt_flt left_deadzone_and_precision;
	// store handle to vr kit of which left deadzone and precision is configured
	void* last_kit_handle;

	// length of to be rendered rays
	float ray_length;

	// keep reference to vr_view_interactor
	vr_view_interactor* vr_view_ptr;

	/// register on device change events
	void on_device_change(void* kit_handle, bool attach)
	{
		if (attach) {
			if (last_kit_handle == 0) {
				vr::vr_kit* kit_ptr = vr::get_vr_kit(kit_handle);
				if (kit_ptr) {
					last_kit_handle = kit_handle;
					left_deadzone_and_precision = kit_ptr->get_controller_throttles_and_sticks_deadzone_and_precision(0);
					cgv::gui::ref_vr_server().provide_controller_throttles_and_sticks_deadzone_and_precision(kit_handle, 0, &left_deadzone_and_precision);
					post_recreate_gui();
				}
			}
		}
		else {
			if (kit_handle == last_kit_handle) {
				last_kit_handle = 0;
				post_recreate_gui();
			}
		}
	}
	/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
	void construct_table(float tw, float td, float th, float tW);
	/// construct boxes that represent a room of dimensions w,d,h and wall width W
	void construct_room(float w, float d, float h, float W, bool walls, bool ceiling);
	/// construct boxes for environment
	void construct_environment(float s, float ew, float ed, float eh, float w, float d, float h);
	/// construct a scene with a table
	void build_scene(float w, float d, float h, float W,
		float tw, float td, float th, float tW)
	{
		construct_room(w, d, h, W, false, false);
		construct_table(tw, td, th, tW);
		construct_environment(0.2f, 3 * w, 3 * d, h, w, d, h);
	}
public:
	vr_test()
	{
		set_name("vr_test");
		build_scene(5, 7, 3, 0.2f, 1.6f, 0.8f, 0.9f, 0.03f);
		vr_view_ptr = 0;
		ray_length = 2;
		last_kit_handle = 0;
		connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_test::on_device_change);
		cgv::gui::connect_gamepad_server();
	}
	std::string get_type_name() const
	{
		return "vr_test";
	}
	void create_gui()
	{
		add_decorator("vr_test", "heading", "level=2");
		add_member_control(this, "ray_length", ray_length, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		if (last_kit_handle) {
			vr::vr_kit* kit_ptr = vr::get_vr_kit(last_kit_handle);
			const std::vector<std::pair<int, int> >* t_and_s_ptr = 0;
			if (kit_ptr)
				t_and_s_ptr = &kit_ptr->get_controller_throttles_and_sticks(0);
			add_decorator("deadzone and precisions", "heading", "level=3");
			int ti = 0;
			int si = 0;
			for (unsigned i = 0; i < left_deadzone_and_precision.size(); ++i) {
				std::string prefix = std::string("unknown[") + cgv::utils::to_string(i) + "]";
				if (t_and_s_ptr) {
					if (t_and_s_ptr->at(i).second == -1)
						prefix = std::string("throttle[") + cgv::utils::to_string(ti++) + "]";
					else
						prefix = std::string("stick[") + cgv::utils::to_string(si++) + "]";
				}
				add_member_control(this, prefix + ".deadzone", left_deadzone_and_precision[i].first, "value_slider", "min=0;max=1;ticks=true;log=true");
				add_member_control(this, prefix + ".precision", left_deadzone_and_precision[i].second, "value_slider", "min=0;max=1;ticks=true;log=true");
			}
		}
		add_gui("boxes", style);
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	void stream_help(std::ostream& os)
	{
		os << "vr_test: no shortcuts defined" << std::endl;
	}
	bool handle(cgv::gui::event& e)
	{
		// check if vr event flag is not set and don't process events in this case
		if ((e.get_flags() & cgv::gui::EF_VR) == 0)
			return false;
		// check event id
		switch (e.get_kind()) {
		case cgv::gui::EID_KEY:
		{
			cgv::gui::vr_key_event& vrke = static_cast<cgv::gui::vr_key_event&>(e);
			if (vrke.get_action() != cgv::gui::KA_RELEASE) {
				switch (vrke.get_key()) {
				case vr::VR_LEFT_BUTTON0:
					std::cout << "button 0 of left controller pressed" << std::endl;
					return true;
				case vr::VR_RIGHT_STICK_RIGHT:
					std::cout << "touch pad of right controller pressed at right direction" << std::endl;
					return true;
				}
			}
			break;
		}
		case cgv::gui::EID_THROTTLE:
		{
			cgv::gui::vr_throttle_event& vrte = static_cast<cgv::gui::vr_throttle_event&>(e);
			std::cout << "throttle " << vrte.get_throttle_index() << " of controller " << vrte.get_controller_index()
				<< " adjusted from " << vrte.get_last_value() << " to " << vrte.get_value() << std::endl;
			return true;
		}
		case cgv::gui::EID_STICK:
		{
			cgv::gui::vr_stick_event& vrse = static_cast<cgv::gui::vr_stick_event&>(e);
			switch (vrse.get_action()) {
			case cgv::gui::SA_TOUCH:
			case cgv::gui::SA_PRESS:
			case cgv::gui::SA_UNPRESS:
			case cgv::gui::SA_RELEASE:
				std::cout << "stick " << vrse.get_stick_index()
					<< " of controller " << vrse.get_controller_index()
					<< " " << cgv::gui::get_stick_action_string(vrse.get_action())
					<< " at " << vrse.get_x() << ", " << vrse.get_y() << std::endl;
				return true;
			case cgv::gui::SA_MOVE:
			case cgv::gui::SA_DRAG:
				std::cout << "stick " << vrse.get_stick_index()
					<< " of controller " << vrse.get_controller_index()
					<< " " << cgv::gui::get_stick_action_string(vrse.get_action())
					<< " from " << vrse.get_last_x() << ", " << vrse.get_last_y()
					<< " to " << vrse.get_x() << ", " << vrse.get_y() << std::endl;
				return true;
			}
			return true;
		}
		case cgv::gui::EID_POSE:
			break;
		}
		return false;
	}
	bool init(cgv::render::context& ctx)
	{
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

		// ensure that the box renderer is initialized
		return true;
	}
	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_box_renderer(ctx, -1);
	}
	void draw(cgv::render::context& ctx)
	{
		if (vr_view_ptr) {
			std::vector<vec3> P;
			std::vector<rgb> C;
			const vr::vr_kit_state* state_ptr = vr_view_ptr->get_current_vr_state();
			if (state_ptr) {
				for (int i = 0; i < 2; ++i) {
					vec3 ray_origin, ray_direction;
					state_ptr->controller[i].put_ray(&ray_origin(0), &ray_direction(0));
					P.push_back(ray_origin);
					P.push_back(ray_origin + ray_length * ray_direction);
					rgb c(float(1 - i), 0, float(i));
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
				glDrawArrays(GL_LINES, 0, P.size());
				prog.disable(ctx);
				cgv::render::attribute_array_binding::disable_global_array(ctx, pi);
				cgv::render::attribute_array_binding::disable_global_array(ctx, ci);
				glLineWidth(1);
			}
		}
		// just draw boxes here
		cgv::render::box_renderer& renderer = cgv::render::ref_box_renderer(ctx);
		renderer.set_render_style(style);
		renderer.set_box_array(ctx, boxes);
		renderer.set_color_array(ctx, box_colors);
		if (renderer.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, boxes.size());
		}
		renderer.disable(ctx);
	}
};

/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
void vr_test::construct_table(float tw, float td, float th, float tW)
{
	// construct table
	boxes.push_back(box3(vec3(-0.5f*tw - tW, th, -0.5f*td - tW), vec3(0.5f*tw + tW, th + tW, 0.5f*td + tW)));
	box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));

	boxes.push_back(box3(vec3(-0.5f*tw, 0, -0.5f*td), vec3(-0.5f*tw + tW, th, -0.5f*td + tW)));
	boxes.push_back(box3(vec3(-0.5f*tw, 0, 0.5f*td), vec3(-0.5f*tw + tW, th, 0.5f*td + tW)));
	boxes.push_back(box3(vec3(0.5f*tw, 0, -0.5f*td), vec3(0.5f*tw + tW, th, -0.5f*td + tW)));
	boxes.push_back(box3(vec3(0.5f*tw, 0, 0.5f*td), vec3(0.5f*tw + tW, th, 0.5f*td + tW)));
	box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));
	box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));
	box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));
	box_colors.push_back(rgb(0.5f, 0.4f, 0.0f));
}
/// construct boxes that represent a room of dimensions w,d,h and wall width W
void vr_test::construct_room(float w, float d, float h, float W, bool walls, bool ceiling)
{
	// construct floor
	boxes.push_back(box3(vec3(-0.5f*w, -W, -0.5f*d), vec3(0.5f*w, 0, 0.5f*d)));
	box_colors.push_back(rgb(0.5f, 0.5f, 0.5f));

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
void vr_test::construct_environment(float s, float ew, float ed, float eh, float w, float d, float h)
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
			box_colors.push_back(rgb(0.2f*distribution(generator)+0.1f, 0.4f*distribution(generator)+0.2f, 0.2f*distribution(generator)+0.1f));
		}
	}
}

#include <cgv/base/register.h>

cgv::base::object_registration<vr_test> vr_test_reg("");

///@}