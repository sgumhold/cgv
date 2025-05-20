#include "fill.h"

namespace cgv {
namespace gpgpu {

const std::string fill::value_argument_name = "u_value";

fill::fill() : algorithm("fill") {
	register_kernel(_kernel, "gpgpu_fill");
}

bool fill::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	if(!value_type.is_valid())
		return false;
	cgv::render::shader_compile_options config = get_configuration({}, { value_type });
	config.snippets.push_back({ "value_typedef", sl::get_type_alias_string("value_type", value_type) });
	return init_kernels(ctx, config);
}

bool fill::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments) {
	buffer.bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	_kernel.enable(ctx);
	_kernel.set_argument(ctx, "u_count", static_cast<uint32_t>(count));
	_kernel.set_arguments(ctx, arguments);

	// TODO: Make configurable.
	const uint32_t group_size = 512;
	uint32_t num_groups = div_round_up(static_cast<uint32_t>(count), group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_kernel.disable(ctx);

	buffer.unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	return true;
}

} // namespace gpgpu
} // namespace cgv
