#include "sort_algorithm.h"

#include <cgv/utils/scan.h>

namespace cgv {
namespace gpgpu {

sort_algorithm::sort_algorithm(const std::string& type_name) : algorithm(type_name, GroupSize::Auto) {}

bool sort_algorithm::resize(cgv::render::context& ctx, size_t size) {
	if(static_cast<size_t>(_num_keys) == size)
		return true;

	if(!is_size_supported(size)) {
		raise_error(errc::size_too_large);
		return false;
	}
	
	_num_keys = static_cast<uint32_t>(size);
	return v_resize(ctx);
}

void sort_algorithm::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer) {
	v_dispatch(ctx, &keys_buffer, nullptr);
}

void sort_algorithm::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer, const cgv::render::vertex_buffer& values_buffer) {
	v_dispatch(ctx, &keys_buffer, &values_buffer);
}

bool sort_algorithm::is_size_supported(size_t size) const {
	constexpr size_t max_size = 0xFFFFFFFF;
	return size > 0 && size <= max_size;
}

} // namespace gpgpu
} // namespace cgv
