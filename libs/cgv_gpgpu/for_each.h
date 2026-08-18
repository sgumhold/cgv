#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation for applying a function to each buffer element.
class CGV_API for_each : public algorithm {
public:
	for_each(GroupSize group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_operation);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments = {});

private:
	compute_kernel _kernel;
};

} // namespace generic

/// GPU compute shader implementation for filling a buffer with a constant value.
template<class T>
class for_each : public generic::for_each {
public:
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");

	using base = generic::for_each;
	using base::base;

	bool init(cgv::render::context& ctx, const std::string& unary_operation) {
		return init(ctx, {}, unary_operation);
	}

	bool init(cgv::render::context& ctx, const argument_definitions& arguments, const std::string& unary_operation) {
		sl::data_type value_type = register_type_representation<T>();
		return base::init(ctx, value_type, arguments, unary_operation);
	}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
