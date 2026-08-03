#pragma once

#include <cgv/render/uniform_buffer.h>

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

namespace detail {

/// GPU compute shader implementation of a configurable scan over a linear range of elements. This class is not intended for direct use. Use inclusive_scan or exclusive_scan instead.
class CGV_API scan : public algorithm {
public:
	enum class Mode {
		Inclusive,
		Exclusive
	};

	scan(const std::string& name, Mode mode, GroupSize group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	template<typename T, typename std::enable_if<!std::is_same<std::string, T>::value && !std::is_same<const char*, T>::value, bool>::type = true>
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, T binary_operation) {
		std::string operation_string = "return " + binary_operation("lhs", "rhs") + ";";
		return init(ctx, value_type, operation_string);
	}

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& binary_operation);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments = {});

	const char* get_init_argument_name() const {
		return _init_argument_name;
	}

private:
	struct uniform_data {
		uint32_t input_begin = 0;
		uint32_t input_end = 0;
		uint32_t output_begin = 0;
		uint32_t num_group_sums = 0;
	};

	void resize(cgv::render::context& ctx, uint32_t size);

	static const char* _init_argument_name;

	Mode _mode = Mode::Exclusive;

	sl::data_type _value_type;
	uint32_t _last_size = 0;

	uint32_t _part_size = 0;
	uint32_t _num_groups = 0;

	uniform_data _uniforms;
	cgv::render::uniform_buffer<uniform_data> _uniform_buffer;

	compute_kernel _scan_local_kernel;
	compute_kernel _scan_global_kernel;
	compute_kernel _propagate_kernel;

	storage_buffer _group_sums_buffer;
};

} // namespace detail

/// GPU compute shader implementation to perform an inclusive scan over a linear range of elements.
class inclusive_scan : public detail::scan {
public:
	inclusive_scan(GroupSize group_size = k_default_group_size) : detail::scan("inclusive_scan", Mode::Inclusive, group_size) {}
};

/// GPU compute shader implementation to perform an exclusive scan over a linear range of elements.
class exclusive_scan : public detail::scan {
public:
	exclusive_scan(GroupSize group_size = k_default_group_size) : detail::scan("exclusive_scan", Mode::Exclusive, group_size) {}
};

} // namespace generic

namespace detail {

/// GPU compute shader implementation of a configurable scan over a linear range of elements. This class is not intended for direct use. Use inclusive_scan or exclusive_scan instead.
template<class T>
class scan : public generic::detail::scan {
public:
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");

	using base = generic::detail::scan;
	using base::base;

	bool init(cgv::render::context& ctx) {
		return init(ctx, "");
	}

	template<typename Op, typename std::enable_if<!std::is_same<std::string, Op>::value && !std::is_same<const char*, Op>::value, bool>::type = true>
	bool init(cgv::render::context& ctx, Op binary_operation) {
		std::string operation_string = "return " + binary_operation("lhs", "rhs") + ";";
		return init(ctx, operation_string);
	}

	bool init(cgv::render::context& ctx, const std::string& binary_operation) {
		sl::data_type value_type = register_type_representation<T>();
		return base::init(ctx, value_type, binary_operation);
	}

	using base::dispatch;

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, T init) {
		argument_binding_list arguments;
		arguments.bind_uniform(get_init_argument_name(), init);
		return base::dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), arguments);
	}

	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, T init) {
		argument_binding_list arguments;
		arguments.bind_uniform(get_init_argument_name(), init);
		return base::dispatch(ctx, input_first, input_last, output_first, arguments);
	}
};

} // namespace detail

/// GPU compute shader implementation to perform an inclusive scan over a linear range of elements.
template<class T>
class inclusive_scan : public detail::scan<T> {
public:
	inclusive_scan(GroupSize group_size = k_default_group_size) : detail::scan<T>("inclusive_scan", Mode::Inclusive, group_size) {}
};

/// GPU compute shader implementation to perform an exclusive scan over a linear range of elements.
template<class T>
class exclusive_scan : public detail::scan<T> {
public:
	exclusive_scan(GroupSize group_size = k_default_group_size) : detail::scan<T>("exclusive_scan", Mode::Exclusive, group_size) {}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
