#pragma once

#include <cg_nui/poseable.h>
#include <cg_nui/translatable.h>
#include <cg_nui/translation_gizmo.h>
#include <cg_nui/rotation_gizmo.h>
#include <cg_nui/scaling_gizmo.h>

/// Example implementation of a collection of primitives that can be grabbed/triggered and then moved individually.
class simple_primitive_container: public cgv::nui::poseable, public cgv::nui::translatable
{
	enum class ActiveGizmoOptions
	{
		AGO_NONE,
		AGO_TRANSLATION,
		AGO_SCALING,
		AGO_ROTATION
	};
	ActiveGizmoOptions active_gizmo{ ActiveGizmoOptions::AGO_NONE };
	ActiveGizmoOptions active_gizmo_ui{ ActiveGizmoOptions::AGO_NONE };

protected:
	// Pointer to the position of the current prim_idx sphere for use by gizmos
	const vec3* active_position_ptr;
	// Pointer to scale of the current prim_idx sphere for use by gizmos
	const vec3* active_scale_ptr;
	// geometry of spheres with color
	std::vector<vec3> positions;
	std::vector<float> radii;
	std::vector<rgb>  colors;
	cgv::render::sphere_render_style srs;
	/// return color modified based on state
	rgb get_modified_color(const rgb& color) const;

	cgv::data::ref_ptr<cgv::nui::translation_gizmo, true> trans_gizmo;
	cgv::data::ref_ptr<cgv::nui::rotation_gizmo, true> rot_gizmo;
	cgv::data::ref_ptr<cgv::nui::scaling_gizmo, true> scale_gizmo;
public:
	simple_primitive_container(const std::string& _name, unsigned sphere_count = 20);
	std::string get_type_name() const override;
	void on_set(void* member_ptr) override;

	bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) override;
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) override;

	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;
	void draw(cgv::render::context& ctx) override;

	void create_gui() override;

	//@name cgv::nui::translatable interface
	//@{
	vec3 get_position() const override;
	void set_position(const vec3& position) override;
	//@}
};

typedef cgv::data::ref_ptr<simple_primitive_container> simple_primitive_container_ptr;
