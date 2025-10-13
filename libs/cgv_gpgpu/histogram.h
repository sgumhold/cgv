#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "storage_buffer.h"
#include "fill.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for computing a histogram from a range of values.
class CGV_API histogram : public algorithm {
public:
	histogram(uint32_t num_bins, uint32_t group_size = CGV_GPGPU_DEFAULT_GROUP_SIZE);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	void destruct(const cgv::render::context& ctx);

	template<typename T, CGV_GPGPU_DISABLE_DERIVED_TYPES(argument_bindings)>
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T lower_limit, T upper_limit, bool clear_bins = true) {
		return dispatch(ctx, begin(buffer), begin(buffer) + count, begin(_bins_buffer), lower_limit, upper_limit, clear_bins);
	}

	template<typename T, CGV_GPGPU_DISABLE_DERIVED_TYPES(argument_bindings)>
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, T lower_limit, T upper_limit, bool clear_bins = true) {
		return dispatch(ctx, input_first, input_last, begin(_bins_buffer), lower_limit, upper_limit, clear_bins);
	}

	template<typename T, CGV_GPGPU_DISABLE_DERIVED_TYPES(argument_bindings)>
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, T lower_limit, T upper_limit, bool clear_bins = true) {
		return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), lower_limit, upper_limit, clear_bins);
	}

	template<typename T, CGV_GPGPU_DISABLE_DERIVED_TYPES(argument_bindings)>
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, T lower_limit, T upper_limit, bool clear_bins = true) {
		argument_binding_list arguments;
		arguments.bind_uniform(_value_type, lower_limit_argument_name, lower_limit);
		arguments.bind_uniform(_value_type, upper_limit_argument_name, upper_limit);
		bool use_remapping = !range_fits_bin_count(lower_limit, upper_limit);
		return dispatch(ctx, input_first, input_last, output_first, arguments, use_remapping, clear_bins);
	}

	const storage_buffer& bins_buffer() const;

	static const std::string lower_limit_argument_name;
	static const std::string upper_limit_argument_name;

private:
	template<typename T, typename std::enable_if<std::is_integral_v<T>, bool>::type = true>
	bool range_fits_bin_count(T lower_limit, T upper_limit) const {
		T length = upper_limit - lower_limit;
		return _num_bins == length + 1;
	}

	template<typename T, typename std::enable_if<std::is_floating_point_v<T>, bool>::type = true>
	bool range_fits_bin_count(T lower_limit, T upper_limit) const {
		return true;
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments, bool use_remapping, bool clear_bins);

	uint32_t _num_bins = 256;
	sl::data_type _value_type;

	compute_kernel _kernel;

	storage_buffer _bins_buffer;

	fill _fill;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
