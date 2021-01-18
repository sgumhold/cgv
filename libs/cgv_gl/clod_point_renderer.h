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


		class CGV_API clod_point_renderer : public cgv::render::renderer{

			struct Vertex {
				vec3 position;
				rgba8 colors;
			};

			struct drawParameters {
				uint32_t  count = 0;
				uint32_t  primCount = 0;
				uint32_t  first = 0;
				uint32_t  baseInstance = 0;
			};

			float CLOD = 1.f;
			float spacing = 1.f; //root spacing
			float scale = 1.f;
			
			shader_program reduce_prog;
			
			std::vector<vec3> positions;
			std::vector<Vertex> input_buffer_data;
			std::vector<rgba8> colors; //alpha channel is later used for storing the clod level
			GLuint input_buffer, render_buffer, draw_parameter_buffer;
			const int input_pos = 0, render_pos = 1, drawp_pos = 3;
		protected:

			void draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index);
		public:
			render_style* create_render_style() const;

			bool init(context& ctx);

			bool enable(context& ctx);
			
			bool disable(context& ctx);

			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
			
			void set_position_array(context& ctx, std::vector<vec3> positions) {
				this->positions = positions;
				input_buffer_data.resize(positions.size());
				for (int i = 0; i < positions.size(); ++i) {
					input_buffer_data[i].position = positions[i];
				}
			}
			template<typename T>
			void sset_color_array(const context& ctx, const std::vector<T>& colors) {
				input_buffer_data.resize(colors.size());
				for (int i = 0; i < positions.size(); ++i) {
					input_buffer_data[i].colors = rgba(colors[i]);
				}
			}

		private:
			void add_shader(context& ctx, shader_program& prog, const std::string& sf, const cgv::render::ShaderType st);

		};
	}
}
#include <cgv/config/lib_end.h>