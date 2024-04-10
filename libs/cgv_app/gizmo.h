#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ray.h>
#include <cgv/render/view.h>
#include <cgv_gl/arrow_render_data.h>
#include <cgv_gl/rectangle_render_data.h>
#include <cgv_gl/sphere_render_data.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API gizmo {
protected:

	enum class InteractionFeature {
		kNone = 0,
		kXAxis = 1,
		kYAxis = 2,
		kZAxis = 3,
		kYZPlane = 4,
		kXZPlane = 5,
		kXYPlane = 6,
		kCenter = 7
	} feature = InteractionFeature::kNone;

	cgv::render::view* view_ptr = nullptr;

	bool align_to_view;
	bool hover;
	bool active;
	float scale;
	float scale_coefficient;

	vec3 position;
	vec3 last_position;
	vec3 offset;
	vec3 flip_factors;

	std::function<void(void)> move_callback;

	struct {
		cgv::render::arrow_renderer arrow;
		cgv::render::rectangle_renderer rectangle;
		cgv::render::sphere_renderer sphere;
	} renderers;

	cgv::render::arrow_render_data<> arrows;
	cgv::render::rectangle_render_data<> rectangles;
	cgv::render::sphere_render_data<> sphere;

	bool geometry_out_of_date;

	void create_geometry();

	bool is_axis(InteractionFeature feature) {
		return feature == InteractionFeature::kXAxis || feature == InteractionFeature::kYAxis || feature == InteractionFeature::kZAxis;
	}

	bool is_plane(InteractionFeature feature) {
		return feature == InteractionFeature::kXYPlane || feature == InteractionFeature::kXZPlane || feature == InteractionFeature::kYZPlane;
	}

	bool is_center(InteractionFeature feature) {
		return feature == InteractionFeature::kCenter;
	}

	int get_axis_idx(InteractionFeature feature) {
		if(!is_center(feature)) {
			int feature_id = static_cast<int>(feature);
			return (feature_id - 1) % 3;
		}
		return -1;
	}

	vec3 get_axis(InteractionFeature feature) {
		vec3 axis(0.0f);
		int idx = get_axis_idx(feature);
		if(idx > -1)
			axis[idx] = 1.0f;
		return axis;
	}

	bool intersect_axis_aligned_rectangle(const cgv::math::ray3& r, int axis, const vec3& p, float scale, float& t) const;

	bool intersect(const cgv::math::ray3& r);

	bool handle_drag(const cgv::math::ray3& r, const vec3& view_dir, bool drag_start);

	vec3 get_flip_factors();

public:
	gizmo();

	void destruct(cgv::render::context& ctx);

	bool handle(cgv::gui::event& e, cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);

	void set_view_ptr(cgv::render::view* view_ptr);

	void enable_view_alignment(bool enable) { align_to_view = enable; }

	void set_scale(float scale) {

		scale_coefficient = scale;
	}

	void set_move_callback(std::function<void(void)> func) {
		move_callback = func;
	}

	void set_position(const vec3& p, bool invoke_callback = false) {

		position = p;
		if(invoke_callback && move_callback)
			move_callback();
	}

	vec3 get_position() const {

		return position;
	}
};

}
}

#include <cgv/config/lib_end.h>
