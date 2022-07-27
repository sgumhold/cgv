#pragma once

#include <cg_nui/gizmo.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/spline_tube_renderer.h>
#include <cg_nui/reusable_gizmo_functionalities.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// A gizmo that provides the means to scale an object along configured axes.
///	Needs at least a scale to manipulate.
///	Optionally takes a position as an anchor point and a rotation to allow for scaling in the object coordinates.
class CGV_API scaling_gizmo : public cgv::nui::gizmo,
	public cgv::nui::gizmo_functionality_configurable_axes
{
	cgv::render::box_render_style brs;
	cgv::render::spline_tube_render_style strs;

	std::vector<rgb> handle_colors;
	float spline_tube_radius{ 0.03f };
	float cube_size{ 0.07f };

	// geometry
	std::vector<vec3> cube_positions;
	std::vector<quat> cube_rotations;
	typedef std::pair<std::vector<vec3>, std::vector<vec4>> spline_data_t;
	std::vector<spline_data_t> splines;
	std::vector<vec3> handle_positions;
	std::vector<vec3> handle_directions;

	float distance_at_grab;
	vec3 scale_at_grab;

	/// Compute the scale-dependent geometry
	void compute_geometry(const vec3& scale);
	/// Draw the handles
	void _draw(cgv::render::context& ctx);
	/// Do proximity check
	bool _compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx);
	/// Do intersection check
	bool _compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx);

protected:
	// pointers to properties of the object the gizmo is attached to
	vec3* scale_ptr{ nullptr };

	// current configuration of the gizmo
	std::vector<rgb> scaling_axes_colors;
	float scaling_axes_length{ 0.2f };
	std::vector<vec3> scaling_axes_scale_ratios;

	bool validate_configuration() override;

	void on_handle_grabbed() override;
	void on_handle_drag() override;

	void precompute_geometry() override;

	//void _draw_local_orientation(cgv::render::context& ctx, const vec3& inverse_translation, const quat& inverse_rotation, const vec3& scale, const mat4& view_matrix) override;
	//void _draw_global_orientation(cgv::render::context& ctx, const vec3& inverse_translation, const quat& rotation, const vec3& scale, const mat4& view_matrix) override;
	//
	//bool _compute_closest_point_local_orientation(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx, const vec3& inverse_translation, const quat& inverse_rotation, const vec3& scale, const mat4& view_matrix) override;
	//bool _compute_closest_point_global_orientation(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx, const vec3& inverse_translation, const quat& rotation, const vec3& scale, const mat4& view_matrix) override;
	//bool _compute_intersection_local_orientation(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx, const vec3& inverse_translation, const quat& inverse_rotation, const vec3& scale, const mat4& view_matrix) override;
	//bool _compute_intersection_global_orientation(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx, const vec3& inverse_translation, const quat& rotation, const vec3& scale, const mat4& view_matrix) override;

public:
	scaling_gizmo() : gizmo("scaling_gizmo") {}

	/// Attach this gizmo to the given object. The given scale will be manipulated and used for the gizmo's
	/// visual representation. The position will be used as the anchor for the visual representation. The
	/// optional rotation is needed if the gizmo should operate in the local coordinate system.
	void attach(base_ptr obj, vec3* scale_ptr, vec3* position_ptr, quat* rotation_ptr = nullptr);
	void detach();

	// Configuration functions
	void configure_axes_directions(std::vector<vec3> axes_directions) override;
	/// Set colors for the visual representation of the axes. If less colors then axes are given then the
	///	last color will be repeated.
	// TODO: Extend with configuration of selection/grab color change
	void configure_axes_coloring(std::vector<rgb> colors);
	/// Set various parameters of the individual axis geometries.
	// TODO: Add more parameters
	void configure_axes_geometry(float radius, float length, float cube_size);
	/// Set ratios between x, y and z that are used to scale with each axis.
	///	Default value is that of the direction of the corresponding axis.
	void configure_axes_scale_ratios(std::vector<vec3> scale_ratios);

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
