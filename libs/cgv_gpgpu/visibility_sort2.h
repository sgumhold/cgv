#pragma once

//#include "algorithm.h"
//#include "storage_buffer.h"

#include <cgv_gpgpu/radix_sort_4x.h>
//#include <cgv_gpgpu/radix_sort_onesweep.h>
#include <cgv_gpgpu/sequence.h>
#include <cgv_gpgpu/transform.h>

#include "lib_begin.h"

// TODO: Put in "routine" namespace.
// TODO: Allow choosing which sort algorithm to use.

namespace cgv {
namespace gpgpu {

class CGV_API visibility_sort2 {
public:
	bool init(cgv::render::context& ctx, const sl::data_type& element_type, const sl::data_type& index_type, size_t element_count, const std::string& key_transform_operation, radix_sort::Order order = radix_sort::Order::kAscending) {
		bool success = true;
		cgv::gpgpu::argument_definitions arguments = { { sl::Type::kVec3, "u_eye_pos" } };
		success &= _distance_transform.init(ctx, element_type, { sl::Type::kFloat }, arguments, key_transform_operation);
		success &= _generate_indices.init(ctx, index_type);
		success &= _sort.init(ctx, sl::Type::kFloat, index_type, order, element_count);
		success &= _distance_buffer.create_or_resize<uint32_t>(ctx, element_count);
		return true;
	}

	void destruct(cgv::render::context& ctx) {
		_distance_transform.destruct(ctx);
		_generate_indices.destruct(ctx);
		_sort.destruct(ctx);
		_distance_buffer.destruct(ctx);
	}

	bool resize(cgv::render::context& ctx, size_t element_count) {
		return _sort.resize(ctx, element_count) && _distance_buffer.create_or_resize<uint32_t>(ctx, element_count);
	}

	bool is_initialized() const {
		return _distance_transform.is_initialized() && _generate_indices.is_initialized() && _sort.is_initialized();
	}

	bool execute(cgv::render::context& ctx, device_buffer_iterator elements_first, device_buffer_iterator elements_last, device_buffer_iterator indices_output, const cgv::vec3& eye_position) {
		bool success = true;
		_distance_transform_arguments.eye_pos = eye_position;
		const size_t count = static_cast<size_t>(distance(elements_first, elements_last));
		success &= _distance_transform.dispatch(ctx, elements_first.buffer(), _distance_buffer, count, _distance_transform_arguments);
		success &= _generate_indices.dispatch(ctx, indices_output.buffer(), count, 0u, 1u);
		_sort.dispatch(ctx, _distance_buffer, indices_output.buffer());
		return success;
	}

private:
	struct distance_transform_args_t : public argument_binding_struct {
		typed_uniform_binding<cgv::vec3> eye_pos = { "u_eye_pos" };
		distance_transform_args_t() {
			connect({ &eye_pos });
		}
	};

	distance_transform_args_t _distance_transform_arguments;
	transform _distance_transform;
	sequence _generate_indices;
	radix_sort_4x _sort;

	storage_buffer _distance_buffer;
};

}
}

#include <cgv/config/lib_end.h>
