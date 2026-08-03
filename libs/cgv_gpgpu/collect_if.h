#pragma once

#include <cgv/render/uniform_buffer.h>

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation for copying values based on a boolean predicate. Unlike copy_if, collect_if makes no guarantee on the ordering of the output.
class CGV_API collect_if : public algorithm {
public:
	collect_if(GroupSize group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_predicate);
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_predicate);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments = {});

	bool read_count(cgv::render::context& ctx, size_t& out) {
		uint32_t count = 0;
		if(_atomic_counter_buffer.copy(ctx, 0, &count, 1)) {
			out = count;
			return true;
		}
		raise_error(errc::buffer_copy_to_host_error);
		return false;
	}

private:
	struct uniform_data {
		uint32_t input_begin = 0;
		uint32_t input_end = 0;
		uint32_t output_begin = 0;
	};

	compute_kernel _kernel;
	cgv::render::uniform_buffer<uniform_data> _uniform_buffer;
	storage_buffer _atomic_counter_buffer;
};

} // namespace generic

/// GPU compute shader implementation for copying values based on a boolean predicate. Unlike copy_if, collect_if makes no guarantee on the ordering of the output.
template<class T>
class collect_if : public generic::collect_if {
public:
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");

	using base = generic::collect_if;
	using base::base;

	bool init(cgv::render::context& ctx, const std::string& unary_predicate) {
		return init(ctx, {}, unary_predicate);
	}

	bool init(cgv::render::context& ctx, const argument_definitions& arguments, const std::string& unary_predicate) {
		sl::data_type value_type = register_type_representation<T>();
		return base::init(ctx, value_type, arguments, unary_predicate);
	}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
