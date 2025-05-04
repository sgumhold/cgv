#pragma once

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API transform : public algorithm {
public:
	transform();

	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const std::string& unary_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const sl::named_variable_list& arguments, const std::string& unary_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const compute_kernel_arguments_declaration& arguments, const std::string& unary_operation);
	bool init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const ckadl& arguments, const std::string& unary_operation);
	
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* input_buffer, const cgv::render::vertex_buffer* output_buffer, size_t count, const compute_kernel_arguments& arguments);
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* input_buffer, const cgv::render::vertex_buffer* output_buffer, size_t count, const compute_kernel_arguments2& arguments);

private:
	cgv::render::shader_compile_options get_configuration(const sl::data_type& input_type, const sl::data_type& output_type, const sl::named_variable_list& arguments, const std::string& unary_operation) const;

	compute_kernel kernel;

	std::map<std::string, size_t> buffer_binding_indices;
	const size_t base_buffer_binding_index = 2;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
