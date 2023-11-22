#pragma once

#include <cg_nui/gizmo.h>
#include <cg_nui/reusable_gizmo_functionalities.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/spline_tube_renderer.h>
#include <cg_nui/scalable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// A gizmo that provides the means to scale an object along configured axes.
///	Needs a scale to manipulate.
class CGV_API scaling_gizmo : public cgv::nui::gizmo,
	public cgv::nui::gizmo_functionality_configurable_axes,
	public cgv::nui::gizmo_functionality_handle_states
{
	// pointers to properties of the object the gizmo is attached to
	vec3* scale_ptr{ nullptr };
	vec3** scale_ptr_ptr{ nullptr };
	scalable* scalable_obj{ nullptr };

	// render styles for the gizmo handles
	cgv::render::box_render_style brs;
	cgv::render::spline_tube_render_style strs;

	// visual properties of the gizmo handles
	float spline_tube_radius{ 0.03f };
	float cube_size{ 0.07f };

	// gizmo handle geometry
	std::vector<vec3> cube_positions;
	std::vector<quat> cube_rotations;
	typedef std::pair<std::vector<vec3>, std::vector<vec4>> spline_data_t;
	std::vector<spline_data_t> splines;
	std::vector<vec3> handle_positions;
	std::vector<vec3> handle_directions;

	// distance of intersection point to object center at the start of the interaction
	float distance_at_grab;
	// scale of the controlled object at the start of the interaction
	vec3 scale_at_grab;

	/// Compute the scale-dependent geometry
	void compute_geometry(const vec3& scale);
	/// Draw the handles
	void _draw(cgv::render::context& ctx, const vec3& scale, const mat4& view_matrix) override;
	/// Do proximity check
	bool _compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx, const vec3& scale,
	                            const mat4& view_matrix) override;
	/// Do intersection check
	bool _compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx,
	                           const vec3& scale, const mat4& view_matrix) override;

protected:
	// Get the position of the attached object
	vec3 get_scale();
	// Set the position of the attached object
	void set_scale(const vec3& scale);
	// Constrain scale to the configured minimum and maximum
	void ensure_scale_constraints(vec3& scale);

	// current configuration of the gizmo
	std::vector<rgb> scaling_axes_colors;
	float scaling_axes_length{ 0.2f };
	std::vector<vec3> scaling_axes_scale_ratios;
	vec3 minimum_scale{ std::numeric_limits<float>::min() };
	vec3 maximum_scale{ std::numeric_limits<float>::max() };

	bool validate_configuration() override;

	void on_handle_grabbed() override;
	void on_handle_released() override;
	void on_handle_drag() override;

	void precompute_geometry() override;

public:
	scaling_gizmo() : gizmo("scaling_gizmo") {}

	// Configuration functions

	/// Set reference to the scale that will be manipulated by this gizmo as a pointer.
	///	If a base object reference is given it will be notified of value changes through on_set.
	void set_scale_reference(vec3* _scale_ptr, cgv::base::base_ptr _on_set_obj = nullptr);
	/// Set reference to the scale that will be manipulated by this gizmo as a pointer to a pointer.
	///	If a base object reference is given it will be notified of value changes through on_set.
	void set_scale_reference(vec3** _scale_ptr_ptr, cgv::base::base_ptr _on_set_obj = nullptr);
	/// Set reference to the scale that will be manipulated by this gizmo as a reference to an object implementing the scalable interface.
	void set_scale_reference(scalable* _scalable_obj);

	void set_axes_directions(std::vector<vec3> axes_directions) override;
	/// Set various parameters of the individual axis geometries.
	// TODO: Add more parameters
	void configure_axes_geometry(float radius, float length, float cube_size);
	/// Set ratios between x, y and z that are used to scale with each axis.
	///	Default value is that of the direction of the corresponding axis.
	void configure_axes_scale_ratios(std::vector<vec3> scale_ratios);
	/// Set minimal and maximal scale that can be set via the gizmo.
	void configure_scale_limits(vec3 minimum_scale, vec3 maximum_scale);
	/// Set minimal scale that can be set via the gizmo.
	void configure_scale_limit_minimum(vec3 minimum_scale);
	/// Set maximal scale that can be set via the gizmo.
	void configure_scale_limit_maximum(vec3 maximum_scale);

	//@name cgv::render::drawable interface
	//@{
	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;
	//@}

	//@name cgv::gui::provider interface
	//@{
	void create_gui() override;
	//@}
};

	}
}

#include <cgv/config/lib_end.h>
