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
			reduce_prog.enable(ctx);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, drawp_pos, draw_parameter_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, render_pos, input_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input_pos, input_buffer);
			glDispatchCompute( static_cast<int>(std::ceil(positions.size()/128)), 1, 1);
			reduce_prog.disable(ctx);
			
			//draw
			ref_prog().enable(ctx);
			glBindBuffer(GL_ARRAY_BUFFER, render_buffer);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 4, GL_INT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			// color attribute
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glEnableClientState(GL_VERTEX_ARRAY);
			glDrawArrays(GL_POINTS, 0, input_buffer_data.size());
			glDisableClientState(GL_VERTEX_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			ref_prog().disable(ctx);
		}

		render_style* clod_point_renderer::create_render_style() const
		{
			return new clod_point_render_style();
		}

		bool clod_point_renderer::init(context& ctx)
		{
			if (!reduce_prog.is_created()) {
				reduce_prog.create(ctx);
				add_shader(ctx, reduce_prog, "pointcloud_clod_filter_points.glcs", cgv::render::ST_COMPUTE);
				reduce_prog.link(ctx);
#ifndef NDEBUG
				std::cerr << reduce_prog.last_error;
#endif // #ifdef NDEBUG
			}
			
			//create shader program
			if (!ref_prog().is_created()) {
				ref_prog().create(ctx);
				add_shader(ctx, ref_prog(), "view.glsl", cgv::render::ST_VERTEX);
				add_shader(ctx, ref_prog(), "pointcloud_clod.glvs", cgv::render::ST_VERTEX);
				add_shader(ctx, ref_prog(), "fragment.glfs", cgv::render::ST_FRAGMENT);
				add_shader(ctx, ref_prog(), "pointcloud_clod.glfs", cgv::render::ST_FRAGMENT);
				ref_prog().link(ctx);
#ifndef NDEBUG
				std::cerr << ref_prog().last_error;
#endif // #ifdef NDEBUG
			}
			return ref_prog().is_linked() && reduce_prog.is_linked();
		}

		bool clod_point_renderer::enable(context& ctx)
		{
			if (!ref_prog().is_linked()) {
				return false;
			}
			GLuint render_prog = get_gl_id(ref_prog().handle); //TODO find a clean way for this

			const clod_point_render_style& srs = get_style<clod_point_render_style>();
			//TODO set uniforms
			vec2 screenSize(ctx.get_width(), ctx.get_height());
			vec4 pivot = ctx.get_modelview_matrix().col(3);
			mat4 transform = ctx.get_modelview_projection_device_matrix();
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "CLOD"), CLOD);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "scale"), scale);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "spacing"), spacing);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "pivot"), pivot);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "screenSize"), screenSize);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "transform"), transform);
			
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "transform"), transform);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "CLOD"), CLOD);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "scale"), scale);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "spacing"), spacing);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "pivot"), pivot);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "screenSize"), screenSize);
			//configure shader to compute everything after one frame
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchOffset"), 0);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchSize"), (int)positions.size());

			{ // create and fill buffers of the compute shader

				drawParameters dp = drawParameters();
				
				reduce_prog.enable(ctx);

				glGenBuffers(1, &input_buffer); //array of {float x;float y;float z;uint colors;};
				glGenBuffers(1, &render_buffer);
				glGenBuffers(1, &draw_parameter_buffer);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Vertex), input_buffer_data.data(), GL_STATIC_READ);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, render_buffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_parameter_buffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(drawParameters), &dp, GL_DYNAMIC_READ);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
			return true;
		}

		bool clod_point_renderer::disable(context& ctx)
		{
			const clod_point_render_style& srs = get_style<clod_point_render_style>();

			if (!attributes_persist()) {
				//TODO reset internal attributes
			}
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


	}
}
