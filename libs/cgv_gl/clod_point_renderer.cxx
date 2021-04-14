#include <algorithm>
#include <random>
#include <unordered_map>
#include <sstream>
#include "clod_point_renderer.h"

//**

namespace cgv {
	namespace render {

		clod_point_renderer& ref_clod_point_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static clod_point_renderer r;
			r.manage_singelton(ctx, "clod_point_renderer", ref_count, ref_count_change);
			return r;
		}

		clod_point_render_style::clod_point_render_style() {}

		bool clod_point_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				//rh.reflect_base(*static_cast<point_render_style*>(this)) &&
				rh.reflect_member("CLOD_factor", CLOD) &&
				rh.reflect_member("spacing", spacing) &&
				rh.reflect_member("scale", scale) &&
				rh.reflect_member("min_millimeters", min_millimeters) &&
				rh.reflect_member("point_size", pointSize) && 
				rh.reflect_member("draw_circles", draw_circles) &&
				rh.reflect_member("point_filter_delay", point_filter_delay);
		}

		void clod_point_renderer::draw_and_compute_impl(context& ctx, size_t start, size_t count)
		{
			int point_filter_delay = get_style<clod_point_render_style>().point_filter_delay;
			
			if (/*point_filter_delay > 0*/ false ) {
				GLuint max_batch_size = (count / point_filter_delay) +1;
				GLint batch_size = std::min(max_batch_size, input_buffer_num_points - remaining_batch_start);
				reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchOffset"), remaining_batch_start);
				reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchSize"), batch_size);
				reduce_prog.set_uniform(ctx, "frustum_extent", 2.0f);
				remaining_batch_start += batch_size;

				// reset draw parameters
				DrawParameters dp = DrawParameters();
				glNamedBufferSubData(draw_parameter_buffer, 0, sizeof(DrawParameters), &dp);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

				// reduce
				reduce_prog.enable(ctx);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, drawp_pos, draw_parameter_buffer);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, render_pos, render_back_buffer);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input_pos, input_buffer);
				glDispatchCompute((batch_size / 128) + 1, 1, 1);
				
				// synchronize
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				reduce_prog.disable(ctx);

				if (remaining_batch_start >= input_buffer_num_points) {
					//TODO Swap draw buffer
					remaining_batch_start = 0;
					glCopyNamedBufferSubData(render_back_buffer, render_buffer, 0, 0, input_buffer_num_points);
				}
			}
			else {
				//configure shader to compute everything after one frame
				reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchOffset"), 0);
				reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchSize"), (int)count);
				reduce_prog.set_uniform(ctx, "frustum_extent", 1.0f);
			
				// reset draw parameters
				DrawParameters dp = DrawParameters();
				glNamedBufferSubData(draw_parameter_buffer, 0, sizeof(DrawParameters), &dp);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
				// reduce
				reduce_prog.enable(ctx);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, drawp_pos, draw_parameter_buffer);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, render_pos, render_buffer);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input_pos, input_buffer);
				glDispatchCompute((input_buffer_num_points / 128) + 1, 1, 1); //with NVIDIA GPUs in debug mode this will spam notifications about buffer usage

				// synchronize
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				reduce_prog.disable(ctx);
			}

			// draw composed buffer
			draw_prog->enable(ctx);
			glBindVertexArray(vertex_array);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_parameter_buffer);
			glDrawArraysIndirect(GL_POINTS, 0);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER,0);
			
			//map buffer into host address space for debugging
			//DrawParameters* device_draw_parameters = static_cast<DrawParameters*>(glMapNamedBufferRange(draw_parameter_buffer, 0, sizeof(DrawParameters), GL_MAP_READ_BIT));
			//glUnmapNamedBuffer(draw_parameter_buffer);
			
			glBindVertexArray(0);
			draw_prog->disable(ctx);
		}

		const render_style* clod_point_renderer::get_style_ptr() const
		{
			if (rs)
				return rs;
			if (default_render_style)
				return default_render_style;
			default_render_style = create_render_style();
			return default_render_style;
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
			}
			//create shader program
			if (!draw_squares_prog.is_created()) {
				draw_squares_prog.build_program(ctx, "point_clod.glpr", true);
			}

			//create shader program
			if (!draw_circle_prog.is_created()) {
				draw_circle_prog.build_program(ctx, "point_clod_circle.glpr", true);
			}
			
			glGenBuffers(1, &input_buffer);
			glGenBuffers(1, &render_buffer);
			glGenBuffers(1, &draw_parameter_buffer);
			glGenBuffers(1, &render_back_buffer);

			glGenVertexArrays(1, &vertex_array);
			glBindVertexArray(vertex_array);
			//position 
			glBindBuffer(GL_ARRAY_BUFFER, render_buffer);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
			glEnableVertexAttribArray(0);
			//color
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Point), (void*)(sizeof(Point::position)));
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			return draw_squares_prog.is_linked() && reduce_prog.is_linked();
		}

		bool clod_point_renderer::enable(context& ctx)
		{
			const clod_point_render_style& prs = get_style<clod_point_render_style>();
			draw_prog = (prs.draw_circles) ? &draw_circle_prog : &draw_squares_prog;

			if (!draw_prog->is_linked()) {
				return false;
			}

			if (buffers_outofdate) {
				resize_buffers(ctx);
				buffers_outofdate = false;
			}

			//const clod_point_render_style& srs = get_style<clod_point_render_style>();
			vec2 screenSize(ctx.get_width(), ctx.get_height());
			//transform to model space since there is no view matrix
			vec4 pivot = inv(ctx.get_modelview_matrix())*dvec4(0.0,0.0,0.0,1.0);

			//mat4 modelview_matrix = ctx.get_modelview_matrix();
			//mat4 projection_matrix = ctx.get_projection_matrix();

			draw_prog->set_uniform(ctx, "CLOD" , prs.CLOD);
			draw_prog->set_uniform(ctx, "scale", prs.scale);
			draw_prog->set_uniform(ctx, "spacing", prs.spacing);
			draw_prog->set_uniform(ctx, "pointSize", prs.pointSize);
			draw_prog->set_uniform(ctx, "minMilimeters", prs.min_millimeters);
			draw_prog->set_uniform(ctx, "screenSize", screenSize);
			draw_prog->set_uniform(ctx, "pivot", pivot);

			//view.glsl uniforms are set on draw_squares_prog.enable(ctx) and  reduce_prog.enable(ctx)
			//draw_squares_prog.set_uniform(ctx, "modelview_matrix", modelview_matrix, true);
			//draw_squares_prog.set_uniform(ctx, "projection_matrix", projection_matrix, true);
			//reduce_prog.set_uniform(ctx, "modelview_matrix", modelview_matrix, true);
			//reduce_prog.set_uniform(ctx, "projection_matrix", projection_matrix, true);

			// compute shader
			reduce_prog.set_uniform(ctx, "CLOD", prs.CLOD);
			reduce_prog.set_uniform(ctx, "scale", prs.scale);
			reduce_prog.set_uniform(ctx, "spacing", prs.spacing);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "pivot"), pivot);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "screenSize"), screenSize);

			float y_view_angle = 45;
			//general point renderer uniforms
			draw_prog->set_uniform(ctx, "use_color_index", false);
			float pixel_extent_per_depth = (float)(2.0 * tan(0.5 * 0.0174532925199 * y_view_angle) / ctx.get_height());
			draw_prog->set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
			return true;
		}

		void clod_point_renderer::clear(const cgv::render::context& ctx)
		{
			reduce_prog.destruct(ctx);
			draw_squares_prog.destruct(ctx);
			draw_circle_prog.destruct(ctx);
			clear_buffers(ctx);
		}

		void clod_point_renderer::draw(context& ctx, size_t start, size_t count)
		{
			draw_and_compute_impl(ctx, start, count);
		}

		bool clod_point_renderer::render(context& ctx, size_t start, size_t count)
		{
			if (enable(ctx)) {
				draw(ctx, start, count);
				return true;
			}
			return false;
		}

		void clod_point_renderer::set_points(cgv::render::context& ctx, const vec3* positions, const rgb8* colors, const uint8_t* lods, const size_t num_points, const unsigned stride)
		{
			std::vector<Point> input_buffer_data(num_points);
			const uint8_t* pos_end = (uint8_t*)positions + (stride * num_points);

			auto input_it = input_buffer_data.begin();

			for (int i = 0; i < num_points; ++i) {
				input_it->position = *positions;
				input_it->color = *colors;
				input_it->level = *lods;
				++input_it;

				if (stride) {
					positions = (vec3*)((uint8_t*)positions + stride);
					colors = (rgb8*)((uint8_t*)colors + stride);
					lods += stride;
				}
				else {
					++positions;
					++colors;
					++lods;
				}

			}
			set_points(ctx, input_buffer_data.data(),input_buffer_data.size());
		}

		void clod_point_renderer::set_render_style(const render_style& rs)
		{
			this->rs = &rs;
		}

		void clod_point_renderer::manage_singelton(context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change)
		{
			switch (ref_count_change) {
			case 1:
				if (ref_count == 0) {
					if (!init(ctx))
						ctx.error(std::string("unable to initialize ") + renderer_name + " singelton");
				}
				++ref_count;
				break;
			case 0:
				break;
			case -1:
				if (ref_count == 0)
					ctx.error(std::string("attempt to decrease reference count of ") + renderer_name + " singelton below 0");
				else {
					if (--ref_count == 0)
						clear(ctx);
				}
				break;
			default:
				ctx.error(std::string("invalid change reference count outside {-1,0,1} for ") + renderer_name + " singelton");
			}
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

		void clod_point_renderer::resize_buffers(context& ctx)
		{ //  fill buffers for the compute shader
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, render_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_size, nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_parameter_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawParameters), nullptr, GL_STREAM_DRAW);
		}

		void clod_point_renderer::clear_buffers(const context& ctx)
		{
			glDeleteBuffers(1, &input_buffer);
			glDeleteBuffers(1, &render_buffer);
			glDeleteBuffers(1, &draw_parameter_buffer);
			glDeleteBuffers(1, &render_back_buffer);
			input_buffer = render_buffer = draw_parameter_buffer = render_back_buffer = 0;
		}
		

}
}


#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct clod_point_render_style_gui_creator : public gui_creator {
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*) {
				if (value_type != cgv::type::info::type_name<cgv::render::clod_point_render_style>::get_name())
					return false;
				cgv::render::clod_point_render_style* rs_ptr = reinterpret_cast<cgv::render::clod_point_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
				p->add_member_control(b, "CLOD factor", rs_ptr->CLOD, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "scale", rs_ptr->scale, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "point spacing", rs_ptr->spacing, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "point size", rs_ptr->pointSize, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "min millimeters", rs_ptr->min_millimeters, "value_slider", "min=0.1;max=10;ticks=true");
				p->add_member_control(b, "draw circles", rs_ptr->draw_circles, "check");
				//p->add_member_control(b, "filter delay", rs_ptr->point_filter_delay, "value_slider", "min=0;max=10;ticks=true");
				return true;
			}
		};

		cgv::gui::gui_creator_registration<clod_point_render_style_gui_creator> cprsgc("clod_point_render_style_gui_creator");
	}
}