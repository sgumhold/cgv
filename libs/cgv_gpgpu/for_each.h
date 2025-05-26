#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for applying a function to each buffer element.
class CGV_API for_each : public algorithm {
public:
	for_each();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_operation);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments = {});

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
