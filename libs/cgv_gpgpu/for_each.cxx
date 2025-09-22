#include "for_each.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

for_each::for_each(uint32_t group_size) : algorithm("for_each", group_size) {}

bool for_each::init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_operation) {
	return init(ctx, value_type, {}, unary_operation);
}

bool for_each::init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_operation) {
	if(!value_type.is_valid())
		return false;

	algorithm_create_info info;
	info.arguments = &arguments;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 1;
	info.options.define_snippet("operation", unary_operation);
	return algorithm::init(ctx, info, { { &_kernel, "gpgpu_for_each" } });
}

void for_each::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool for_each::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(buffer), begin(buffer) + count, arguments);
}

bool for_each::dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments) {
	if(!is_valid_range(first, last))
		return false;

	first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_begin", first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_end", last.index());
	_kernel.set_arguments(ctx, arguments);
	bind_buffer_like_arguments(ctx, arguments);

	// TODO: make configurable
	_group_size = 512;

	uint32_t num_groups = cgv::math::div_round_up(static_cast<uint32_t>(cgv::gpgpu::distance(first, last)), _group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	unbind_buffer_like_arguments(ctx, arguments);
	_kernel.disable(ctx);

	first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	return true;
}

} // namespace gpgpu
} // namespace cgv
