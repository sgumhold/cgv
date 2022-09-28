#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_library.h>
#include <cgv_gl/gl/gl.h>

#include <iostream>

#include "lib_begin.h"

using namespace cgv::render;

namespace cgv {
namespace gpgpu {

/** Definition of base functionality for highly parallel gpu algorithms. */
class CGV_API gpu_algorithm : public render_types {
private:
	/// members for timing measurements
	GLuint time_query = 0;

protected:
	bool _is_initialized = false;

	void create_buffer(GLuint& buffer, size_t size, GLenum usage = GL_DYNAMIC_COPY);

	void delete_buffer(GLuint& buffer);

	virtual bool load_shader_programs(context& ctx) = 0;

	unsigned calculate_padding(unsigned num_elements, unsigned num_elements_per_group) {
		unsigned n_pad = num_elements_per_group - (num_elements % num_elements_per_group);
		if(num_elements % num_elements_per_group == 0)
			n_pad = 0;
		return n_pad;
	}

	unsigned calculate_num_groups(unsigned num_elements, unsigned num_elements_per_group) {
		return (num_elements + num_elements_per_group - 1) / num_elements_per_group;
	}

public:
	gpu_algorithm() {}
	~gpu_algorithm() {}

	void destruct(context& ctx);

	bool init(context& ctx, size_t count);

	bool is_initialized() const { return _is_initialized; }

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

