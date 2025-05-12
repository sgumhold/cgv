#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for generating a sequence of numbers.
class CGV_API sequence : public algorithm {
public:
	sequence();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	template<typename T>
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T init, T step) {
		argument_binding_list arguments = {
			{ "u_init", init },
			{ "u_step", step }
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
