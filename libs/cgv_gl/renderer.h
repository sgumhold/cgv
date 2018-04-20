#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		/// base class for all render styles
		struct CGV_API render_style
		{
			virtual ~render_style();
		};
		/// attribute array manager used to upload arrays to gpu
		class CGV_API attribute_array_manager
		{
		protected:
			/// attribue array binding used to store array pointers
			attribute_array_binding aab;
			/// store vertex buffers generated per attribute location
			std::map<int, vertex_buffer*> vbos;
			/// give renderer access to protected members
			friend class renderer;
			///
			template <typename T>
			bool set_attribute_array(context& ctx, int loc, const T& array) {
				bool res;
				vertex_buffer*& vbo_ptr = vbos[loc];
				if (vbo_ptr) {
					if (vbo_ptr->get_size_in_bytes() == array_descriptor_traits <T>::get_size(array))
						res = vbo_ptr->replace(ctx, 0, array_descriptor_traits <T>::get_address(array), array_descriptor_traits < T>::get_nr_elements(array));
					else {
						vbo_ptr->destruct(ctx);
						res = vbo_ptr->create(ctx, array);
					}
				}
				else {
					vbo_ptr = new vertex_buffer();
					res = vbo_ptr->create(ctx, array);
				}
				if (res)
					res = ctx.set_attribute_array_void(&aab, loc, array_descriptor_traits <T>::get_type_descriptor(array), vbo_ptr, 0, array_descriptor_traits < T>::get_nr_elements(array));
				return res;
			}
			///
			template <typename T>
			bool set_attribute_array(context& ctx, int loc, const T* array_ptr, unsigned nr_elements, unsigned stride) {
				bool res;
				vertex_buffer*& vbo_ptr = vbos[loc];
				if (vbo_ptr) {
					if (vbo_ptr->get_size_in_bytes() == nr_elements * sizeof(T))
						res = vbo_ptr->replace(ctx, 0, array_ptr, nr_elements);
					else {
						vbo_ptr->destruct(ctx);
						res = vbo_ptr->create(ctx, array_ptr, nr_elements);
					}
				}
				else {
					vbo_ptr = new vertex_buffer();
					res = vbo_ptr->create(ctx, array_ptr, nr_elements);
				}
				if (res)
					res = ctx.set_attribute_array_void(&aab, loc, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(*array_ptr), true), vbo_ptr, 0, nr_elements);
				return res;
			}
			///
			bool set_attribute_array(context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes);
		public:
			/// default initialization
			attribute_array_manager();
			/// destructor calls destruct
			~attribute_array_manager();
			///
			bool init(context& ctx);
			///
			bool enable(context& ctx);
			///
			bool disable(context& ctx);
			///
			void destruct(context& ctx);
		};
		/// abstract base class for all renderers that handles a shader program and position / color attribute
		class CGV_API renderer
		{
		private:
			/// shader program
			shader_program prog;
			/// if attribue array manager is set, use it for attribute management
			attribute_array_manager* aam_ptr;
			/// otherwise keep track of enabled arrays
			std::set<int> enabled_attribute_arrays;
			/// default render style
			render_style* default_render_style;
			/// current render style, can be set by user
			const render_style* rs;
		protected:
			/// return whether attributes persist after a call to disable
			bool attributes_persist() const { return aam_ptr != 0; }
			/// derived renderer classes have access to shader program
			shader_program& ref_prog() { return prog; }
			/// access to style
			template <typename T>
			const T& get_style() const { return *static_cast<const T*>(rs);  }
			/// track whether color attribute is defined
			mutable bool has_colors;
			/// track whether position attribute is defined
			mutable bool has_positions;
			/// virtual method that creates a default render style
			virtual render_style* create_render_style() const = 0;

			template <typename T>
			bool set_attribute_array(context& ctx, int loc, const T& array) {
				if (aam_ptr)
					return aam_ptr->set_attribute_array(ctx, loc, array);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, array);
			}
			template <typename T>
			bool set_attribute_array(context& ctx, int loc, const T* array_ptr, unsigned nr_elements, unsigned stride) {
				if (aam_ptr)
					return aam_ptr->set_attribute_array(ctx, loc, array_ptr, nr_elements, stride);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, array_ptr, nr_elements, stride);
			}
			bool set_attribute_array(context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes);
		public:
			/// construct and init attribute tracking flags
			renderer();
			/// destructor deletes default renderer style
			virtual ~renderer();
			/// provide an attribute manager that is used in successive calls to attribute array setting methods and in the enable and disable method, if a nullptr is provided attributes are managed through deprecated VertexAttributePointers - in this case a call to disable deattaches all attribute arrays which have to be set before the next enable call again
			void set_attribute_array_manager(attribute_array_manager* aam_ptr = 0);
			/// reference given render style
			void set_render_style(const render_style& rs);
			/// abstract initialize method creates default render style, derived renderers to load the shader program
			virtual bool init(context& ctx);
			/// templated method to set the position attribute from a vector of positions of type T
			template <typename T>
			void set_position_array(context& ctx, const std::vector<T>& positions) { has_positions = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "position"), positions); }
			/// templated method to set the position attribute from a vector of positions of type T
			template <typename T>
			void set_position_array(context& ctx, const T* positions, size_t nr_elements, size_t stride_in_bytes = 0) { has_positions = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "position"), positions, nr_elements, stride_in_bytes); }
			/// template method to set the position attribute from a vertex buffer object, the element type must be given as explicit template parameter
			void set_position_array(context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes);
			/// template method to set the position attribute from a vertex buffer object, the element type must be given as explicit template parameter
			template <typename ElementType>
			void set_position_array(context& ctx, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes = 0) { has_positions = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "position"), vbo, offset_in_bytes, nr_elements, stride_in_bytes); }
			/// template method to set the color attribute from a vector of colors of type T
			template <typename T>
			void set_color_array(context& ctx, const std::vector<T>& colors) { has_colors = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "color"), colors); }
			/// template method to set the color attribute from a vector of colors of type T
			template <typename T>
			void set_color_array(context& ctx, const T* colors, size_t nr_elements, size_t stride_in_bytes = 0) { has_colors = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "color"), colors, nr_elements, stride_in_bytes); }
			/// template method to set the color attribute from a vertex buffer object, the element type must be given as explicit template parameter
			void set_color_array(context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes = 0);
			/// call to validate, whether essential position attribute is defined
			virtual bool validate_attributes(context& ctx) const;
			/// validate attributes and if successful, enable renderer
			bool validate_and_enable(context& ctx);
			/// enable renderer
			virtual bool enable(context& ctx);
			/// disable renderer
			virtual bool disable(context& ctx);
			/// the clear function destructs the shader program
			virtual void clear(context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>