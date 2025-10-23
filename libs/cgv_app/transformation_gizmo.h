#pragma once

#include <cgv/math/ray.h>
#include <cgv_gl/box_render_data.h>
#include <cgv_gl/cone_render_data.h>
#include <cgv_gl/rectangle_render_data.h>
#include <cgv_gl/sphere_render_data.h>

#include "gizmo.h"

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API transformation_gizmo : public gizmo {
public:
	enum class Mode {
		kTranslation,
		kRotation,
		kScale,
		kModel
	};
	
	transformation_gizmo();

	std::string get_type_name() const override { return "transformation_gizmo"; }

	bool init(cgv::render::context&) override;

	void clear(cgv::render::context&) override;

	Mode get_mode() const;
	
	void set_mode(Mode mode);

	vec3 get_scale() const;

	void set_scale(const vec3& scale);

	std::function<void(GizmoAction, Mode)> on_change;

private:
	void create_geometry() override;

	void draw_geometry(cgv::render::context& ctx) override;

	bool intersect_bounding_box(const cgv::math::ray3& ray) override;

	bool intersect(const cgv::math::ray3& ray) override;

	bool start_drag(const cgv::math::ray3& ray) override;

	bool drag(const cgv::math::ray3& ray) override;

	void end_drag(const cgv::math::ray3& ray) override;

	std::pair<float, float> ray_ray_closest_approach(const cgv::math::ray3& r0, const cgv::math::ray3& r1) const;

	const float _center_radius = 0.15f;
	const float _axis_radius = 0.01f;
	const float _handle_size = 0.1f;
	const float _plane_size = 0.2f;
	const size_t _ring_segment_count = 32;

	std::vector<vec2> _ring_points;

	vec3 _scale = { 1.0f };

	Mode _mode = Mode::kTranslation;

	Mode _interaction_mode = Mode::kTranslation;
	vec3 _drag_start_position = { 0.0f };
	quat _drag_start_rotation;
	vec3 _drag_start_scale = { 1.0f };

	cgv::render::box_renderer _box_renderer;
	cgv::render::cone_renderer _cone_renderer;
	cgv::render::rectangle_renderer _rectangle_renderer;
	cgv::render::sphere_renderer _sphere_renderer;

	cgv::render::box_render_data<rgba> _boxes;
	cgv::render::cone_render_data<rgba> _cones;
	cgv::render::rectangle_render_data<rgba> _rectangles;
	cgv::render::sphere_render_data<rgba> _sphere;
};

}
}

#include <cgv/config/lib_end.h>
