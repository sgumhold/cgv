#pragma once
#include <unordered_map>

#include "cgv/render/render_types.h"
#include "cgv/render/context.h"
#include "cgv_gl/sphere_renderer.h"
#include "cgv_gl/box_renderer.h"
#include "cgv_gl/arrow_renderer.h"
#include "cgv_gl/spline_tube_renderer.h"

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
	namespace nui {

class CGV_API debug_visualization_helper;
extern CGV_API void ref_debug_visualization_helper(debug_visualization_helper** instance, int** reference_count);
extern CGV_API debug_visualization_helper& ref_debug_visualization_helper();
extern CGV_API debug_visualization_helper& ref_debug_visualization_helper(render::context& ctx, int ref_count_change);


struct debug_value_config_position
{
	float sphere_radius{ 0.05f };
	rgb color{ rgb(0.6f, 0.6f, 0.6f) };
};
struct debug_value_config_vector
{
	vec3 position{ vec3(0.0f) };
	float shaft_radius{ 0.05f };
	rgb color{ rgb(0.6f, 0.6f, 0.6f) };
};
struct debug_value_config_coordinate_system
{
	bool show_translation{ true };
	bool show_rotation{ true };
	bool show_scale{ true };
	vec3 position{ 0.0f };
	float base_axis_length{ 0.15f };
	float axis_radius{ 0.0075f };
	float center_radius{ 0.02f };
	rgb center_color{ rgb(0.7f, 0.7f, 0.7f) };
};
struct debug_value_config_ray
{
	float length{ 15.0f };
	float start_offset{ 0.0f };
	float ray_radius{ 0.005f };
	float origin_radius{ 0.02f };
	rgb origin_color{ rgb(0.7f, 0.7f, 0.7f) };
	rgb ray_color{ rgb(1.0f, 0.2f, 0.2f) };
	bool show_origin{ true };
};

/// Singleton that can visualize different types of values (e.g. vectors or positions) for debugging purposes.
///	Values are registered and later addressed by the returned handle and can have individual settings (e.g. color or size) set.
///	The value has to be explicitly updated through the update_debug_value functions.
///	Provides a ui listing all registered values and their properties.
class CGV_API debug_visualization_helper
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
		debug_value_config_position config;
		debug_value_position(vec3 value) : value(value) {}
	};
	struct debug_value_vector : debug_value
	{
		vec3 value;
		int arrow_geometry_idx { -1 };
		debug_value_config_vector config;
		debug_value_vector(vec3 value) : value(value) {}
	};
	struct debug_value_coordinate_system : debug_value
	{
		mat4 value;
		int arrow0_geometry_idx{ -1 };
		int arrow1_geometry_idx{ -1 };
		int arrow2_geometry_idx{ -1 };
		int sphere_geometry_idx{ -1 };
		debug_value_config_coordinate_system config;
		debug_value_coordinate_system(mat4 value) : value(value) {}
	};
	struct debug_value_ray : debug_value
	{
		vec3 origin;
		vec3 direction;
		int spline_geometry_idx{ -1 };
		int sphere_geometry_idx{ -1 };
		debug_value_config_ray config;
		debug_value_ray(vec3 origin, vec3 direction) : origin(origin), direction(direction) {}
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
	cgv::render::arrow_render_style ars;

	std::vector<vec3> sphere_positions;
	std::vector<float> sphere_radii;
	std::vector<rgb> sphere_colors;
	cgv::render::sphere_render_style srs;

	typedef std::pair<std::vector<vec3>, std::vector<vec4>> spline_data_t;
	std::vector<spline_data_t> splines;
	std::vector<float> spline_radii;
	std::vector<rgb> spline_colors;
	cgv::render::spline_tube_render_style strs;

	std::vector<rgb> coordinate_system_arrow_colors{ rgb(1.0, 0.0, 0.0), rgb(0.0, 1.0, 0.0), rgb(0.0, 0.0, 1.0) };

	void construct_arrow(int& idx, vec3 position, vec3 direction, float shaft_radius, rgb color);
	void construct_sphere(int& idx, vec3 position, float radius, rgb color);
	void construct_spline(int& idx, std::vector<vec3> positions, std::vector<vec4> tangents, float radius, rgb color);
	void construct_position_geometry(debug_value_position* debug_value);
	void construct_vector_geometry(debug_value_vector* debug_value);
	void construct_coordinate_system_geometry(debug_value_coordinate_system* debug_value);
	void construct_ray_geometry(debug_value_ray* debug_value);
	void reconstruct_geometry();
public:
	void manage_singleton(render::context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change);

	/// Register a positional value (typically visualized as a sphere), returns handle to the value.
	int register_debug_value_position(vec3 value);
	/// Register a positional value (typically visualized as a sphere), returns handle to the value.
	int register_debug_value_position();
	/// Register a vector value (direction and length) (typically visualized as an arrow), returns handle to the value.
	int register_debug_value_vector(vec3 value);
	/// Register a vector value (direction and length) (typically visualized as an arrow), returns handle to the value.
	int register_debug_value_vector();
	/// Register a coordinate system value (mat4 transformation), returns handle to the value.
	int register_debug_value_coordinate_system(mat4 value);
	/// Register a coordinate system value (mat4 transformation), returns handle to the value.
	int register_debug_value_coordinate_system();
	/// Register a ray value, returns handle to the value.
	int register_debug_value_ray(vec3 origin, vec3 direction);
	/// Register a ray value, returns handle to the value.
	int register_debug_value_ray();

	void deregister_debug_value(int handle);

	/// Get current config of positional value. Change, then call the corresponding set config function with the changed config.
	debug_value_config_position get_config_debug_value_position(int handle);
	/// Set config of positional value to given config.
	void set_config_debug_value_position(int handle, debug_value_config_position config);
	/// Get current config of vector value. Change, then call the corresponding set config function with the changed config.
	debug_value_config_vector get_config_debug_value_vector(int handle);
	/// Set config of vector value to given config.
	void set_config_debug_value_vector(int handle, debug_value_config_vector config);
	/// Get current config of coordinate system value. Change, then call the corresponding set config function with the changed config.
	debug_value_config_coordinate_system get_config_debug_value_coordinate_system(int handle);
	/// Set config of coordinate system value to given config.
	void set_config_debug_value_coordinate_system(int handle, debug_value_config_coordinate_system config);
	/// Get current config of ray value. Change, then call the corresponding set config function with the changed config.
	debug_value_config_ray get_config_debug_value_ray(int handle);
	/// Set config of ray value to given config.
	void set_config_debug_value_ray(int handle, debug_value_config_ray config);

	void enable_debug_value_visualization(int handle);
	void disable_debug_value_visualization(int handle);

	void update_debug_value_position(int handle, vec3 value);
	void update_debug_value_vector_direction(int handle, vec3 value);
	void update_debug_value_vector_position(int handle, vec3 value);
	void update_debug_value_coordinate_system(int handle, mat4 value);
	void update_debug_value_ray(int handle, vec3 origin, vec3 direction);
	void update_debug_value_ray_origin(int handle, vec3 origin);
	void update_debug_value_ray_direction(int handle, vec3 direction);

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