#include "utils.h"

namespace cgv {
namespace gpgpu {

bool ensure_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer, size_t size_in_bytes, cgv::render::VertexBufferType type, cgv::render::VertexBufferUsage usage) {
	if(buffer.is_created() && buffer.type == type && buffer.usage == usage) {
		return buffer.resize(ctx, size_in_bytes);
	} else {
		buffer.destruct(ctx);
		buffer = cgv::render::vertex_buffer(type, usage);
		return buffer.create(ctx, size_in_bytes);
	}
}

uint32_t next_power_of_two(uint32_t x) {
	uint32_t pot = 1;
	while(x > pot)
		pot <<= 1;
	return pot;
}

uint32_t next_multiple_greater_than(uint32_t x, uint32_t y) {
	uint32_t remainder = y - (x % y);
	if(x % y == 0)
		remainder = 0;
	return x + remainder;
}

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
