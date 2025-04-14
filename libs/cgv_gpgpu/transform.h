#pragma once

#include <cgv_gpgpu/algorithm.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API transform : public cgv::gpgpu::algorithm {
public:
	transform();

	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const const std::string& unary_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const sl::named_variable_list& arguments, const const std::string& unary_operation);
	//void destruct(const cgv::render::context& ctx);

	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* input_buffer, const cgv::render::vertex_buffer* output_buffer, size_t count, const cgv::gpgpu::uniform_argument_list& arguments);

private:
	cgv::render::shader_compile_options get_configuration(const sl::data_type& input_type, const sl::data_type& output_type, const sl::named_variable_list& arguments, const const std::string& unary_operation) const;

	cgv::gpgpu::compute_kernel kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
