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

		clod_point_render_style::clod_point_render_style() : halo_color(1.0, 1.0, 1.0, 1.0) {
			CLOD = 1.f;
			spacing = 1.f;
			scale = 1.f;
			min_millimeters = 1.f;
			pointSize = 1.f;
			draw_circles = false;
			point_filter_delay = 0;
			
			blend_width_in_pixel = 1.0;
			halo_width_in_pixel = 0.0;
			percentual_halo_width = 0.0;

			orient_splats = true;
			blend_points = false;
		}
		
		void clod_point_renderer::reduce_points(context& ctx, size_t start, size_t count)
		{
			{
				//configure shader to compute everything after one frame
				reduce_prog.set_uniform(ctx, uniforms.batch_offset, (int)start);
				reduce_prog.set_uniform(ctx, uniforms.batch_size, (int)count);
				reduce_prog.set_uniform(ctx, uniforms.frustum_extent, 1.0f);
				reduce_prog.set_uniform(ctx, uniforms.target_buffer_size, max_drawn_points);
				reduce_prog.set_uniform_array(ctx, "protection_zone_points", &culling_protection_zones[0].point, 2);
				reduce_prog.enable(ctx);

				GLuint64 startTime, stopTime;
				unsigned int queryID[2];
				glGenQueries(2, queryID);
				glQueryCounter(queryID[0], GL_TIMESTAMP);

				// run computation
				glDispatchCompute((input_buffer_num_points / 128) + 1, 1, 1); //with NVIDIA GPUs in debug mode this will spam notifications about buffer usage

				// synchronize
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				glQueryCounter(queryID[1], GL_TIMESTAMP);

				GLint stopTimerAvailable = 0;
				while (!stopTimerAvailable) {
					glGetQueryObjectiv(queryID[1],
						GL_QUERY_RESULT_AVAILABLE,
						&stopTimerAvailable);
				}

				// get query results
				glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &startTime);
				glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stopTime);

				//std::cout << "Time spent of point reduction on the GPU: " << (stopTime - startTime) / 1000000.0 << "ms\n" << std::endl;

				reduce_prog.disable(ctx);
			}
		}

		void clod_point_renderer::reduce_chunks(context& ctx, const uint32_t* chunk_starts, const uint32_t* chunk_point_counts, const uint32_t* reduction_sources, uint32_t num_reduction_sources)
		{
			reduce_prog.set_uniform(ctx, uniforms.frustum_extent, 1.0f);
			reduce_prog.set_uniform_array(ctx, "protection_zone_points", &culling_protection_zones[0].point, 2);
			reduce_prog.set_uniform(ctx, uniforms.target_buffer_size, max_drawn_points);
			reduce_prog.enable(ctx);

			for (int i = 0; i < num_reduction_sources; ++i) {
				auto chunk_id = reduction_sources[i];
				reduce_prog.set_uniform(ctx, uniforms.batch_offset, (int)chunk_starts[chunk_id]);
				reduce_prog.set_uniform(ctx, uniforms.batch_size, (int)chunk_point_counts[chunk_id]);
				glDispatchCompute((chunk_point_counts[chunk_id] / 128) + 1, 1, 1);
			}

			// synchronize
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			reduce_prog.disable(ctx);
		}

		void clod_point_renderer::draw_points(context& ctx)
		{
			// draw composed buffer
			draw_prog_ptr->enable(ctx);

			GLuint64 startTime, stopTime;
			unsigned int queryID[2];
			glGenQueries(2, queryID);
			glQueryCounter(queryID[0], GL_TIMESTAMP);

			glBindVertexArray(vertex_array);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_parameter_buffer);
			glDrawArraysIndirect(GL_POINTS, 0);
			
			//map buffer into host address space for debugging
			
			//DrawParameters* device_draw_parameters = static_cast<DrawParameters*>(glMapNamedBufferRange(draw_parameter_buffer, 0, sizeof(DrawParameters), GL_MAP_READ_BIT));
			//glUnmapNamedBuffer(draw_parameter_buffer);

			glQueryCounter(queryID[1], GL_TIMESTAMP);

			GLint stopTimerAvailable = 0;
			while (!stopTimerAvailable) {
				glGetQueryObjectiv(queryID[1],
					GL_QUERY_RESULT_AVAILABLE,
					&stopTimerAvailable);
			}

			// get query results
			glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &startTime);
			glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stopTime);

			//std::cout << "Time spent of drawing points on the GPU: " << (stopTime - startTime) / 1000000.0 << "ms\n" << std::endl;

			draw_prog_ptr->disable(ctx);
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

				uniforms.batch_offset = reduce_prog.get_uniform_location(ctx, "batch_offset");
				uniforms.batch_size = reduce_prog.get_uniform_location(ctx, "batch_size");
				uniforms.CLOD = reduce_prog.get_uniform_location(ctx, "CLOD");
				uniforms.frustum_extent = reduce_prog.get_uniform_location(ctx, "frustum_extent");
				uniforms.pivot = reduce_prog.get_uniform_location(ctx, "pivot");
				uniforms.protection_zone_points = reduce_prog.get_uniform_location(ctx, "protection_zone_points");
				uniforms.scale = reduce_prog.get_uniform_location(ctx, "scale");
				uniforms.screenSize = reduce_prog.get_uniform_location(ctx, "screenSize");
				uniforms.spacing = reduce_prog.get_uniform_location(ctx, "spacing");
				uniforms.target_buffer_size = reduce_prog.get_uniform_location(ctx, "target_buffer_size");
			}
			//create shader program
			if (!draw_prog.is_created()) {
				draw_prog.build_program(ctx, "point_clod.glpr", true);
			}

			glGenBuffers(1, &input_buffer);
			glGenBuffers(1, &render_buffer);
			glGenBuffers(1, &index_buffer);
			glGenBuffers(1, &draw_parameter_buffer);
			glGenBuffers(1, &render_back_buffer);

			// vertex arrays
			glGenVertexArrays(1, &vertex_array);
			glBindVertexArray(vertex_array);
			// position 
			glBindBuffer(GL_ARRAY_BUFFER, render_buffer);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
			glEnableVertexAttribArray(0);
			// color
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Point), (void*)(sizeof(vec3)));
			glEnableVertexAttribArray(1);
			// index
			glBindBuffer(GL_ARRAY_BUFFER, index_buffer);
			glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, (void*)0);
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			// fixed size buffers
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_parameter_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawParameters), nullptr, GL_STREAM_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			// map draw parameter buffer to memory
			//draw_parameters_mapping = static_cast<const DrawParameters*>(glMapNamedBufferRange(
			//	draw_parameter_buffer, 0, sizeof(DrawParameters), GL_MAP_READ_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_PERSISTENT_BIT));

			draw_prog_ptr = &draw_prog;
			return draw_prog.is_linked() && reduce_prog.is_linked();
		}

		bool clod_point_renderer::enable(context& ctx)
		{
			const clod_point_render_style& prs = get_style<clod_point_render_style>();

			if (!draw_prog_ptr->is_linked()) {
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

			draw_prog_ptr->set_uniform(ctx, "CLOD" , prs.CLOD);
			draw_prog_ptr->set_uniform(ctx, "scale", prs.scale);
			draw_prog_ptr->set_uniform(ctx, "spacing", prs.spacing);
			draw_prog_ptr->set_uniform(ctx, "pointSize", prs.pointSize);
			draw_prog_ptr->set_uniform(ctx, "minMilimeters", prs.min_millimeters);
			draw_prog_ptr->set_uniform(ctx, "screenSize", screenSize);
			draw_prog_ptr->set_uniform(ctx, "pivot", pivot);
			draw_prog_ptr->set_uniform(ctx, "draw_circles", prs.draw_circles);
			draw_prog_ptr->set_uniform(ctx, "halo_color", prs.halo_color);
			draw_prog_ptr->set_uniform(ctx, "halo_color_strength", prs.halo_color_strength);


			//view.glsl uniforms are set on draw_prog.enable(ctx) and  reduce_prog.enable(ctx)
			//draw_prog.set_uniform(ctx, "modelview_matrix", modelview_matrix, true);
			//draw_prog.set_uniform(ctx, "projection_matrix", projection_matrix, true);
			//reduce_prog.set_uniform(ctx, "modelview_matrix", modelview_matrix, true);
			//reduce_prog.set_uniform(ctx, "projection_matrix", projection_matrix, true);

			// compute shader
			reduce_prog.set_uniform(ctx, uniforms.CLOD, prs.CLOD);
			reduce_prog.set_uniform(ctx, uniforms.scale, prs.scale);
			reduce_prog.set_uniform(ctx, uniforms.spacing, prs.spacing);
			reduce_prog.set_uniform(ctx, uniforms.pivot, pivot);
			reduce_prog.set_uniform(ctx, uniforms.screenSize, screenSize);

			float y_view_angle = 45;
			//general point renderer uniforms
			draw_prog_ptr->set_uniform(ctx, "use_color_index", false);
			float pixel_extent_per_depth = (float)(2.0 * tan(0.5 * 0.0174532925199 * y_view_angle) / ctx.get_height());
			draw_prog_ptr->set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);

			// reduce shader buffers
			
			// reset draw parameters, using SubData version is important here to keep the mapping
			DrawParameters dp = DrawParameters();
			glNamedBufferSubData(draw_parameter_buffer, 0, sizeof(DrawParameters), &dp);
			// bind buffer for reduce shader
			
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, drawp_pos, draw_parameter_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, render_pos, render_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input_pos, input_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index_pos, index_buffer);

			return true;
		}

		bool clod_point_renderer::disable(context& ctx)
		{
			draw_prog_ptr = &draw_prog;
			// draw related stuff
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			glBindVertexArray(0);
			// reduce related stuff
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			return false;
		}

		void clod_point_renderer::clear(const cgv::render::context& ctx)
		{
			reduce_prog.destruct(ctx);
			draw_prog.destruct(ctx);
			clear_buffers(ctx);
		}

		void clod_point_renderer::draw(context& ctx, size_t start, size_t count)
		{
			reduce_points(ctx, start, count);
			draw_points(ctx);
		}

		bool clod_point_renderer::render(context& ctx, size_t start, size_t count)
		{
			if (enable(ctx)) {
				draw(ctx, start, count);
				return true;
			}
			return disable(ctx);
		}

		void clod_point_renderer::set_points(cgv::render::context& ctx, const Point* pnts, const size_t num_points) {
			assert(input_buffer != 0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, num_points * sizeof(Point), pnts, GL_STATIC_READ);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			input_buffer_size = num_points * sizeof(Point);
			input_buffer_num_points = num_points;
			buffers_outofdate = true;
		}

		void clod_point_renderer::set_points(cgv::render::context& ctx, const vec3* positions, const rgb8* colors, const uint8_t* lods, const size_t num_points, const unsigned stride)
		{
			std::vector<Point> input_buffer_data(num_points);
			const uint8_t* pos_end = (uint8_t*)positions + (stride * num_points);

			auto input_it = input_buffer_data.begin();

			for (int i = 0; i < num_points; ++i) {
				input_it->position() = *positions;
				input_it->color() = *colors;
				input_it->level() = *lods;
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

		void clod_point_renderer::set_max_drawn_points(cgv::render::context& ctx, const unsigned max_points)
		{
			if (max_drawn_points != max_points) {
				max_drawn_points = max_points;
				if (render_buffer != 0) {
					resize_buffers(ctx);
				}
			}
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

		void clod_point_renderer::set_prog(shader_program& one_shot_prog)
		{
			draw_prog_ptr = &one_shot_prog;
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
			glBufferData(GL_SHADER_STORAGE_BUFFER, max_drawn_points*sizeof(Point), nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, max_drawn_points *sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		void clod_point_renderer::clear_buffers(const context& ctx)
		{
			glDeleteBuffers(1, &input_buffer);
			glDeleteBuffers(1, &index_buffer);
			glDeleteBuffers(1, &render_buffer);
			glDeleteBuffers(1, &draw_parameter_buffer);
			glDeleteBuffers(1, &render_back_buffer);
			
			input_buffer = render_buffer = draw_parameter_buffer = render_back_buffer = index_buffer = 0;
		}
		
		bool clod_point_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				//rh.reflect_base(*static_cast<cgv::render::clod_point_render_style*>(this)) &&
				rh.reflect_member("CLOD_factor", CLOD) &&
				rh.reflect_member("spacing", spacing) &&
				rh.reflect_member("scale", scale) &&
				rh.reflect_member("min_millimeters", min_millimeters) &&
				rh.reflect_member("point_size", pointSize) &&
				rh.reflect_member("draw_circles", draw_circles) &&
				// splat reflects
				rh.reflect_member("blend_points", blend_points) &&
				rh.reflect_member("orient_splats", orient_splats) &&
				rh.reflect_member("blend_width_in_pixel", blend_width_in_pixel) &&
				rh.reflect_member("halo_width_in_pixel", halo_width_in_pixel) &&
				//rh.reflect_member("halo_color", halo_color) &&
				rh.reflect_member("halo_color_strength", halo_color_strength) &&
				rh.reflect_member("percentual_halo_width", percentual_halo_width);
		}

		cgv::reflect::extern_reflection_traits<clod_point_render_style, clod_point_render_style_reflect> get_reflection_traits(const clod_point_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<clod_point_render_style, clod_point_render_style_reflect>();
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