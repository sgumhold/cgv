#include "a_buffer.h"

namespace cgv {
	namespace render {

		void a_buffer::ensure_buffer(GLuint& buffer, GLenum target, GLsizeiptr size, const void* data, GLenum usage)
		{
			if (buffer == 0) {
				glGenBuffers(1, &buffer);
				glBindBuffer(target, buffer);
				glBufferData(target, size, data, usage);
				glBindBuffer(target, 0);
			}
		}

		void a_buffer::destruct_buffer(GLuint& buffer)
		{
			if (buffer != 0) {
				glDeleteBuffers(1, &buffer);
				buffer = 0;
			}
		}
		void a_buffer::destruct_buffers(context& ctx)
		{
			destruct_buffer(node_buffer_counter);
			destruct_buffer(head_pointer_buffer);
			destruct_buffer(node_buffer);
		}
		void a_buffer::ensure_buffers(context& ctx)
		{
			unsigned num_pixels = ctx.get_width() * ctx.get_height();
			// Generate buffers for the a-buffer transparency implementation
			ensure_buffer(node_buffer_counter, GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_STATIC_DRAW); // atomic counter variable
			ensure_buffer(head_pointer_buffer, GL_SHADER_STORAGE_BUFFER, num_pixels * sizeof(GLuint), NULL); // linked list head pointer buffer
			if (node_buffer != 0 && last_nodes_per_pixel != nodes_per_pixel)
				destruct_buffer(node_buffer);
			ensure_buffer(node_buffer, GL_SHADER_STORAGE_BUFFER, nodes_per_pixel * 3 * num_pixels * sizeof(GLuint), NULL); // buffer to store linked list nodes, each with 3 32-bit values; reserve space for as much as 64 fragments per pixel
			last_nodes_per_pixel = nodes_per_pixel;
		}
		void a_buffer::update_defines(shader_define_map& defines, bool include_binding_points)
		{
			if (fragments_per_pixel == 32)
				defines.erase("MAX_FRAGMENTS");
			else
				defines["MAX_FRAGMENTS"] = cgv::utils::to_string(fragments_per_pixel);

			if (!include_binding_points)
				return;

			if (node_counter_binding_point == 0)
				defines.erase("NODE_COUNTER_BINDING_POINT");
			else
				defines["NODE_COUNTER_BINDING_POINT"] = cgv::utils::to_string(node_counter_binding_point);
			if (head_pointers_binding_point == 0)
				defines.erase("HEAD_POINTERS_BINDING_POINT");
			else
				defines["HEAD_POINTERS_BINDING_POINT"] = cgv::utils::to_string(head_pointers_binding_point);
			if (nodes_binding_point == 1)
				defines.erase("NODES_BINDING_POINT");
			else
				defines["NODES_BINDING_POINT"] = cgv::utils::to_string(nodes_binding_point);
		}
		void a_buffer::update_defines(shader_define_map& defines)
		{
			update_defines(defines, true);
		}
		a_buffer::a_buffer(unsigned _fragments_per_pixel, unsigned _nodes_per_pixel, int _depth_tex_unit, 
			int _node_counter_binding_point, int _head_pointers_binding_point, int _nodes_binding_point)
			: depth_tex("[D]")
		{
			depth_tex.set_min_filter(TF_NEAREST);
			depth_tex.set_mag_filter(TF_NEAREST);
			fragments_per_pixel = _fragments_per_pixel;
			nodes_per_pixel = _nodes_per_pixel;
			last_nodes_per_pixel = 0;
			node_buffer_counter = 0;
			head_pointer_buffer = 0;
			node_buffer = 0;

			depth_tex_unit = _depth_tex_unit;
			node_counter_binding_point = _node_counter_binding_point;
			head_pointers_binding_point = _head_pointers_binding_point;
			nodes_binding_point = _nodes_binding_point;

			init_frame_called = false;
		}
		bool a_buffer::init(context& ctx)
		{
			if (!clear_ssbo_prog.build_files(ctx, "a_buffer_clear", true))
				return false;
			update_defines(defines, false);
			if (!a_buffer_prog.build_program(ctx, "a_buffer.glpr", true, defines))
				return false;
			last_defines = defines;
			return true;
		}
		void a_buffer::destruct(context& ctx)
		{
			clear_ssbo_prog.destruct(ctx);
			a_buffer_prog.destruct(ctx);
			depth_tex.destruct(ctx);
			destruct_buffers(ctx);
		}
		void a_buffer::init_frame(context& ctx)
		{
			// Ensure depth texture and node buffers of correct size
			if (depth_tex.is_created() && (ctx.get_width() != depth_tex.get_width() || ctx.get_height() != depth_tex.get_height())) {
				depth_tex.destruct(ctx);
				destruct_buffer(head_pointer_buffer);
				destruct_buffer(node_buffer);
			}
			if (!depth_tex.is_created())
				depth_tex.create(ctx, TT_2D, ctx.get_width(), ctx.get_height());
			ensure_buffers(ctx);

			// Clear the atomic counter
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, node_buffer_counter);
			GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
			memset(ptr, 0, sizeof(GLuint));
			glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

			// Clear the head pointer buffer
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, head_pointer_buffer);
			unsigned buffer_size = ctx.get_width() * ctx.get_height();
			clear_ssbo_prog.enable(ctx);
			clear_ssbo_prog.set_uniform(ctx, "size", buffer_size);
			clear_ssbo_prog.set_uniform(ctx, "clear_value", -1);
			glDispatchCompute(GLuint(ceil(buffer_size / 4)), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			clear_ssbo_prog.disable(ctx);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

			// check for rebuild of program
			update_defines(defines, false);
			if (defines != last_defines) {
				if (a_buffer_prog.is_created())
					a_buffer_prog.destruct(ctx);
				if (a_buffer_prog.build_program(ctx, "a_buffer.glpr", true, defines)) {
					last_defines = defines;
					std::cout << "a_buffer: rebuilt shader program" << std::endl;
				}
			}
			init_frame_called = true;
		}
		bool a_buffer::enable(context& ctx, shader_program& prog, int tex_unit)
		{
			// in first call after init_frame, copy depth buffer to depth texture
			if (init_frame_called) {
				depth_tex.replace_from_buffer(ctx, 0, 0, 0, 0, ctx.get_width(), ctx.get_height());
				init_frame_called = false;
			}
			// set program uniforms
			if (!prog.set_uniform(ctx, "viewport_dims", ivec2(ctx.get_width(), ctx.get_height()))) {
				std::cerr << "ERROR in a_buffer::enable(): uniform ivec2 viewport_dims not found in program." << std::endl;
				return false;
			}
			if (!prog.set_uniform(ctx, "nodes_per_pixel", (int)nodes_per_pixel)) {
				std::cerr << "ERROR in a_buffer::enable(): uniform int nodes_per_pixel not found in program." << std::endl;
				return false;
			}
			// enable depth texture
			if (tex_unit == -1)
				tex_unit = depth_tex_unit;
			depth_tex.enable(ctx, tex_unit);
			if (!prog.set_uniform(ctx, "depth_tex", tex_unit)) {
				std::cerr << "ERROR in a_buffer::enable(): uniform sampler2D depth_tex not found in program." << std::endl;
				return false;
			}
			// Bind buffers for first a-buffer pass			
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, node_counter_binding_point, node_buffer_counter);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, head_pointers_binding_point, head_pointer_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, nodes_binding_point, node_buffer);
			return true;
		}
		// return current number of nodes in node buffer
		size_t a_buffer::disable(context& ctx)
		{
			// Unbind texture and buffers
			depth_tex.disable(ctx);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, node_counter_binding_point, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, head_pointers_binding_point, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, nodes_binding_point, 0);

			// Clear atomic counter, read it back and use it as return value
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, node_buffer_counter);
			GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
			GLuint node_cnt = *ptr;
			glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
			return node_cnt;
		}
		void a_buffer::finish_frame(context& ctx)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, head_pointer_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, node_buffer);

			a_buffer_prog.enable(ctx);
			a_buffer_prog.set_uniform(ctx, "viewport_dims", ivec2(ctx.get_width(), ctx.get_height()));
			a_buffer_prog.set_uniform(ctx, "nodes_per_pixel", nodes_per_pixel);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			a_buffer_prog.disable(ctx);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

			glDisable(GL_BLEND);
		}
	}
}