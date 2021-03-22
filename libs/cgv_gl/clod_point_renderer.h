#pragma once
#include <cgv/render/context.h>
#include <cgv_gl/point_renderer.h>
#include <atomic>
#include <mutex>

#include "gl/lib_begin.h"

// source code of the continous level of detail point cloud renderer adapted for the cgv framework

// original paper: "Real-Time Continuous Level of Detail Rendering of Point Clouds" from Schutz, Markus and Krosl, Katharina and Wimmer, Michael
// the reference implementation can be found at https://github.com/m-schuetz/ieeevr_2019_clod

namespace cgv {
	namespace render {

		/** render style for sphere rendere */
		struct CGV_API clod_point_render_style : public group_render_style
		{
			/*@name clod rendering attributes*/
			//@{
			float CLOD = 1.f;
			// root spacing in the paper
			float spacing = 1.f;
			float scale = 1.f;
			// minimal size of the visible points
			float min_millimeters = 1.f;
			float pointSize = 1.f;
			//@}

			/// construct with default values
			clod_point_render_style();
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};


		//*  */
		class CGV_API clod_point_renderer : public render_types {
		public:
			// internal point format
			struct Point {
				vec3 position;
				rgb8 colors;
				uint8_t level = 0;
			};
			
		private:
			// stores parameters generated for the draw shaders, for an explaination search for OpenGL Indirect rendering (https://www.khronos.org/opengl/wiki/Vertex_Rendering#Indirect_rendering)
			struct DrawParameters {
				GLuint  count = 0; //element count
				GLuint  primCount = 1;
				GLuint  first = 0;
				GLuint  baseInstance = 0;
			};

			shader_program reduce_prog;		// filters points from the input buffer and writes them to the render_buffer (compute shader)
			shader_program draw_prog;		// draws render_buffer (vertex, geometry, fragment shader)

			std::vector<Point> input_buffer_data;
			//vertex_buffer vert_input_buffer;


			GLuint vertex_array = 0;
			GLuint input_buffer = 0, render_buffer = 0, draw_parameter_buffer = 0;
			const int input_pos = 0, render_pos = 1, drawp_pos = 3;

			bool buffers_outofdate = true;
			// controls execution of the point filter, with values greater than 1 point filtering is scattered across multiple frames, and extended frustum culling is used
			int point_filter_delay = 0;

			/// default render style
			mutable render_style* default_render_style = nullptr;
			/// current render style, can be set by user
			const render_style* rs = nullptr;

		protected:

			void draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count);

			const render_style* get_style_ptr() const;

			template <typename T>
			const T& get_style() const { return *static_cast<const T*>(get_style_ptr()); }

			void set_positions(context& ctx, const std::vector<vec3>& positions);

			// set point colors
			template<typename T>
			void set_colors(const context& ctx, const std::vector<T>& colors) {
				size_t size = std::min(colors.size(), input_buffer_data.size());
				for (int i = 0; i < size; ++i) {
					input_buffer_data[i].colors = rgb8(colors[i]);
				}
				buffers_outofdate = true;
			}

			// add lod information for each point
			template<typename T>
			void set_lods(const std::vector<T>& lod) {
				//input_buffer_data.resize(lod.size());
				for (int i = 0; i < lod.size(); ++i) {
					input_buffer_data[i].level = lod[i]; //set LOD level (lower levels should be more coarse than higher levels)
				}
				buffers_outofdate = true;
			}

		public:
			clod_point_renderer() = default;

			render_style* create_render_style() const;

			bool init(context& ctx);

			bool enable(context& ctx);
			
			bool disable(context& ctx);

			void clear(const cgv::render::context& ctx);

			/// @param use_strips : unused
			/// @param use_adjacency : unused
			/// @param strip_restart_index : unused
			void draw(context& ctx, size_t start=0, size_t count=0);
			/// @param use_strips : unused
			/// @param use_adjacency : unused
			/// @param strip_restart_index : unused
			bool render(context& ctx, size_t start, size_t count);

			template<typename T>
			void set_points(const std::vector<T>& pnts) {
				input_buffer_data.resize(pnts.size());
				memcpy(input_buffer_data.data(),pnts.data(), sizeof(Point) * pnts.size());
				buffers_outofdate = true;
			}

			void set_points(const vec3* positions, const rgb8* colors, const uint8_t* lods, const size_t num_points, const unsigned stride) {
				std::vector<Point> input_buffer_data(num_points);
				const uint8_t* pos_end = (uint8_t*)positions + (stride*num_points);
				
				auto input_it = input_buffer_data.begin();
				for (int i = 0; i < num_points;++i) {
					input_it->position = *positions;
					input_it->colors = *colors;
					input_it->level = *lods;
					++input_it;

					positions = (vec3*)((uint8_t*)positions + stride);
					colors = (rgb8*)((uint8_t*)colors + stride);
					lods += stride;
				}
			}

			void set_render_style(const render_style& rs);

		private:
			void add_shader(context& ctx, shader_program& prog, const std::string& sf, const cgv::render::ShaderType st);
			void fill_buffers(context& ctx);
			void clear_buffers(const context& ctx);
		};

	}
}
#include <cgv/config/lib_end.h>