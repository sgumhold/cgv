#pragma once

#include <cgv_gpgpu/algorithm.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API fill : public algorithm {
public:
	fill();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, uniform_arguments& arguments);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* value_buffer, size_t count, const uniform_binding_list& value);
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* value_buffer, size_t count, const uniform_arguments& arguments);

private:
	cgv::render::shader_compile_options get_configuration(const sl::data_type& value_type) const;

	compute_kernel kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
