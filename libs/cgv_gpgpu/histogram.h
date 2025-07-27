#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for computing a histogram from a range of values.
class CGV_API histogram : public algorithm {
public:
	histogram();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	void destruct(const cgv::render::context& ctx);

	/*
	template<typename T, typename std::enable_if<!std::is_base_of<argument_bindings, T>::value, bool>::type = true>
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T value) {
		argument_binding_list arguments = {
			{ value_argument_name, value }
		};
		return dispatch(ctx, buffer, count, arguments);
	}

	template<typename T, typename std::enable_if<!std::is_base_of<argument_bindings, T>::value, bool>::type = true>
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, T value) {
		argument_binding_list arguments = {
			{ value_argument_name, value }
		};
		return dispatch(ctx, first, last, arguments);
	}*/

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count); //, const argument_bindings& arguments);
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first);//, const argument_bindings& arguments);

	//static const std::string value_argument_name;

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
