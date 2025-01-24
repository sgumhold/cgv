#pragma once

#include <cgv/media/mesh/dynamic_mesh.h>

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace mesh {

/// @brief Extension of dynamic_mesh that supports regression of joint locations from shaped mesh
/// 			as well as pose dependent shape corrections for which the entries of the joint
/// 			rotation matrices are used
/// @tparam T The coordinate base-type
template <typename T = float>
class CGV_API adaptive_skinned_mesh : public dynamic_mesh<T>
{
public:
	using typename simple_mesh_base::idx_type;
	using typename simple_mesh_base::idx2_type;
	using typename simple_mesh_base::idx3_type;
	using typename simple_mesh_base::idx4_type;
	using typename simple_mesh_base::mat_type;
	using typename simple_mesh<T>::vec2_type;
	using typename simple_mesh<T>::vec3_type;
	using typename simple_mesh<T>::mesh_type;
	using typename simple_mesh<T>::mat3_type;
	using typename dynamic_mesh<T>::lbs_source_mode;
	/// number of blend shapes that define mesh shape
	unsigned nr_shapes = 0;
	/// per joint for each mesh position a weight used to regress the joint locations
	std::vector<std::vector<T>> joint_regressors;
	/// alternative representation of joint regressors with one vector per joint and base mesh/shape
	std::vector<std::vector<vec3_type>> alternative_joint_regressors;
	/// regressed joint locations
	std::vector<vec3_type> joint_locations;
protected:
	/// storage for positions after applying the shape blend shapes
	std::vector<vec3_type> shaped_positions;
	/// convertex vector of spin vectors resembling the pose into a vector of rotation matrices
	void compute_rotation_matrices(const std::vector<vec3_type>& pose, std::vector<mat3_type>& rotation_matrices);
	/// given vector of rotation matrices, compute vector of pose correction weights by skipping matrix of base joint, subtracting identity matrix and flatten transpose of resulting 3x3 matrices
	std::vector<T> compute_pose_correction_vector(const std::vector<mat3_type>& rotation_matrices) const;
	/// compute the alternative joint regressors after shape blend shapes have been defined
	void compute_alternative_joint_regressors();
	/// given the vertex locations after applying the shape blend shapes, compute the member variable \c joint_locations
	void compute_joint_locations(const std::vector<vec3_type>& P);
public:
	/// return the number of blend shapes used to for shape adaptation
	size_t get_nr_shapes() const { return nr_shapes; }
	/// return number of pose correction blend shapes
	size_t get_nr_pose_corrections() const { return this->get_nr_blend_shapes() - get_nr_shapes(); }
	/// given shape weights, compute the rest pose shaped mesh vertex locations into the dynamic_mesh<T> member \c shaped_positions
	void shape_mesh(const std::vector<T>& shape, bool use_parallel_implementation = false);
	/// starting with vertex position in member \c shaped_positions, optionally apply pose correction, perform linear blend skinning and finally recompute surface normals
	void pose_mesh(const vec3_type& translation, const std::vector<vec3_type>& pose, bool apply_pose_correction = true, bool apply_lbs = true, bool use_parallel_implementation = false);
};
		}
	}
}

#include <cgv/config/lib_end.h>