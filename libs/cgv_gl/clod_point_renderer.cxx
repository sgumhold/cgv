#include "clod_point_renderer.h"

namespace cgv {
	namespace render {
		//from opengl_context.cxx
		GLuint get_gl_id(const void* handle)
		{
			return (const GLuint&)handle - 1;
		}

		clod_point_render_style::clod_point_render_style()
		{
			float clod_factor;
		}

		void clod_point_renderer::draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			//renderer::draw_impl(ctx, type, start, count, use_strips, use_adjacency, strip_restart_index);

			//run compute shader
			
			/*
			reduce_prog.enable(ctx);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, drawp_pos, draw_parameter_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, render_pos, input_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input_pos, input_buffer);
			glDispatchCompute( static_cast<int>(std::ceil(positions.size()/128)), 1, 1);
			reduce_prog.disable(ctx);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			*/

			
			//draw
			//glBindBuffer(GL_ARRAY_BUFFER, render_buffer);
			//test render shaders
			
			draw_prog.enable(ctx);
			glBindVertexArray(vertex_array);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, input_buffer_data.size());
			glBindVertexArray(0);
			draw_prog.disable(ctx);
		}

		render_style* clod_point_renderer::create_render_style() const
		{
			return new clod_point_render_style();
		}

		bool clod_point_renderer::init(context& ctx)
		{
			ref_point_renderer(ctx,1);
			if (!reduce_prog.is_created()) {
				reduce_prog.create(ctx);
				add_shader(ctx, reduce_prog, "pointcloud_clod_filter_points.glcs", cgv::render::ST_COMPUTE);
				reduce_prog.link(ctx);
#ifndef NDEBUG
				std::cerr << reduce_prog.last_error;
#endif // #ifdef NDEBUG
			}
			
			//create shader program
			if (!draw_prog.is_created()) {
				draw_prog.create(ctx);
				add_shader(ctx, draw_prog, "view.glsl", cgv::render::ST_VERTEX);
				add_shader(ctx, draw_prog, "pointcloud_clod.glvs", cgv::render::ST_VERTEX);
				add_shader(ctx, draw_prog, "fragment.glfs", cgv::render::ST_FRAGMENT);
				add_shader(ctx, draw_prog, "pointcloud_clod.glfs", cgv::render::ST_FRAGMENT);
				draw_prog.link(ctx);
#ifndef NDEBUG
				std::cerr << draw_prog.last_error;
#endif // #ifdef NDEBUG
			}

			glGenBuffers(1, &input_buffer); //array of {float x;float y;float z;uint colors;};
			glGenBuffers(1, &render_buffer);
			glGenBuffers(1, &draw_parameter_buffer);

			glGenVertexArrays(1, &vertex_array);
			glBindVertexArray(vertex_array);
			//position
			glBindBuffer(GL_ARRAY_BUFFER,input_buffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			//color
			glVertexAttribPointer(1, 1, GL_INT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			return draw_prog.is_linked() && reduce_prog.is_linked();
		}

		bool clod_point_renderer::enable(context& ctx)
		{
			if (!draw_prog.is_linked()) {
				return false;
			}

			if (buffers_outofdate) {
				fill_buffers(ctx);
				buffers_outofdate = false;
			}

			//const clod_point_render_style& srs = get_style<clod_point_render_style>();
			//TODO set uniforms
			vec2 screenSize(ctx.get_width(), ctx.get_height());
			vec4 pivot = ctx.get_modelview_matrix().col(3);
			mat4 transform = ctx.get_modelview_projection_device_matrix();
			mat4 modelview_matrix = ctx.get_modelview_matrix();
			mat4 projection_matrix = ctx.get_projection_matrix();
			mat4 inverse_modelview_matrix = inv(modelview_matrix);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "CLOD"), CLOD);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "scale"), scale);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "spacing"), spacing);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "pivot"), pivot);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "screenSize"), screenSize);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "transform"), transform);
			//view.glsl
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "modelview_matrix"), modelview_matrix);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "projection_matrix"), projection_matrix);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "inverse_modelview_matrix"), inverse_modelview_matrix);

			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "transform"), transform);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "CLOD"), CLOD);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "scale"), scale);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "spacing"), spacing);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "pivot"), pivot);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "screenSize"), screenSize);
			//configure shader to compute everything after one frame
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchOffset"), 0);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchSize"), (int)positions.size());

			return true;
		}

		bool clod_point_renderer::disable(context& ctx)
		{
			/*
			const clod_point_render_style& srs = get_style<clod_point_render_style>();

			if (!attributes_persist()) {
				//TODO reset internal attributes
			}
			*/
			return true;
		}

		void clod_point_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_and_compute_impl(ctx, cgv::render::PT_POINTS, start, count, use_strips, use_adjacency, strip_restart_index);
		}

		void clod_point_renderer::add_shader(context& ctx, shader_program& prog, const std::string& sf,const cgv::render::ShaderType st)
		{
#ifndef NDEBUG
			std::cout << "add shader " << sf << '\n';
#endif // #ifdef NDEBUG
			prog.attach_file(ctx, sf, st);
#ifndef NDEBUG
			if (prog.last_error.size() > 0) {
				std::cerr << prog.last_error << '\n';
				prog.last_error = "";
			}	
#endif // #ifdef NDEBUG

		}

		void clod_point_renderer::fill_buffers(context& ctx)
		{ //  fill buffers for the compute shader
			drawParameters dp = drawParameters();

			glBindBuffer(GL_ARRAY_BUFFER, input_buffer);
			glBufferData(GL_ARRAY_BUFFER, input_buffer_data.size() * sizeof(Vertex), input_buffer_data.data(), GL_STATIC_READ);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			//glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
			//glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Vertex), input_buffer_data.data(), GL_STATIC_READ);
			
			//glBindBuffer(GL_SHADER_STORAGE_BUFFER, render_buffer);
			//glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

			//glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_parameter_buffer);
			//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(drawParameters), &dp, GL_DYNAMIC_READ);

			//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		}

		void clod_point_renderer::clear_buffers(context& ctx)
		{
			glDeleteBuffers(1, &input_buffer);
			glDeleteBuffers(1, &render_buffer);
			glDeleteBuffers(1, &draw_parameter_buffer);
			input_buffer = render_buffer = draw_parameter_buffer = 0;
		}


	}
}
