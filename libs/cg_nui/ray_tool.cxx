#include <cgv/base/base.h>
#include "ray_tool.h"
#include <cgv/math/pose.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {

ray_tool::ray_tool(const std::string& _name, int32_t _controller_index)
	: controller_tool(_name, _controller_index)
{
	ray_length = 3.0f;
	pick_all = false;
}

void ray_tool::check_for_contacts(const cgv::gui::vr_pose_event& vrpe)
{
	// compute intersections
	vec3 origin, direction;
	vrpe.get_state().controller[vrpe.get_trackable_index()].put_ray(&origin(0), &direction(0));
	if (pick_all)
		scene_node->compute_all_intersections(contact, origin, direction);
	else
		scene_node->compute_first_intersection(contact, origin, direction);
}

void ray_tool::on_set(void* member_ptr)
{

}

int ray_tool::mri_ref_count = 0;
cgv::render::mesh_render_info ray_tool::mri;

bool ray_tool::init(cgv::render::context& ctx)
{
	if (!mri.is_constructed()) {
		cgv::media::mesh::simple_mesh<> mesh;
		if (mesh.read("D:/data/mesh/vive_controller_1_5/vr_controller_vive_1_5.obj")) {
			mri.construct(ctx, mesh);
			mri.bind(ctx, ctx.ref_surface_shader_program(true), true);
		}
	}
	++mri_ref_count;
	return controller_tool::init(ctx);
}
void ray_tool::clear(cgv::render::context& ctx)
{
	controller_tool::clear(ctx);
	if (--mri_ref_count == 0)
		mri.destruct(ctx);
}
void ray_tool::draw(cgv::render::context& ctx)
{
	if (mri.is_constructed()) {
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(cgv::math::pose4<float>(controller_pose));
		mri.draw_all(ctx);
		ctx.pop_modelview_matrix();
	}
	
	// draw ray
	std::vector<vec3> P;
	std::vector<float> R;
	std::vector<rgb> C;
	vec3 ray_origin = reinterpret_cast<const vec3&>(controller_pose(0,3));
	vec3 ray_direction = -reinterpret_cast<const vec3&>(controller_pose(0, 2));

	P.push_back(ray_origin);
	R.push_back(0.002f);
	P.push_back(ray_origin + ray_length * ray_direction);
	R.push_back(0.003f);
	rgb c(float(1 - controller_index), 0.5f * (int)interaction_state, float(controller_index));
	C.push_back(c);
	C.push_back(c);
	auto& cr = cgv::render::ref_cone_renderer(ctx);
	cr.set_render_style(rcrs);
	cr.set_position_array(ctx, P);
	cr.set_color_array(ctx, C);
	cr.set_radius_array(ctx, R);
	cr.render(ctx, 0, P.size());

	controller_tool::draw(ctx);
}

void ray_tool::stream_help(std::ostream& os)
{
	controller_tool::stream_help(os);
}

bool ray_tool::handle(cgv::gui::event& e)
{
	if ((e.get_flags() & cgv::gui::EF_VR) == 0)
		return false;

	if (controller_tool::handle(e))
		return true;
	return false;

/*	// check event id
	switch (e.get_kind()) {
	case cgv::gui::EID_KEY:
	{
		cgv::gui::vr_key_event& vrke = static_cast<cgv::gui::vr_key_event&>(e);
		if (vrke.get_controller_index() != controller_index)
			return false;
		if (vrke.get_action() != cgv::gui::KA_RELEASE) {
			switch (vrke.get_key()) {
			case vr::VR_LEFT_STICK_DOWN:
			case vr::VR_RIGHT_STICK_DOWN:
				if (interaction_state == IS_GRAB) {
					const vec3& z = reinterpret_cast<const vec3&>(controller_pose(0, 2));
					for (auto& c : contact.contacts) {
						if (!c.container->translatable())
							continue;
						float delta = std::min(0.1f, c.distance);
						c.position += delta * z;
						c.container->set_position(c.primitive_index,
							c.container->get_position(c.primitive_index)
							+ delta * z);
					}
					return true;
				}
				break;
			case vr::VR_LEFT_STICK_UP:
			case vr::VR_RIGHT_STICK_UP:
				if (interaction_state == IS_GRAB) {
					const vec3& z = reinterpret_cast<const vec3&>(controller_pose(0, 2));
					for (auto& c : contact.contacts) {
						if (!c.container->translatable())
							continue;
						float delta = -0.1f;
						c.position += delta * z;
						c.container->set_position(c.primitive_index,
							c.container->get_position(c.primitive_index)
							+ delta * z);
					}
					return true;
				}
				break;
			case vr::VR_LEFT_BUTTON0:
			case vr::VR_RIGHT_BUTTON0:
				break;
			}
		}
		break;
	}
	case cgv::gui::EID_THROTTLE:
	{
		cgv::gui::vr_throttle_event& vrte = static_cast<cgv::gui::vr_throttle_event&>(e);
		if (vrte.get_controller_index() != controller_index)
			return false;
		break;
	}
	case cgv::gui::EID_STICK:
	{
		cgv::gui::vr_stick_event& vrse = static_cast<cgv::gui::vr_stick_event&>(e);
		if (vrse.get_controller_index() != controller_index)
			return false;
		switch (vrse.get_action()) {
		case cgv::gui::SA_TOUCH:
			start_grab(reinterpret_cast<const mat34&>(vrse.get_state().controller[vrse.get_controller_index()].pose[0]));
			break;
		case cgv::gui::SA_RELEASE:
			end_grab();
			break;
		case cgv::gui::SA_PRESS:
		case cgv::gui::SA_UNPRESS:
			break;
		case cgv::gui::SA_MOVE:
		case cgv::gui::SA_DRAG:
			break;
		}
		break;
	}
	case cgv::gui::EID_POSE:
		cgv::gui::vr_pose_event& vrpe = static_cast<cgv::gui::vr_pose_event&>(e);
		if (vrpe.get_trackable_index() != controller_index)
			return false;
		controller_pose = reinterpret_cast<const mat34&>(vrpe.get_state().controller[controller_index].pose[0]);
		move(vrpe);
		post_redraw();
		return true;
	}
	return false;
	*/
}

void ray_tool::create_gui()
{
	add_member_control(this, "pick_all", pick_all, "check");
	controller_tool::create_gui();
}

	}
}