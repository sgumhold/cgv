#pragma once

#include <cg_nui/poseable.h>
#include <cgv_gl/box_renderer.h>
#include <cg_nui/translation_gizmo.h>
#include <cg_nui/rotation_gizmo.h>
#include <cg_nui/scaling_gizmo.h>

/// Example implementation of a single object that can be grabbed/triggered and then moved and also uses gizmos for translation, rotation and scaling.
///	The poseable class handles grabbing and moving the object.
///	The object also creates and manages three gizmos for translation, rotation and scaling.
class simple_object : public cgv::nui::poseable, public cgv::nui::transforming,
	public cgv::nui::translatable, public cgv::nui::rotatable, public cgv::nui::scalable
{
	// Render style and shader for rounded box
	cgv::render::box_render_style brs;
	static cgv::render::shader_program prog;

	enum class ActiveGizmoOptions
	{
		AGO_NONE,
		AGO_TRANSLATION,
		AGO_SCALING,
		AGO_ROTATION
	};
	// Gizmo type that is currently active
	ActiveGizmoOptions active_gizmo{ ActiveGizmoOptions::AGO_TRANSLATION };
	// Gizmo type that was selected to be active in the ui
	ActiveGizmoOptions active_gizmo_ui{ ActiveGizmoOptions::AGO_TRANSLATION };

protected:
	// geometry of box with color
	// position is provided by translatable interface
	quat rotation;
	vec3 extent;
	rgb  color;
	/// return color modified based on state
	rgb get_modified_color(const rgb& color) const;

	// Reference to the translation gizmo instance
	cgv::data::ref_ptr<cgv::nui::translation_gizmo, true> trans_gizmo;
	// Reference to the rotation gizmo instance
	cgv::data::ref_ptr<cgv::nui::rotation_gizmo, true> rot_gizmo;
	// Reference to the scaling gizmo instance
	cgv::data::ref_ptr<cgv::nui::scaling_gizmo, true> scale_gizmo;

	//@name cgv::nui::posable interface
	//@{
	bool _compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) override;
	bool _compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) override;
	//@}

public:
	simple_object(const std::string& _name, const vec3& _position, const rgb& _color = rgb(0.5f,0.5f,0.5f), const vec3& _extent = vec3(0.3f,0.2f,0.1f), const quat& _rotation = quat(1,0,0,0));
	//@name cgv::base::base interface
	//@{
	std::string get_type_name() const override;
	//@}

	// Deferred initialization of the gizmos.
	// As the gizmos require references to objects retrieved by traversing the scene graph they have to be created after these objects are put in the hierarchy.
	void initialize_gizmos(cgv::base::node_ptr root, cgv::base::node_ptr anchor, bool anchor_influenced_by_gizmo = true, bool root_influenced_by_gizmo = false);

	//@name cgv::render::drawable interface
	//@{
	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;
	void draw(cgv::render::context& ctx) override;
	void finish_draw(cgv::render::context&) override;
	//@}

	//@name cgv::base::base interface
	//@{
	void on_set(void* member_ptr) override;
	//@}
	//@name cgv::gui::provider interface
	//@{
	void create_gui() override;
	//@}

	//@name cgv::nui::transforming interface
	//@{
	mat4 get_model_transform() const override;
	mat4 get_inverse_model_transform() const override;
	//@}
};

typedef cgv::data::ref_ptr<simple_object> simple_object_ptr;
