#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl_context.h>

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		
		/// base class for all render styles
		struct CGV_API render_style : public render_types
		{
			virtual ~render_style();
		};
		/// forward declaration of renderer
		class CGV_API renderer;
		/// attribute array manager used to upload arrays to gpu
		class CGV_API attribute_array_manager
		{
		protected:
			/// store default buffer usage
			VertexBufferUsage default_usage; 
			/// attribue array binding used to store array pointers
			attribute_array_binding aab;
			/// store vertex buffers generated per attribute location
			std::map<int, vertex_buffer*> vbos;
			/// give renderer access to protected members
			friend class renderer;
			/// 
			template <typename T>
			bool set_indices(const context& ctx, const T& array)
			{
				bool res;
				vertex_buffer*& vbo_ptr = vbos[-1];
				if (vbo_ptr) {
					if (vbo_ptr->get_size_in_bytes() == array_descriptor_traits <T>::get_size(array))
						res = vbo_ptr->replace(ctx, 0, array_descriptor_traits <T>::get_address(array), array_descriptor_traits < T>::get_nr_elements(array));
					else {
						vbo_ptr->destruct(ctx);
						res = vbo_ptr->create(ctx, array);
					}
				}
				else {
					vbo_ptr = new vertex_buffer(VBT_INDICES, default_usage);
					res = vbo_ptr->create(ctx, array);
				}
				if (res)
					res = ctx.set_element_array(&aab, vbo_ptr);
				return res;
			}
			/// 
			template <typename T>
			bool set_indices(const context& ctx, const T* array, size_t count)
			{
				bool res;
				vertex_buffer*& vbo_ptr = vbos[-1];
				if (vbo_ptr) {
					if (vbo_ptr->get_size_in_bytes() == count * get_type_size(cgv::type::info::type_id<T>::get_id()))
						res = vbo_ptr->replace(ctx, 0, array, count);
					else {
						vbo_ptr->destruct(ctx);
						res = vbo_ptr->create(ctx, array, count);
					}
				}
				else {
					vbo_ptr = new vertex_buffer(VBT_INDICES, default_usage);
					res = vbo_ptr->create(ctx, array, count);
				}
				if (res)
					res = ctx.set_element_array(&aab, vbo_ptr);
				return res;
			}
			/// whether aam contains an index buffer
			bool has_index_buffer() const;
			///
			void remove_indices(const context& ctx);
			///
			template <typename T>
			bool set_attribute_array(const context& ctx, int loc, const T& array) {
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
					vbo_ptr = new vertex_buffer(VBT_VERTICES, default_usage);
					res = vbo_ptr->create(ctx, array);
				}
				if(res)
					res = ctx.set_attribute_array_void(&aab, loc, array_descriptor_traits <T>::get_type_descriptor(array), vbo_ptr, 0, array_descriptor_traits < T>::get_nr_elements(array));
				return res;
			}
			///
			template <typename T>
			bool set_attribute_array(const context& ctx, int loc, const T* array_ptr, size_t nr_elements, unsigned stride) {
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
					vbo_ptr = new vertex_buffer(VBT_VERTICES, default_usage);
					res = vbo_ptr->create(ctx, array_ptr, nr_elements);
				}
				if (res)
					res = ctx.set_attribute_array_void(&aab, loc, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(*array_ptr), true), vbo_ptr, 0, nr_elements);
				return res;
			}
			///
			bool set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes);

			template <typename C, typename T>
			bool set_composed_attribute_array(const context& ctx, int loc, const C* array_ptr, size_t nr_elements, const T& elem) {
				bool res;
				vertex_buffer*& vbo_ptr = vbos[loc];
				if (vbo_ptr) {
					if (vbo_ptr->get_size_in_bytes() == nr_elements * sizeof(C))
						res = vbo_ptr->replace(ctx, 0, array_ptr, nr_elements);
					else {
						vbo_ptr->destruct(ctx);
						res = vbo_ptr->create(ctx, array_ptr, nr_elements);
					}
				}
				else {
					vbo_ptr = new vertex_buffer(VBT_VERTICES, default_usage);
					res = vbo_ptr->create(ctx, array_ptr, nr_elements);
				}
				if (res)
					res = ctx.set_attribute_array_void(&aab, loc, 
						type_descriptor(element_descriptor_traits<T>::get_type_descriptor(elem), true), 
						vbo_ptr, 
						reinterpret_cast<const void*>(reinterpret_cast<const cgv::type::uint8_type*>(&elem) - reinterpret_cast<const cgv::type::uint8_type*>(array_ptr)), 
						nr_elements, sizeof(C));
				return res;
			}
			template <typename C, typename T>
			bool ref_composed_attribute_array(const context& ctx, int loc, int loc_ref, const C* array_ptr, size_t nr_elements, const T& elem) {
				vertex_buffer*& vbo_ptr = vbos[loc_ref];
				if (!vbo_ptr)
					return false;
				return ctx.set_attribute_array_void(&aab, loc,
					type_descriptor(element_descriptor_traits<T>::get_type_descriptor(elem), true),
					vbo_ptr,
					reinterpret_cast<const void*>(reinterpret_cast<const cgv::type::uint8_type*>(&elem) - reinterpret_cast<const cgv::type::uint8_type*>(array_ptr)),
					nr_elements, sizeof(C));
			}
		public:
			/// default initialization
			attribute_array_manager(VertexBufferUsage _default_usage = VBU_STREAM_DRAW);
			/// destructor calls destruct
			~attribute_array_manager();
			/// check whether the given attribute is available
			bool has_attribute(const context& ctx, int loc) const;
			/// returns the handle to the OpenGL buffer as managed by the attribute array manager if specified
			int get_buffer_handle(int loc) const {
				auto it = vbos.find(loc);
				if(it != vbos.end()) {
					const vertex_buffer* vbo_ptr = it->second;// vbos[loc];
					if(vbo_ptr->handle)
						return (const int&)vbo_ptr->handle - 1;
				}
				return (const int&)-1;
			}
			///
			bool init(context& ctx);
			///
			bool enable(context& ctx);
			///
			bool disable(context& ctx);
			///
			void destruct(const context& ctx);
		};
		/// abstract base class for all renderers that handles a shader program and position / color attribute
		class CGV_API renderer : public render_types
		{
		private:
			/// shader program
			shader_program prog;
			/// shader program
			shader_program* prog_ptr;
			/// otherwise keep track of enabled arrays
			std::set<int> enabled_attribute_arrays;
			/// default render style
			mutable render_style* default_render_style;
			/// current render style, can be set by user
			const render_style* rs;
			/// pointer to indices in CPU memory
			const void* indices;
			/// pointer to index buffer
			const vertex_buffer* index_buffer_ptr;
			/// type of indices
			cgv::type::info::TypeId index_type;
			/// count of indices
			size_t index_count;
			/// declare default attribute array manager used in core profile
			attribute_array_manager default_aam;
			/// if attribute array manager is set, use it for attribute management
			attribute_array_manager* aam_ptr;
		protected:
			/// check for attribute array manager
			bool has_aam() const { return aam_ptr != 0 && aam_ptr != &default_aam; }
			/// check for attribute
			bool has_attribute(const context& ctx, const std::string& attr_name) {
				return aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, attr_name));
			}
			/// access to render style
			const render_style* get_style_ptr() const;
			/// return whether attributes persist after a call to disable
			bool attributes_persist() const { return has_aam(); }
		public:
			/// derived renderer classes have access to shader program
			shader_program& ref_prog() { return *prog_ptr; }
			/// set external shader program up to next call to the disable() function, which is also called by the render() function
			void set_prog(shader_program& one_shot_prog);
		protected:
			/// access to style
			template <typename T>
			const T& get_style() const { return *static_cast<const T*>(get_style_ptr());  }
			/// track whether color attribute is defined
			mutable bool has_colors;
			/// track whether position attribute is defined
			mutable bool has_positions;
			/// virtual method that creates a default render style
			virtual render_style* create_render_style() const = 0;

			template <typename T>
			bool set_attribute_array(const context& ctx, int loc, const T& array) {
				if (aam_ptr)
					return aam_ptr->set_attribute_array(ctx, loc, array);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, array);
			}
			template <typename T>
			bool set_attribute_array(const context& ctx, int loc, const T* array_ptr, size_t nr_elements, unsigned stride) {
				if (aam_ptr)
					return aam_ptr->set_attribute_array(ctx, loc, array_ptr, nr_elements, stride);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, array_ptr, nr_elements, stride);
			}
			bool set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes);
			/// in case that several attributes are stored interleaved, call this function for the first and ref_composed_attribute_array() for all others
			template <typename C, typename T>
			bool set_composed_attribute_array(const context& ctx, int loc, const C* array_ptr, size_t nr_elements, const T& elem) {
				if (aam_ptr)
					return aam_ptr->set_composed_attribute_array(ctx, loc, array_ptr, nr_elements, elem);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, &elem, nr_elements, sizeof(C));
			}
			/// in case that several attributes are stored interleaved, call set_composed_attribute_array() for the first and this function for all others
			template <typename C, typename T>
			bool ref_composed_attribute_array(const context& ctx, int loc, int loc_ref, const C* array_ptr, size_t nr_elements, const T& elem) {
				if (aam_ptr)
					return aam_ptr->ref_composed_attribute_array(ctx, loc, loc_ref, array_ptr, nr_elements, elem);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, &elem, nr_elements, sizeof(C));
			}
			/// default implementation of draw method with support for indexed rendering and different primitive types
			void draw_impl(context& ctx, PrimitiveType pt, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index);
			/// default implementation of instanced draw method with support for indexed rendering and different primitive types
			void draw_impl_instanced(context& ctx, PrimitiveType type, size_t start, size_t count, size_t instance_count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index);
		public:
			/// construct and init attribute tracking flags
			renderer();
			/// destructor deletes default renderer style
			virtual ~renderer();
			/// used by derived classes to manage singletons
			void manage_singleton(context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change);
			/// call this before setting attribute arrays to manage attribute array in given manager
			virtual void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			virtual void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// this function is deprecated, please use enable_attribute_array_manager() and disable_attribute_manager() instead
			DEPRECATED("deprecated, use enable_attribute_array_manager() paired with disable_attribute_manager instead().")
				virtual void set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr = 0);
			/// reference given render style
			void set_render_style(const render_style& rs);
			/// abstract initialize method creates default render style, derived renderers to load the shader program
			virtual bool init(context& ctx);
			/// templated method to set the position attribute from a single position of type T
			template <typename T>
			void set_position(const context& ctx, const T& position) { has_positions = true; ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "position"), position); }
			/// templated method to set the position attribute from a vector of positions of type T
			template <typename T>
			void set_position_array(const context& ctx, const std::vector<T>& positions) { has_positions = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "position"), positions); }
			/// templated method to set the position attribute from a vector of positions of type T
			template <typename T>
			void set_position_array(const context& ctx, const T* positions, size_t nr_elements, unsigned stride_in_bytes = 0) { has_positions = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "position"), positions, nr_elements, stride_in_bytes); }
			/// method to set the position attribute from a vertex buffer object
			void set_position_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes);
			/// template method to set the position attribute from a vertex buffer object, the element type must be given as explicit template parameter
			template <typename T>
			void set_position_array(const context& ctx, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0) { set_position_array(ctx, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(T()), true), vbo, offset_in_bytes, nr_elements, stride_in_bytes); }
			/// templated method to set the color attribute from a single color of type T
			template <typename T>
			void set_color(const context& ctx, const T& color) { has_colors = true; ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "color"), color); }
			/// template method to set the color attribute from a vector of colors of type T
			template <typename T>
			void set_color_array(const context& ctx, const std::vector<T>& colors) { has_colors = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "color"), colors); }
			/// template method to set the color attribute from a vector of colors of type T
			template <typename T>
			void set_color_array(const context& ctx, const T* colors, size_t nr_elements, unsigned stride_in_bytes = 0) { has_colors = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "color"), colors, nr_elements, stride_in_bytes); }
			/// method to set the color attribute from a vertex buffer object, the element type must be given as explicit template parameter
			void set_color_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0);
			/// template method to set the color attribute from a vertex buffer object, the element type must be given as explicit template parameter
			template <typename T>
			void set_color_array(const context& ctx, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0) { set_color_array(ctx, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(T()), true), vbo, offset_in_bytes, nr_elements, stride_in_bytes); }
			/// <summary>
			/// Set the indices for indexed rendering from a vector. If an attribute array manager is
			/// enabled and keep_on_cpu is false (default), create GPU index buffer and transfer indices 
			/// into it.
			/// </summary>
			/// <typeparam name="T">index type must be uint8_t, uint16_t, or uint32_t</typeparam>
			/// <param name="ctx">opengl context in which indexed rendering takes place</param>
			/// <param name="indices">vector of indices</param>
			/// <param name="keep_on_cpu">flag whether indices should be kept in CPU memory</param>
			/// <returns>this can only fail if indices cannot be copied to GPU buffer</returns>
			template <typename T>
			bool set_indices(const context& ctx, const std::vector<T>& indices, bool keep_on_cpu = false) {
				this->indices = &indices.front();
				index_buffer_ptr = 0;
				index_count = indices.size();
				index_type = cgv::type::info::type_id<T>::get_id();
				if (!keep_on_cpu && aam_ptr)
					return aam_ptr->set_indices(ctx, indices);
				return true;
			}
			/// <summary>
			/// Set the indices for indexed rendering from an array given as a pointer. If an attribute array 
			/// manager is enabled and keep_on_cpu is false (default), create GPU index buffer and transfer 
			/// indices into it.
			/// </summary>
			/// <typeparam name="T">index type must be uint8_t, uint16_t, or uint32_t</typeparam>
			/// <param name="ctx">opengl context in which indexed rendering takes place</param>
			/// <param name="indices">pointer to array containing the indices</param>
			/// <param name="nr_indices">number of indices in the array</param>
			/// <param name="keep_on_cpu">flag whether indices should be kept in CPU memory</param>
			/// <returns>this can only fail if indices cannot be copied to GPU buffer</returns>
			template <typename T>
			bool set_indices(const context& ctx, const T* indices, size_t nr_indices, bool keep_on_cpu = false) {
				this->indices = indices;
				index_buffer_ptr = 0;
				index_count = nr_indices;
				index_type = cgv::type::info::type_id<T>::get_id();
				if (!keep_on_cpu && aam_ptr)
					return aam_ptr->set_indices(ctx, indices, nr_indices);
				return true;
			}
			/// 

			/// <summary>
			/// Set the indices for indexed rendering from a GPU buffer. If an attribute array manager
			/// is enabled its index buffer is removed through this call.
			/// </summary>
			/// <typeparam name="T">index type must be uint8_t, uint16_t, or uint32_t</typeparam>
			/// <param name="ctx">opengl context in which indexed rendering takes place</param>
			/// <param name="vbo">GPU buffer</param>
			/// <param name="count">number of indices in the GPU buffer</param>
			/// <returns>in current implementation this succeeds always</returns>
			template <typename T>
			bool set_indices(const context& ctx, const vertex_buffer& vbo, size_t count) { 
				index_buffer_ptr = &vbo;
				indices = 0;
				index_count = count;
				index_type = cgv::type::info::type_id<T>::get_id();
				if (aam_ptr)
					aam_ptr->remove_indices(ctx);
				return true;
			}
			/// return whether indices have been defined
			bool has_indices() const {
				if(aam_ptr)
					return aam_ptr->has_index_buffer();
				return index_count > 0;
			}
			/// remove previously set indices
			void remove_indices(const context& ctx);
			/*! Returns the OpenGL handle to the buffer of the given attribute name as managed by the attribute array manager.
				Returns -1 if the buffer or attribute array manager does not exist.
				Take caution when manipulating the buffer. */
			int get_vbo_handle(const context& ctx, const attribute_array_manager& aam, const std::string& attr_name) {
				return aam.get_buffer_handle(ref_prog().get_attribute_location(ctx, attr_name));
			}
			/*! Returns the OpenGL handle to the element buffer holding the indices for indexed rendering as managed by the attribute array manager.
				Returns -1 if the buffer or attribute array manager does not exist.
				Take caution when manipulating the buffer. */
			int get_index_buffer_handle(const attribute_array_manager& aam) {
				return aam.get_buffer_handle(-1);
			}
			/// call to validate, whether essential position attribute is defined
			virtual bool validate_attributes(const context& ctx) const;
			/// validate attributes and if successful, enable renderer
			bool validate_and_enable(context& ctx);
			/// enable renderer
			virtual bool enable(context& ctx);
			//! Draw a range of vertices or indexed elements.
			/*! Call this function only successful enabeling via validate_and_enable() or enable().
			    Capsulates glDrawArrays and glDrawElements calls. Overloaded implementations of specific 
			    renderers choose the to be used gl primitive type and whether to use an instanced draw call.
			    \sa render()
			    \param count number of to be drawn vertices/elements
			    \param start index of first to be drawn vertex/element
				\param use_strips whether to generate primitives in strips (only for line or triangle primitives) 
				\param use_adjacency whether to specify adjacency information (only for line or triangle primitives)
				\param strip_restart_index extraorindary index used to mark end of strips (only for strip based drawing) 
				Strip based drawing can be combined with the use of adjacency information. Default implementation
				uses triangle primitives. */
			virtual void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
			/// disable renderer
			virtual bool disable(context& ctx);
			//! Convenience function that draws vertex or indexed element with this renderer.
			/*! This function effectively calls validate_and_enable(), draw() and disable(), passes
			    its parameters to draw and returns the result of validate_and_enable(). draw() and disable()
				are only executed if validate_and_enable() succeeds. For performance reasons this function
				should not be used for several successive draw calls due to the unnecessary enabling and disabling
				between render calls.
				Typically, this function does not need to be overloaded by specific renderers.
				\sa draw()
				\sa validate_and_enable()*/
			virtual bool render(context& ctx, size_t start, size_t count, 
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
			/// the clear function destructs the shader program
			virtual void clear(const context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>