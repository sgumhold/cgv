#include "gpu_algorithm.h"

namespace cgv {
namespace gpgpu {

std::map<std::string, int> gpu_algorithm::get_program_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog) const {
	GLuint id = reinterpret_cast<GLuint>(prog.handle) - 1;

	std::map<std::string, int> map;

	if(id == 0)
		return map;
	
	GLint num_active_uniforms = 0;
	glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &num_active_uniforms);
	
	std::vector<GLchar> buffer(256);
	for(int i = 0; i < num_active_uniforms; ++i) {
		GLint array_size = 0;
		GLenum type = 0;
		GLsizei actual_length = 0;
		glGetActiveUniform(id, i, buffer.size(), &actual_length, &array_size, &type, buffer.data());
		std::string name(static_cast<char*>(buffer.data()), actual_length);

		int location = prog.get_uniform_location(ctx, name);
		if(location > -1)
			map[name] = location;
	}

	return map;
}

void gpu_algorithm::ensure_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer, size_t size, cgv::render::VertexBufferType type, cgv::render::VertexBufferUsage usage) {

	if(buffer.is_created() && buffer.type == type && buffer.usage == usage) {
		buffer.resize(ctx, size);
	} else {
		buffer.destruct(ctx);
		buffer = cgv::render::vertex_buffer(type, usage);
		buffer.create(ctx, size);
	}
}

void gpu_algorithm::delete_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer) {

	buffer.destruct(ctx);
}

}
}
