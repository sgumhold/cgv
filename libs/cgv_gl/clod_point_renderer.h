#pragma once
#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/reflect/reflect_extern.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_reflect_types/render/context.h>
#include <atomic>
#include <mutex>
#include "renderer.h"

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
			// clod factor, affects the distance dependent target spacing
			float CLOD;
			/** := root spacing, match this to your inputs point spacing in the octrees root level, 
				too low values may cause most of the pointcloud to go invisible */
			float spacing;
			// scales the target spacing
			float scale;
			// minimal size of the visible points
			float min_millimeters;
			float pointSize;
			// draw circles instead of squares
			bool draw_circles = false;
			// allow point subset computation to run across multiple rendered frames (broken)
			int point_filter_delay = 0;
			//@}

			/* @name splat rendering attributes*/
			//@{
			/// set to 1 in constructor 
			float blend_width_in_pixel;
			/// set to 0 in constructor
			float halo_width_in_pixel;
			/// set to 0 in constructor
			float percentual_halo_width;
			/// color of halo with opacity channel
			rgba halo_color;
			/// strength in [0,1] of halo color with respect to color of primitive
			float halo_color_strength = 0.0;
			/// set to true in constructor
			bool orient_splats;
			/// set to false in constructor
			bool blend_points;
			//@}
			/// construct with default values
			clod_point_render_style();
		};


		//*  */
		class CGV_API clod_point_renderer : public render_types {
		public:
			// internal point format
			struct Point {
				vec3 p_position;
				rgb8 p_color;
				uint8_t p_level = 0;

				inline vec3& position() {
					return p_position;
				}
				/// returns the level of detail
				inline uint8_t& level() {
					return p_level;
				}
				inline rgb8& color() {
					return p_color;
				}
				inline const vec3& position() const {
					return p_position;
				}
				/// return the level of detail
				inline const uint8_t& level() const {
					return p_level;
				}
				inline const rgb8& color() const {
					return p_color;
				}
			};

		private:
			// stores parameters generated for the draw shaders, for an explaination search OpenGL Indirect rendering (https://www.khronos.org/opengl/wiki/Vertex_Rendering#Indirect_rendering)
			struct DrawParameters {
				GLuint  count = 0; //element count
				GLuint  primCount = 1;
				GLuint  first = 0;
				GLuint  baseInstance = 0;
			};
			
			struct NoCullZone {
				vec3 point[2] = { vec3(0.0),vec3(0.0) };
				float squareRadius[2] = { 0.0,0.0 };
			};
			shader_program reduce_prog;		// filters points from the input buffer and writes them to the render_buffer (compute shader)
			shader_program* draw_prog_ptr;
			shader_program draw_prog;	// draws render_buffer (vertex, geometry, fragment shader)
			
			GLuint vertex_array = 0;
			GLuint input_buffer = 0, render_buffer = 0, draw_parameter_buffer = 0, render_back_buffer = 0;
			/// the refered buffer contains indices of points after reduction step
			GLuint index_buffer = 0;
			/// buffer layout positions for the reduce shader program
			const int input_pos = 0, render_pos = 1, index_pos = 2,drawp_pos = 3;

			GLsizeiptr input_buffer_size = 0;
			GLuint input_buffer_num_points = 0;
			GLint remaining_batch_start = 0;

			bool buffers_outofdate = true;	//implies protection zone out of date

			/// default render style
			mutable render_style* default_render_style = nullptr;
			/// current render style, can be set by user
			const render_style* rs = nullptr;

			NoCullZone culling_protection_zone;

		protected:			
			const render_style* get_style_ptr() const;

			template <typename T>
			const T& get_style() const { return *static_cast<const T*>(get_style_ptr()); }

		public:
			clod_point_renderer() = default;

			render_style* create_render_style() const;

			bool init(context& ctx);

			/// sets most uniforms and resizes/updates other buffers if needed
			bool enable(context& ctx);

			bool disable(context& ctx);

			void clear(const cgv::render::context& ctx);

			/// reduces and renders the input by calling reduce_points and draw_points
			void draw(context& ctx, size_t start=0, size_t count=0);
			
			bool render(context& ctx, size_t start, size_t count);

			/// this method can be used if the data format of pnts matches with the internal format given by the Point struct
			inline void set_points(cgv::render::context& ctx,const Point* pnts, const size_t num_points) {
				assert(input_buffer != 0);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, num_points * sizeof(Point), pnts, GL_STATIC_READ);
				input_buffer_size = num_points * sizeof(Point);
				input_buffer_num_points = num_points;
				buffers_outofdate = true;
			}

			/// to use the clod point renderer lods are required, to generate them use the classes inside libs/pointcloud/octree.h
			/// @param positions : pointer to first points position
			/// @param color : pointer to first points color
			/// @param lods : pointer to firsts points level of detail
			/// @param num_points : number of points to draw
			/// @param stride : stride in bytes, zero if positions, color and lods are not stored interleaved
			void set_points(cgv::render::context& ctx, const vec3* positions, const rgb8* colors, const uint8_t* lods, const size_t num_points, const unsigned stride = 0);

			void set_render_style(const render_style& rs);

			void manage_singelton(context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change);

			/// set a custom shader program that is used for one draw call
			void set_prog(shader_program& one_shot_prog);

			/* methods for step wise operation */

			/// run point reduction step on the input data, yout need to call enable first
			void reduce_points(context& ctx, size_t start, size_t count, size_t max_reduced_points);
			/// render reduced points, you need to call reduce_points first to fill the render_buffer
			void draw_points(context& ctx);

			/* accessors for opengl buffers */

			/// get the opengl id used to access the index buffer
			inline GLuint get_index_buffer() {
				assert(index_buffer != 0);
				return index_buffer;
			}
			/// get the opengl id used to access the draw parameters
			inline GLuint get_draw_parameters() {
				assert(draw_parameter_buffer != 0);
				return draw_parameter_buffer;
			}
			/// get the opengl id used to access the draw buffer
			inline GLuint get_reduced_points() {
				assert(render_buffer != 0);
				return render_buffer;
			}

			inline void set_protection_zone(const vec3 position, const float radius, const unsigned index) {
				assert(index < 2);
				culling_protection_zone.point[index] = position;
				culling_protection_zone.squareRadius[index] = radius * radius;
			}

		private:
			void add_shader(context& ctx, shader_program& prog, const std::string& sf, const cgv::render::ShaderType st);
			void resize_buffers(context& ctx);
			void clear_buffers(const context& ctx);
		};


		struct CGV_API clod_point_render_style_reflect : public clod_point_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<clod_point_render_style, clod_point_render_style_reflect> get_reflection_traits(const clod_point_render_style&);

	}
}
#include <cgv/config/lib_end.h>