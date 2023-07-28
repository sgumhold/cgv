#pragma once

#include <cgv/media/mesh/simple_mesh.h>

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace mesh {

/// the simple_mesh class is templated over the coordinate type that defaults to float
template <typename T = float>
class CGV_API dynamic_mesh : public simple_mesh<T>
{
public:
	 enum class blend_shape_mode { direct, indexed, range_indexed };
protected:
	std::vector<vec3>     reference_positions, intermediate_positions;
	std::vector<vec3>     blend_shape_data;
	std::vector<uint32_t> blend_shape_indices;
	struct blend_shape
	{
		blend_shape_mode mode;
		vec2i blend_shape_data_range;
		vec2i blend_shape_index_range;
	};
	std::vector<blend_shape> blend_shapes;

public:
	 enum class vertex_weight_mode { dense, sparse, fixed };
protected:
	/** mode specifies how vertex weights are stored :
	*   dense  ... number joint times number vertices weights are stored - no indices necessary
	*   sparse ... per vertex varying number of indexed weights - vertex_weight_index_begins tells for
	               each vertex the index of the first weight in vertex_weight_data, which is also the
				   index of the first vertex index in vertex_weight_indices 
		fixed  ... max_nr_weights_per_vertex are stored for each vertex with max_nr_weights_per_vertex
		           times the number of vertices entries in vertex_weight_data and vertex_weight_indices.
				   vertex_weight_index_begins is not used. If less than max_nr_weights_per_vertex are
				   used the additional weights are set to 0.0 and the additional indices to 0. */
	vertex_weight_mode weight_mode;
	/// in case of fixed vertex_weight_mode the number of weights per vertex
	int32_t          max_nr_weights_per_vertex = -1;
	/// for each joint the index of the parent joint or -1 for root joints
	std::vector<int32_t>  joint_parents;
	/// continuous storage of all vertex weights
	std::vector<T>        vertex_weight_data;
	/// continuous storage of vertex indices
	std::vector<uint32_t> vertex_weight_indices;
	/// for each vertex the first index in vertex_weight_data and vertex_weight_indices
	std::vector<uint32_t> vertex_weight_index_begins;
public:
	/**@name additional positional attributes */
	//@{
	/// store current positions in reference positions
	void store_in_reference_positions();
	/// store current positions in intermediate positions
	void store_in_intermediate_positions();
	/// return const reference to intermediate position vector
	const std::vector<vec3>& get_intermediate_positions() const;
	/// copy intermediate positions to positions
	void recover_intermediate_positions();
	//@}

	/**@name blend shapes*/
	//@{
	/// add a new blend shape
	uint32_t add_blend_shape(blend_shape_mode mode, idx_type nr_data, idx_type nr_indices = 0);
	///
	void add_blend_shape_data(const vec3& d);
	///
	void add_blend_shape_index(idx_type i);
	///
	bool has_blend_shape_vector(idx_type bi, idx_type vi) const;
	///
	vec3 get_blend_shape_vector(idx_type bi, idx_type vi) const;
	///
	size_t get_nr_blend_shapes() const;
	//! this function applies weights.size() number of blend shapes starting at offset blend_shape_offset and stores result in mesh position attribute
	/*! If only_add is false, the position is initialized to the reference_position attribute - otherwise the
		blend shapes are just added to the current mesh position attribute. */
	void apply_blend_shapes(const std::vector<T>& weights, idx_type blend_shape_offset = 0, bool only_add = false);
	//@}

	/**@name skinning*/
	//@{
	///
	void set_vertex_weight_mode(vertex_weight_mode mode);
	///
	void begin_vertex_weight_vertex();
	/// return the begin index for vertex weights of given vertex
	idx_type vertex_weight_begin(idx_type vi) const;
	/// return the end index for vertex weights of given vertex
	idx_type vertex_weight_end(idx_type vi) const;
	/// return the vertex weight of vertex vi and joint ji
	T get_vertex_weight(idx_type vi, idx_type ji) const;
	///
	void add_vertex_weight_data(T w);
	///
	void add_vertex_weight_index(idx_type i);
	///
	size_t get_nr_vertex_weights() const;
	///
	size_t get_nr_vertex_weight_indices() const;
	///
	idx_type get_nr_joints() const;
	/// compute joint transformations from reference joint locations, target translation and target spin vectors
	std::vector<mat4> compute_joint_transformations(const std::vector<vec3>& reference_joint_locations,
		const vec3& translation, const std::vector<vec3>& target_spin_vectors) const;
	/// compute joint transformations from reference joint locations, target translation and target rotation matrices
	std::vector<mat4> compute_joint_transformations(const std::vector<vec3>& reference_joint_locations,
		const vec3& translation, const std::vector<mat3>& target_rotations) const;
	/// return const reference to joint parent vector
	const std::vector<int32_t>& get_joint_parents() const;
	/// return mutable reference to joint parent vector
	std::vector<int32_t>& ref_joint_parents();
	/// different sources to compute lbs
	enum class lbs_source_mode { reference, position, intermediate };
	//! perform linear blend skinning on reference positions or the current mesh position attribute
	/*! the joint matrices define per joint the transformation from reference positions or intermediate positions.*/
	void lbs(const std::vector<mat4>& joint_matrices, lbs_source_mode mode);
	//@}
};
		}
	}
}

#include <cgv/config/lib_end.h>