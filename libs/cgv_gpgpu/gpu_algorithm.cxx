#include "gpu_algorithm.h"

namespace cgv {
namespace gpgpu {

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

void gpu_algorithm::begin_time_query() {

	time_query = 0;
	glGenQueries(1, &time_query);

	glBeginQuery(GL_TIME_ELAPSED, time_query);
}

float gpu_algorithm::end_time_query() {

	glEndQuery(GL_TIME_ELAPSED);

	GLint done = false;
	while(!done) {
		glGetQueryObjectiv(time_query, GL_QUERY_RESULT_AVAILABLE, &done);
	}
	GLuint64 elapsed_time = 0;
	glGetQueryObjectui64v(time_query, GL_QUERY_RESULT, &elapsed_time);

	return elapsed_time / 1000000.0f;
}

}
}
