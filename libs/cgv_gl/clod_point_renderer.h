#pragma once
#include <cgv/render/context.h>
#include <cgv_gl/point_renderer.h>

#include "gl/lib_begin.h"

// [WIP] clod point cloud renderer

namespace cgv {
	namespace render {

		/** render style for sphere rendere */
		struct CGV_API clod_point_render_style : public render_style
		{
			/*@name clod rendering attributes*/
			//@{
			
			//@}

			/// construct with default values
			clod_point_render_style();
		};


		class CGV_API clod_point_renderer : public cgv::render::render_types {

			struct Vertex {
				vec3 position;
				rgba8 colors;
			};

			struct DrawParameters {
				uint32_t  count = 0; //element count
			};

			float CLOD = 1.f;
			float spacing = 1.f; //root spacing
			float scale = 1.f;
			
			shader_program reduce_prog;
			shader_program draw_prog;

			std::vector<vec3> positions;
			std::vector<Vertex> input_buffer_data;
			std::vector<rgba8> colors; //alpha channel is later used for storing the clod level
			GLuint vertex_array = 0;
			GLuint input_buffer = 0, render_buffer = 0, draw_parameter_buffer = 0;
			const int input_pos = 0, render_pos = 1, drawp_pos = 3;
			bool buffers_outofdate = true;


			//test
			point_render_style prs;
		protected:

			void draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index);
		public:
			render_style* create_render_style() const;

			bool init(context& ctx);

			bool enable(context& ctx);
			
			bool disable(context& ctx);

			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
			
			void set_positions(context& ctx, std::vector<vec3> positions) {
				this->positions = positions;
				input_buffer_data.resize(positions.size());
				for (int i = 0; i < positions.size(); ++i) {
					input_buffer_data[i].position = positions[i];
				}
				buffers_outofdate = true;
			}
			template<typename T>
			void set_colors(const context& ctx, const std::vector<T>& colors) {
				input_buffer_data.resize(colors.size());
				for (int i = 0; i < positions.size(); ++i) {
					input_buffer_data[i].colors = rgba(colors[i]);
					input_buffer_data[i].colors.alpha() = 1;
				}
				buffers_outofdate = true;
			}

		private:
			void add_shader(context& ctx, shader_program& prog, const std::string& sf, const cgv::render::ShaderType st);
			void fill_buffers(context& ctx);
			void clear_buffers(context& ctx);
		};
	}
}
#include <cgv/config/lib_end.h>