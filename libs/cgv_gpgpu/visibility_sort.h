#pragma once

#include <memory>

#include "sort.h"
#include "sequence.h"
#include "storage_buffer.h"
#include "transform.h"

namespace cgv {
namespace gpgpu {

class visibility_sort {
public:
	visibility_sort() {}

	bool init(cgv::render::context& ctx, const sl::data_type& element_type, const sl::data_type& index_type, size_t element_count, const std::string& key_transform_operation, SortOrder order = SortOrder::Ascending, SortImplementation implementation = SortImplementation::Default) {
		cgv::gpgpu::argument_definitions arguments = { { sl::Type::Vec3, _distance_transform_arguments.eye_pos.name() } };
		return init(ctx, element_type, index_type, element_count, arguments, key_transform_operation, order, implementation);
	}

	bool init(cgv::render::context& ctx, const sl::data_type& element_type, const sl::data_type& index_type, size_t element_count, const argument_definitions& key_transform_arguments, const std::string& key_transform_operation, SortOrder order = SortOrder::Ascending, SortImplementation implementation = SortImplementation::Default) {
		bool success = true;
		_index_type = index_type;
		success &= _distance_transform.init(ctx, element_type, { sl::Type::Float }, key_transform_arguments, key_transform_operation);
		success &= _generate_indices.init(ctx, index_type);
		success &= _sort.init(ctx, sl::Type::Float, index_type, order, element_count, implementation);
		success &= _distance_buffer.create_or_resize<uint32_t>(ctx, element_count);
		return success;
	}

	void destruct(const cgv::render::context& ctx) {
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

	bool execute(cgv::render::context& ctx, device_buffer_iterator elements_first, device_buffer_iterator elements_last, device_buffer_iterator output_indices, const argument_bindings& arguments) {
		bool success = true;
		success &= _distance_transform.dispatch(ctx, elements_first, elements_last, begin(_distance_buffer), arguments);
		
		argument_binding_list generate_indices_arguments;
		generate_indices_arguments.bind_uniform(_generate_indices.get_init_argument_name(), _index_type, 0);
		generate_indices_arguments.bind_uniform(_generate_indices.get_step_argument_name(), _index_type, 1);
		success &= _generate_indices.dispatch(ctx, output_indices, output_indices + static_cast<size_t>(distance(elements_first, elements_last)), generate_indices_arguments);
		_sort.dispatch(ctx, _distance_buffer, output_indices.buffer());
		return success;
	}

	bool execute(cgv::render::context& ctx, const cgv::render::vertex_buffer& elements, size_t count, const cgv::render::vertex_buffer& output_indices, const argument_bindings& arguments) {
		return execute(ctx, begin(elements), begin(elements) + count, begin(output_indices), arguments);
	}

	bool execute(cgv::render::context& ctx, device_buffer_iterator elements_first, device_buffer_iterator elements_last, device_buffer_iterator output_indices, const cgv::vec3& eye_position) {
		_distance_transform_arguments.eye_pos.bind(&eye_position);
		return execute(ctx, elements_first, elements_last, output_indices, _distance_transform_arguments);
	}

	bool execute(cgv::render::context& ctx, const cgv::render::vertex_buffer& elements, size_t count, const cgv::render::vertex_buffer& output_indices, const cgv::vec3& eye_position) {
		return execute(ctx, begin(elements), begin(elements) + count, begin(output_indices), eye_position);
	}

private:
	struct distance_transform_args_t : public argument_binding_struct {
		typed_uniform_binding<cgv::vec3> eye_pos = { "u_eye_pos" };
		
		distance_transform_args_t() {
			connect({ &eye_pos });
		}

		distance_transform_args_t(const distance_transform_args_t& other) {
			connect({ &eye_pos });
		}
	};

	sl::data_type _index_type;

	distance_transform_args_t _distance_transform_arguments;
	generic::transform _distance_transform;
	generic::sequence _generate_indices;
	generic::sort_by_key _sort;

	storage_buffer _distance_buffer;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
