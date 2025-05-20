#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// ensure the buffer is created and has the specified properties
extern CGV_API bool ensure_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer, size_t size, cgv::render::VertexBufferType type = cgv::render::VertexBufferType::VBT_STORAGE, cgv::render::VertexBufferUsage usage = cgv::render::VertexBufferUsage::VBU_STREAM_COPY);

/// return the lowest power of two greater than or equal to x
extern CGV_API uint32_t next_power_of_two(uint32_t x);
/// return the lowest multiple of y greater than or equal to x
extern CGV_API uint32_t next_multiple_greater_than(uint32_t x, uint32_t y);

/// return ceil(a/b) for integers
template<typename T>
T div_round_up(T a, T b) {
	assert(a != T(0));
	assert(b != T(0));
	return (a + b - T(1)) / b;
}

template<typename T>
class double_buffer_wrapper {
public:
	double_buffer_wrapper(T* first, T* second) : _first(first), _second(second) {}

	T* first() { return _first; }
	T* second() { return _second; }

	void swap() { std::swap(_first, _second); }
private:
	T* _first = nullptr;
	T* _second = nullptr;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
