#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for transforming input values to output values using a given operation.
class CGV_API transform : public algorithm {
public:
	transform();

	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const std::string& unary_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const argument_definitions& arguments, const std::string& unary_operation);
	
	void destruct(cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments = {});

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
