#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for filling a buffer with a constant value.
class CGV_API fill : public algorithm {
public:
	fill();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	template<typename T>
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T value) {
		argument_binding_list arguments = {
			{ "u_value", value }
		};
		return dispatch(ctx, buffer, count, arguments);
	}

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments);

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
