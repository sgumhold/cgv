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

		class clod_point_renderer;

		extern CGV_API clod_point_renderer& ref_clod_point_renderer(context& ctx, int ref_count_change = 0);

		/** render style for sphere rendere */
		struct CGV_API clod_point_render_style : public render_style
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
			// draw circles instead of squares
			bool draw_circles = false;
			// allow point subset computation to run across multiple rendered frames
			int point_filter_delay = 0;
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
				rgb8 color;
				uint8_t level = 0;
			};
			
		private:
			// stores parameters generated for the draw shaders, for an explaination search OpenGL Indirect rendering (https://www.khronos.org/opengl/wiki/Vertex_Rendering#Indirect_rendering)
			struct DrawParameters {
				GLuint  count = 0; //element count
				GLuint  primCount = 1;
				GLuint  first = 0;
				GLuint  baseInstance = 0;
			};

			shader_program reduce_prog;		// filters points from the input buffer and writes them to the render_buffer (compute shader)
			shader_program* draw_prog;
			shader_program draw_squares_prog;	// draws render_buffer (vertex, geometry, fragment shader)
			shader_program draw_circle_prog;
			
			GLuint vertex_array = 0;
			GLuint input_buffer = 0, render_buffer = 0, draw_parameter_buffer = 0, render_back_buffer = 0;
			const int input_pos = 0, render_pos = 1, drawp_pos = 3;

			GLsizeiptr input_buffer_size = 0;
			GLuint input_buffer_num_points = 0;
			GLint remaining_batch_start = 0;

			bool buffers_outofdate = true;


			/// default render style
			mutable render_style* default_render_style = nullptr;
			/// current render style, can be set by user
			const render_style* rs = nullptr;

		protected:

			void draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count);

			const render_style* get_style_ptr() const;

			template <typename T>
			const T& get_style() const { return *static_cast<const T*>(get_style_ptr()); }

		public:
			clod_point_renderer() = default;

			render_style* create_render_style() const;

			bool init(context& ctx);

			bool enable(context& ctx);
			
			bool disable(context& ctx);

			void clear(const cgv::render::context& ctx);

			void draw(context& ctx, size_t start=0, size_t count=0);
			
			bool render(context& ctx, size_t start, size_t count);

			/// this method can be used if the data format of pnts matches with the internal format given by the Point struct
			inline void set_points(cgv::render::context& ctx,const void* pnts, const size_t num_points) {
				assert(input_buffer != 0);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, num_points * sizeof(Point), pnts, GL_STATIC_READ);
				input_buffer_size = num_points * sizeof(Point);
				input_buffer_num_points = num_points;
				buffers_outofdate = true;
			}

			/// for using the clod point renderer lods are required, to generate them use the classes inside libs/pointcloud/octree.h
			/// @param positions : pointer to first points position
			/// @param color : pointer to first points color
			/// @param lods : pointer to firsts points level of detail
			/// @param num_points : number of points to draw
			/// @param stride : stride in bytes, zero if positions, color and lods are not stored interleaved
			void set_points(cgv::render::context& ctx, const vec3* positions, const rgb8* colors, const uint8_t* lods, const size_t num_points, const unsigned stride = 0);

			void set_render_style(const render_style& rs);

			void manage_singelton(context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change);

		private:
			void add_shader(context& ctx, shader_program& prog, const std::string& sf, const cgv::render::ShaderType st);
			void resize_buffers(context& ctx);
			void clear_buffers(const context& ctx);
		};

	}
}
#include <cgv/config/lib_end.h>