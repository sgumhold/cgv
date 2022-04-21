#pragma once
#include <unordered_map>

#include "cgv/render/render_types.h"
#include "cgv/render/context.h"
#include "cgv_gl/sphere_renderer.h"
#include "cgv_gl/box_renderer.h"
#include "cgv_gl/arrow_renderer.h"
#include "cgv_gl/spline_tube_renderer.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

class CGV_API debug_visualization_helper;
extern CGV_API void ref_debug_visualization_helper(debug_visualization_helper** instance, int** reference_count);
extern CGV_API debug_visualization_helper& ref_debug_visualization_helper();
extern CGV_API debug_visualization_helper& ref_debug_visualization_helper(render::context& ctx, int ref_count_change);

/// Singleton that can visualize different types of values (e.g. vectors or positions) for debugging purposes.
///	Values are registered and later addressed by the returned handle and can have individual settings (e.g. color or size) set.
///	The value has to be explicitly updated through the update_debug_value functions.
///	Provides a ui listing all registered values and their properties.
class CGV_API debug_visualization_helper : cgv::render::render_types
{
private:
	struct debug_value
	{
		bool is_enabled{ true };
		virtual ~debug_value() {}
	};
	struct debug_value_position : debug_value
	{
		vec3 value;
		int sphere_geometry_idx{ -1 };
		float sphere_radius{ 0.05f };
		rgb color{ rgb(0.6f, 0.6f, 0.6f) };
		debug_value_position(vec3 value) : value(value) {}
	};
	struct debug_value_vector : debug_value
	{
		vec3 value;
		int arrow_geometry_idx { -1 };
		vec3 position;
		float shaft_radius{ 0.05f };
		rgb color{ rgb(0.6f, 0.6f, 0.6f) };
		debug_value_vector(vec3 value) : value(value) {}
	};

	std::vector<int> unused_handles;

	std::unordered_map<int, debug_value*> debug_values;

	int register_debug_value(debug_value* value);
	template<typename T>
	T* retrieve_debug_value(int handle);

	std::vector<vec3> arrow_positions;
	std::vector<vec3> arrow_directions;
	std::vector<float> arrow_shaft_radii;
	std::vector<rgb> arrow_colors;

	std::vector<vec3> sphere_positions;
	std::vector<float> sphere_radii;
	std::vector<rgb> sphere_colors;

	void construct_arrow(int& idx, vec3 position, vec3 direction, float shaft_radius, rgb color);
	void construct_sphere(int& idx, vec3 position, float radius, rgb color);
	void construct_position_geometry(debug_value_position* debug_value);
	void construct_vector_geometry(debug_value_vector* debug_value);
	void reconstruct_geometry();
public:
	void manage_singleton(render::context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change);

	/// Register a positional value (typically visualized as a sphere), returns handle to the value.
	int register_debug_value_position(vec3 value);
	/// Register a vector value (direction and length) (typically visualized as an arrow), returns handle to the value.
	int register_debug_value_vector(vec3 value);

	void deregister_debug_value(int handle);

	/// Change settings of positional value
	void configure_debug_value_position(int handle, float sphere_radius, rgb color);
	/// Change settings of vector value
	void configure_debug_value_vector(int handle, float shaft_radius, rgb color);

	void enable_debug_value_visualization(int handle);
	void disable_debug_value_visualization(int handle);

	void update_debug_value_position(int handle, vec3 value);
	void update_debug_value_vector_direction(int handle, vec3 value);
	void update_debug_value_vector_position(int handle, vec3 value);

	bool init(render::context& ctx);
	void clear(render::context& ctx);
	void draw(render::context& ctx);
};

template <typename T>
T* debug_visualization_helper::retrieve_debug_value(int handle)
{
	if (debug_values.count(handle) == 0)
		return nullptr;
	return dynamic_cast<T*>(debug_values[handle]);
}

	}
}

#include <cgv/config/lib_end.h>