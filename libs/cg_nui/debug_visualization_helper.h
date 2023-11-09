#pragma once
#include <unordered_map>

#include "cgv/render/render_types.h"
#include "cgv/render/context.h"
#include "cgv_gl/sphere_renderer.h"
#include "cgv_gl/box_renderer.h"
#include "cgv_gl/arrow_renderer.h"
#include "cgv_gl/spline_tube_renderer.h"
#include "cgv/base/node.h"

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
	namespace nui {

// Functions for handling singleton

class CGV_API debug_visualization_helper;
extern CGV_API void ref_debug_visualization_helper(debug_visualization_helper** instance, int** reference_count);
extern CGV_API debug_visualization_helper& ref_debug_visualization_helper();
extern CGV_API debug_visualization_helper& ref_debug_visualization_helper(render::context& ctx, int ref_count_change);

// Configurations for different types of debug primitives

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
struct debug_value_config_cylinder
{
	rgb color{ 0.355f, 0.728f, 0.960f };
};
struct debug_value_config_box
{
	rgb color{ 0.355f, 0.728f, 0.960f };
};

/// Singleton that can visualize different types of values (e.g. vectors or positions) for debugging purposes.
/// (Basically a helper that hides all the boilerplate code necessary to render various primitives.)
///	Values are registered and later addressed by the returned handle and can have individual settings (e.g. color or size) set.
///	The value has to be explicitly updated through the update_debug_value functions.
///	Provides a ui listing all registered values and their properties.
///	See nui::translation_gizmo and vr_lab_test classes for an example of setting up and using this helper.
class CGV_API debug_visualization_helper
{
private:
	/// Base struct for debug primitives. Each primitive can be enabled or disabled.
	struct debug_value
	{
		bool is_enabled{ true };
		virtual ~debug_value() {}
	};
	/// Position primitive has a vec3 value and is rendered as a sphere.
	struct debug_value_position : debug_value
	{
		vec3 value;
		int sphere_geometry_idx{ -1 };
		debug_value_config_position config;
		debug_value_position(vec3 value) : value(value) {}
	};
	/// Vector primitive has a vec3 value and is rendered as an arrow at a configured position.
	struct debug_value_vector : debug_value
	{
		vec3 value;
		int arrow_geometry_idx { -1 };
		debug_value_config_vector config;
		debug_value_vector(vec3 value) : value(value) {}
	};
	/// Coordinate system primitive has a mat4 (transform) value and is rendered as three arrows, one for each main axis, and a origin sphere at a configured position.
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
	/// Ray primitive has a vec3 value for the origin and a vec3 value for the direction and is rendered as a line of configured length.
	struct debug_value_ray : debug_value
	{
		vec3 origin;
		vec3 direction;
		int spline_geometry_idx{ -1 };
		int sphere_geometry_idx{ -1 };
		debug_value_config_ray config;
		debug_value_ray(vec3 origin, vec3 direction) : origin(origin), direction(direction) {}
	};
	/// Cylinder primitive has a vec3 origin and a vec3 direction (also defining the length) as well as a float radius. It is rendered using the spline renderer.
	struct debug_value_cylinder : debug_value
	{
		vec3 origin;
		vec3 direction;
		float radius;
		int spline_geometry_idx{ -1 };
		debug_value_config_cylinder config;
		debug_value_cylinder(vec3 origin, vec3 direction, float radius) : origin(origin), direction(direction), radius(radius) {}
	};
	/// Box primitive has a vec3 origin and a vec3 extent along the three main coordinate axes. It is rendered using the box renderer.
	struct debug_value_box : debug_value
	{
		vec3 origin;
		vec3 extent;
		int box_geometry_idx{ -1 };
		debug_value_config_box config;
		debug_value_box(vec3 origin, vec3 extent) : origin(origin), extent(extent) {}
	};
	/// List of reusable handle indices
	std::vector<int> unused_handles;
	/// List of registered debug primitives
	std::unordered_map<int, debug_value*> debug_values;
	/// Register a debug primitive to visualize
	int register_debug_value(debug_value* value);
	/// Retrieve a reference to the debug primitive with the given handle
	template<typename T>
	T* retrieve_debug_value(int handle);

	// Geometry and configuration for the different renderers used to visualize the debug primitives

	std::vector<vec3> arrow_positions;
	std::vector<vec3> arrow_directions;
	std::vector<float> arrow_shaft_radii;
	std::vector<rgb> arrow_colors;
	cgv::render::arrow_render_style ars;

	std::vector<vec3> sphere_positions;
	std::vector<float> sphere_radii;
	std::vector<rgb> sphere_colors;
	cgv::render::sphere_render_style srs;

	std::vector<vec3> box_positions;
	std::vector<vec3> box_extents;
	std::vector<rgb> box_colors;
	cgv::render::box_render_style brs;

	typedef std::pair<std::vector<vec3>, std::vector<vec4>> spline_data_t;
	std::vector<spline_data_t> splines;
	std::vector<float> spline_radii;
	std::vector<rgb> spline_colors;
	cgv::render::spline_tube_render_style strs;

	std::vector<rgb> coordinate_system_arrow_colors{ rgb(1.0, 0.0, 0.0), rgb(0.0, 1.0, 0.0), rgb(0.0, 0.0, 1.0) };

	/// Create and set geometry for an arrow rendering primitive
	void construct_arrow(int& idx, vec3 position, vec3 direction, float shaft_radius, rgb color);
	/// Create and set geometry for a sphere rendering primitive
	void construct_sphere(int& idx, vec3 position, float radius, rgb color);
	/// Create and set geometry for a box rendering primitive
	void construct_box(int& idx, vec3 position, vec3 extent, rgb color);
	/// Create and set geometry for a spline rendering primitive
	void construct_spline(int& idx, std::vector<vec3> positions, std::vector<vec4> tangents, float radius, rgb color);

	/// Create and set required rendering geometry for an enabled position debug primitive
	void construct_position_geometry(debug_value_position* debug_value);
	/// Create and set required rendering geometry for an enabled vector debug primitive
	void construct_vector_geometry(debug_value_vector* debug_value);
	/// Create and set required rendering geometry for an enabled coordinate system debug primitive
	void construct_coordinate_system_geometry(debug_value_coordinate_system* debug_value);
	/// Create and set required rendering geometry for an enabled ray debug primitive
	void construct_ray_geometry(debug_value_ray* debug_value);
	/// Create and set required rendering geometry for an enabled cylinder debug primitive
	void construct_cylinder_geometry(debug_value_cylinder* debug_value);
	/// Create and set required rendering geometry for an enabled box debug primitive
	void construct_box_geometry(debug_value_box* debug_value);

	/// Clear and recreate all rendering geometry for all registered and enabled debug primitives
	void reconstruct_geometry();
public:
	void manage_singleton(render::context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change);

	/// Register a positional debug primitive (typically visualized as a sphere) with a starting value, returns its handle.
	int register_debug_value_position(vec3 value);
	/// Register a positional debug primitive (typically visualized as a sphere), returns its handle.
	int register_debug_value_position();
	/// Register a vector debug primitive (direction and length) (typically visualized as an arrow) with a starting value, returns its handle.
	int register_debug_value_vector(vec3 value);
	/// Register a vector debug primitive (direction and length) (typically visualized as an arrow), returns its handle.
	int register_debug_value_vector();
	/// Register a coordinate system debug primitive (mat4 transformation) with a starting value, returns its handle.
	int register_debug_value_coordinate_system(mat4 value);
	/// Register a coordinate system debug primitive (mat4 transformation), returns its handle.
	int register_debug_value_coordinate_system();
	/// Register a ray debug primitive with a starting value, returns its handle.
	int register_debug_value_ray(vec3 origin, vec3 direction);
	/// Register a ray debug primitive, returns its handle.
	int register_debug_value_ray();
	/// Register a cylinder debug primitive with a starting value, returns its handle.
	int register_debug_value_cylinder(vec3 origin, vec3 direction, float radius);
	/// Register a cylinder debug primitive, returns its handle.
	int register_debug_value_cylinder();
	/// Register a box debug primitive with a starting value, returns its handle.
	int register_debug_value_box(vec3 origin, vec3 extent);
	/// Register a box debug primitive, returns its handle.
	int register_debug_value_box();
	/// Deregister (remove) any debug primitive by its handle
	void deregister_debug_value(int handle);

	/// Get current config of position debug primitive. Change, then call the corresponding set config function with the changed config.
	debug_value_config_position get_config_debug_value_position(int handle);
	/// Set config of position debug primitive to given config.
	void set_config_debug_value_position(int handle, debug_value_config_position config);
	/// Get current config of vector debug primitive. Change, then call the corresponding set config function with the changed config.
	debug_value_config_vector get_config_debug_value_vector(int handle);
	/// Set config of vector debug primitive to given config.
	void set_config_debug_value_vector(int handle, debug_value_config_vector config);
	/// Get current config of coordinate system debug primitive. Change, then call the corresponding set config function with the changed config.
	debug_value_config_coordinate_system get_config_debug_value_coordinate_system(int handle);
	/// Set config of coordinate system debug primitive to given config.
	void set_config_debug_value_coordinate_system(int handle, debug_value_config_coordinate_system config);
	/// Get current config of ray debug primitive. Change, then call the corresponding set config function with the changed config.
	debug_value_config_ray get_config_debug_value_ray(int handle);
	/// Set config of ray debug primitive to given config.
	void set_config_debug_value_ray(int handle, debug_value_config_ray config);
	/// Get current config of cylinder debug primitive. Change, then call the corresponding set config function with the changed config.
	debug_value_config_cylinder get_config_debug_value_cylinder(int handle);
	/// Set config of cylinder debug primitive to given config.
	void set_config_debug_value_cylinder(int handle, debug_value_config_cylinder config);
	/// Get current config of box debug primitive. Change, then call the corresponding set config function with the changed config.
	debug_value_config_box get_config_debug_value_box(int handle);
	/// Set config of box debug primitive to given config.
	void set_config_debug_value_box(int handle, debug_value_config_box config);

	/// Enable (show) a debug primitive
	void enable_debug_value_visualization(int handle);
	/// Disable (hide) a debug primitive
	void disable_debug_value_visualization(int handle);

	/// Change the value of a position debug value
	void update_debug_value_position(int handle, vec3 value);
	/// Change the direction value of a vector debug value
	void update_debug_value_vector_direction(int handle, vec3 value);
	/// Change the position value of a vector debug value
	void update_debug_value_vector_position(int handle, vec3 value);
	/// Change the value of a coordinate system debug value
	void update_debug_value_coordinate_system(int handle, mat4 value);
	/// Change the values of a ray debug value
	void update_debug_value_ray(int handle, vec3 origin, vec3 direction);
	/// Change the origin value of a ray debug value
	void update_debug_value_ray_origin(int handle, vec3 origin);
	/// Change the direction value of a ray debug value
	void update_debug_value_ray_direction(int handle, vec3 direction);
	/// Change the values of a cylinder debug value
	void update_debug_value_cylinder(int handle, vec3 origin, vec3 direction, float radius);
	/// Change the origin value of a cylinder debug value
	void update_debug_value_cylinder_origin(int handle, vec3 origin);
	/// Change the direction value of a cylinder debug value
	void update_debug_value_cylinder_direction(int handle, vec3 direction);
	/// Change the radius value of a cylinder debug value
	void update_debug_value_cylinder_radius(int handle, float radius);
	/// Change the values of a box debug value
	void update_debug_value_box(int handle, vec3 origin, vec3 extent);
	/// Change the origin value of a box debug value
	void update_debug_value_box_origin(int handle, vec3 origin);
	/// Change the extent value of a box debug value
	void update_debug_value_box_extent(int handle, vec3 extent);

	/// Corresponds to a drawable::init function. Has to be called by some object inheriting the drawable interface.
	bool init(render::context& ctx);
	/// Corresponds to a drawable::clear function. Has to be called by some object inheriting the drawable interface.
	void clear(render::context& ctx);
	/// Corresponds to a drawable::draw function. Has to be called by some object inheriting the drawable interface.
	void draw(render::context& ctx);

	/// Print sub-hierarchy (scene graph) with given node as root
	static void print_hierarchy(base::node_ptr node);
	/// Print entire hierarchy (scene graph) by first traversing upwards as far as possible
	static void print_full_hierarchy(base::node_ptr node);
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