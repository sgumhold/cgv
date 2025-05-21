#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

namespace cgv {
namespace gpgpu {

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

class vertex_double_buffer_wrapper : public double_buffer_wrapper<const cgv::render::vertex_buffer> {
public:
	using double_buffer_wrapper::double_buffer_wrapper;

	void bind_all(cgv::render::context& ctx, uint32_t first_index, uint32_t second_index) {
		if(binding_type_override != cgv::render::VertexBufferType::VBT_UNDEF) {
			first()->bind(ctx, first_index);
			second()->bind(ctx, second_index);
		} else {
			first()->bind(ctx, binding_type_override, first_index);
			second()->bind(ctx, binding_type_override, second_index);
		}
	}

	void unbind_all(cgv::render::context& ctx, uint32_t first_index, uint32_t second_index) {
		if(binding_type_override != cgv::render::VertexBufferType::VBT_UNDEF) {
			first()->unbind(ctx, first_index);
			second()->unbind(ctx, second_index);
		} else {
			first()->unbind(ctx, binding_type_override, first_index);
			second()->unbind(ctx, binding_type_override, second_index);
		}
	}

	cgv::render::VertexBufferType binding_type_override = cgv::render::VertexBufferType::VBT_UNDEF;
};

} // namespace gpgpu
} // namespace cgv
