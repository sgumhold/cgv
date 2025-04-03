#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/math/fvec.h>
#include <cgv/math/quaternion.h>
#include <cgv/math/ray.h>
#include <cgv/render/drawable.h>
#include <cgv/render/view.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

struct plane {
	vec3 origin = { 0.0f };
	vec3 normal = { 0.0f };

	bool valid() const {
		return length(normal) > std::numeric_limits<float>::epsilon();
	}
};

enum class GizmoAction {
	kDragStart,
	kDrag,
	kDragEnd
};

enum class GizmoOrientation {
	kGlobal,
	kLocal
};

class CGV_API gizmo :
	public cgv::base::node,
	public cgv::gui::event_handler,
	public cgv::render::drawable {
public:
	std::string get_type_name() const override { return "gizmo"; }

	bool init(cgv::render::context&) override = 0;

	void clear(cgv::render::context&) override = 0;

	void finish_frame(cgv::render::context&) override;

	bool handle(cgv::gui::event& e) override;

	void stream_help(std::ostream& os) override {};

	GizmoOrientation get_orientation() const;

	void set_orientation(GizmoOrientation orientation);

	vec3 get_position() const;

	void set_position(const vec3& position);

	quat get_rotation() const;

	void set_rotation(const quat& rotation);

	float size_scale = 1.0f;
	bool keep_screen_size_constant = true;
	bool lock_size_during_interaction = false;

protected:
	enum class InteractionFeature {
		kNone = 0,
		kAxis,
		kPlane,
		kCenter
	};

	enum class AxisId {
		kX = 0,
		kY = 1,
		kZ = 2
	};

	InteractionFeature _interaction_feature = InteractionFeature::kNone;
	AxisId _interaction_axis_id = AxisId::kX;
	plane _interaction_plane;
	cgv::math::ray3 _drag_start_ray;
	float _drag_start_t = 0.0f;

	float get_size() const;

	const cgv::render::view* get_view() const;

	void set_geometry_out_of_date();

	bool captured_mouse() const;

	bool is_hovered() const;

	int axis_id_to_index(AxisId axis) const;

	AxisId index_to_axis_id(int idx) const;

	vec3 get_axis(int index) const;

	vec3 get_axis_mask(InteractionFeature feature, AxisId axis_id) const;

private:
	virtual void create_geometry() = 0;

	virtual void draw_geometry(cgv::render::context& ctx) = 0;

	virtual bool intersect_bounding_box(const cgv::math::ray3& ray) { return true; }

	virtual bool intersect(const cgv::math::ray3& ray) { return false; }

	virtual bool start_drag(const cgv::math::ray3& ray) { return false; }

	virtual bool drag(const cgv::math::ray3& ray) { return false; }

	virtual void end_drag(const cgv::math::ray3& ray) {}

	cgv::render::view* _view = nullptr;

	bool _geometry_out_of_date = true;
	float _size = -1.0f;

	bool _captured_mouse = false;
	bool _hovered = false;

	vec3 _position = { 0.0f };
	quat _rotation = {};
	GizmoOrientation _orientation = GizmoOrientation::kGlobal;
};

}
}

#include <cgv/config/lib_end.h>
