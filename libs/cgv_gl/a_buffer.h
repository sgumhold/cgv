#pragma once 

#include <cgv/render/shader_program.h>
#include <cgv/render/texture.h>
#include <cgv_gl/gl/gl.h>

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {

/** This class provides a_buffer functionality.
	Compare cgv/test/a_buffer_test for an example of using this class.*/
class CGV_API a_buffer : public render_types
{
protected:
	texture depth_tex;
	unsigned nodes_per_fragment;
	// Buffers
	GLuint node_buffer_counter;
	GLuint head_pointer_buffer;
	GLuint node_buffer;

	shader_program clear_ssbo_prog;
	shader_program a_buffer_prog;
	
	static void ensure_buffer(GLuint& buffer, GLenum target, GLsizeiptr size, const void* data, GLenum usage = GL_DYNAMIC_DRAW);
	static void destruct_buffer(GLuint& buffer);
	void destruct_buffers(context& ctx);
	void ensure_buffers(context& ctx);
public:
	/// construct with 64 nodes per fragment
	a_buffer();
	/// construct internally used programs
	bool init(context& ctx);
	/// destruct all render objects
	void destruct(context& ctx);
	/// ensure that a_buffer size corresponds to context size
	void init_frame(context& ctx);
	/// enable writing fragments to a_buffer with provided program
	void enable(context& ctx, shader_program& prog);
	/// finish writing fragments to a_buffer and return current number of nodes in node buffer
	size_t disable(context& ctx);
	/// per fragment sort nodes and blend over current framebuffer
	void finish_frame(context& ctx);
};
	}
}

#include <cgv/config/lib_end.h>