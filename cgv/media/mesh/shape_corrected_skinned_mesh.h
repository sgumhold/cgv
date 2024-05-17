#pragma once

#include <cgv/media/mesh/dynamic_mesh.h>

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace mesh {

/// @brief Extension of dynamic_mesh that supports regression of joint locations from shaped mesh
/// @tparam T The coordinate base-type
template <typename T = float>
class CGV_API shape_corrected_skinned_mesh : public dynamic_mesh<T>
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
	/// number of blend shapes that define mesh shape
	unsigned nr_shapes = 0;
	/// per joint for each mesh position a weight used to regress the joint locations
	std::vector<std::vector<T>> joint_regressors;
	/// regressed joint locations
	std::vector<vec3> joint_locations;
protected:
	/// storage for positions after applying the shape blend shapes
	std::vector<vec3> shaped_positions;

	void compute_rotation_matrices(const std::vector<vec3>& pose, std::vector<mat3>& rotation_matrices);
	std::vector<T> compute_pose_correction_vector(const std::vector<vec3>& pose, const std::vector<mat3>& rotation_matrices) const;
	void compute_joint_locations(const std::vector<vec3>& P);
public:
	size_t get_nr_shapes() const { return nr_shapes; }
	void shape_mesh(const std::vector<T>& shape, bool use_parallel_implementation = false);
	void pose_mesh(const vec3& translation, const std::vector<vec3>& pose, bool apply_pose_correction = true, bool use_parallel_implementation = false);
};
		}
	}
}

#include <cgv/config/lib_end.h>