#include "a_buffer.h"

namespace cgv {
	namespace render {

		void a_buffer::ensure_buffers(context& ctx)
		{
			unsigned num_pixels = ctx.get_width() * ctx.get_height();
			// Generate buffers for the a-buffer transparency implementation
			node_counter_buffer.create_or_resize(ctx, sizeof(GLuint));
			head_pointer_buffer.create_or_resize(ctx, num_pixels * sizeof(GLuint));
			node_buffer.create_or_resize(ctx, nodes_per_pixel * 3 * num_pixels * sizeof(GLuint));
		}
		void a_buffer::update_shader_program_options(shader_compile_options& options, bool include_binding_points)
		{
			options.define_macro_if_not_default("MAX_FRAGMENTS", fragments_per_pixel, 32u);
			if(include_binding_points) {
				options.define_macro_if_not_default("NODE_COUNTER_BINDING_POINT", node_counter_binding_point, 0);
				options.define_macro_if_not_default("HEAD_POINTERS_BINDING_POINT", head_pointers_binding_point, 0);
				options.define_macro_if_not_default("NODES_BINDING_POINT", nodes_binding_point, 1);
			}
		}
		void a_buffer::update_shader_program_options(shader_compile_options& options)
		{
			update_shader_program_options(options, true);
		}
		a_buffer::a_buffer(unsigned _fragments_per_pixel, unsigned _nodes_per_pixel, int _depth_tex_unit, 
			int _node_counter_binding_point, int _head_pointers_binding_point, int _nodes_binding_point)
			: depth_tex("[D]")
		{
			depth_tex.set_min_filter(TF_NEAREST);
			depth_tex.set_mag_filter(TF_NEAREST);
			fragments_per_pixel = _fragments_per_pixel;
			nodes_per_pixel = _nodes_per_pixel;

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
			update_shader_program_options(prog_options, false);
			if (!a_buffer_prog.build_program(ctx, "a_buffer.glpr", prog_options, true))
				return false;
			last_prog_options = prog_options;
			return true;
		}
		void a_buffer::destruct(context& ctx)
		{
			clear_ssbo_prog.destruct(ctx);
			a_buffer_prog.destruct(ctx);
			depth_tex.destruct(ctx);
			node_counter_buffer.destruct(ctx);
			head_pointer_buffer.destruct(ctx);
			node_buffer.destruct(ctx);
		}
		void a_buffer::init_frame(context& ctx)
		{
			// Ensure depth texture and node buffers of correct size
			if (depth_tex.is_created() && (ctx.get_width() != depth_tex.get_width() || ctx.get_height() != depth_tex.get_height()))
				depth_tex.destruct(ctx);

			if (!depth_tex.is_created())
				depth_tex.create(ctx, TT_2D, ctx.get_width(), ctx.get_height());

			ensure_buffers(ctx);

			// Clear the atomic counter
			const GLuint zero = 0;
			node_counter_buffer.replace(ctx, 0, &zero, 1);

			// Clear the head pointer buffer using a compute shader
			head_pointer_buffer.bind(ctx, 0);
			unsigned buffer_size = ctx.get_width() * ctx.get_height();
			clear_ssbo_prog.enable(ctx);
			clear_ssbo_prog.set_uniform(ctx, "size", buffer_size);
			clear_ssbo_prog.set_uniform(ctx, "clear_value", -1);
			glDispatchCompute(GLuint(ceil(buffer_size / 4)), 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			clear_ssbo_prog.disable(ctx);
			head_pointer_buffer.unbind(ctx, 0);

			// Check if the shader program needs to be rebuilt
			update_shader_program_options(prog_options, false);
			if (prog_options != last_prog_options) {
				if (a_buffer_prog.is_created())
					a_buffer_prog.destruct(ctx);
				if (a_buffer_prog.build_program(ctx, "a_buffer.glpr", prog_options, true)) {
					last_prog_options = prog_options;
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
			node_counter_buffer.bind(ctx, node_counter_binding_point);
			head_pointer_buffer.bind(ctx, head_pointers_binding_point);
			node_buffer.bind(ctx, nodes_binding_point);

			return true;
		}
		// return current number of nodes in node buffer
		void a_buffer::disable(context& ctx, size_t* node_count)
		{
			// Unbind texture and buffers
			depth_tex.disable(ctx);

			node_counter_buffer.unbind(ctx, node_counter_binding_point);
			head_pointer_buffer.unbind(ctx, head_pointers_binding_point);
			node_buffer.unbind(ctx, nodes_binding_point);

			// Read back node count if requested
			if(node_count) {
				GLuint temp = 0;
				node_counter_buffer.copy(ctx, 0, &temp, 1);
				*node_count = temp;
			}
		}
		void a_buffer::finish_frame(context& ctx)
		{
			ctx.push_blend_state();
			ctx.enable_blending();
			ctx.set_blend_func_back_to_front();

			head_pointer_buffer.bind(ctx, 0);
			node_buffer.bind(ctx, 1);

			a_buffer_prog.enable(ctx);
			a_buffer_prog.set_uniform(ctx, "viewport_dims", ivec2(ctx.get_width(), ctx.get_height()));
			a_buffer_prog.set_uniform(ctx, "nodes_per_pixel", nodes_per_pixel);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			a_buffer_prog.disable(ctx);

			head_pointer_buffer.unbind(ctx, 0);
			node_buffer.unbind(ctx, 1);

			ctx.pop_blend_state();
		}
	}
}