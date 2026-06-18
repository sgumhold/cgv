#pragma once

#include <cgv/render/uniform_buffer.h>

#include "algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for performing a scatter operation on a range of elements.
class CGV_API scatter : public algorithm {
public:
	scatter(uint32_t group_size = k_default_group_size);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& map, const cgv::render::vertex_buffer& output_buffer, size_t count);
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator map_first, device_buffer_iterator output_first);

private:
	struct uniform_data {
		uint32_t input_begin = 0;
		uint32_t input_end = 0;
		uint32_t output_begin = 0;
		uint32_t map_begin = 0;
	};

	compute_kernel _kernel;
	cgv::render::uniform_buffer<uniform_data> _uniform_buffer;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
