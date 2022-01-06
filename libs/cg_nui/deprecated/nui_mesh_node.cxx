#include <cgv/base/base.h>
#include <cgv/math/ftransform.h>
#include "nui_mesh_node.h"
#include "ray_axis_aligned_box_intersection.h"

namespace cgv {
	namespace nui {
		nui_mesh_node::nui_mesh_node(const std::string& _name, ScalingMode _scaling_mode)
			: nui_node(_name, _scaling_mode)
		{
			new_mesh_file_name = false;
			show_surface = true;
			show_wireframe = false;
		}
		nui_mesh_node::~nui_mesh_node()
		{
		}
		void nui_mesh_node::set_file_name(const std::string& _file_name)
		{
			mesh_file_name = _file_name;
			new_mesh_file_name = true;
		}

		uint32_t nui_mesh_node::get_nr_primitives() const
		{
			return 1;
		}

		nui_mesh_node::box3 nui_mesh_node::get_bounding_box(uint32_t i) const
		{
			return mesh_box;
		}

		bool nui_mesh_node::compute_closest_point(contact_info& info, const vec3& pos)
		{
			// TODO: implement
			return nui_node::compute_closest_point(info, pos);
		}
		bool nui_mesh_node::compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight)
		{
			// TODO: implement
			return nui_node::compute_closest_oriented_point(info, pos, normal, orientation_weight);
		}
		bool nui_mesh_node::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
		{
			// TODO: implement
			return nui_node::compute_first_intersection(info, start, direction);
		}
		int nui_mesh_node::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
		{
			// TODO: implement
			return nui_node::compute_all_intersections(info, start, direction, only_entry_points);
		}

		bool nui_mesh_node::init(cgv::render::context& ctx)
		{
			return true;
		}
		void nui_mesh_node::init_frame(cgv::render::context& ctx)
		{
			if (new_mesh_file_name) {
				if (M.read(mesh_file_name)) {
					mesh_box = M.compute_box();
					box_outofdate = true;
					MI.destruct(ctx);
					MI.construct(ctx, M);
					MI.bind(ctx, ctx.ref_surface_shader_program(true), true);
					MI.bind_wireframe(ctx, cgv::render::ref_cone_renderer(ctx).ref_prog(), true);
					cone_style.radius = float(0.1f*mesh_box.get_extent().length() / sqrt(M.get_nr_positions()));
				}
				new_mesh_file_name = false;
			}
		}
		void nui_mesh_node::clear(cgv::render::context& ctx)
		{
			MI.destruct(ctx);
		}
		void nui_mesh_node::draw(cgv::render::context& ctx)
		{
			nui_node::draw(ctx);
			if (show_surface)
				MI.draw_all(ctx);
			if (show_wireframe) {
				auto& cr = ref_cone_renderer(ctx);
				cr.set_render_style(cone_style);
				if (cr.enable(ctx)) {
					MI.draw_wireframe(ctx);
					cr.disable(ctx);
				}
			}
		}
		void nui_mesh_node::create_gui()
		{
			nui_node::create_gui();
			add_member_control(this, "show_surface", show_surface, "check");
			add_member_control(this, "show_wireframe", show_wireframe, "check");
			if (begin_tree_node("cone style", cone_style, false, "level=3")) {
				align("\a");
				add_gui("cone_style", cone_style);
				align("\b");
				end_tree_node(cone_style);
			}
		}

	}
}