#include "histogram.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

const std::string histogram::lower_limit_argument_name = "u_lower_limit";
const std::string histogram::upper_limit_argument_name = "u_upper_limit";

histogram::histogram(uint32_t num_bins, uint32_t group_size) : algorithm("histogram", group_size), _num_bins(num_bins) {}

bool histogram::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	if(!value_type.is_valid() || !value_type.is_scalar() || value_type.type() == sl::Type::kBool)
		return false;

	_value_type = value_type;

	algorithm_create_info info;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 2;
	info.options.define_macro("NUM_BINS", _num_bins);

	sl::Type base_type = value_type.type();
	info.options.define_macro_if_true(base_type == sl::Type::kFloat || base_type == sl::Type::kDouble, "VALUE_TYPE_IS_FLOATING_POINT");
	
	if(algorithm::init(ctx, info, { { &_kernel, "gpgpu_histogram" } })) {
		_bins_buffer.create_or_resize<uint32_t>(ctx, _num_bins);
		return _fill.init(ctx, sl::Type::kUInt);
	}

	return false;
}

void histogram::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	_bins_buffer.destruct(ctx);
	_fill.destruct(ctx);
	algorithm::destruct(ctx);
}

const storage_buffer& histogram::bins_buffer() const {
	return _bins_buffer;
}

bool histogram::dispatch(cgv::render::context & ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments, bool use_remapping, bool clear_bins) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(compatible(input_first, output_first))
		return false;

	if(clear_bins)
		_fill.dispatch(ctx, output_first, output_first + _num_bins, 0);

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_input_begin", input_first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_input_end", input_last.index());
	_kernel.set_argument<uint32_t>(ctx, "u_output_begin", output_first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_use_remapping", use_remapping);
	_kernel.set_arguments(ctx, arguments);

	// TODO: Expose num groups and group size as setup parameters
	const uint32_t _num_groups = 256;
	const uint32_t _group_size = 512;

	uint32_t count = static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last));
	uint32_t num_groups = std::min(_num_groups, cgv::math::div_round_up(count, _group_size));
	dispatch_compute(num_groups, 1, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_kernel.disable(ctx);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	return true;
}


} // namespace gpgpu
} // namespace cgv
