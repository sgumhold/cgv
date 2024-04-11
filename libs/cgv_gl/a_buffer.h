#pragma once 

#include <cgv/render/shader_program.h>
#include <cgv/render/texture.h>
#include <cgv_gl/gl/gl.h>

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {

/** This class provides a_buffer functionality.
	Compare cgv/test/a_buffer_test for an example of using this class.*/
class CGV_API a_buffer
{
	unsigned last_fragments_per_pixel;
	unsigned last_nodes_per_pixel;
	shader_define_map last_defines;
	shader_define_map defines;
	bool init_frame_called;
protected:
	/// Depth texture used to emulate depth buffer
	texture depth_tex;
	/// Buffers used to store per pixel frament lists
	unsigned node_buffer_counter;
	unsigned head_pointer_buffer;
	unsigned node_buffer;
	/// Default texture unit used for depth texture
	int depth_tex_unit;
	/// Buffer binding point indices
	int node_counter_binding_point; 
	int head_pointers_binding_point;
	int nodes_binding_point;
	// GPU objects
	shader_program clear_ssbo_prog;
	shader_program a_buffer_prog;
	
	static void ensure_buffer(GLuint& buffer, GLenum target, GLsizeiptr size, const void* data, GLenum usage = GL_DYNAMIC_DRAW);
	static void destruct_buffer(GLuint& buffer);
	void destruct_buffers(context& ctx);
	void ensure_buffers(context& ctx);

	void update_defines(shader_define_map& defines, bool include_binding_points);
public:
	/// construct and configure
	a_buffer(unsigned _fragments_per_pixel = 32, unsigned _nodes_per_pixel = 64, int _depth_tex_unit = 0, 
		int _node_counter_binding_point = 0, int _head_pointers_binding_point = 0, int _nodes_binding_point = 1);
	/// to be handled fragments per pixel (changes are applied when in init_frame() function)
	unsigned fragments_per_pixel;
	/// to be reserved number of fragment nodes per pixel (changes are applied when in init_frame() function)
	unsigned nodes_per_pixel;
	/// construct internally used programs
	bool init(context& ctx);
	/// destruct all render objects
	void destruct(context& ctx);
	/// ensure that a_buffer size corresponds to context size
	void init_frame(context& ctx);
	/// update the provided shader defines with respect to a_buffer settings. Call before enabling a program.
	void update_defines(shader_define_map& defines);
	//! Enable writing fragments to a_buffer with provided program
	/*! In first call after each init frame call, the current depth buffer is copied to a_buffer's depth texture.
		Function sets necessary uniforms in program and binds depth texture and buffers.
		In a_buffer constructor depth texture unit can be overwritten by third argument.
		Function fails by returning false if program query of binding points to buffers fail.
	*/
	bool enable(context& ctx, shader_program& prog, int tex_unit = -1);
	/// finish writing fragments to a_buffer and return current number of nodes in node buffer
	size_t disable(context& ctx);
	/// per fragment sort nodes and blend over current framebuffer
	void finish_frame(context& ctx);

	int& ref_depth_tex_unit() { return depth_tex_unit; }
	int& ref_node_counter_binding_point () { return  node_counter_binding_point  ; }
	int& ref_head_pointers_binding_point() { return	 head_pointers_binding_point ; }
	int& ref_nodes_binding_point		() { return	 nodes_binding_point		 ; }
};
	}
}

#include <cgv/config/lib_end.h>