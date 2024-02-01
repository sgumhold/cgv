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
	/// members for timing measurements
	GLuint time_query = 0;

protected:
	bool is_initialized_ = false;

	void ensure_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer, size_t size, cgv::render::VertexBufferType type = cgv::render::VertexBufferType::VBT_STORAGE, cgv::render::VertexBufferUsage usage = cgv::render::VertexBufferUsage::VBU_STREAM_COPY);

	void delete_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer);

	virtual bool load_shader_programs(cgv::render::context& ctx) = 0;

	unsigned calculate_padding(unsigned num_elements, unsigned num_elements_per_group) {
		unsigned n_pad = num_elements_per_group - (num_elements % num_elements_per_group);
		if(num_elements % num_elements_per_group == 0)
			n_pad = 0;
		return n_pad;
	}

	template<typename T> T calculate_num_groups(T num_elements, T num_elements_per_group);

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

	virtual void destruct(const cgv::render::context& ctx) = 0;

	virtual bool init(cgv::render::context& ctx, size_t count) = 0;

	bool is_initialized() const { return is_initialized_; }

	void begin_time_query();

	float end_time_query();

	// helper method to read contents of a buffer back to CPU main memory
	template<typename T>
	std::vector<T> read_buffer(GLuint buffer, unsigned int count) {

		std::vector<T> data(count, (T)0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
		void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(&data[0], ptr, data.size() * sizeof(T));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		return data;
	}
};

}
}

#include <cgv/config/lib_end.h>
