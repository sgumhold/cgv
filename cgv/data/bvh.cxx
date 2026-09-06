#include "bvh.h"

#include <numeric>
#include <stack>

#include <cgv/math/intersection.h>

namespace cgv {
namespace data {

void bvh::build(const std::vector<const ray_intersectable*>& primitives, size_t max_depth) {
	_max_depth = max_depth;

	// Store the pointers to the given primitives and calculate the bounding box of all primitives.
	this->_primitives = primitives;
	cgv::box3 bounds;
	for(const ray_intersectable* primitive : primitives)
		bounds.add_axis_aligned_box(primitive->get_bounds());

	// Create the root node containing all primitves and no children.
	_root->bounds = bounds;
	_root->primitive_indices.resize(_primitives.size());
	std::iota(_root->primitive_indices.begin(), _root->primitive_indices.end(), 0);
	_root->child_a.reset();
	_root->child_b.reset();

	// Start recursively building the hierarchy by splitting the root node.
	split(_root.get());
}

void bvh::split(bvh_node* node, int depth) {
	// Stop splitting if the maximum depth is reached
	if(depth == _max_depth)
		return;

	// Determine the split axis as the axis with the largest extent
	const unsigned split_axis = cgv::math::max_index(node->bounds.get_extent());
	const float split_position = node->bounds.get_center()[split_axis];

	node->child_a = std::make_unique<bvh_node>();
	node->child_b = std::make_unique<bvh_node>();
	bvh_node* child_a = node->child_a.get();
	bvh_node* child_b = node->child_b.get();

	// Loop over all primitives in the current node and partition them into the first and second child node based on their center position.
	for(const size_t i : node->primitive_indices) {
		const ray_intersectable* primitive = _primitives[i];
		cgv::box3 primitive_bounds = primitive->get_bounds();
		bool in_a = primitive_bounds.get_center()[split_axis] < split_position;
		bvh_node* child = in_a ? child_a : child_b;
		child->primitive_indices.push_back(i);
		child->bounds.add_axis_aligned_box(primitive_bounds);
	}

	// Clear the primitives from the node to reduce memory consumption
	node->primitive_indices.clear();
	// Recursively split the two child nodes
	split(child_a, depth + 1);
	split(child_b, depth + 1);
}

bvh_result bvh::closest_intersection(const cgv::ray3& ray) const {
	// Keep track of the discovered nodes during depth-first traversal
	std::stack<bvh_node*> node_stack;
	node_stack.push(_root.get());

	// Keep track of the closest intersection distance
	float min_t = std::numeric_limits<float>::max();
	cgv::vec2 bounds_ts;
	bvh_result result;
	ray_intersection_info intersection;

	while(!node_stack.empty()) {
		const bvh_node* node = node_stack.top();
		node_stack.pop();

		// First test if the ray hits the current node's bounding box and only proceed if this is the case.
		if(cgv::math::ray_box_intersection(ray, node->bounds.get_min_pnt(), node->bounds.get_max_pnt(), bounds_ts) > 0) {
			// A node without child nodes indicates a leaf node possibly containing primitives.
			if(!node->child_a && !node->child_b) {
				// Test all primitives inside the current node forintersection with the ray and record the closest inetrsection.
				for(const size_t i : node->primitive_indices) {
					const ray_intersectable* primitive = _primitives[i];
					if(primitive->intersect(ray, intersection)) {
						if(intersection.t > std::numeric_limits<float>::epsilon() && intersection.t < min_t) {
							min_t = intersection.t;
							result.intersection = intersection;
							result.primitive = primitive;
							result.primitive_index = i;
						}
					}
				}
			} else {
				// If this is not a leaf node push its two child nodes onto the stack to proceeed traversing the hierarchy.
				node_stack.push(node->child_a.get());
				node_stack.push(node->child_b.get());
			}
		}
	}

	// A primitive was hit if the resulting intersection has a valid distance.
	if(result.intersection.t < std::numeric_limits<float>::max())
		result.is_hit = true;

	return result;
}

} // namespace data
} // namespace cgv
