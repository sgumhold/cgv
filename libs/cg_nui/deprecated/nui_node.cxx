#include <cgv/base/base.h>
#include <cgv/math/ftransform.h>
#include "nui_node.h"
#include "ray_axis_aligned_box_intersection.h"

namespace cgv {
	namespace nui {
		void nui_node::stream_help(std::ostream& os)
		{

		}
		bool nui_node::handle(cgv::gui::event& e)
		{
			if ((e.get_flags() & cgv::gui::EF_NUI) != 0) {
				std::cout << "node received nui event: ";
				e.stream_out(std::cout);
				std::cout << std::endl;
			}
			return false;
		}

		nui_node::nui_node(const std::string& _name, ScalingMode _scaling_mode)
			: cgv::base::group(_name), nui_interactable(_scaling_mode)
		{
			translation = vec3(0.0f);
			rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
			scale = vec3(1.0f);
			interaction_capabilities = IC_ALL;
		}
		nui_node::~nui_node()
		{
		}
		nui_node::mat4 nui_node::get_model_matrix() const
		{
			mat4 O;
			rotation.put_homogeneous_matrix(O);
			switch (scaling_mode) {
			case SM_UNIFORM: return cgv::math::translate4<float>(translation) * O * cgv::math::scale4<float>(vec3(scale[0]));
			case SM_NON_UNIFORM: return cgv::math::translate4<float>(translation) * O * cgv::math::scale4<float>(scale);
			default: return cgv::math::translate4<float>(translation) * O;
			}
		}
		nui_node::mat4 nui_node::get_inverse_model_matrix() const
		{
			mat4 O;
			rotation.inverse().put_homogeneous_matrix(O);
			switch (scaling_mode) {
			case SM_UNIFORM: return cgv::math::scale4<float>(vec3(1.0f / scale[0])) * O * cgv::math::translate4<float>(-translation);
			case SM_NON_UNIFORM: return cgv::math::scale4<float>(scale) * O * cgv::math::translate4<float>(translation);
			default: return O * cgv::math::translate4<float>(-translation);
			}
		}
		nui_node::dmat4 nui_node::get_node_to_world_transformation() const
		{
			dmat4 T = get_model_matrix();
			nui_node_ptr P = get_parent()->cast<nui_node>();
			if (P.empty())
				return T;
			return P->get_node_to_world_transformation() * T;
		}
		nui_node::dmat4 nui_node::get_world_to_node_transformation() const
		{
			dmat4 T = get_inverse_model_matrix();
			nui_node_ptr P = get_parent()->cast<nui_node>();
			if (P.empty())
				return T;
			return T * P->get_world_to_node_transformation();
		}
		void nui_node::integrate_child_node(nui_node_ptr child_node_ptr, bool init_drawable)
		{
			append_child(child_node_ptr);
			cgv::render::context* ctx_ptr = get_context();
			if (init_drawable && ctx_ptr) {
				ctx_ptr->make_current();
				child_node_ptr->init(*ctx_ptr);
			}
		}
		void nui_node::desintegrate_child_node(nui_node_ptr child_node_ptr, bool clear_drawable)
		{
			remove_child(child_node_ptr);
			cgv::render::context* ctx_ptr = get_context();
			if (clear_drawable && ctx_ptr) {
				ctx_ptr->make_current();
				child_node_ptr->clear(*ctx_ptr);
			}
		}

		uint32_t nui_node::get_nr_primitives() const
		{
			return get_nr_children();
		}

		nui_node::box3 nui_node::get_bounding_box(uint32_t i) const
		{
			return get_child(i).up_cast<nui_node>()->compute_and_transform_bounding_box();
		}

		nui_node::box3 nui_node::compute_and_transform_bounding_box() const
		{
			box3 b = compute_bounding_box();
			mat4 M = get_model_matrix();
			box3 B;
			for (int i = 0; i < 8; ++i)
				B.add_point(reinterpret_cast<const vec3&>(M*vec4(b.get_corner(i),1.0f)));
			return B;
		}

		void nui_node::correct_contact_info(contact_info::contact& C, const mat4& M, const mat4& inv_M, const vec3& pos)
		{
			
			C.normal = reinterpret_cast<const vec3&>(vec4(C.normal, 0.0f) * inv_M);
			C.position = reinterpret_cast<const vec3&>(M * vec4(C.position, 1.0f));
			switch (scaling_mode) {
			case SM_NONE: break;
			case SM_UNIFORM:
				C.distance *= scale[0];
				C.normal.normalize();
				break;
			case SM_NON_UNIFORM:
				C.distance = (C.position - pos).length();
				C.normal.normalize();
				break;
			}
		}
		bool nui_node::compute_closest_point(contact_info& info, const vec3& pos)
		{
			if (!intersectable())
				return false;
			bool result = false;
			mat4 M = get_model_matrix();
			mat4 inv_M = get_inverse_model_matrix();
			vec3 pos_local = reinterpret_cast<const vec3&>(inv_M * vec4(pos, 1.0f));
			if (get_nr_children() > 0) {
				for (uint32_t ci = 0; ci < get_nr_children(); ++ci) {
					if (get_child(ci)->cast<nui_node>()->compute_closest_point(info, pos_local)) {
						correct_contact_info(info.contacts.back(), M, inv_M, pos);
						result = true;
					}
				}
			}
			else {
				const box3& B = compute_bounding_box();
				contact_info::contact C;
				C.container = 0;
				C.primitive_index = -1;
				C.node = this;
				compute_closest_box_point(C, B.get_center(), B.get_extent(), pos_local);
				if (info.consider_closest_contact(C)) {
					correct_contact_info(info.contacts.back(), M, inv_M, pos);
					result = true;
				}
			}
			return result;
		}		
		bool nui_node::compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight)
		{
			if (!intersectable())
				return false;
			bool result = false;
			mat4 M = get_model_matrix();
			mat4 inv_M = get_inverse_model_matrix();
			vec3 pos_local = reinterpret_cast<const vec3&>(inv_M * vec4(pos, 1.0f));
			vec3 normal_local = reinterpret_cast<const vec3&>(vec4(normal, 0.0f) * M);
			if (scaling_mode != SM_NONE)
				normal_local.normalize();
			if (get_nr_children() > 0) {
				for (uint32_t ci = 0; ci < get_nr_children(); ++ci) {
					if (get_child(ci)->cast<nui_node>()->compute_closest_oriented_point(info, pos_local, normal_local, orientation_weight)) {
						correct_contact_info(info.contacts.back(), M, inv_M, pos);
						result = true;
					}
				}
			}
			else {
				const box3& B = compute_bounding_box();
				contact_info::contact C;
				C.container = 0;
				C.primitive_index = -1;
				C.node = this;
				compute_closest_box_point(C, B.get_center(), B.get_extent(), pos_local);
				if (info.consider_closest_contact(C)) {
					correct_contact_info(info.contacts.back(), M, inv_M, pos);
					result = true;
				}
			}
			return result;
		}
		bool nui_node::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
		{
			if (!intersectable())
				return false;
			bool result = false;
			mat4 M = get_model_matrix();
			mat4 inv_M = get_inverse_model_matrix();
			vec3 start_local = reinterpret_cast<const vec3&>(inv_M * vec4(start, 1.0f));
			vec3 direction_local = reinterpret_cast<const vec3&>(inv_M * vec4(direction, 0.0f));
			// check for box intersection first
			const box3& box = compute_bounding_box();
			float t;
			vec3 p, n;
			if (get_nr_children() > 0) {
				if (ray_axis_aligned_box_intersection(start_local, direction_local, box, t, p, n, 0.000001f)) {
					for (uint32_t ci = 0; ci < get_nr_children(); ++ci) {
						nui_node_ptr C = get_child(ci)->cast<nui_node>();
						if (C->compute_first_intersection(info, start_local, direction_local)) {
							correct_contact_info(info.contacts.front(), M, inv_M, start);
							result = true;
						}
					}
				}
			}
			else {
				const box3& B = compute_bounding_box();
				contact_info::contact C;
				C.node = this;
				C.container = 0;
				C.primitive_index = -1;
				if (compute_box_intersection(B.get_center(), B.get_extent(), start_local, direction_local, C) > 0) {
					if (info.consider_closest_contact(C)) {
						correct_contact_info(info.contacts.back(), M, inv_M, start);
						result = true;
					}
				}
			}
			return result;
		}
		int  nui_node::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
		{
			if (!intersectable())
				return 0;
			int result = 0;
			mat4 M = get_model_matrix();
			mat4 inv_M = get_inverse_model_matrix();
			vec3 start_local = reinterpret_cast<const vec3&>(inv_M * vec4(start, 1.0f));
			vec3 direction_local = reinterpret_cast<const vec3&>(inv_M * vec4(direction, 0.0f));
			// check for box intersection first
			const box3& box = compute_bounding_box();
			float t;
			vec3 p, n;
			if (get_nr_children() > 0) {
				if (ray_axis_aligned_box_intersection(start_local, direction_local, box, t, p, n, 0.000001f)) {
					for (uint32_t ci = 0; ci < get_nr_children(); ++ci) {
						nui_node_ptr C = get_child(ci)->cast<nui_node>();
						size_t count = info.contacts.size();
						result += C->compute_all_intersections(info, start_local, direction_local, only_entry_points);
						for (uint32_t i = count; i < info.contacts.size(); ++i)
							correct_contact_info(info.contacts[i], M, inv_M, start);
					}
				}
			}
			else {
				const box3& B = compute_bounding_box();
				contact_info::contact C1, C2;
				int nr_intersections = compute_box_intersection(B.get_center(), B.get_extent(), start_local, direction_local, C1, &C2);
				result += nr_intersections;
				if (nr_intersections > 0) {
					correct_contact_info(C1, M, inv_M, start);
					info.contacts.push_back(C1);
				}
				if (nr_intersections > 1) {
					correct_contact_info(C2, M, inv_M, start);
					info.contacts.push_back(C2);
				}
			}
			return result;
		}

		void nui_node::draw(cgv::render::context& ctx)
		{
			ctx.push_modelview_matrix();
			ctx.mul_modelview_matrix(get_model_matrix());
		}
		void nui_node::finish_draw(cgv::render::context& ctx)
		{
			ctx.pop_modelview_matrix();
		}

		void nui_node::create_gui()
		{
			add_decorator(get_name(), "heading", "level=3");
			add_gui("interaction_capabilities", interaction_capabilities, "bit_field_control",
				"enums='approach=1,translate=2,rotate=4,uniform_scale=8,scale=16,throw=32'");
			add_member_control(this, "scaling_mode", scaling_mode, "dropdown", "enums='none,uniform,non uniform'");
			add_gui("scale", scale, "", "long_label=true;main_label='first';options='min=0.01;max=100;log=true;ticks=true'");
			add_gui("translation", translation, "", "long_label=true;main_label='first';options='min=-10;max=10;ticks=true'");
			add_gui("rotation", static_cast<vec4&>(rotation), "direction", "long_label=true;main_label='first';options='min=-1;max=1;ticks=true'");
		}
	}
}