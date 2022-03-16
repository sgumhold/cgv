#pragma once

#include <cg_nui/grabable_interactable.h>

/// Example implementation of a collection of primitives that can be grabbed/triggered and then moved individually.
class simple_primitive_container: public cgv::nui::grabable_interactable
{
protected:
	// pointer to position of currently active sphere
	vec3* active_position;
	// geometry of spheres with color
	std::vector<vec3> positions;
	std::vector<float> radii;
	std::vector<rgb>  colors;
	cgv::render::sphere_render_style srs;
	/// return color modified based on state
	rgb get_modified_color(const rgb& color) const;
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
};

typedef cgv::data::ref_ptr<simple_primitive_container> simple_primitive_container_ptr;
