#include "adaptive_skinned_mesh.h"
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace media {
		namespace mesh {

template <typename T>
void adaptive_skinned_mesh<T>::compute_rotation_matrices(const std::vector<vec3>& pose, std::vector<mat3>& rotation_matrices)
{
	// convert pose (excluding global orientation) to flattened vector of rotation matrices
	rotation_matrices.clear();
	for (unsigned ji = 0; ji < pose.size(); ++ji) {
		auto R = rotate3s(pose[ji]);
//		if (transpose_rotations)
//			R = transpose(R);
		rotation_matrices.push_back(R);
	}
}

template <typename T>
void adaptive_skinned_mesh<T>::compute_joint_locations(const std::vector<vec3>& P)
{
	joint_locations.resize(joint_regressors.size());
	std::fill(joint_locations.begin(), joint_locations.end(), cgv::vec3(0.0f));
	for (unsigned vi = 0; vi < P.size(); ++vi) {
		for (size_t ji = 0; ji < joint_regressors.size(); ++ji)
			joint_locations[ji] += joint_regressors[ji][vi] * P[vi];
	}
}

template <typename T>
void adaptive_skinned_mesh<T>::compute_aternate_joint_regressors()
{
	alternate_joint_regressors.resize(joint_regressors.size());
	/*std::fill(joint_locations.begin(), joint_locations.end(), cgv::vec3(0.0f));
	for (unsigned vi = 0; vi < P.size(); ++vi) {
		for (size_t ji = 0; ji < joint_regressors.size(); ++ji)
			joint_locations[ji] += joint_regressors[ji][vi] * P[vi];
	}
	*/
}

template <typename T>
std::vector<T> adaptive_skinned_mesh<T>::compute_pose_correction_vector(const std::vector<vec3>& pose, const std::vector<mat3>& rotation_matrices) const
{
	// flattened vector of rotation matrices
	std::vector<T> PCs;
	for (unsigned ji = 1; ji < pose.size(); ++ji) {
		cgv::mat3 PC = rotation_matrices[ji] - cgv::math::identity3<T>();
	//	if (transpose_corrections != transpose_rotations)
		PC = transpose(PC);
		for (unsigned i = 0; i < 9; ++i)
			PCs.push_back(PC[i]);
	}
	return PCs;
}

template <typename T>
void adaptive_skinned_mesh<T>::shape_mesh(const std::vector<T>& shape, bool use_parallel_implementation)
{
	this->apply_blend_shapes(shape, 0, false, use_parallel_implementation);
	shaped_positions = this->positions;
	compute_joint_locations(shaped_positions);
}

template <typename T>
void adaptive_skinned_mesh<T>::pose_mesh(const vec3& translation, const std::vector<vec3>& pose, bool apply_pose_correction, bool use_parallel_implementation)
{
	std::vector<mat3> rotation_matrices;
	compute_rotation_matrices(pose, rotation_matrices);
	// on pose change apply pose correction to mesh positions
	this->positions = shaped_positions;
	if (apply_pose_correction)
		apply_blend_shapes(compute_pose_correction_vector(pose, rotation_matrices), nr_shapes, true, use_parallel_implementation);
	this->lbs(compute_joint_transformations(joint_locations, translation, rotation_matrices), lbs_source_mode::position);
	this->compute_vertex_normals(use_parallel_implementation);
}


template class adaptive_skinned_mesh<float>;

		}
	}
}