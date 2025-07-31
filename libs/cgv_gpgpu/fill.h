#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for filling a buffer with a constant value.
class CGV_API fill : public algorithm {
public:
	fill();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	void destruct(const cgv::render::context& ctx);

	template<typename T, CGV_GPGPU_DISABLE_DERIVED_TYPES(argument_bindings)>
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T value) {
		return dispatch(ctx, begin(buffer), begin(buffer) + count, value);
	}

	template<typename T, CGV_GPGPU_DISABLE_DERIVED_TYPES(argument_bindings)>
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, T value) {
		argument_binding_list arguments = {
			{ _value_type, value_argument_name, value }
		};
		return dispatch(ctx, first, last, arguments);
	}

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments);
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments);

	static const std::string value_argument_name;

private:
	sl::data_type _value_type;
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
