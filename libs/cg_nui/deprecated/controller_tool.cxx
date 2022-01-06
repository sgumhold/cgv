#include <cgv/base/base.h>
#include "controller_tool.h"
#include "primitive_container.h"
#include <cgv/math/pose.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace nui {

controller_tool::controller_tool(const std::string& _name, int32_t _controller_index)
	: nui_tool(_name, TT_VR_CONTROLLER), controller_index(_controller_index)
{
	grab_start_time = 0;
	grab_click_time = 0.3f;
	interaction_state = IS_NONE;
	controller_pose.identity();
	srs.radius = 0.005f;
	wbrs.line_width = 2.0f;
	show_over_boxes = true;
	show_parent_boxes = true;
}

bool controller_tool::init(cgv::render::context& ctx)
{
	cgv::render::ref_cone_renderer(ctx, 1);
	cgv::render::ref_box_wire_renderer(ctx, 1);
	cgv::render::ref_sphere_renderer(ctx, 1);
	return true;
}
void controller_tool::clear(cgv::render::context& ctx)
{
	cgv::render::ref_cone_renderer(ctx, -1);
	cgv::render::ref_box_wire_renderer(ctx, -1);
	cgv::render::ref_sphere_renderer(ctx, -1);
}
void controller_tool::draw(cgv::render::context& ctx)
{
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
		if (!show_over_boxes)
			return;
		auto& wbr = cgv::render::ref_box_wire_renderer(ctx);
		wbr.set_render_style(wbrs);
		std::vector<box3> boxes;
		std::vector<rgb> box_colors;
		std::vector<quat> box_rotations;
		for (uint32_t i = 0; i < contact.contacts.size(); ++i) {
			const auto& c = contact.contacts[i];
			boxes.push_back(c.get_oriented_bounding_box());
			box_colors.push_back(controller_index == 0 ? rgb(1, 0.5f, 0.5f) : rgb(0.5f, 0.5f, 1));
			box_rotations.push_back(c.get_orientation());
			if (i+1 == contact.contacts.size() || c.node != contact.contacts[i+1].node) {
				nui_node* P = c.get_parent();
				if (P && show_parent_boxes) {
					boxes.push_back(P->compute_bounding_box());
					box_colors.push_back(controller_index == 0 ? rgb(1, 0.75f, 0.75f) : rgb(0.75f, 0.75f, 1));
					box_rotations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
				}
				nui_node* N = P ? P : c.node;
				ctx.push_modelview_matrix();
				ctx.mul_modelview_matrix(N->get_node_to_world_transformation());
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

void controller_tool::stream_help(std::ostream& os)
{

}

void controller_tool::start_grab(const mat34& controller_pose)
{
	if (interaction_state == IS_OVER) {
		// filter contacts for movable
		uint32_t i = contact.contacts.size();
		while (i > 0) {
			--i;
			if (!contact.contacts[i].translatable() &&
				!contact.contacts[i].rotatable())
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
					c.get_position(),
					c.get_orientation(),
					c.position, c.normal));
			}
			// store controller pose at start of grab
			dmat3 O(cgv::math::pose_orientation(controller_pose));
			grab_orientation = dquat(O);
			grab_position = cgv::math::pose_position(controller_pose);
		}
	}

}

void controller_tool::move(const cgv::gui::vr_pose_event& vrpe)
{
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
		dquat q = qo * ql;
		q.normalize();

		dmat4 T_tool_rot;
		q.put_homogeneous_matrix(T_tool_rot);
		dmat4 T_tool = cgv::math::translate4<double>(pos) * T_tool_rot * cgv::math::translate4<double>(-last_pos);
		//std::cout << "T_tool =\n" << T_tool << std::endl;
		// iterate intersection points of current controller
		for (auto& c : contact.contacts) {
			uint32_t i = &c - &contact.contacts[0];

			auto* P = c.get_parent();
			dmat4 T_node, T_node_inv;
			if (P) {
				T_node = P->get_node_to_world_transformation();
				T_node_inv = P->get_world_to_node_transformation();
			}
			else {
				T_node.identity();
				T_node_inv.identity();
			}
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

			mat3 R;
			R.set_col(0, reinterpret_cast<const vec3&>(T_prim_new.col(0)));
			R.set_col(1, reinterpret_cast<const vec3&>(T_prim_new.col(1)));
			R.set_col(2, reinterpret_cast<const vec3&>(T_prim_new.col(2)));
			quat q1(R);
			q1.normalize();
			dvec3 r = reinterpret_cast<const vec3&>(T_prim_new.col(3));

			box3 old_box = c.get_bounding_box();
			c.update_orientation(q1);
			c.update_position(r);
			box3 new_box = c.get_bounding_box();
			c.check_box_update(old_box, new_box);
			// update primitive position with position change and rotation				

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

		check_for_contacts(vrpe);
		
		// remove all intersections that are not approachable
		uint32_t i = contact.contacts.size();
		while (i > 0) {
			--i;
			if (!contact.contacts[i].approachable())
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
}

void controller_tool::end_grab()
{
	if (interaction_state == IS_GRAB) {
		interaction_state = IS_OVER;
		grab_start_infos.clear();
	}
}


bool controller_tool::handle(cgv::gui::event& e)
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
		switch (vrke.get_key()) {
		case vr::VR_LEFT_STICK_DOWN:
		case vr::VR_RIGHT_STICK_DOWN:
			if (vrke.get_action() != cgv::gui::KA_RELEASE) {
				if (interaction_state == IS_GRAB)
					end_grab();
				return true;
			}
			break;
		case vr::VR_LEFT_BUTTON0:
		case vr::VR_RIGHT_BUTTON0:
			if (vrke.get_action() != cgv::gui::KA_RELEASE) {
				start_grab(reinterpret_cast<const mat34&>(vrke.get_state().controller[vrke.get_controller_index()].pose[0]));
				grab_start_time = vrke.get_time();
			}
			else {
				if (interaction_state == IS_GRAB && vrke.get_time() - grab_start_time > grab_click_time)
					end_grab();
			}
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
}

void controller_tool::create_gui()
{
	add_member_control(this, "grab_click_time", grab_click_time, "value_slider", "min=0.01;max=1;ticks=true;log=true");
	add_member_control(this, "show_over_boxes", show_over_boxes, "check");
	add_member_control(this, "show_parent_boxes", show_parent_boxes, "check");
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