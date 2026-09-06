#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "representation.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation of transforming input values to intermediary values and performing a parallel reduction over the intermediary values.
class CGV_API transform_reduce : public algorithm {
public:
	transform_reduce(uint32_t group_count = 256, GroupSize group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& value_type, const std::string& unary_transform_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_transform_operation);

	template<typename T, typename std::enable_if<!std::is_same<std::string, T>::value && !std::is_same<const char*, T>::value, bool>::type = true>
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& value_type, const std::string& unary_transform_operation, T binary_reduce_operation) {
		std::string operation_string = "return " + binary_reduce_operation("lhs", "rhs") + ";";
		return init(ctx, input_type, value_type, unary_transform_operation, binary_reduce_operation);
	}

	template<typename T, typename std::enable_if<!std::is_same<std::string, T>::value && !std::is_same<const char*, T>::value, bool>::type = true>
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_transform_operation, T binary_reduce_operation) {
		std::string operation_string = "return " + binary_reduce_operation("lhs", "rhs") + ";";
		return init(ctx, input_type, value_type, arguments, unary_transform_operation, operation_string);
	}

	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& value_type, const std::string& unary_transform_operation, const std::string& binary_reduce_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_transform_operation, const std::string& binary_reduce_operation);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output, const argument_bindings& arguments = {});

	template<typename T>
	bool read_result(cgv::render::context& ctx, T& out) {
		std::vector<T> res(1);
		if(_group_reduction_buffer.copy(ctx, res)) {
			out = res.front();
			return true;
		}
		raise_error(errc::buffer_copy_to_host_error);
		return false;
	}

	const char* get_init_argument_name() const {
		return _init_argument_name;
	}

private:
	static const char* _init_argument_name;
	uint32_t _num_groups = 256;
	compute_kernel _global_reduction_kernel;
	compute_kernel _group_reduction_kernel;
	storage_buffer _group_reduction_buffer;
};

} // namespace generic

/// GPU compute shader implementation of parallel reduction.
template<class InputT, class ValueT>
class transform_reduce : public generic::transform_reduce {
public:
	static_assert(type_representation<InputT>::value, "InputT must be representable as sl::data_type");
	static_assert(type_representation<ValueT>::value, "ValueT must be representable as sl::data_type");

	using base = generic::transform_reduce;
	using base::base;

	bool init(cgv::render::context& ctx, const std::string& unary_transform_operation) {
		return init(ctx, argument_definitions{}, unary_transform_operation);
	}

	
	bool init(cgv::render::context& ctx, const argument_definitions& arguments, const std::string& unary_transform_operation) {
		return init(ctx, arguments, unary_transform_operation, sl::operation::plus());
	}

	template<class ReduceOp>
	bool init(cgv::render::context& ctx, const std::string& unary_transform_operation, ReduceOp binary_reduce_operation) {
		sl::data_type input_type = register_type_representation<InputT>();
		sl::data_type value_type = register_type_representation<ValueT>();
		return base::init(ctx, input_type, value_type, unary_transform_operation, binary_reduce_operation);
	}

	template<class ReduceOp>
	bool init(cgv::render::context& ctx, const argument_definitions& arguments, const std::string& unary_transform_operation, ReduceOp binary_reduce_operation) {
		sl::data_type input_type = register_type_representation<InputT>();
		sl::data_type value_type = register_type_representation<ValueT>();
		return base::init(ctx, input_type, value_type, arguments, unary_transform_operation, binary_reduce_operation);
	}

	using base::dispatch;

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, ValueT init) {
		return dispatch(ctx, begin(buffer), begin(buffer) + count, init);
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, ValueT init) {
		argument_binding_list arguments;
		arguments.bind_uniform(get_init_argument_name(), init);
		return base::dispatch(ctx, first, last, arguments);
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output, ValueT init) {
		argument_binding_list arguments;
		arguments.bind_uniform(get_init_argument_name(), init);
		return base::dispatch(ctx, input_first, input_last, output, arguments);
	}

	bool read_result(cgv::render::context& ctx, ValueT& out) {
		return base::read_result<ValueT>(ctx, out);
	}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
