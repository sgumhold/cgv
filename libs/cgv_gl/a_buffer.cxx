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
			ensure_buffer(node_buffer, GL_SHADER_STORAGE_BUFFER, nodes_per_fragment * 3 * num_pixels * sizeof(GLuint), NULL); // buffer to store linked list nodes, each with 3 32-bit values; reserve space for as much as 64 fragments per pixel
		}
		a_buffer::a_buffer() : depth_tex("[D]")
		{
			depth_tex.set_min_filter(TF_NEAREST);
			depth_tex.set_mag_filter(TF_NEAREST);
			nodes_per_fragment = 64;
			node_buffer_counter = 0;
			head_pointer_buffer = 0;
			node_buffer = 0;
		}
		bool a_buffer::init(context& ctx)
		{
			if (!clear_ssbo_prog.build_files(ctx, "clear_ssbo", true))
				return false;
			if (!a_buffer_prog.build_program(ctx, "a_buffer.glpr"))
				return false;
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
		}
		void a_buffer::enable(context& ctx, shader_program& prog)
		{
			depth_tex.replace_from_buffer(ctx, 0, 0, 0, 0, ctx.get_width(), ctx.get_height());
			depth_tex.enable(ctx, 0);

			// Bind buffers for first a-buffer pass
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, node_buffer_counter);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, head_pointer_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, node_buffer);

			prog.set_uniform(ctx, "viewport_dims", ivec2(ctx.get_width(), ctx.get_height()));
			prog.set_uniform(ctx, "depth_tex", 0);
		}
		// return current number of nodes in node buffer
		size_t a_buffer::disable(context& ctx)
		{
			depth_tex.disable(ctx);

			// Clear the atomic counter
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
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
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			a_buffer_prog.disable(ctx);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
			glDisable(GL_BLEND);
		}
	}
}