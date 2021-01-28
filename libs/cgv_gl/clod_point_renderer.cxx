/*
Tests
near clipping			works
extended frustum culling	disabled
reduce compute shader	works? (test with different lod levels missing)
render shaders	unfinished
*/

#include "clod_point_renderer.h"

//#define CLOD_PR_RENDER_TEST_MODE _TM_

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

			// reset draw parameters
			DrawParameters dp = DrawParameters();
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_parameter_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawParameters), &dp, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			
			reduce_prog.enable(ctx);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, drawp_pos, draw_parameter_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, render_pos, render_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input_pos, input_buffer);
			glDispatchCompute((positions.size()/128)+1, 1, 1);

			// synchronize
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			reduce_prog.disable(ctx);

			// draw resulting buffer
			draw_prog.enable(ctx);
			glBindVertexArray(vertex_array);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_parameter_buffer);
			glDrawArraysIndirect(GL_POINTS, 0);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER,0);
#ifdef CLOD_PR_RENDER_TEST_MODE
			glDrawArrays(GL_POINTS, 0, input_buffer_data.size()); //TEST
#else
			
			//map buffer into host address space
			//DrawParameters* device_draw_parameters = static_cast<DrawParameters*>(glMapNamedBufferRange(draw_parameter_buffer, 0, sizeof(DrawParameters), GL_MAP_READ_BIT));
			//glUnmapNamedBuffer(draw_parameter_buffer);
			
#endif // CLOD_PR_RENDER_TEST_MODE
			glBindVertexArray(0);
			draw_prog.disable(ctx);
		}

		render_style* clod_point_renderer::create_render_style() const
		{
			return new clod_point_render_style();
		}

		bool clod_point_renderer::init(context& ctx)
		{
			if (!reduce_prog.is_created()) {
				reduce_prog.create(ctx);
				add_shader(ctx, reduce_prog, "view.glsl", cgv::render::ST_COMPUTE);
				add_shader(ctx, reduce_prog, "point_clod_filter_points.glcs", cgv::render::ST_COMPUTE);
				reduce_prog.link(ctx);
#ifndef NDEBUG
				std::cerr << reduce_prog.last_error;
#endif // #ifdef NDEBUG
			}
			
			//create shader program
			if (!draw_prog.is_created()) {
				draw_prog.build_program(ctx, "point_clod.glpr", true);
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
#ifdef CLOD_PR_RENDER_TEST_MODE
			glBindBuffer(GL_ARRAY_BUFFER,input_buffer); //test
#else 
			glBindBuffer(GL_ARRAY_BUFFER, render_buffer);
#endif
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			glEnableVertexAttribArray(0);
			//color
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(sizeof(Vertex::position)));
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
			
			mat4 modelview_matrix = ctx.get_modelview_matrix();
			mat4 projection_matrix = ctx.get_projection_matrix();

			draw_prog.set_uniform(ctx, "CLOD" , prs.CLOD);
			draw_prog.set_uniform(ctx, "scale", prs.scale);
			draw_prog.set_uniform(ctx, "spacing", prs.spacing);
			draw_prog.set_uniform(ctx, "pointSize", prs.pointSize);
			draw_prog.set_uniform(ctx, "minMilimeters", prs.min_millimeters);
			draw_prog.set_uniform(ctx, "screenSize", screenSize);
			draw_prog.set_uniform(ctx, "pivot", pivot);
			
			//view.glsl
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "modelview_matrix"), modelview_matrix);
			draw_prog.set_uniform(ctx, draw_prog.get_uniform_location(ctx, "projection_matrix"), projection_matrix);

			// compute shader
			reduce_prog.set_uniform(ctx, "modelview_matrix", modelview_matrix);
			reduce_prog.set_uniform(ctx, "projection_matrix", projection_matrix);
			reduce_prog.set_uniform(ctx, "CLOD", prs.CLOD);
			reduce_prog.set_uniform(ctx, "scale", prs.scale);
			reduce_prog.set_uniform(ctx, "spacing", prs.spacing);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "pivot"), pivot);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "screenSize"), screenSize);
			//configure shader to compute everything after one frame

			//TODO adapt every frame
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchOffset"), 0);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchSize"), (int)positions.size());


			//testcode
			float reference_point_size = 0.01f;
			float y_view_angle = 45;
			//general point renderer uniforms
			draw_prog.set_uniform(ctx, "use_color_index", false);
			draw_prog.set_uniform(ctx, "measure_point_size_in_pixel", prs.measure_point_size_in_pixel);
			draw_prog.set_uniform(ctx, "reference_point_size", reference_point_size);
			draw_prog.set_uniform(ctx, "use_group_point_size", prs.use_group_point_size);
			float pixel_extent_per_depth = (float)(2.0 * tan(0.5 * 0.0174532925199 * y_view_angle) / ctx.get_height());
			draw_prog.set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
			draw_prog.set_uniform(ctx, "blend_width_in_pixel", prs.blend_width_in_pixel);
			draw_prog.set_uniform(ctx, "percentual_halo_width", 0.01f * prs.percentual_halo_width);
			draw_prog.set_uniform(ctx, "halo_width_in_pixel", prs.halo_width_in_pixel);
			draw_prog.set_uniform(ctx, "halo_color", prs.halo_color);
			draw_prog.set_uniform(ctx, "halo_color_strength", prs.halo_color_strength);
			
			draw_prog.set_uniform(ctx, "use_group_color", false);
			draw_prog.set_uniform(ctx, "use_group_transformation", false);
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
			/*
			glBindBuffer(GL_ARRAY_BUFFER, input_buffer);
			glBufferData(GL_ARRAY_BUFFER, input_buffer_data.size() * sizeof(Vertex), input_buffer_data.data(), GL_STATIC_READ);
			glBindBuffer(GL_ARRAY_BUFFER, 0);*/
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Vertex), input_buffer_data.data(), GL_STATIC_READ);
			
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, render_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
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
