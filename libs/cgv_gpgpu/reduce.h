#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation of parallel reduction.
class CGV_API reduce : public algorithm {
public:
	reduce(uint32_t group_count = 256, uint32_t group_size = 128);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	template<typename T, typename std::enable_if<!std::is_same<std::string, T>::value && !std::is_same<const char*, T>::value, bool>::type = true>
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, T binary_operation) {
		std::string operation_string = "return " + binary_operation("lhs", "rhs") + ";";
		return init(ctx, value_type, operation_string);
	}

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& binary_operation);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output, const argument_bindings& arguments = {});

	template<typename T>
	bool read_result(cgv::render::context& ctx, T& out) {
		std::vector<T> res(1);
		if(_group_reduction_buffer.copy(ctx, res)) {
			out = res.front();
			return true;
		}
		return false;
	}

	static const std::string init_argument_name;

private:
	//uint32_t _group_size = 128;
	uint32_t _num_groups = 256;

	compute_kernel _kernel;

	storage_buffer _group_reduction_buffer;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
