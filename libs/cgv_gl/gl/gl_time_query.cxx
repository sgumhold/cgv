#include "gl_time_query.h"

#include <cgv_gl/gl/gl.h>

namespace cgv {
	namespace render {
		namespace gl {

gl_time_query::gl_time_query()
{
}

gl_time_query::~gl_time_query()
{
	if(is_initialized())
		destruct(*ctx_ptr);
}

bool gl_time_query::init(context& ctx)
{
	if(!ensure_glew_initialized()) {
		last_error = "could not initialize glew";
		return false;
	}
	if(!GLEW_ARB_timer_query) {
		last_error = "missing ARB timer query extension";
		return false;
	}
	
	glGenQueries(1, &query);

	ctx_ptr = &ctx;
	return query != 0;
}

void gl_time_query::destruct(context& ctx)
{
	glDeleteQueries(1, &query);
	query = 0;
	
	ctx_ptr = nullptr;
}

bool gl_time_query::is_initialized() const
{
	return ctx_ptr != nullptr;
}

void gl_time_query::begin()
{
	glBeginQuery(GL_TIME_ELAPSED, query);
}

double gl_time_query::end()
{
	glEndQuery(GL_TIME_ELAPSED);

	GLint done = false;
	while(!done)
		glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);

	GLuint64 elapsed_time = 0;
	glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);

	return double(elapsed_time);
}

		} // namespace cgv
	} // namespace render
} // namespace gl
