#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "representation.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation for filling a buffer with a constant value.
class CGV_API fill : public algorithm {
public:
	fill(GroupSize group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments);
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments);

	const char* get_value_argument_name() const {
		return _value_argument_name;
	}

private:
	static const char* _value_argument_name;
	compute_kernel _kernel;
};

} // namespace generic

/// GPU compute shader implementation for filling a buffer with a constant value.
template<class T>
class fill : public generic::fill {
public:
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");

	using base = generic::fill;
	using base::base;

	bool init(cgv::render::context& ctx) {
		sl::data_type value_type = register_type_representation<T>();
		return base::init(ctx, value_type);
	}

	using base::dispatch;

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T value) {
		return dispatch(ctx, begin(buffer), begin(buffer) + count, value);
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, T value) {
		argument_binding_list arguments;
		arguments.bind_uniform(get_value_argument_name(), value);
		return base::dispatch(ctx, first, last, arguments);
	}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
