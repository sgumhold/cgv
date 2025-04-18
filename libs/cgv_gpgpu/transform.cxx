#include "transform.h"

namespace cgv {
namespace gpgpu {

transform::transform() : algorithm("transform") {
	register_kernel(kernel, "gpgpu_transform");
}

bool transform::init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const std::string& unary_operation) {
	return init(ctx, input_type, output_type, {}, unary_operation);
}

bool transform::init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const sl::named_variable_list& arguments, const std::string& unary_operation) {
	if(!input_type.is_valid() || !output_type.is_valid())
		return false;
	cgv::render::shader_compile_options config = get_configuration(input_type, output_type, arguments, unary_operation);
	return init_kernels(ctx, config);
}

bool transform::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* input_buffer, const cgv::render::vertex_buffer* output_buffer, size_t count, const compute_kernel_arguments& arguments) {
	input_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	kernel.enable(ctx);
	kernel.set_argument(ctx, "u_count", static_cast<uint32_t>(count));
	kernel.set_arguments(ctx, arguments);

	// TODO: Make configurable.
	const uint32_t group_size = 512;
	uint32_t num_groups = div_round_up(static_cast<uint32_t>(count), group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	kernel.disable(ctx);

	input_buffer->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_buffer->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	return true;
}

cgv::render::shader_compile_options transform::get_configuration(const sl::data_type& input_type, const sl::data_type& output_type, const sl::named_variable_list& arguments, const std::string& unary_operation) const {
	cgv::render::shader_compile_options config;
	config.snippets.push_back({ "input_type_def", sl::get_typedef_str("input_type", input_type) });
	config.snippets.push_back({ "output_type_def", sl::get_typedef_str("output_type", output_type) });
	config.snippets.push_back({ "operation", unary_operation });
	config.snippets.push_back({ "arguments", to_string(arguments, "uniform") });
	return config;
}

} // namespace gpgpu
} // namespace cgv
