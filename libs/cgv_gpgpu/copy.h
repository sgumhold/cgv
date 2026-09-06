#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation for copying values of a range.
class CGV_API copy : public algorithm {
public:
	copy(GroupSize group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count);
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first);

private:
	compute_kernel _kernel;
};

} // namespace generic

/// GPU compute shader implementation for copying values of a range.
template<class T>
class copy : public generic::copy {
public:
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");

	using base = generic::copy;
	using base::base;

	bool init(cgv::render::context& ctx) {
		sl::data_type value_type = register_type_representation<T>();
		return base::init(ctx, value_type);
	}
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
