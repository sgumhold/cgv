#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "storage_buffer.h"
#include "fill.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation for computing a histogram from a range of values.
class CGV_API histogram : public algorithm {
public:
	histogram(uint32_t group_count = 256, GroupSize group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, uint32_t num_bins);

	void destruct(const cgv::render::context& ctx);

	const storage_buffer& bins_buffer() const;

	const char* get_lower_limit_argument_name() const {
		return _lower_limit_argument_name;
	}

	const char* get_upper_limit_argument_name() const {
		return _upper_limit_argument_name;
	}

protected:
	template<typename T, typename std::enable_if<std::is_integral_v<T>, bool>::type = true>
	bool range_fits_bin_count(T lower_limit, T upper_limit) const {
		T length = upper_limit - lower_limit;
		return _num_bins == static_cast<uint32_t>(length + 1);
	}

	template<typename T, typename std::enable_if<std::is_floating_point_v<T>, bool>::type = true>
	bool range_fits_bin_count(T lower_limit, T upper_limit) const {
		return true;
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments, bool use_remapping, bool clear_bins);

private:
	static const char* _lower_limit_argument_name;
	static const char* _upper_limit_argument_name;

	uint32_t _num_bins = 256;
	uint32_t _num_groups = 256;

	compute_kernel _kernel;

	storage_buffer _bins_buffer;
	cgv::gpgpu::fill<uint32_t> _fill;
};

} // namespace generic

/// GPU compute shader implementation for computing a histogram from a range of values.
template<class T>
class histogram : public generic::histogram {
public:
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");

	using base = generic::histogram;
	using base::base;

	bool init(cgv::render::context& ctx, uint32_t num_bins) {
		sl::data_type value_type = register_type_representation<T>();
		return base::init(ctx, value_type, num_bins);
	}

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, T lower_limit, T upper_limit, bool clear_bins = true) {
		return dispatch(ctx, begin(buffer), begin(buffer) + count, begin(bins_buffer()), lower_limit, upper_limit, clear_bins);
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, T lower_limit, T upper_limit, bool clear_bins = true) {
		return dispatch(ctx, input_first, input_last, begin(bins_buffer()), lower_limit, upper_limit, clear_bins);
	}

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, T lower_limit, T upper_limit, bool clear_bins = true) {
		return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), lower_limit, upper_limit, clear_bins);
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, T lower_limit, T upper_limit, bool clear_bins = true) {
		argument_binding_list arguments;
		arguments.bind_uniform(get_lower_limit_argument_name(), lower_limit);
		arguments.bind_uniform(get_upper_limit_argument_name(), upper_limit);
		bool use_remapping = !range_fits_bin_count(lower_limit, upper_limit);
		return base::dispatch(ctx, input_first, input_last, output_first, arguments, use_remapping, clear_bins);
	}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
