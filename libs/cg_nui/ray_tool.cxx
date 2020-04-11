#include <cgv/base/base.h>
#include "ray_tool.h"
#include <cg_vr/vr_events.h>
#include <cgv/math/pose.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {

ray_tool::ray_tool(const std::string& _name, int32_t _controller_index)
	: nui_tool(_name, TT_VR_CONTROLLER), controller_index(_controller_index)
{
	interaction_state = IS_NONE;
	controller_pose.identity();
	ray_length = 3.0f;
	srs.radius = 0.005f;
	wbrs.line_width = 2.0f;
	pick_all = false;
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
	cgv::render::ref_rounded_cone_renderer(ctx, 1);
	cgv::render::ref_box_wire_renderer(ctx, 1);
	cgv::render::ref_sphere_renderer(ctx, 1);
	return true;
}
void ray_tool::clear(cgv::render::context& ctx)
{
	if (--mri_ref_count == 0)
		mri.destruct(ctx);

	cgv::render::ref_rounded_cone_renderer(ctx, -1);
	cgv::render::ref_box_wire_renderer(ctx, -1);
	cgv::render::ref_sphere_renderer(ctx, -1);
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
	auto& cr = cgv::render::ref_rounded_cone_renderer(ctx);
	cr.set_render_style(rcrs);
	cr.set_position_array(ctx, P);
	cr.set_color_array(ctx, C);
	cr.set_radius_array(ctx, R);
	cr.render(ctx, 0, P.size());

	if (!contact.contacts.empty()) {
		// draw contact points
		auto& sr = cgv::render::ref_sphere_renderer(ctx);
		sr.set_position_array(ctx, 
			&contact.contacts.front().position, 
			contact.contacts.size(), sizeof(contact_info::contact));
		sr.set_render_style(srs);
		if (sr.validate_and_enable(ctx)) {
			ctx.set_color(controller_index == 0 ? rgb(1, 0, 0) : rgb(0, 0, 1));
			sr.draw(ctx, 0, contact.contacts.size());
			sr.disable(ctx);
		}

		// draw wired boxes around contact primitives
		auto& wbr = cgv::render::ref_box_wire_renderer(ctx);
		wbr.set_render_style(wbrs);
		std::vector<box3> boxes;
		std::vector<rgb> box_colors;
		std::vector<quat> box_rotations;
		for (uint32_t i = 0; i < contact.contacts.size(); ++i) {
			const auto& c = contact.contacts[i];
			boxes.push_back(c.container->get_oriented_bounding_box(c.primitive_index));
			box_colors.push_back(controller_index == 0 ? rgb(1, 0.5f, 0.5f) : rgb(0.5f, 0.5f, 1));
			box_rotations.push_back(c.container->get_orientation(c.primitive_index));
			if (i+1 == contact.contacts.size() || c.container->get_parent() != contact.contacts[i+1].container->get_parent()) {
				nui_node* N = c.container->get_parent();
				boxes.push_back(N->compute_bounding_box());
				box_colors.push_back(controller_index == 0 ? rgb(1, 0.75f, 0.75f) : rgb(0.75f, 0.75f, 1));
				box_rotations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
				ctx.push_modelview_matrix();
				ctx.mul_modelview_matrix(c.container->get_parent()->get_node_to_world_transformation());
				wbr.set_box_array(ctx, boxes);
				wbr.set_color_array(ctx, box_colors);
				wbr.set_rotation_array(ctx, box_rotations);
				wbr.render(ctx, 0, boxes.size());
				ctx.pop_modelview_matrix();
				boxes.clear();
				box_colors.clear();
				box_rotations.clear();
			}
		}
	}
}

void ray_tool::stream_help(std::ostream& os)
{

}

bool ray_tool::handle(cgv::gui::event& e)
{
	if ((e.get_flags() & cgv::gui::EF_VR) == 0)
		return false;
	// check event id
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
			if (interaction_state == IS_OVER) {
				// filter contacts for movable
				uint32_t i = contact.contacts.size();
				while (i > 0) {
					--i;
					if (!contact.contacts[i].container->translatable() &&
						!contact.contacts[i].container->rotatable())
						contact.contacts.erase(contact.contacts.begin() + i);
				}
				// if contact points remain
				if (contact.contacts.empty()) {
					interaction_state = IS_NONE;
					grab_start_infos.clear();
				}
				else {
					// store primitive transformations at start of grab
					interaction_state = IS_GRAB;
					for (const auto& c : contact.contacts) {
						grab_start_infos.push_back(grab_start_info(
							c.container->get_position(c.primitive_index),
							c.container->get_orientation(c.primitive_index),
							c.position, c.normal));
					}
					// append controller pose at start of grab
					const mat34& controller_pose = reinterpret_cast<const mat34&>(vrse.get_state().controller[vrse.get_controller_index()].pose[0]);
					dmat3 O(cgv::math::pose_orientation(controller_pose));
					grab_orientation = dquat(O);
					grab_position = cgv::math::pose_position(controller_pose);
				}
			}
			break;
		case cgv::gui::SA_RELEASE:
			if (interaction_state == IS_GRAB) {
				interaction_state = IS_OVER;
				grab_start_infos.clear();
			}
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
		if (interaction_state == IS_GRAB) {
			// in grab mode apply relative transformation to grabbed boxes

			// get previous and current controller position
			dvec3 last_pos = grab_position; // vrpe.get_last_position();
			dvec3 pos = vrpe.get_position();
			// get rotation from previous to current orientation
			//dmat3 last_orientation = vrpe.get_last_orientation();
			dmat3 orientation = vrpe.get_orientation();
			// for numerical reasons compute rotation as quaternion 
			dquat qo(orientation);
			dquat ql(grab_orientation); //(last_orientation);
			ql.conjugate();
			dquat q = qo*ql;
			q.normalize();

			dmat4 T_tool_rot;
			q.put_homogeneous_matrix(T_tool_rot);
			dmat4 T_tool = cgv::math::translate4<double>(pos) * T_tool_rot * cgv::math::translate4<double>(-last_pos);
			//std::cout << "T_tool =\n" << T_tool << std::endl;
			// iterate intersection points of current controller
			for (auto& c : contact.contacts) {
				uint32_t i = &c - &contact.contacts[0];
			
				auto* N = c.container->get_parent();
				dmat4 T_node = N->get_node_to_world_transformation();
				dmat4 T_node_inv = N->get_world_to_node_transformation();
				dmat4 T_tool_local = T_node_inv * T_tool * T_node;
				//std::cout << "T_node =\n" << T_node << std::endl;
				//std::cout << "T_node_inv =\n" << T_node << std::endl;
				//std::cout << "T_tool_local =\n" << T_tool_local << std::endl;

				mat4 T_prim;
				grab_start_infos[i].orientation.put_homogeneous_matrix(T_prim);
				T_prim.set_col(3, vec4(grab_start_infos[i].position, 1.0f));

				//std::cout << "orient = " << grab_start_infos[i].orientation << std::endl;
				//std::cout << "transl = " << grab_start_infos[i].position << std::endl;
				//std::cout << "T_prim =\n" << T_prim << std::endl;

				mat4 T_prim_new = T_tool_local * T_prim;
				//std::cout << "T_prim_new =\n" << T_prim_new << std::endl;

				if (c.container->rotatable()) {
					mat3 R;
					R.set_col(0, reinterpret_cast<const vec3&>(T_prim_new.col(0)));
					R.set_col(1, reinterpret_cast<const vec3&>(T_prim_new.col(1)));
					R.set_col(2, reinterpret_cast<const vec3&>(T_prim_new.col(2)));
					//std::cout << "R =\n" << R << std::endl;

					quat q1(R);
					//std::cout << "q1 = " << q1 << std::endl;
					//
					//if (true) {
					//	dquat q1 = grab_start_infos[i].orientation; // c.container->get_orientation(c.primitive_index);
					//	q1 = q * q1;
					//	q1.normalize();
					//	std::cout << "q1_validate = " << q1 << std::endl;
					//}
					//dmat3 R;
					//R.set_col(0, reinterpret_cast<const dvec3&>(T_tool_local* dvec4(R1.col(0), 0.0)));
					//R.set_col(1, reinterpret_cast<const dvec3&>(T_tool_local* dvec4(R1.col(1), 0.0)));
					//R.set_col(2, reinterpret_cast<const dvec3&>(T_tool_local* dvec4(R1.col(2), 0.0)));
					//dquat q(R);
					//q.normalize();
					//c.container->set_orientation(c.primitive_index, q);
					
					q1.normalize();
					c.container->set_orientation(c.primitive_index, q1);
				}
				// update primitive position with position change and rotation				

				dvec3 r = reinterpret_cast<const vec3&>(T_prim_new.col(3));
				
				/*
				std::cout << "r = " << r << std::endl;

				if (true) {
					dvec3 r = grab_start_infos[i].position; // c.container->get_position(c.primitive_index);
					r -= last_pos;
					q.rotate(r);
					r += pos;
					std::cout << "r_validate = " << r << std::endl;
				}
				*/
				c.container->set_position(c.primitive_index, r);

				// update contact position
				r = grab_start_infos[i].contact_point; // c.position;
				r -= last_pos;
				q.rotate(r);
				r += pos;
				c.position = r;
				
				// update contact normal
				r = grab_start_infos[i].contact_normal; // c.normal;
				q.rotate(r);
				c.normal = r;
			}
		}
		else {// not grab
			// clear intersections of current controller
			contact.contacts.clear();

			// compute intersections
			vec3 origin, direction;
			vrpe.get_state().controller[vrpe.get_trackable_index()].put_ray(&origin(0), &direction(0));
			if (pick_all)
				scene_node->compute_all_intersections(contact, origin, direction);
			else
				scene_node->compute_first_intersection(contact, origin, direction);

			// remove all intersections that are not approachable
			uint32_t i = contact.contacts.size();
			while (i > 0) {
				--i;
				if (!contact.contacts[i].container->approachable())
					contact.contacts.erase(contact.contacts.begin() + i);
			}
			// update state based on whether we have found at least 
			// one intersection with controller ray
			if (contact.contacts.empty())
				interaction_state = IS_NONE;
			else {
				if (interaction_state == IS_NONE)
					interaction_state = IS_OVER;
				//for (const auto& c : contact.contacts) {
				//	std::cout << c.container->get_primitive_type() << "[" << c.primitive_index << "]  " << c.texcoord << std::endl;
				//}
			}
		}
		post_redraw();
		return true;
	}
	return false;
}

void ray_tool::create_gui()
{
	add_member_control(this, "pick_all", pick_all, "check");
	if (begin_tree_node("cone style", rcrs)) {
		align("\a");
		add_gui("style", rcrs);
		align("\b");
		end_tree_node(rcrs);
	}
	if (begin_tree_node("sphere style", srs)) {
		align("\a");
		add_gui("style", srs);
		align("\b");
		end_tree_node(srs);
	}
	if (begin_tree_node("wired box style", wbrs)) {
		align("\a");
		add_gui("style", wbrs);
		align("\b");
		end_tree_node(srs);
	}
}

	}
}