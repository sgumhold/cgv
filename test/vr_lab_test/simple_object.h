#pragma once

#include <cg_nui/grabable_interactable.h>
#include <cgv_gl/box_renderer.h>

/// Example implementation of a single object that can be grabbed/triggered and then moved.
class simple_object :
	public cgv::nui::grabable_interactable
{
	cgv::render::box_render_style brs;
	static cgv::render::shader_program prog;

protected:
	// geometry of box with color
	vec3 position;
	quat rotation;
	vec3 extent;
	rgb  color;
	/// return color modified based on state
	rgb get_modified_color(const rgb& color) const;

public:
	simple_object(const std::string& _name, const vec3& _position, const rgb& _color = rgb(0.5f,0.5f,0.5f), const vec3& _extent = vec3(0.3f,0.2f,0.1f), const quat& _rotation = quat(1,0,0,0));
	std::string get_type_name() const override;

	bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) override;
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) override;

	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;
	void draw(cgv::render::context& ctx) override;

	void create_gui() override;
};

typedef cgv::data::ref_ptr<simple_object> simple_object_ptr;
