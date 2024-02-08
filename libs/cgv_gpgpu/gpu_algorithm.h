#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_library.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv_gl/gl/gl.h>

#include <iostream>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** Definition of base functionality for highly parallel gpu algorithms. */
class CGV_API gpu_algorithm {
private:
	template<typename T>
	T calculate_num_groups_void(T num_elements, T num_elements_per_group) const {
		assert(T(0) != num_elements_per_group);
		assert(T(0) != num_elements);
		return (num_elements + num_elements_per_group - T(1)) / num_elements_per_group;
	}

protected:
	bool is_initialized_ = false;

	void ensure_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer, size_t size, cgv::render::VertexBufferType type = cgv::render::VertexBufferType::VBT_STORAGE, cgv::render::VertexBufferUsage usage = cgv::render::VertexBufferUsage::VBU_STREAM_COPY);

	void delete_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer);

	virtual bool load_shader_programs(cgv::render::context& ctx) = 0;

	unsigned calculate_padding(unsigned num_elements, unsigned num_elements_per_group) const {
		unsigned n_pad = num_elements_per_group - (num_elements % num_elements_per_group);
		if(num_elements % num_elements_per_group == 0)
			n_pad = 0;
		return n_pad;
	}

	unsigned calculate_num_groups(unsigned num_elements, unsigned num_elements_per_group) const {
		return calculate_num_groups_void(num_elements, num_elements_per_group);
	}

	uvec2 calculate_num_groups(uvec2 num_elements, uvec2 num_elements_per_group) const {
		return calculate_num_groups_void(num_elements, num_elements_per_group);
	}

	uvec3 calculate_num_groups(uvec3 num_elements, uvec3 num_elements_per_group) const {
		return calculate_num_groups_void(num_elements, num_elements_per_group);
	}

	void dispatch_compute1d(unsigned num_groups) {
		glDispatchCompute(num_groups, 1u, 1u);
	}

	void dispatch_compute2d(uvec2 num_groups) {
		glDispatchCompute(num_groups[0], num_groups[1], 1u);
	}

	void dispatch_compute3d(uvec3 num_groups) {
		glDispatchCompute(num_groups[0], num_groups[1], num_groups[2]);
	}

public:
	gpu_algorithm() {}
	~gpu_algorithm() {}

	virtual bool init(cgv::render::context& ctx, size_t count) = 0;

	virtual void destruct(const cgv::render::context& ctx) = 0;

	bool is_initialized() const { return is_initialized_; }
};

}
}

#include <cgv/config/lib_end.h>
