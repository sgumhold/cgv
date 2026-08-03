#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "representation.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation of parallel reduction.
class CGV_API reduce : public algorithm {
public:
	reduce(uint32_t group_count = 256, GroupSize group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	template<typename T, typename std::enable_if<!std::is_same<std::string, T>::value && !std::is_same<const char*, T>::value, bool>::type = true>
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, T binary_operation) {
		std::string operation_string = "return " + binary_operation("lhs", "rhs") + ";";
		return init(ctx, value_type, operation_string);
	}

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& binary_operation);

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
	compute_kernel _kernel;
	storage_buffer _group_reduction_buffer;
};

} // namespace generic

/// GPU compute shader implementation of parallel reduction.
template<class T>
class reduce : public generic::reduce {
public:
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");

	using base = generic::reduce;
	using base::base;

	bool init(cgv::render::context& ctx) {
		return init(ctx, sl::operation::plus());
	}

	template<typename Op>
	bool init(cgv::render::context& ctx, Op binary_operation) {
		sl::data_type value_type = register_type_representation<T>();
		return base::init(ctx, value_type, binary_operation);
	}

	using base::dispatch;

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T init) {
		return dispatch(ctx, begin(buffer), begin(buffer) + count, init);
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, T init) {
		argument_binding_list arguments;
		arguments.bind_uniform(get_init_argument_name(), init);
		return base::dispatch(ctx, first, last, arguments);
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output, T init) {
		argument_binding_list arguments;
		arguments.bind_uniform(get_init_argument_name(), init);
		return base::dispatch(ctx, input_first, input_last, output, arguments);
	}

	bool read_result(cgv::render::context& ctx, T& out) {
		return base::read_result<T>(ctx, out);
	}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
