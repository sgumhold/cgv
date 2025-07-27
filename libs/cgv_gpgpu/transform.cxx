#include "transform.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

transform::transform() : algorithm("transform") {}

bool transform::init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const std::string& unary_operation) {
	return init(ctx, input_type, output_type, {}, unary_operation);
}

bool transform::init(cgv::render::context& ctx, const sl::data_type& input_type, const sl::data_type& output_type, const argument_definitions& arguments, const std::string& unary_operation) {
	if(!input_type.is_valid() || !output_type.is_valid())
		return false;

	set_buffer_binding_indices(arguments.buffers, 2);
	cgv::render::shader_compile_options config = get_configuration(arguments, { input_type, output_type });
	config.snippets.push_back({ "input_typedef", sl::get_type_alias_string("input_type", input_type) });
	config.snippets.push_back({ "output_typedef", sl::get_type_alias_string("output_type", output_type) });
	config.snippets.push_back({ "operation", unary_operation });

	return algorithm::init(ctx, { { &_kernel, "gpgpu_transform" } }, config);
}

void transform::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool transform::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), arguments);
}

bool transform::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(same(input_first, output_first))
		return false;

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_input_begin", input_first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_input_end", input_last.index());
	_kernel.set_argument<uint32_t>(ctx, "u_output_begin", output_first.index());
	_kernel.set_arguments(ctx, arguments);
	bind_buffer_arguments(ctx, arguments);

	// TODO: Make configurable.
	const uint32_t group_size = 512;
	uint32_t num_groups = cgv::math::div_round_up(static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last)), group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	unbind_buffer_arguments(ctx, arguments);
	_kernel.disable(ctx);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	return true;
}

} // namespace gpgpu
} // namespace cgv
