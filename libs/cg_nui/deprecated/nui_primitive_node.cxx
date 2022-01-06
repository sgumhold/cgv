#include <cgv/base/base.h>
#include <cgv/math/ftransform.h>
#include "nui_primitive_node.h"
#include "ray_axis_aligned_box_intersection.h"
#include <random>

namespace cgv {
	namespace nui {

		nui_primitive_node::nui_primitive_node(const std::string& _name, ScalingMode _scaling_mode)
			: nui_node(_name, _scaling_mode)
		{
			spheres = 0;
			boxes = 0;
			rectangles = 0;
		}
		nui_primitive_node::~nui_primitive_node()
		{
			for (auto& pcp : primitive_containers)
				delete pcp;
			spheres = 0;
			boxes = 0;
			rectangles = 0;
		}
		void nui_primitive_node::create_sphere_container(bool use_radii, bool _use_colors, SphereRenderType _render_type)
		{
			spheres = new sphere_container(this, use_radii, _use_colors, _render_type);
			primitive_containers.push_back(spheres);
		}
		void nui_primitive_node::create_box_container(bool _use_colors, bool _use_orientations, BoxRenderType _render_type)
		{
			boxes = new box_container(this, _use_colors, _use_orientations, _render_type);
			primitive_containers.push_back(boxes);
		}
		void nui_primitive_node::create_rectangle_container(bool _use_colors, bool _use_orientations, bool _use_texcoords)
		{
			rectangles = new rectangle_container(this, _use_colors, _use_orientations, _use_texcoords);
			primitive_containers.push_back(rectangles);
		}


		/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
		void nui_primitive_node::construct_table(float tw, float td, float th, float tW) {
			// construct table
			rgba table_clr(0.3f, 0.2f, 0.0f, 1.0f);
			ref_boxes()->add_box(
				box3(vec3(-0.5f * tw - 2 * tW, th - tW, -0.5f * td - 2 * tW),
					vec3(0.5f * tw + 2 * tW, th, 0.5f * td + 2 * tW)),
				table_clr);

			ref_boxes()->add_box(
				box3(vec3(-0.5f * tw, 0, -0.5f * td), vec3(-0.5f * tw - tW, th - tW, -0.5f * td - tW)), table_clr);
			ref_boxes()->add_box(
				box3(vec3(-0.5f * tw, 0, 0.5f * td), vec3(-0.5f * tw - tW, th - tW, 0.5f * td + tW)), table_clr);
			ref_boxes()->add_box(
				box3(vec3(0.5f * tw, 0, -0.5f * td), vec3(0.5f * tw + tW, th - tW, -0.5f * td - tW)), table_clr);
			ref_boxes()->add_box(
				box3(vec3(0.5f * tw, 0, 0.5f * td), vec3(0.5f * tw + tW, th - tW, 0.5f * td + tW)), table_clr);
		}

		/// construct boxes that represent a room of dimensions w,d,h and wall width W
		void nui_primitive_node::construct_room(float w, float d, float h, float W, bool walls, bool ceiling) {
			// construct floor
			ref_boxes()->add_box(
				box3(vec3(-0.5f * w, -W, -0.5f * d), vec3(0.5f * w, 0, 0.5f * d)),
				rgba(0.2f, 0.2f, 0.2f, 1.0f));

			if (walls) {
				// construct walls
				ref_boxes()->add_box(
					box3(vec3(-0.5f * w, -W, -0.5f * d - W), vec3(0.5f * w, h, -0.5f * d)),
					rgba(0.8f, 0.5f, 0.5f, 1.0f));
				ref_boxes()->add_box(
					box3(vec3(-0.5f * w, -W, 0.5f * d), vec3(0.5f * w, h, 0.5f * d + W)),
					rgba(0.8f, 0.5f, 0.5f, 1.0f));

				ref_boxes()->add_box(
					box3(vec3(0.5f * w, -W, -0.5f * d - W), vec3(0.5f * w + W, h, 0.5f * d + W)),
					rgba(0.5f, 0.8f, 0.5f, 1.0f));
			}
			if (ceiling) {
				// construct ceiling
				ref_boxes()->add_box(
					box3(vec3(-0.5f * w - W, h, -0.5f * d - W), vec3(0.5f * w + W, h + W, 0.5f * d + W)),
					rgba(0.5f, 0.5f, 0.8f, 1.0f));
			}
		}

		/// construct boxes for environment
		void nui_primitive_node::construct_environment(float s, float ew, float ed, float w, float d, float h) 
		{
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
					rgba color = cgv::media::color<float, cgv::media::HLS, cgv::media::OPACITY>(distribution(generator), 0.1f * distribution(generator) + 0.15f, 0.3f, 1.0f);
					ref_boxes()->add_box(box3(vec3(x, 0.0f, z), vec3(x + s, h, z + s)), color);
				}
			}
		}

		/// construct a scene with a table
		void nui_primitive_node::construct_lab(float w, float d, float h, float W, float tw, float td, float th, float tW)
		{
			construct_room(w, d, h, W, false, false);
			construct_table(tw, td, th, tW);
			construct_environment(0.3f, 3 * w, 3 * d, w, d, h);
		}

		uint32_t nui_primitive_node::get_nr_primitives() const
		{
			return primitive_containers.size() + get_nr_children();
		}
		nui_primitive_node::box3 nui_primitive_node::get_bounding_box(uint32_t i) const
		{
			if (i < get_nr_children())
				return nui_node::get_bounding_box(i);
			else
				return primitive_containers[i - get_nr_children()]->compute_bounding_box();
		}

		bool nui_primitive_node::compute_closest_point(contact_info& info, const vec3& pos)
		{
			if (!intersectable())
				return false;
			bool result = false;
			mat4 M = get_model_matrix();
			mat4 inv_M = get_inverse_model_matrix();
			vec3 pos_local = reinterpret_cast<const vec3&>(inv_M * vec4(pos, 1.0f));
			for (auto pcp : primitive_containers) {
				if (!pcp->intersectable())
					continue;
				if (pcp->compute_closest_point(info, pos_local)) {
					correct_contact_info(info.contacts.back(), M, inv_M, pos);
					result = true;
				}
			}
			for (uint32_t ci = 0; ci < get_nr_children(); ++ci) {
				if (get_child(ci)->cast<nui_node>()->compute_closest_point(info, pos_local)) {
					correct_contact_info(info.contacts.back(), M, inv_M, pos);
					result = true;
				}
			}
			return result;
		}
		bool nui_primitive_node::compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight)
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
			for (auto pcp : primitive_containers) {
				if (!pcp->intersectable())
					continue;
				if (pcp->compute_closest_oriented_point(info, pos_local, normal_local, orientation_weight)) {
					correct_contact_info(info.contacts.back(), M, inv_M, pos);
					result = true;
				}
			}
			for (uint32_t ci = 0; ci < get_nr_children(); ++ci) {
				if (get_child(ci)->cast<nui_node>()->compute_closest_oriented_point(info, pos_local, normal_local, orientation_weight)) {
					correct_contact_info(info.contacts.back(), M, inv_M, pos);
					result = true;
				}
			}
			return result;
		}
		bool nui_primitive_node::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
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
			if (ray_axis_aligned_box_intersection(start_local, direction_local, box, t, p, n, 0.000001f)) {
				for (auto pcp : primitive_containers) {
					if (!pcp->intersectable())
						continue;
					if (pcp->compute_first_intersection(info, start_local, direction_local)) {
						correct_contact_info(info.contacts.front(), M, inv_M, start);
						result = true;
					}
				}
				for (uint32_t ci = 0; ci < get_nr_children(); ++ci) {
					if (get_child(ci)->cast<nui_node>()->compute_first_intersection(info, start_local, direction_local)) {
						correct_contact_info(info.contacts.front(), M, inv_M, start);
						result = true;
					}
				}
			}
			return result;
		}
		int nui_primitive_node::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
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
			if (ray_axis_aligned_box_intersection(start_local, direction_local, box, t, p, n, 0.000001f)) {
				for (auto pcp : primitive_containers) {
					if (!pcp->intersectable())
						continue;
					size_t count = info.contacts.size();
					result += pcp->compute_all_intersections(info, start_local, direction_local, only_entry_points);
					for (uint32_t i = count; i < info.contacts.size(); ++i)
						correct_contact_info(info.contacts[i], M, inv_M, start);
				}
				for (uint32_t ci = 0; ci < get_nr_children(); ++ci) {
					nui_node_ptr C = get_child(ci)->cast<nui_node>();
					size_t count = info.contacts.size();
					result += C->compute_all_intersections(info, start_local, direction_local, only_entry_points);
					for (uint32_t i = count; i < info.contacts.size(); ++i)
						correct_contact_info(info.contacts[i], M, inv_M, start);
				}
			}
			return result;
		}

		bool nui_primitive_node::init(cgv::render::context& ctx)
		{
			bool res_all = true;
			for (auto pc : primitive_containers) {
				pc->set_context(&ctx);
				bool res = pc->init(ctx);
				res_all &= res;
			}
			return res_all;
		}
		void nui_primitive_node::init_frame(cgv::render::context& ctx)
		{
			for (auto pc : primitive_containers)
				pc->init_frame(ctx);
		}
		void nui_primitive_node::clear(cgv::render::context& ctx)
		{
			for (auto pc : primitive_containers)
				pc->clear(ctx);
		}
		void nui_primitive_node::draw(cgv::render::context& ctx)
		{
			nui_node::draw(ctx);
			for (auto pc : primitive_containers)
				pc->draw(ctx);
		}

	}
}