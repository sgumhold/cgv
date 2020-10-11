#include "box_container.h"
#include "ray_axis_aligned_box_intersection.h"

namespace cgv {
	namespace nui {

		box_container::box_container(nui_node* _parent, bool _use_colors, bool _use_orientations, BoxRenderType _render_type)
			: primitive_container(_parent, PT_BOX, _use_colors, _use_orientations, SM_NON_UNIFORM), render_type(_render_type)
		{
		}

		std::string box_container::get_primitive_type() const
		{
			return "box";
		}

		uint32_t box_container::add_box(const box3& box)
		{
			center_positions.push_back(box.get_center());
			scales.push_back(box.get_extent());
			if (use_colors)
				colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
			if (use_orientations)
				orientations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
			return uint32_t(center_positions.size() - 1);
		}

		uint32_t box_container::add_box(const box3& box, const quat& orientation)
		{
			center_positions.push_back(box.get_center());
			scales.push_back(box.get_extent());
			if (use_colors)
				colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
			if (use_orientations)
				orientations.push_back(orientation);
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t box_container::add_box(const box3& box, const rgba& color)
		{
			center_positions.push_back(box.get_center());
			scales.push_back(box.get_extent());
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t box_container::add_box(const box3& box, const quat& orientation, const rgba& color)
		{
			center_positions.push_back(box.get_center());
			scales.push_back(box.get_extent());
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(orientation);
			return uint32_t(center_positions.size() - 1);
		}
		box_container::box3 box_container::get_oriented_bounding_box(uint32_t i) const
		{
			const vec3& c = center_positions[i];
			vec3 e = scales[i];
			return box3(c - 0.5f * e, c + 0.5f * e);
		}
		bool box_container::compute_closest_point(contact_info& info, const vec3& pos)
		{
			bool result = false;
			contact_info::contact C;
			C.container = this;
			C.node = get_parent();
			for (const auto& c : center_positions) {
				C.primitive_index = uint32_t(&c - &center_positions.front());
				compute_closest_box_point(C, pos, c, scales[C.primitive_index], use_orientations ? &orientations[C.primitive_index] : 0);
				if (info.consider_closest_contact(C))
					result = true;
			}
			return result;
		}
		bool box_container::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
		{
			bool result = false;
			contact_info::contact C;
			C.container = this;
			C.node = get_parent();
			for (const auto& c : center_positions) {
				C.primitive_index = uint32_t(&c - &center_positions.front());
				const vec3& e = scales[C.primitive_index];
				if ((use_orientations ? 
					 compute_box_intersection(c, e, get_rotation(C.primitive_index), start, direction, C) :
					 compute_box_intersection(c, e, start, direction, C)) > 0) {
					if (info.consider_closest_contact(C))
						result = true;
				}
			}
			return result;
		}
		int box_container::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
		{
			int result = 0;
			contact_info::contact C1, C2;
			C1.container = C2.container = this;
			C1.node = C2.node = get_parent();
			for (const auto& c : center_positions) {
				C1.primitive_index = C2.primitive_index = uint32_t(&c - &center_positions.front());
				const vec3& e = scales[C1.primitive_index];
				contact_info::contact ci1, ci2;
				int nr_intersections = use_orientations ?
					compute_box_intersection(c, e, get_rotation(C1.primitive_index), start, direction, C1, &C2) :
					compute_box_intersection(c, e, start, direction, C1, &C2);
				if (nr_intersections == 0)
					continue;
				++result;
				info.contacts.push_back(C1);
				if (nr_intersections == 1)
					continue;
				++result;
				info.contacts.push_back(C2);
			}
			return result;
		}

		bool box_container::init(cgv::render::context& ctx)
		{
			cgv::render::ref_box_renderer(ctx, 1);
			cgv::render::ref_box_wire_renderer(ctx, 1);
			return true;
		}

		void box_container::clear(cgv::render::context& ctx)
		{
			cgv::render::ref_box_renderer(ctx, -1);
			cgv::render::ref_box_wire_renderer(ctx, -1);
		}

		void box_container::prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr) const
		{
			primitive_container::prepare_render(ctx, r, rs, indices_ptr);
			if (&rs == &brs) {
				auto& br = reinterpret_cast<cgv::render::box_renderer&>(r);
				br.set_extent_array(ctx, scales);
				if (use_orientations)
					br.set_rotation_array(ctx, orientations);
			}
			else {
				auto& bwr = reinterpret_cast<cgv::render::box_wire_renderer&>(r);
				bwr.set_extent_array(ctx, scales);
				if (use_orientations)
					bwr.set_rotation_array(ctx, orientations);
			}
		}

		void box_container::draw(cgv::render::context& ctx)
		{
			switch (render_type) {
			case BRT_SOLID: 
				render(ctx, cgv::render::ref_box_renderer(ctx), brs); 
				break;
			case BRT_WIREFRAME: 
				render(ctx, cgv::render::ref_box_wire_renderer(ctx), bwrs); 
				break;
			case BRT_SOLID_AND_WIREFRAME: 
				render(ctx, cgv::render::ref_box_wire_renderer(ctx), bwrs);
				render(ctx, cgv::render::ref_box_renderer(ctx), brs); 
				break;
			}
		}
		const cgv::render::render_style* box_container::get_render_style() const 
		{
			if (render_type == BRT_SOLID)
				return &brs;
			else
				return &bwrs;
		}
	}
}