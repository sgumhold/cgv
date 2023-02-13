#pragma once

#include <cg_nui/poseable.h>
#include <cgv_gl/box_renderer.h>
#include <cg_nui/translation_gizmo.h>
#include <cg_nui/rotation_gizmo.h>
#include <cg_nui/scaling_gizmo.h>

/// Example implementation of a single object that can be grabbed/triggered and then moved.
class simple_object : public cgv::nui::poseable, public cgv::nui::transforming,
	public cgv::nui::translatable, public cgv::nui::rotatable, public cgv::nui::scalable
{
	cgv::render::box_render_style brs;
	static cgv::render::shader_program prog;

	enum class ActiveGizmoOptions
	{
		AGO_NONE,
		AGO_TRANSLATION,
		AGO_SCALING,
		AGO_ROTATION
	};
	ActiveGizmoOptions active_gizmo{ ActiveGizmoOptions::AGO_NONE };
	ActiveGizmoOptions active_gizmo_ui{ ActiveGizmoOptions::AGO_NONE };

	std::vector<vec3> positions;

	// DEBUG TO REMOVE
	int debug_coord_system_handle0;
	int debug_coord_system_handle1;

protected:
	// geometry of box with color
	// position is provided by translatable interface
	quat rotation;
	vec3 extent;
	rgb  color;
	/// return color modified based on state
	rgb get_modified_color(const rgb& color) const;

	cgv::data::ref_ptr<cgv::nui::translation_gizmo, true> trans_gizmo;
	cgv::data::ref_ptr<cgv::nui::rotation_gizmo, true> rot_gizmo;
	cgv::data::ref_ptr<cgv::nui::scaling_gizmo, true> scale_gizmo;

	bool _compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) override;
	bool _compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) override;

public:
	simple_object(const std::string& _name, const vec3& _position, const rgb& _color = rgb(0.5f,0.5f,0.5f), const vec3& _extent = vec3(0.3f,0.2f,0.1f), const quat& _rotation = quat(1,0,0,0));
	std::string get_type_name() const override;

	void initialize_gizmos(cgv::base::node_ptr root, cgv::base::node_ptr anchor, bool anchor_influenced_by_gizmo = true, bool root_influenced_by_gizmo = false);

	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;
	void draw(cgv::render::context& ctx) override;
	void finish_draw(cgv::render::context&) override;

	void on_set(void* member_ptr) override;
	void create_gui() override;

	const mat4& get_model_transform() const override;
	const mat4& get_inverse_model_transform() const override;

	//@name cgv::nui::rotatable interface
	//@{
	quat get_rotation() const override;
	void set_rotation(const quat& rotation) override;
	//@}

	//@name cgv::nui::scalable interface
	//@{
	vec3 get_scale() const override;
	void set_scale(const vec3& scale) override;
	//@}
};

typedef cgv::data::ref_ptr<simple_object> simple_object_ptr;
