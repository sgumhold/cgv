#include <algorithm>
#include <random>
#include <unordered_map>
#include <sstream>
#include "clod_point_renderer.h"

//**

namespace cgv {
	namespace render {

		clod_point_render_style::clod_point_render_style() {}

		bool clod_point_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				//rh.reflect_base(*static_cast<point_render_style*>(this)) &&
				rh.reflect_member("CLOD_factor", CLOD) &&
				rh.reflect_member("spacing", spacing) &&
				rh.reflect_member("scale", scale) &&
				rh.reflect_member("min_millimeters", min_millimeters) &&
				rh.reflect_member("point_size", pointSize);
		}

		void clod_point_renderer::draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count)
		{
			//renderer::draw_impl(ctx, type, start, count, use_strips, use_adjacency, strip_restart_index);
			
			//TODO add option to spread calculation over multiple frames
			//configure shader to compute everything after one frame
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchOffset"), 0);
			reduce_prog.set_uniform(ctx, reduce_prog.get_uniform_location(ctx, "uBatchSize"), (int)count);
			reduce_prog.set_uniform(ctx, "frustum_extent", 1.0f);
			//reduce_prog.set_uniform(ctx, "frustum_extent", 2.0f);

			// reset draw parameters
			DrawParameters dp = DrawParameters();
			glNamedBufferSubData(draw_parameter_buffer, 0, sizeof(DrawParameters), &dp);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			// reduce
			reduce_prog.enable(ctx);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, drawp_pos, draw_parameter_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, render_pos, render_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input_pos, input_buffer);
			glDispatchCompute((input_buffer_data.size()/128)+1, 1, 1); //with NVIDIA GPUs this will spam notifications about buffer usage in debug mode

			// synchronize
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			reduce_prog.disable(ctx);

			// draw composed buffer
			draw_prog.enable(ctx);
			glBindVertexArray(vertex_array);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_parameter_buffer);
			glDrawArraysIndirect(GL_POINTS, 0);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER,0);
			
			//for debugging, map buffer into host address space
			//DrawParameters* device_draw_parameters = static_cast<DrawParameters*>(glMapNamedBufferRange(draw_parameter_buffer, 0, sizeof(DrawParameters), GL_MAP_READ_BIT));
			//glUnmapNamedBuffer(draw_parameter_buffer);
			
			glBindVertexArray(0);
			draw_prog.disable(ctx);
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
			
			glGenBuffers(1, &input_buffer); //array of {float x;float y;float z;uint color;};
			glGenBuffers(1, &render_buffer);
			glGenBuffers(1, &draw_parameter_buffer);

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
			vec2 screenSize(ctx.get_width(), ctx.get_height());
			//transform to model space since there is no view matrix
			vec4 pivot = inv(ctx.get_modelview_matrix())*dvec4(0.0,0.0,0.0,1.0);

			//mat4 modelview_matrix = ctx.get_modelview_matrix();
			//mat4 projection_matrix = ctx.get_projection_matrix();
			
			const clod_point_render_style& prs = get_style<clod_point_render_style>();

			draw_prog.set_uniform(ctx, "CLOD" , prs.CLOD);
			draw_prog.set_uniform(ctx, "scale", prs.scale);
			draw_prog.set_uniform(ctx, "spacing", prs.spacing);
			draw_prog.set_uniform(ctx, "pointSize", prs.pointSize);
			draw_prog.set_uniform(ctx, "minMilimeters", prs.min_millimeters);
			draw_prog.set_uniform(ctx, "screenSize", screenSize);
			draw_prog.set_uniform(ctx, "pivot", pivot);
			

			//view.glsl uniforms, set on draw_prog.enable(ctx) and  reduce_prog.enable(ctx)
			//draw_prog.set_uniform(ctx, "modelview_matrix", modelview_matrix, true);
			//draw_prog.set_uniform(ctx, "projection_matrix", projection_matrix, true);
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
			draw_prog.set_uniform(ctx, "use_color_index", false);
			//draw_prog.set_uniform(ctx, "use_group_point_size", prs.use_group_point_size);
			float pixel_extent_per_depth = (float)(2.0 * tan(0.5 * 0.0174532925199 * y_view_angle) / ctx.get_height());
			draw_prog.set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
			//draw_prog.set_uniform(ctx, "blend_width_in_pixel", prs.blend_width_in_pixel);
			//draw_prog.set_uniform(ctx, "percentual_halo_width", 0.01f * prs.percentual_halo_width);
			//draw_prog.set_uniform(ctx, "halo_width_in_pixel", prs.halo_width_in_pixel);
			//draw_prog.set_uniform(ctx, "halo_color", prs.halo_color);
			//draw_prog.set_uniform(ctx, "halo_color_strength", prs.halo_color_strength);
			return true;
		}

		bool clod_point_renderer::disable(context& ctx)
		{
			
			const clod_point_render_style& srs = get_style<clod_point_render_style>();			
			return true;
		}

		void clod_point_renderer::clear(const cgv::render::context& ctx)
		{
			reduce_prog.destruct(ctx);
			draw_prog.destruct(ctx);
			clear_buffers(ctx);
		}

		void clod_point_renderer::draw(context& ctx, size_t start, size_t count)
		{
			draw_and_compute_impl(ctx, cgv::render::PT_POINTS, start, count);
		}

		bool clod_point_renderer::render(context& ctx, size_t start, size_t count)
		{
			if (enable(ctx)) {
				draw(ctx, start, count);
				return true;
			}
			return false;
		}

		void clod_point_renderer::set_render_style(const render_style& rs)
		{
			this->rs = &rs;
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
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Point), input_buffer_data.data(), GL_STATIC_READ);
			
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, render_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, input_buffer_data.size() * sizeof(Point), nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_parameter_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawParameters), nullptr, GL_STREAM_DRAW);
		}

		void clod_point_renderer::clear_buffers(const context& ctx)
		{
			glDeleteBuffers(1, &input_buffer);
			glDeleteBuffers(1, &render_buffer);
			glDeleteBuffers(1, &draw_parameter_buffer);
			input_buffer = render_buffer = draw_parameter_buffer = 0;
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
				return true;
			}
		};

		cgv::gui::gui_creator_registration<clod_point_render_style_gui_creator> cprsgc("clod_point_render_style_gui_creator");
	}
}