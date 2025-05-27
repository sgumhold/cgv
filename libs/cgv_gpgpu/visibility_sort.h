#pragma once

#include <memory>

#include "radix_sort.h"
#include "radix_sort_4x.h"
#include "radix_sort_onesweep.h"
#include "sequence.h"
#include "storage_buffer.h"
#include "transform.h"

namespace cgv {
namespace gpgpu {

class visibility_sort {
public:
	visibility_sort() {}

	template<typename SortT>
	static visibility_sort construct_with() {
		static_assert(std::is_base_of<radix_sort, SortT>::value, "SortT must inherit from cgv::gpgpu::radix_sort");
		visibility_sort instance;
		instance._sort = std::make_shared<SortT>();
		return instance;
	}

	bool init(cgv::render::context& ctx, const sl::data_type& element_type, const sl::data_type& index_type, size_t element_count, const std::string& key_transform_operation, SortOrder order = SortOrder::kAscending) {
		bool success = true;
		cgv::gpgpu::argument_definitions arguments = { { sl::Type::kVec3, _distance_transform_arguments.eye_pos.name() } };
		success &= _distance_transform.init(ctx, element_type, { sl::Type::kFloat }, arguments, key_transform_operation);
		success &= _generate_indices.init(ctx, index_type);
		if(!_sort)
			_sort = get_default_sort_implementation(ctx);
		success &= _sort->init(ctx, sl::Type::kFloat, index_type, order, element_count);
		success &= _distance_buffer.create_or_resize<uint32_t>(ctx, element_count);
		return success;
	}

	void destruct(cgv::render::context& ctx) {
		_distance_transform.destruct(ctx);
		_generate_indices.destruct(ctx);
		_sort->destruct(ctx);
		_distance_buffer.destruct(ctx);
	}

	bool resize(cgv::render::context& ctx, size_t element_count) {
		return _sort->resize(ctx, element_count) && _distance_buffer.create_or_resize<uint32_t>(ctx, element_count);
	}

	bool is_initialized() const {
		return _distance_transform.is_initialized() && _generate_indices.is_initialized() && _sort && _sort->is_initialized();
	}

	bool execute(cgv::render::context& ctx, device_buffer_iterator elements_first, device_buffer_iterator elements_last, device_buffer_iterator indices_output, const cgv::vec3& eye_position) {
		bool success = true;
		_distance_transform_arguments.eye_pos = eye_position;
		success &= _distance_transform.dispatch(ctx, elements_first, elements_last, begin(_distance_buffer), _distance_transform_arguments);
		success &= _generate_indices.dispatch(ctx, indices_output, indices_output + static_cast<size_t>(distance(elements_first, elements_last)), 0u, 1u);
		_sort->dispatch(ctx, _distance_buffer, indices_output.buffer());
		return success;
	}

private:
	std::shared_ptr<radix_sort> get_default_sort_implementation(cgv::render::context& ctx) const {
		if(ctx.get_gpu_vendor_id() == cgv::render::GPUVendorID::GPU_VENDOR_NVIDIA) {
			/* TODO: Check for GPU support of following shader extensions:
				#extension GL_KHR_shader_subgroup_basic : enable
				#extension GL_KHR_shader_subgroup_ballot : enable
				#extension GL_KHR_shader_subgroup_arithmetic : enable
				Maybe check the device name too.
				Only use onesweep if all requirements are met.
			*/
			return std::make_shared<radix_sort_onesweep>();
		}
		return std::make_shared<radix_sort_4x>();
	}

	struct distance_transform_args_t : public argument_binding_struct {
		typed_uniform_binding<cgv::vec3> eye_pos = { "u_eye_pos" };
		
		distance_transform_args_t() {
			connect({ &eye_pos });
		}

		distance_transform_args_t(const distance_transform_args_t& other) {
			connect({ &eye_pos });
		}
	};

	distance_transform_args_t _distance_transform_arguments;
	transform _distance_transform;
	sequence _generate_indices;
	std::shared_ptr<radix_sort> _sort;

	storage_buffer _distance_buffer;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
