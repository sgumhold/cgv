#include "for_each.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

for_each::for_each() : algorithm("for_each") {
	register_kernel(_kernel, "gpgpu_for_each");
}

bool for_each::init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_operation) {
	return init(ctx, value_type, {}, unary_operation);
}

bool for_each::init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_operation) {
	if(!value_type.is_valid())
		return false;
	set_buffer_binding_indices(arguments.buffers, 2);
	cgv::render::shader_compile_options config = get_configuration(arguments, { value_type });
	config.snippets.push_back({ "value_typedef", sl::get_type_alias_string("value_type", value_type) });
	config.snippets.push_back({ "operation", unary_operation });

	return init_kernels(ctx, config);
}

bool for_each::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments) {
	buffer.bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	_kernel.enable(ctx);
	_kernel.set_argument(ctx, "u_count", static_cast<uint32_t>(count));
	_kernel.set_arguments(ctx, arguments);
	bind_buffer_arguments(ctx, arguments);

	// TODO: Make configurable.
	const uint32_t group_size = 512;
	uint32_t num_groups = cgv::math::div_round_up(static_cast<uint32_t>(count), group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	unbind_buffer_arguments(ctx, arguments);
	_kernel.disable(ctx);

	buffer.unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	return true;
}

} // namespace gpgpu
} // namespace cgv
