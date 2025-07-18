#include "sequence.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

const std::string sequence::init_argument_name = "u_init";
const std::string sequence::step_argument_name = "u_step";

sequence::sequence() : algorithm("sequence") {}

bool sequence::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	if(!value_type.is_valid() || value_type.is_compound())
		return false;
	cgv::render::shader_compile_options config = get_configuration({}, { value_type });
	config.snippets.push_back({ "value_typedef", sl::get_type_alias_string("value_type", value_type) });

	return algorithm::init(ctx, { { &_kernel, "gpgpu_sequence" } }, config);
}

void sequence::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool sequence::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(buffer), begin(buffer) + count, arguments);
}

bool sequence::dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments) {
	if(!is_valid_range(first, last))
		return false;

	first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_begin", first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_end", last.index());
	_kernel.set_arguments(ctx, arguments);

	// TODO: Make configurable.
	const uint32_t group_size = 512;
	uint32_t num_groups = cgv::math::div_round_up(static_cast<uint32_t>(cgv::gpgpu::distance(first, last)), group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	_kernel.disable(ctx);

	first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	return true;
}

} // namespace gpgpu
} // namespace cgv
