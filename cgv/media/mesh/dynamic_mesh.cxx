#include "dynamic_mesh.h"

#include <cgv/math/ftransform.h>
#include <cgv/math/inv.h>

namespace cgv {
	namespace media {
		namespace mesh {

template <typename T>
uint32_t dynamic_mesh<T>::add_blend_shape(blend_shape_mode mode, idx_type nr_data, idx_type nr_indices)
{
	blend_shape bs = { mode, vec2i(idx_type(blend_shape_data.size()),idx_type(blend_shape_data.size()+nr_data)),
		vec2i(idx_type(blend_shape_indices.size()), idx_type(blend_shape_indices.size()+nr_indices)) };
	blend_shapes.push_back(bs);
	return uint32_t(blend_shapes.size() - 1);
}
template <typename T>
void dynamic_mesh<T>::add_blend_shape_data(const vec3& d)
{
	blend_shape_data.push_back(d);
}
template <typename T>
void dynamic_mesh<T>::add_blend_shape_index(idx_type i)
{
	blend_shape_indices.push_back(i);
}
template <typename T>
bool dynamic_mesh<T>::has_blend_shape_vector(idx_type bi, idx_type vi) const
{
	const auto& bs = blend_shapes[bi];
	switch (bs.mode) {
	case blend_shape_mode::direct:
		/// If the blend shape data is stored in blend_shape_mode::direct mode, this function will always return true as
		/// there is no indirection.
		return true;
	case blend_shape_mode::indexed:
		for (idx_type i = bs.blend_shape_index_range[0]; i < bs.blend_shape_index_range[1]; ++i)
			if (blend_shape_indices[i] == vi)
				return true;
		break;
	case blend_shape_mode::range_indexed: 
		for (idx_type i = bs.blend_shape_index_range[0]; i < bs.blend_shape_index_range[1]; i+=2)
			if (blend_shape_indices[i] <= vi && vi < blend_shape_indices[i+1])
				return true;
		break;
	}
	return false;
}
template <typename T>
typename dynamic_mesh<T>::vec3 dynamic_mesh<T>::get_blend_shape_vector(idx_type bi, idx_type vi) const
{
	const auto& bs = blend_shapes[bi];
	switch (bs.mode) {
	case blend_shape_mode::direct: 
		return blend_shape_data[bi*this->get_nr_positions()+vi];
	case blend_shape_mode::indexed:
		for (idx_type i = bs.blend_shape_index_range[0]; i < bs.blend_shape_index_range[1]; ++i)
			if (blend_shape_indices[i] == vi)
				return blend_shape_data[i];
		break;
	case blend_shape_mode::range_indexed: 
		for (idx_type off = 0, i = bs.blend_shape_index_range[0]; i < bs.blend_shape_index_range[1];
												off += blend_shape_indices[i+1]-blend_shape_indices[i], i+=2)
			if (blend_shape_indices[i] <= vi && vi < blend_shape_indices[i+1])
				return blend_shape_data[off + vi- blend_shape_indices[i]];
		break;
	}
	return vec3(T(0));
}
template <typename T>
size_t dynamic_mesh<T>::get_nr_blend_shapes() const
{
	return blend_shapes.size();
}
template <typename T>
void dynamic_mesh<T>::set_vertex_weight_mode(vertex_weight_mode mode)
{
	weight_mode = mode;
}
template <typename T>
void dynamic_mesh<T>::begin_vertex_weight_vertex()
{
	vertex_weight_index_begins.push_back(uint32_t(vertex_weight_data.size()));
}
template <typename T>
typename dynamic_mesh<T>::idx_type dynamic_mesh<T>::vertex_weight_begin(idx_type vi) const
{
	switch (weight_mode) {
	case vertex_weight_mode::dense:
		return uint32_t(vi * get_nr_joints());
	case vertex_weight_mode::sparse:
		return vertex_weight_index_begins[vi];
	case vertex_weight_mode::fixed:
		return uint32_t(vi * max_nr_weights_per_vertex);
	}
	return -1;
}
template <typename T>
typename dynamic_mesh<T>::idx_type dynamic_mesh<T>::vertex_weight_end(idx_type vi) const
{
	switch (weight_mode) {
	case vertex_weight_mode::dense:
		return uint32_t((vi+1)*get_nr_joints());
	case vertex_weight_mode::sparse:
		return uint32_t(vi >= vertex_weight_index_begins.size() ? vertex_weight_data.size() : vertex_weight_index_begins[vi]);
	case vertex_weight_mode::fixed:
		return uint32_t((vi+1)*max_nr_weights_per_vertex);
	}
	return -1;
}
template <typename T>
T dynamic_mesh<T>::get_vertex_weight(idx_type vi, idx_type ji) const
{
	switch (weight_mode) {
	case vertex_weight_mode::dense:
		return vertex_weight_data[vi * get_nr_joints() + ji];
	default:
		for (idx_type i = vertex_weight_begin(vi); i < vertex_weight_end(vi); ++i)
			if (vertex_weight_indices[i] == ji)
				return vertex_weight_data[i];
		break;
	}
	return T(0);
}

template <typename T>
void dynamic_mesh<T>::add_vertex_weight_data(T w)
{
	vertex_weight_data.push_back(w);
}
template <typename T>
void dynamic_mesh<T>::add_vertex_weight_index(idx_type i)
{
	vertex_weight_indices.push_back(i);
}
template <typename T>
size_t dynamic_mesh<T>::get_nr_vertex_weights() const
{
	return vertex_weight_data.size();
}
template <typename T>
size_t dynamic_mesh<T>::get_nr_vertex_weight_indices() const
{
	return vertex_weight_indices.size();
}
template <typename T>
typename dynamic_mesh<T>::idx_type dynamic_mesh<T>::get_nr_joints() const
{
	return idx_type(joint_parents.size());
}
template <typename T>
std::vector<typename dynamic_mesh<T>::mat4> dynamic_mesh<T>::compute_joint_transformations(const std::vector<vec3>& reference_joint_locations,
	const vec3& translation, const std::vector<vec3>& target_spin_vectors) const
{
	std::vector<mat3> target_rotations;
	for (const auto& sv : target_spin_vectors)
		target_rotations.push_back(cgv::math::rotate3s<T>(sv));
	return compute_joint_transformations(reference_joint_locations, translation, target_rotations);
}
template <typename T>
std::vector<typename dynamic_mesh<T>::mat4> dynamic_mesh<T>::compute_joint_transformations(const std::vector<vec3>& reference_joint_locations,
	const vec3& translation, const std::vector<mat3>& target_rotations) const
{
	// compute joint transformations in rest pose and in target pose
	std::vector<mat4> Is, Ts;
	mat4 I = cgv::math::identity4<float>();
	vec3& t = reinterpret_cast<vec3&>(I.col(3));
	t = reference_joint_locations.front();
	Is.push_back(I);
	Ts.push_back(pose4(target_rotations.front(), t + translation));
	unsigned ji;
	for (ji = 1; ji < target_rotations.size(); ++ji) {
		t = reference_joint_locations[ji] - reference_joint_locations[joint_parents[ji]];
		Is.push_back(I);
		Ts.push_back(pose4(target_rotations[ji], t));
	}
	// concatenate poses
	for (ji = 1; ji < target_rotations.size(); ++ji) {
		Is[ji] = Is[joint_parents[ji]] * Is[ji];
		Ts[ji] = Ts[joint_parents[ji]] * Ts[ji];
	}
	// compute final rest pose to pose transformations
	for (ji = 0; ji < target_rotations.size(); ++ji)
		Ts[ji] = Ts[ji] * inv(Is[ji]);

	return Ts;
}


template <typename T>
void dynamic_mesh<T>::apply_blend_shapes(const std::vector<T>& weights, idx_type blend_shape_offset, bool only_add, bool use_parallel_implementation)
{
	if (!only_add)
		this->positions = reference_positions;
	if (use_parallel_implementation) {
		// here we assume that all blend shapes have mode direct and address whole mesh

		// first extract per blend shape pointers to the blend shape data start
		std::vector<const vec3*> bs_ptrs;
		int n = this->get_nr_positions();
		for (size_t bi = blend_shape_offset; bi < weights.size() + blend_shape_offset; ++bi) {
			const auto& bs = blend_shapes[bi];
			assert(bs.mode == blend_shape_mode::direct);
			assert(bs.blend_shape_data_range[1] - bs.blend_shape_data_range[0] == n);
			bs_ptrs.push_back(&blend_shape_data[bs.blend_shape_data_range[0]]);
		}
		// next apply them to mesh positions
#pragma omp parallel for
		for (int vi = 0; vi < n; ++vi) {
			vec3 p(T(0));
			for (int wi = 0; wi < weights.size(); ++wi)
				p += weights[wi]*bs_ptrs[wi][vi];
			this->position(vi) += p;
		}
	}
	else {
		for (idx_type bi = blend_shape_offset, wi = 0; wi < weights.size(); ++wi, ++bi) {
			const auto& bs = blend_shapes[bi];
			switch (bs.mode) {
			case blend_shape_mode::direct:
				for (int j=0, i = bs.blend_shape_data_range[0]; i < int(bs.blend_shape_data_range[1]); ++i, ++j)
					this->positions[j] += weights[wi] * blend_shape_data[i];
				break;
			case blend_shape_mode::indexed:
				for (uint32_t i = bs.blend_shape_data_range[0], j = bs.blend_shape_index_range[0]; i < bs.blend_shape_data_range[1]; ++i, ++j)
					this->positions[blend_shape_indices[j]] += weights[wi] * blend_shape_data[i];
				break;
			case blend_shape_mode::range_indexed:
				for (uint32_t i = bs.blend_shape_data_range[0], j = bs.blend_shape_index_range[0]; j < bs.blend_shape_index_range[1]; j += 2)
					for (uint32_t k = blend_shape_indices[j]; k < blend_shape_indices[j + 1]; ++k, ++i)
						this->positions[k] += weights[wi] * blend_shape_data[i];
				break;
			}
		}
	}
}
template <typename T>
const std::vector<typename dynamic_mesh<T>::vec3>& dynamic_mesh<T>::get_intermediate_positions() const
{
	return intermediate_positions;
}

template <typename T>
const std::vector<int32_t>& dynamic_mesh<T>::get_joint_parents() const
{
	return joint_parents;
}
template <typename T>
std::vector<int32_t>& dynamic_mesh<T>::ref_joint_parents()
{
	return joint_parents;
}
template <typename T>
void dynamic_mesh<T>::store_in_reference_positions()
{
	reference_positions = this->positions;
}
template <typename T>
void dynamic_mesh<T>::store_in_intermediate_positions()
{
	intermediate_positions = this->positions;
}
template <typename T>
void dynamic_mesh<T>::recover_intermediate_positions()
{
	this->positions = intermediate_positions;
}

template <typename T>
void dynamic_mesh<T>::lbs(const std::vector<mat4>& joint_matrices, lbs_source_mode mode)
{
	std::vector<vec3> tmp;
	if (mode == lbs_source_mode::position)
		tmp = this->positions;
	const std::vector<vec3>& P = mode == lbs_source_mode::position ? tmp : (
		mode == lbs_source_mode::intermediate ? intermediate_positions : reference_positions);
	switch (weight_mode) {
	case vertex_weight_mode::dense:
		for (size_t pi = 0, wi = 0; pi < P.size(); ++pi) {
			vec4 p = vec4(P[pi],1.0f);
			vec4 q = vec4(0.0f);
			for (unsigned ji = 0; ji < joint_parents.size(); ++ji, ++wi)
				q += vertex_weight_data[wi] * (joint_matrices[ji] * p);
			this->position(idx_type(pi)) = q;
		}
		break;
	case vertex_weight_mode::sparse:
		for (size_t pi = 0; pi < P.size(); ++pi) {
			vec4 p = vec4(P[pi], 1.0f);
			vec4 q = vec4(0.0f);
			size_t beg = vertex_weight_index_begins[pi];
			size_t end = pi+1 < P.size() ? vertex_weight_index_begins[pi+1] : vertex_weight_indices.size();
			for (size_t wi = beg; wi < end; ++wi)
				q += vertex_weight_data[wi] * (joint_matrices[vertex_weight_indices[wi]] * p);
			this->position(idx_type(pi)) = q;
		}
		break;
	case vertex_weight_mode::fixed:
		for (size_t pi = 0; pi < P.size(); ++pi) {
			vec4 p = vec4(P[pi], 1.0f);
			vec4 q = vec4(0.0f);
			size_t beg = vertex_weight_index_begins[pi];
			size_t end = pi + 1 < P.size() ? vertex_weight_index_begins[pi + 1] : vertex_weight_indices.size();
			for (size_t wi = beg; wi < end; ++wi)
				q += vertex_weight_data[wi] * (joint_matrices[vertex_weight_indices[wi]] * p);
			this->position(idx_type(pi)) = q;
		}
		break;
	}
}

template class dynamic_mesh<float>;
template class dynamic_mesh<double>;

		}
	}
}