#pragma once

#include <cgv/media/mesh/simple_mesh.h>

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace mesh {

/// @brief The dynamic_mesh provides interpolation between several blend shapes of a mesh.
/// @tparam T The coordinate base-type
template <typename T = float>
class CGV_API dynamic_mesh : public simple_mesh<T>
{
public:
	using typename simple_mesh_base::idx_type;
	using typename simple_mesh_base::vec2i;
	using typename simple_mesh_base::vec3i;
	using typename simple_mesh_base::vec4i;
	using typename simple_mesh_base::mat_type;
	using typename simple_mesh<T>::vec2;
	using typename simple_mesh<T>::vec3;
	using typename simple_mesh<T>::vec4;
	using typename simple_mesh<T>::box_type;
	using typename simple_mesh<T>::clr_type;
	using typename simple_mesh<T>::mesh_type;
	using typename simple_mesh<T>::mat3;
	using typename simple_mesh<T>::mat4;
	
	/// @brief specifies how the blend shapes are stored
	enum class blend_shape_mode {
		/// All blend shapes have a uniform dimension and stored successively
		direct,
		/// Blend shapes index into a shared data buffer
		indexed,
		/// If the indices are numerically sorted then this mode allows to refer to respective ranges via a begin and end pair
		range_indexed
	};

  protected:
	/// Storage for vertex positions in the bind-pose
	std::vector<vec3> reference_positions;
	/// Storage for vertex positions after Linear Blend Skinning
	std::vector<vec3> intermediate_positions;
	/// Storage for the blend shape vertices
	std::vector<vec3> blend_shape_data;
	/// Storage for the indices (if the blend shapes are defined by indices)
	std::vector<uint32_t> blend_shape_indices;
	/// @brief Captures where and how the blandshape data is stored
	struct blend_shape
	{
		/// How the blendshape is stored
		blend_shape_mode mode;
		/// The range of indices (indices into a std::vector!) which hold the blendshape vertices
		vec2i blend_shape_data_range;
		/// The range of indices (indices into a std::vector!) which hold the indices into the data-buffer
		vec2i blend_shape_index_range;
	};
	/// @brief Storage for all the blendshape definitions
	std::vector<blend_shape> blend_shapes;

public:
	/// @brief specifies how vertex weights are stored
	enum class vertex_weight_mode {
		/// number joints times number vertices weights are stored - no indices necessary
		dense,
		/// per vertex varying number of indexed weights - vertex_weight_index_begins tells for
		/// each vertex the index of the first weight in vertex_weight_data,
		/// which is also the index of the first vertex index in vertex_weight_indices
		sparse,
		/// max_nr_weights_per_vertex are stored for each vertex with max_nr_weights_per_vertex
		/// times the number of vertices entries in vertex_weight_data and
		/// vertex_weight_indices.vertex_weight_index_begins is not used.If less than max_nr_weights_per_vertex are used
		/// the additional weights are set to 0.0 and
		/// the additional indices to 0.
		fixed
	};

protected:
	/// How the vertex weights are stored for this instance
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
	/// @brief Add a new blend shape
	/// @param[in] mode			How the blend shape data is stored and referenced
	/// @param[in] nr_data		How many data_points this blend shape will have
	/// @param[in] nr_indices	How many indices this blend shape will use
	/// @return the index of the newly created blend shape
	uint32_t add_blend_shape(blend_shape_mode mode, idx_type nr_data, idx_type nr_indices = 0);
	/// @brief Add another data point to the data buffer
	void add_blend_shape_data(const vec3& d);
	/// @brief Add another index to the index buffer
	void add_blend_shape_index(idx_type i);
	/// @brief Query if a blend shape has a vertex with a given ID
	/// @param[in] bi	Index of the desired blend shape
	/// @param[in] vi	Index of the desired vertex
	bool has_blend_shape_vector(idx_type bi, idx_type vi) const;
	/// @brief Returns one vertex of one blend shape
	/// @param[in] bi	Index of the desired blend shape
	/// @param[in] vi	Index of the desired vertex
	vec3 get_blend_shape_vector(idx_type bi, idx_type vi) const;
	/// @brief Return how many blend shapes this mesh has
	size_t get_nr_blend_shapes() const;
	//! this function applies weights.size() number of blend shapes starting at offset blend_shape_offset and stores result in mesh position attribute
	/*! If only_add is false, the position is initialized to the reference_position attribute - otherwise the
		blend shapes are just added to the current mesh position attribute. */
	/// @brief Performs a weighted accumulation of blend shapes.
	/// @param[in] weights the respective weights for each blendshape
	/// @param[in] blend_shape_offset relative to the first blend shape, where to begin applying the weighted accumulation
	/// @param[in] only_add If false then \ref cgv::media::mesh::simple_mesh "simple_mesh::positions" will be initialized to the reference_position attribute. Otherwise the
	/// blend shapes are just added to the current mesh position attribute.
	void apply_blend_shapes(const std::vector<T>& weights, idx_type blend_shape_offset = 0, bool only_add = false, bool use_parallel_implementation = false);
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
	/// @brief Which source to use for the Linear Blend Skinning
	enum class lbs_source_mode {
		/// The reference positions
		reference,
		/// The current position attribute of the mesh
		position,
		/// The intermediate position attribute of the mesh
		intermediate };
	//! perform linear blend skinning on reference positions or the current mesh position attribute
	/*! the joint matrices define per joint the transformation from reference positions or intermediate positions.*/
	void lbs(const std::vector<mat4>& joint_matrices, lbs_source_mode mode);
	//@}
};
		}
	}
}

#include <cgv/config/lib_end.h>