#pragma once

#include <limits>
#include <memory>
#include <vector>

#include <cgv/math/fray.h>
#include <cgv/math/fvec.h>
#include <cgv/media/axis_aligned_box.h>

#include "lib_begin.h"

namespace cgv {
namespace data {

/// Holds information about a single intersection of a ray and primitive.
struct ray_intersection_info {
	/// The ray parameter
	float t = std::numeric_limits<float>::max();
	/// The intersection surface normal
	cgv::vec3 normal;
	/// The intersection surface texture coordinates
	cgv::vec2 uv;
};

/// An interface to define objects that are intersectable with a ray.
class ray_intersectable {
public:
	ray_intersectable() {}
	virtual ~ray_intersectable() {}

	/// @brief Return the axis-aligned bounding box of the primitive.
	virtual cgv::box3 get_bounds() const = 0;

	/// @brief Test if the object is intersected by the given ray.
	/// The info struct collects additional information about the intersection.
	/// The ray parameter t must be set to ensure correct behaviour in any potential outside traversal procedures.
	/// The remaining data is optional and may be set as available by the primitive and needed by the caller site.
	/// 
	/// @param ray The ray to intersect with.
	/// @param info The ray_intersection_info to collect additional intersection data.
	/// @return True if the object is intersected by the ray; false otherwise.
	virtual bool intersect(const cgv::ray3& ray, ray_intersection_info& info) const = 0;
};

/// A single node of a bounding volume hierarchy.
/// Each node may hold pointers to two child nodes.
struct bvh_node {
	/// The node's axis aligned bounding box
	cgv::box3 bounds = {};
	/// The list of primitive indices contained in this node. After building the tree, only leaf nodes will have a non-empty list.
	std::vector<size_t> primitive_indices;
	/// The first child node of this node
	std::unique_ptr<bvh_node> child_a;
	/// The second child node of this node
	std::unique_ptr<bvh_node> child_b;
};

/// The result of a BVH intersection test.
/// The members may only contain valid data if is_hit is true.
struct bvh_result {
	/// True if a primitive was hit.
	bool is_hit = false;
	/// Intersection information of the hit
	ray_intersection_info intersection;
	/// The index of the hit primitive as given during BVH construction
	size_t primitive_index = std::numeric_limits<size_t>::max();
	/// A pointer to the hit primitive
	const ray_intersectable* primitive = nullptr;
};

/**
 * A Bounding Volume Hierarchy(BVH) using axis-aligned bounding boxes to partition primitives into a binary tree structure for efficient ray intersection tests.
 * Nodes are split using the midpoint heuristic that splits primitives at the geometric center point of a node's longest axis, sorting primitives based on their bounding box centroid.
 * 
 * Example:
 * Create a custom primitive class usable with the BVH:
 * 
 * struct my_primitive : public ray_intersectable {
 *		... custom fields
 *		
 *		cgv::box3 get_bounds() const override {
 *			...return axis-aligned bounding box of the primitive
 *		};
 * 
 *		bool intersect(const cgv::ray3& ray, ray_intersection_info& info) const override {
 *			...return true if ray intersects primitive and fill out intersection info with at least the ray parameter t
 *		};
 * };
 * 
 * Create some primitives:
 * 
 * std::vector<ray_intersectable*> primitives;
 * primitives.push_back(new my_primitive(...));
 * ...
 * 
 * Attention: The call-site must manage the lifetime of the primitives!
 * 
 * To build the BVH:
 * bvh scene;
 * scene.build(primitives, 8) // using depth 8 (sensible values depend on the scene complexity but are typically between 4 and 32)
 * 
 * To test for a closest intersection:
 * 
 * cgv::ray3 ray(...); // some test ray
 * bvh_result result = scene.closest_intersection(ray);
 * 
 * if(result.is_hit) {
 *		// A primitive was hit. result.intersection contains the ray parameter of the intersection point and other optional attributes
 *		// result further contains the pointer to the hit primitive and the index into the 'primitives' list used to build the BVH
 *      const my_primitive* hit_primitive = result.primitive;
 *		...or
 *		my_primitive* hit_primitive = primitives[result.primitive_index];
 * } else {
 *		...no hit
 * }
 * 
 */
class CGV_API bvh {
public:
	/// @brief Build the hierarchy over the given primitives.
	/// Attention: The primitives are merely borrowed during the build process and afterwards by the created nodes.
	/// The call site must ensure the primitives' lifetime as long as the bvh is used. Modifying any primitive
	/// attributes that alter its bounding box after building the BVH results in undefined behaviour. The BVH
	/// must be built again after modifying such attributes to ensure correct calculation of intersections.
	/// 
	/// @param primitives The primitives to consider during building.
	/// @param max_depth The maximum depth of the resulting tree.
	void build(const std::vector<const ray_intersectable*>& primitives, size_t max_depth = 8);

	/// @brief Calculate the closest intersection with positive distance, if any, of the given ray with the stored primitives.
	/// @param ray The ray to test.
	/// @return The intersection result.
	bvh_result closest_intersection(const cgv::ray3& ray) const;

private:
	// Keep a list of primitives inside the hierarchy. The nodes contain indices that point to this list.
	std::vector<const ray_intersectable*> _primitives;
	/// The maximum depth of the tree.
	int _max_depth = 8;
	/// The root node of the hierarchy
	std::unique_ptr<bvh_node> _root = std::make_unique<bvh_node>();

	/// @brief Split a bvh_node by creating its children.
	/// The node is split recursively until the maximum depth is reached or it contains no primitives.
	/// 
	/// @param node The node to split.
	/// @param depth The depth of the given node. Used to test against the maximum depth.
	void split(bvh_node* node, int depth = 0);
};

} // namespace data
} // namespace cgv

#include <cgv/config/lib_end.h>
