#pragma once

#include "algorithm.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation of parallel reduction.
class CGV_API reduce : public algorithm {
public:
	reduce();
	reduce(uint32_t group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	template<typename T, typename std::enable_if<!std::is_same<std::string, T>::value && !std::is_same<const char*, T>::value, bool>::type = true>
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, T binary_operation) {
		std::string operation_string = "return " + binary_operation("lhs", "rhs") + ";";
		return init(ctx, value_type, operation_string);
	}

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& binary_operation);

	// TODO: choose thread count based on shared memory availability.
	// TODO: have option to put reduced value in first place of buffer or in _group_reduction_buffer.
	// TODO: compute approx elements per thread and define num_groups based on that and group_size
	// TODO: aim for 4 to 8 elements per thread? Or even more?
	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments = {});

	static const std::string init_argument_name;

private:
	uint32_t _group_size = 128;
	uint32_t _num_groups = 256;

	compute_kernel _kernel;

	storage_buffer _group_reduction_buffer;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
