#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for generating a sequence of numbers.
class CGV_API sequence : public algorithm {
public:
	sequence();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	void destruct(cgv::render::context& ctx);

	template<typename T, typename std::enable_if<!std::is_base_of<argument_bindings, T>::value, bool>::type = true>
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T init, T step) {
		argument_binding_list arguments = {
			{ init_argument_name, init },
			{ step_argument_name, step }
		};
		return dispatch(ctx, buffer, count, arguments);
	}

	template<typename T, typename std::enable_if<!std::is_base_of<argument_bindings, T>::value, bool>::type = true>
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, T init, T step) {
		argument_binding_list arguments = {
			{ init_argument_name, init },
			{ step_argument_name, step }
		};
		return dispatch(ctx, first, last, arguments);
	}

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments);
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments);

	static const std::string init_argument_name;
	static const std::string step_argument_name;

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
