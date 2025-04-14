#pragma once

#include <cgv_gpgpu/algorithm.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API fill : public cgv::gpgpu::algorithm {
public:
	fill();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);
	//void destruct(const cgv::render::context& ctx);

	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* value_buffer, size_t count, const cgv::gpgpu::uniform_argument_list& value);

private:
	cgv::render::shader_compile_options get_configuration(const sl::data_type& value_type) const;

	cgv::gpgpu::compute_kernel kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
