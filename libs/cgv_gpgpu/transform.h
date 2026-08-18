#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation for transforming input values to output values using a given operation.
class CGV_API transform : public algorithm {
public:
	transform(GroupSize group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const std::string& unary_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const argument_definitions& arguments, const std::string& unary_operation);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments = {});

private:
	compute_kernel _kernel;
};

} // namespace generic

/// GPU compute shader implementation for transforming input values to output values using a given operation.
template<class InputT, class OutputT>
class transform : public generic::transform {
public:
	static_assert(type_representation<InputT>::value, "InputT must be representable as sl::data_type");
	static_assert(type_representation<OutputT>::value, "OutputT must be representable as sl::data_type");

	using base = generic::transform;
	using base::base;

	bool init(cgv::render::context& ctx, const std::string& unary_operation) {
		return init(ctx, {}, unary_operation);
	}

	bool init(cgv::render::context& ctx, const argument_definitions& arguments, const std::string& unary_operation) {
		sl::data_type input_type = register_type_representation<InputT>();
		sl::data_type output_type = register_type_representation<OutputT>();
		return base::init(ctx, input_type, output_type, arguments, unary_operation);
	}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
