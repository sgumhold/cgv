#include "fill.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {
namespace generic {

const const char* fill::_value_argument_name = "u_value";

fill::fill(GroupSize group_size) : algorithm("fill", group_size) {}

bool fill::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	algorithm_create_info info;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 1;

	return algorithm::init(ctx, info, { { &_kernel, "gpgpu_fill" } });
}

void fill::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool fill::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(buffer), begin(buffer) + count, arguments);
}

bool fill::dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments) {
	if(!is_valid_range(first, last)) {
		raise_error(errc::invalid_range);
		return false;
	}

	first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_begin", first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_end", last.index());
	_kernel.set_arguments(ctx, arguments);

	uint32_t num_groups = cgv::math::div_round_up(static_cast<uint32_t>(distance(first, last)), _group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_kernel.disable(ctx);

	first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);

	return true;
}

} // namespace generic
} // namespace gpgpu
} // namespace cgv
