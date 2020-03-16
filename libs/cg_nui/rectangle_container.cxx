#include "rectangle_container.h"

namespace cgv {
	namespace nui {

		rectangle_container::rectangle_container(bool _use_colors, bool _use_orientations, bool _use_texcoords)
			: primitive_container(PT_RECTANGLE, _use_colors, _use_orientations, SM_NON_UNIFORM), use_texcoords(_use_texcoords)
		{
		}

		uint32_t rectangle_container::add_rectangle(const vec3& center, const vec2& extent)
		{
			center_positions.push_back(center);
			scales.push_back(vec3(extent, 1.0f));
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
			scales.push_back(vec3(extent, 1.0f));
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
			scales.push_back(vec3(extent, 1.0f));
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
			scales.push_back(vec3(extent, 1.0f));
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
			scales.push_back(vec3(extent, 1.0f));
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
			scales.push_back(vec3(extent, 1.0f));
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
			scales.push_back(vec3(extent, 1.0f));
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
			scales.push_back(vec3(extent, 1.0f));
			if (use_colors)
				colors.push_back(color);
			if (use_orientations)
				orientations.push_back(orientation);
			if (use_texcoords)
				texcoord_ranges.push_back(reinterpret_cast<const vec4&>(texcoord_range));
			return uint32_t(center_positions.size() - 1);
		}

		rectangle_container::box3 rectangle_container::compute_bounding_box() const
		{
			box3 B;
			for (const auto& c : center_positions) {
				vec3 e = scales[&c - &center_positions.front()];
				box3 b(c - 0.5f * e, c + 0.5f * e);
				if (use_orientations) {
					for (int i = 0; i < 4; ++i)
						B.add_point(b.get_corner(i));
				}
				else
					B.add_axis_aligned_box(b);
			}
			return B;
		}

		void rectangle_container::compute_closest_point(contact_info& info, const vec3& pos)
		{
			for (const auto& c : center_positions) {
				uint32_t i = uint32_t(&c - &center_positions.front());
				vec3 h = 0.5f * scales[i];
				vec3 p = pos;
				p -= c;
				if (use_orientations)
					orientations[i].inverse_rotate(p);
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
				vec3 n(0.0f);
				float distance;
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
				distance = sqrt(sqr_dist);
				n.normalize();
				if (use_orientations)
					orientations[i].rotate(n);
				p += c;
				consider_closest_point(i, info, distance, p, n);
			}
		}

		void rectangle_container::compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight)
		{
			std::cerr << "not implemented" << std::endl;
		}

		int rectangle_container::compute_intersection(
			const vec3& rectangle_center, const vec2& rectangle_extent,
			const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr)
		{
			float t_result;
			vec3 p_result = ray_start;
			// ray starts on rectangle
			if (fabs(ray_start[2]) < 0.000001f)
				t_result = 0;
			else {
				t_result = -ray_start[2] / ray_direction[2];
				// intersection is before ray start
				if (t_result < 0)
					return 0;
				p_result += t_result * ray_direction;
			}
			// check if ray plane intersection is inside of rectangle extent
			if (fabs(p_result[0]) > 0.5f * rectangle_extent[0] ||
				fabs(p_result[1]) > 0.5f * rectangle_extent[1])
				return 0;
			vec3 n_result(0.0f, 0.0f, 1.0f);
			C.distance = t_result;
			C.position = p_result;
			C.normal = n_result;
			return 1;
		}
		int rectangle_container::compute_intersection(
			const vec3& rectangle_center, const vec2& rectangle_extent, const quat& rectangle_rotation,
			const vec3& ray_start, const vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr)
		{
			vec3 ro = ray_start - rectangle_center;
			rectangle_rotation.inverse_rotate(ro);
			vec3 rd = ray_direction;
			rectangle_rotation.inverse_rotate(rd);
			int cnt = compute_intersection(rectangle_center, rectangle_extent, ro, rd, C, C2_ptr);
			// transform result back
			if (cnt > 0) {
				rectangle_rotation.rotate(C.position);
				C.position += rectangle_center;
				rectangle_rotation.rotate(C.normal);
			}
			if (cnt > 1) {
				rectangle_rotation.rotate(C2_ptr->position);
				C2_ptr->position += rectangle_center;
				rectangle_rotation.rotate(C2_ptr->normal);
			}
			return cnt;
		}

		void rectangle_container::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
		{
			for (const auto& c : center_positions) {
				uint32_t i = uint32_t(&c - &center_positions.front());
				const vec2& e = reinterpret_cast<const vec2&>(scales[i]);
				contact_info::contact ci;
				if ((use_orientations ?
					compute_intersection(c, e, get_rotation(i), start, direction, ci) :
					compute_intersection(c, e, start, direction, ci)) > 0) {
					ci.primitive_index = i;
					ci.container = this;
					if (info.contacts.empty())
						info.contacts.push_back(ci);
					else if (ci.distance < info.contacts.front().distance)
						info.contacts.front() = ci;
				}
			}
		}

		void rectangle_container::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
		{
			for (const auto& c : center_positions) {
				uint32_t i = uint32_t(&c - &center_positions.front());
				const vec2& e = reinterpret_cast<const vec2&>(scales[i]);
				contact_info::contact ci1, ci2;
				int cnt = use_orientations ?
					compute_intersection(c, e, get_rotation(i), start, direction, ci1, &ci2) :
					compute_intersection(c, e, start, direction, ci1, &ci2);
				if (cnt == 0)
					continue;
				ci1.primitive_index = i;
				ci1.container = this;
				info.contacts.push_back(ci1);
				if (cnt == 1)
					continue;
				ci2.primitive_index = i;
				ci2.container = this;
				info.contacts.push_back(ci1);
			}
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