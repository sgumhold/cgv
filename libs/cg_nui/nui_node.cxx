#include <cgv/base/base.h>
#include <cgv/math/ftransform.h>
#include "nui_node.h"

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

		nui_node::nui_node(const std::string& _name, bool use_scale) 
			: cgv::base::group(_name), apply_scale(use_scale)
		{
			translation = vec3(0.0f);
			rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
			scale = vec3(1.0f);
			spheres = 0;
		}
		nui_node::~nui_node()
		{
			for (auto& pcp : primitive_containers)
				delete pcp;
			spheres = 0;
		}
		nui_node::mat4 nui_node::get_model_matrix() const
		{
			mat4 O;
			rotation.put_homogeneous_matrix(O);
			if (apply_scale)
				return cgv::math::translate4<float>(translation) * O * cgv::math::scale4<float>(scale);
			else
				return cgv::math::translate4<float>(translation) * O;
		}

		void nui_node::create_sphere_container(bool use_radii, bool _use_colors, SphereRenderType _render_type)
		{
			spheres = new sphere_container(use_radii, _use_colors, _render_type);
			primitive_containers.push_back(spheres);
		}
		void nui_node::create_box_container(bool _use_colors, bool _use_orientations, BoxRenderType _render_type)
		{
			boxes = new box_container(_use_colors, _use_orientations, _render_type);
			primitive_containers.push_back(boxes);
		}
		void nui_node::create_rectangle_container(bool _use_colors, bool _use_orientations, bool _use_texcoords)
		{
			rectangles = new rectangle_container(_use_colors, _use_orientations, _use_texcoords);
			primitive_containers.push_back(rectangles);
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
		nui_node::box3 nui_node::compute_bounding_box() const
		{
			box3 B;
			for (auto pcp : primitive_containers)
				B.add_axis_aligned_box(pcp->compute_bounding_box());
			return B;
		}
		void nui_node::compute_closest_point(contact_info& info, const vec3& pos)
		{
			for (auto pcp : primitive_containers)
				pcp->compute_closest_point(info, pos);
		}
		void nui_node::compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight)
		{
			for (auto pcp : primitive_containers)
				pcp->compute_closest_oriented_point(info, pos, normal, orientation_weight);
		}
		void nui_node::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
		{
			for (auto pcp : primitive_containers)
				pcp->compute_first_intersection(info, start, direction);
		}
		void nui_node::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
		{
			for (auto pcp : primitive_containers)
				pcp->compute_all_intersections(info, start, direction, only_entry_points);
		}

		bool nui_node::init(cgv::render::context& ctx)
		{
			bool res_all = true;
			for (auto pc : primitive_containers) {
				pc->set_context(&ctx);
				bool res = pc->init(ctx);
				res_all &= res;
			}
			return res_all;
		}
		void nui_node::init_frame(cgv::render::context& ctx)
		{
			for (auto pc : primitive_containers)
				pc->init_frame(ctx);
		}
		void nui_node::clear(cgv::render::context& ctx)
		{
			for (auto pc : primitive_containers)
				pc->clear(ctx);
		}
		void nui_node::draw(cgv::render::context& ctx)
		{
			for (auto pc : primitive_containers)
				pc->draw(ctx);
		}

	}
}