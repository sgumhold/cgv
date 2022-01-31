#include "rectangle_container.h"

namespace cgv {
	namespace nui {

		rectangle_container::rectangle_container(nui_node* _parent, bool _use_colors, bool _use_orientations, bool _use_texcoords)
			: primitive_container(_parent, PT_RECTANGLE, _use_colors, _use_orientations, SM_NON_UNIFORM), use_texcoords(_use_texcoords)
		{
		}

		std::string rectangle_container::get_primitive_type() const
		{
			return "rectangle";
		}

		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 0.0f));
			if (use_colors)
				colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
			if (use_orientations)
				orientations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
			if (use_texcoords)
				texcoord_ranges.push_back(vec4(0.0f, 0.0f, 1.0f, 1.0f));
			return uint32_t(center_positions.size() - 1);
		}

		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent, const quat& orientation)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 0.0f));
			if (use_colors)
				colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
			if (use_orientations)
				orientations.push_back(orientation);
			if (use_texcoords)
				texcoord_ranges.push_back(vec4(0.0f, 0.0f, 1.0f, 1.0f));
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent, const rgba& color)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 0.0f));
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
			if (use_texcoords)
				texcoord_ranges.push_back(vec4(0.0f, 0.0f, 1.0f, 1.0f));
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent, const quat& orientation, const rgba& color)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 0.0f));
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(orientation);
			if (use_texcoords)
				texcoord_ranges.push_back(vec4(0.0f, 0.0f, 1.0f, 1.0f));
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent, const box2& texcoord_range)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 0.0f));
			if (use_colors)
				colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
			if (use_orientations)
				orientations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
			if (use_texcoords)
				texcoord_ranges.push_back(reinterpret_cast<const vec4&>(texcoord_range));
			return uint32_t(center_positions.size() - 1);
		}

		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent, const box2& texcoord_range, const quat& orientation)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 0.0f));
			if (use_colors)
				colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
			if (use_orientations)
				orientations.push_back(orientation);
			if (use_texcoords)
				texcoord_ranges.push_back(reinterpret_cast<const vec4&>(texcoord_range));
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent, const box2& texcoord_range, const rgba& color)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 0.0f));
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(quat(1.0f, 0.0f, 0.0f, 0.0f));
			if (use_texcoords)
				texcoord_ranges.push_back(reinterpret_cast<const vec4&>(texcoord_range));
			return uint32_t(center_positions.size() - 1);
		}
		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent, const box2& texcoord_range, const quat& orientation, const rgba& color)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 0.0f));
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(orientation);
			if (use_texcoords)
				texcoord_ranges.push_back(reinterpret_cast<const vec4&>(texcoord_range));
			return uint32_t(center_positions.size() - 1);
		}

		rectangle_container::box3 rectangle_container::get_oriented_bounding_box(uint32_t i) const
		{
			const vec3& c = center_positions[i];
			vec3 e = scales[i];
			return box3(c - 0.5f * e, c + 0.5f * e);
		}

		bool rectangle_container::compute_closest_point(contact_info& info, const vec3& pos)
		{
			bool result = false;
			contact_info::contact C;
			C.container = this;
			C.node = get_parent();
			for (const auto& c : center_positions) {
				C.primitive_index = uint32_t(&c - &center_positions.front());
				vec3 h = 0.5f * scales[C.primitive_index];
				vec3& p = C.position;
				p = pos-c;
				if (use_orientations)
					orientations[C.primitive_index].inverse_rotate(p);
				bool inside[3];
				float closest[3];
				float distances[3];
				int inside_count = 0;
				closest[2] = 0;
				distances[2] = fabs(p[2]);
				inside[2] = false;
				for (int j = 0; j < 2; ++j) {
					closest[j] = p[j] < 0.0f ? -1.0f : 1.0f;
					distances[j] = fabs(p[j] - closest[j] * h[j]);
					inside[j] = (p[j] >= -h[j]) && (p[j] <= h[j]);
					if (inside[j])
						++inside_count;
				}
				vec3& n = C.normal;
				n.zeros();
				float sqr_dist = 0;
				for (int j = 0; j < 3; ++j) {
					if (!inside[j]) {
						sqr_dist += distances[j] * distances[j];
						if (j < 2)
							n[j] = closest[j];
						else
							n[2] = p[2] > 0.0f ? 1.0f : -1.0f;
						p[j] = closest[j] * h[j];
					}
				}
				C.distance = sqrt(sqr_dist);
				n.normalize();
				vec3 tc((p[0] + h[0]) / scales[C.primitive_index][0], (p[1] + h[1]) / scales[C.primitive_index][1], 0.0f);
				if (use_orientations) {
					orientations[C.primitive_index].rotate(p);
					orientations[C.primitive_index].rotate(n);
				}
				p += c;
				if (info.consider_closest_contact(C))
					result = true;
			}
			return result;
		}
		bool rectangle_container::compute_intersection(const vec3& rectangle_center, const vec2& rectangle_extent,
			const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C)
		{
			float t_result;
			vec3 p_result = ray_start - rectangle_center;
			// ray starts on rectangle
			if (fabs(ray_start[2]) < 0.000001f)
				t_result = 0;
			else {
				t_result = -p_result[2] / ray_direction[2];
				// intersection is before ray start
				if (t_result < 0)
					return false;
				p_result += t_result * ray_direction;
			}
			// check if ray plane intersection is inside of rectangle extent
			if (fabs(p_result[0]) > 0.5f * rectangle_extent[0] ||
				fabs(p_result[1]) > 0.5f * rectangle_extent[1])
				return false;
			vec3 n_result(0.0f, 0.0f, 1.0f);
			C.distance = t_result;
			C.position = p_result + rectangle_center;
			C.normal = n_result;
			C.texcoord = vec3((vec2(2, &p_result[0]) + 0.5f * rectangle_extent) / rectangle_extent, 0.0f);
			return true;
		}
		bool rectangle_container::compute_intersection(
			const vec3& rectangle_center, const vec2& rectangle_extent, const quat& rectangle_rotation,
			const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C)
		{
			vec3 ro = ray_start - rectangle_center;
			rectangle_rotation.inverse_rotate(ro);
			ro += rectangle_center;
			vec3 rd = ray_direction;
			rectangle_rotation.inverse_rotate(rd);
			if (compute_intersection(rectangle_center, rectangle_extent, ro, rd, C)) {
				// transform result back
				C.position -= rectangle_center;
				rectangle_rotation.rotate(C.position);
				C.position += rectangle_center;
				rectangle_rotation.rotate(C.normal);
				return true;
			}
			return false;
		}

		bool rectangle_container::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
		{
			bool result = false;
			contact_info::contact C;
			C.container = this;
			C.node = get_parent();
			for (const auto& c : center_positions) {
				C.primitive_index = uint32_t(&c - &center_positions.front());
				const vec2& e = reinterpret_cast<const vec2&>(scales[C.primitive_index]);
				if ((use_orientations ?
					compute_intersection(c, e, get_rotation(C.primitive_index), start, direction, C) :
					compute_intersection(c, e, start, direction, C))) {
					if (info.consider_closest_contact(C))
						result = true;
				}
			}
			return result;
		}

		int rectangle_container::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
		{
			int result = 0;
			contact_info::contact C;
			C.container = this;
			C.node = get_parent();
			for (const auto& c : center_positions) {
				C.primitive_index = uint32_t(&c - &center_positions.front());
				const vec2& e = reinterpret_cast<const vec2&>(scales[C.primitive_index]);
				if (use_orientations ?
					compute_intersection(c, e, get_rotation(C.primitive_index), start, direction, C) :
					compute_intersection(c, e, start, direction, C)) {
					++result;
					info.contacts.push_back(C);
				}
			}
			return result;
		}

		bool rectangle_container::init(cgv::render::context& ctx)
		{
			cgv::render::ref_rectangle_renderer(ctx, 1);
			return true;
		}

		void rectangle_container::clear(cgv::render::context& ctx)
		{
			cgv::render::ref_rectangle_renderer(ctx, -1);
		}

		void rectangle_container::prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr) const
		{
			primitive_container::prepare_render(ctx, r, rs, indices_ptr);
			auto& rr = reinterpret_cast<cgv::render::rectangle_renderer&>(r);
			rr.set_extent_array(ctx, reinterpret_cast<const vec2*>(&scales.front()), scales.size(), sizeof(vec3));
			if (use_orientations)
				rr.set_rotation_array(ctx, orientations);
			if (use_texcoords)
				rr.set_texcoord_array(ctx, texcoord_ranges);
		}

		void rectangle_container::draw(cgv::render::context& ctx)
		{
			auto& r = cgv::render::ref_rectangle_renderer(ctx);
			prepare_render(ctx, r, srs);
			if (!r.validate_and_enable(ctx))
				return;
			if (use_texcoords && tex)
				tex->enable(ctx);
			r.draw(ctx, 0, center_positions.size());
			if (use_texcoords && tex)
				tex->disable(ctx);
			r.disable(ctx);
		}
	}
}