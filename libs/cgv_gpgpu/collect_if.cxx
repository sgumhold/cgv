#include "collect_if.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

collect_if::collect_if() : algorithm("collect_if") {}

bool collect_if::init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_predicate) {
	return init(ctx, value_type, {}, unary_predicate);
}

bool collect_if::init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_predicate) {
	if(!value_type.is_valid())
		return false;

	algorithm_create_info info;
	info.arguments = &arguments;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 3;
	info.options.define_snippet("predicate", unary_predicate);

	_uniform_buffer.create(ctx);
	_atomic_counter_buffer.create_or_resize<int32_t>(ctx, 1);

	return algorithm::init(ctx, info, { { &_kernel, "gpgpu_collect_if" }, });
}

void collect_if::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	_uniform_buffer.destruct(ctx);
	_atomic_counter_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

bool collect_if::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), arguments);
}

bool collect_if::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(compatible(input_first, output_first))
		return false;

	// Reset atomic_counter_buffer to zero.
	_atomic_counter_buffer.ctx_ptr = &ctx;
	GLuint id = 0;
	_atomic_counter_buffer.put_id(id);
	int32_t zero = 0;
	glNamedBufferSubData(id, 0, sizeof(int32_t), &zero);

	uint32_t count = static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last));
	uint32_t num_groups = cgv::math::div_round_up(count, _group_size);

	uniform_data uniforms;
	uniforms.input_begin = static_cast<uint32_t>(input_first.index());
	uniforms.input_end = static_cast<uint32_t>(input_last.index());
	uniforms.output_begin = static_cast<uint32_t>(output_first.index());
	_uniform_buffer.replace(ctx, uniforms);
	_uniform_buffer.bind(ctx, 0);

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_atomic_counter_buffer.bind(ctx, 2);

	_kernel.enable(ctx);
	_kernel.set_arguments(ctx, arguments);
	bind_buffer_like_arguments(ctx, arguments);

	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	unbind_buffer_like_arguments(ctx, arguments);
	_kernel.disable(ctx);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_atomic_counter_buffer.unbind(ctx, 2);

	_uniform_buffer.unbind(ctx, 0);

	return true;
}

} // namespace gpgpu
} // namespace cgv
