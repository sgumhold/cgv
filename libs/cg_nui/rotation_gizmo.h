#pragma once

#include <cg_nui/gizmo.h>
#include <cg_nui/reusable_gizmo_functionalities.h>
#include <cgv_gl/spline_tube_renderer.h>
#include <cg_nui/rotatable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// A gizmo that provides the means to rotate an object around configured axes.
///	Needs a rotation to manipulate.
class CGV_API rotation_gizmo : public cgv::nui::gizmo,
	public cgv::nui::gizmo_functionality_configurable_axes,
	public cgv::nui::gizmo_functionality_handle_states
{
	// pointers to properties of the object the gizmo is attached to
	quat* rotation_ptr{ nullptr };
	quat** rotation_ptr_ptr{ nullptr };
	rotatable* rotatable_obj{ nullptr };

	cgv::render::spline_tube_render_style strs;

	float ring_spline_radius{ 0.05f };
	unsigned ring_nr_spline_segments = 4;
	float ring_radius;

	typedef std::pair<std::vector<vec3>, std::vector<vec4>> spline_data_t;
	std::vector<spline_data_t> ring_splines;
	std::vector<spline_data_t> precomputed_ring_splines;

	quat rotation_at_grab;

	int projected_point_handle;
	int direction_at_grab_handle, direction_currently_handle;

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
	// Get the rotation of the attached object
	quat get_rotation();
	// Set the rotation of the attached object
	void set_rotation(const quat& rotation);

	// current configuration of the gizmo
	std::vector<float> rotation_axes_radii{ 0.2f };

	bool validate_configuration() override;

	void on_handle_grabbed() override;
	void on_handle_released() override;
	void on_handle_drag() override;

	void precompute_geometry() override;

public:
	rotation_gizmo() : gizmo("rotation_gizmo") {}

	// Configuration functions

	/// Set reference to the rotation that will be manipulated by this gizmo as a pointer.
	///	If a base object reference is given it will be notified of value changes through on_set.
	void set_rotation_reference(quat* _rotation_ptr, cgv::base::base_ptr _on_set_obj = nullptr);
	/// Set reference to the rotation that will be manipulated by this gizmo as a pointer to a pointer.
	///	If a base object reference is given it will be notified of value changes through on_set.
	void set_rotation_reference(quat** _rotation_ptr_ptr, cgv::base::base_ptr _on_set_obj = nullptr);
	/// Set reference to the rotation that will be manipulated by this gizmo as a reference to an object implementing the rotatable interface.
	void set_rotation_reference(rotatable* _rotatable_obj);

	void set_axes_directions(std::vector<vec3> axes) override;
	/// Set various parameters of the individual ring geometries.
	void configure_axes_geometry(float ring_radius, float ring_tube_radius);

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