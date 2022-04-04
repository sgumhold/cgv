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
	public cgv::nui::gizmo_functionality_local_world_orientation,
	public cgv::nui::gizmo_functionality_configurable_axes
{
	cgv::render::box_render_style brs;
	cgv::render::spline_tube_render_style strs;

	std::vector<rgb> handle_colors;
	float spline_tube_radius{ 0.03f };
	float cube_size{ 0.07f };

	// only have to be recomputed if the position or rotation of the object changes
	std::vector<vec3> cube_positions;
	std::vector<quat> cube_rotations;
	typedef std::pair<std::vector<vec3>, std::vector<vec4>> spline_data_t;
	std::vector<spline_data_t> splines;

	vec3 scale_at_grab;

protected:
	// pointers to properties of the object the gizmo is attached to
	vec3* scale_ptr{ nullptr };

	// current configuration of the gizmo
	std::vector<rgb> scaling_axes_colors;
	float scaling_axes_length{ 0.2f };

	void on_handle_grabbed() override;
	void on_handle_drag() override;

	void compute_geometry() override;

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
