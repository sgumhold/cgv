#pragma once

#include <cg_nui/gizmo.h>
#include <cg_nui/reusable_gizmo_functionalities.h>
#include <cgv_gl/arrow_renderer.h>
#include <cg_nui/translatable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// A gizmo that provides the means to translate an object along configured axes.
///	Needs at least a position to manipulate.
///	Optionally takes a rotation to allow for translation in the object coordinates.
class CGV_API translation_gizmo : public cgv::nui::gizmo,
	public cgv::nui::gizmo_functionality_configurable_axes,
	public cgv::nui::gizmo_functionality_handle_states,
	public cgv::nui::gizmo_functionality_absolute_axes_rotation
{
	// pointers to properties of the object the gizmo is attached to
	vec3* position_ptr{ nullptr };
	vec3** position_ptr_ptr{ nullptr };
	translatable* translatable_obj{ nullptr };

	cgv::render::arrow_render_style ars;

	float arrow_radius { 0.05f };

	// only have to be recomputed if the position or rotation of the object changes
	std::vector<vec3> arrow_positions;
	std::vector<vec3> arrow_directions;

	vec3 position_at_grab;

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

	// DEBUG TO REMOVE
	int debug_coord_system_handle0;
	int debug_ray_handle0;
	int debug_ray_handle1;

protected:
	// Get the position of the attached object
	vec3 get_position();
	// Set the position of the attached object
	void set_position(const vec3& position);

	// current configuration of the gizmo
	float translation_axes_length { 0.2f };

	bool validate_configuration() override;

	void on_handle_grabbed() override;
	void on_handle_drag() override;
	void on_handle_released() override;

	void precompute_geometry() override;

public:
	translation_gizmo() : gizmo("translation_gizmo") {}

	// Configuration functions

	/// Set reference to the position that will be manipulated by this gizmo as a pointer.
	///	If a base object reference is given it will be notified of value changes through on_set.
	void set_position_reference(vec3* _position_ptr, cgv::base::base_ptr _on_set_obj = nullptr);
	/// Set reference to the position that will be manipulated by this gizmo as a pointer to a pointer.
	///	If a base object reference is given it will be notified of value changes through on_set.
	void set_position_reference(vec3** _position_ptr_ptr, cgv::base::base_ptr _on_set_obj = nullptr);
	/// Set reference to the position that will be manipulated by this gizmo as a reference to an object implementing the translatable interface.
	void set_position_reference(translatable* _translatable_obj);

	void set_axes_directions(std::vector<vec3> axes) override;
	/// Set various parameters of the individual arrow geometries.
	// TODO: Add more parameters
	void configure_axes_geometry(float radius, float length);

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