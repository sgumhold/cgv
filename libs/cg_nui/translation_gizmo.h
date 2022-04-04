#pragma once

#include <cg_nui/gizmo.h>
#include <cg_nui/reusable_gizmo_functionalities.h>
#include <cgv_gl/arrow_renderer.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// A gizmo that provides the means to translate an object along configured axes.
///	Needs at least a position to manipulate.
///	Optionally takes a rotation to allow for translation in the object coordinates.
class CGV_API translation_gizmo : public cgv::nui::gizmo,
	public cgv::nui::gizmo_functionality_local_world_orientation,
	public cgv::nui::gizmo_functionality_configurable_axes
{
	cgv::render::arrow_render_style ars;

	std::vector<rgb> arrow_colors;
	float arrow_radius { 0.05f };

	// only have to be recomputed if the position or rotation of the object changes
	std::vector<vec3> arrow_positions;
	std::vector<vec3> arrow_directions;

	vec3 position_at_grab;

protected:
	// pointers to properties of the object the gizmo is attached to
	vec3* position_ptr { nullptr };

	// current configuration of the gizmo
	std::vector<rgb> translation_axes_colors;
	float translation_axes_length { 0.2f };

	void on_handle_grabbed() override;
	void on_handle_drag() override;

	void compute_geometry() override;

public:
	translation_gizmo() : gizmo("translation_gizmo") {}

	/// Attach this gizmo to the given object. The given position will be manipulated and used as the anchor
	/// for the gizmo's visual representation. The optional rotation is needed if the gizmo should operate
	///	in the local coordinate system. The optional scale is needed if the gizmo's visuals should follow the given scale.
	void attach(base_ptr obj, vec3* position_ptr, quat* rotation_ptr = nullptr, vec3* scale_ptr = nullptr);
	void detach();

	// Configuration functions
	void configure_axes_directions(std::vector<vec3> axes) override;
	/// Set colors for the visual representation of the axes. If less colors then axes are given then the
	///	last color will be repeated.
	// TODO: Extend with configuration of selection/grab color change
	void configure_axes_coloring(std::vector<rgb> colors);
	/// Set various parameters of the individual arrow geometries.
	// TODO: Add more parameters
	void configure_axes_geometry(float radius, float length);

	//@name cgv::nui::grabable interface
	//@{
	bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) override;
	//@}
	//@name cgv::nui::pointable interface
	//@{
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) override;
	//@}

	//@name cgv::render::drawable interface
	//@{
	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;
	void draw(cgv::render::context& ctx) override;
	//@}

	//@name cgv::gui::provider interface
	//@{
	void create_gui() override;
	//@}
};

	}
}

#include <cgv/config/lib_end.h>