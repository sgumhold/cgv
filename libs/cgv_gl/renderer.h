#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl_context.h>

#include "attribute_array_manager.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		
		/// base class for all render styles
		struct CGV_API render_style
		{
			virtual ~render_style();
		};
		
		/// abstract base class for all renderers that handles a shader program and position / color attribute
		class CGV_API renderer
		{
		private:
			/// shader program
			shader_program prog;
			/// shader program
			shader_program* prog_ptr;
			/// shader define maps
			shader_define_map defines;
			/// last shader define maps
			shader_define_map last_defines;
//#ifdef _DEBUG
			/* TODO: FIXME: Find out why _DEBUG is sometimes not set in CMake Ninja builds under Linux, causing crashes
			                due to inconsistent object layout in memory between different modules using the renderers */
			/// count of render calls with current program configuration (used to detect frequent rebuilds)
			int current_prog_render_count;
//#endif
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
			bool has_attribute(const context& ctx, const std::string& name) {
				return aam_ptr->has_attribute(ctx, get_prog_attribute_location(ctx, name, false));
			}
			/// access to render style
			const render_style* get_style_ptr() const;
			/// return whether attributes persist after a call to disable
			bool attributes_persist() const { return has_aam(); }
			/// overload to update the shader defines based on the current render style; only called if internal shader program is used
			virtual void update_defines(shader_define_map& defines) {}
			/// overload to build shader program based on the passed defines
			virtual bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines) { return false; }
		public:
			/// access to shader define map to update defines not handled by render style
			shader_define_map& ref_defines() { return defines; }
			/// derived renderer classes have access to shader program
			shader_program& ref_prog() { return *prog_ptr; }
			/// set external shader program up to next call to disable() or render()
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

			int get_prog_attribute_location(const context& ctx, const std::string & name, bool error_check = true) {
				int loc = ref_prog().get_attribute_location(ctx, name);
#ifdef _DEBUG
				if(loc < 0 && error_check)
					std::cerr << "Warning: cgv_gl::renderer attribute " << name << " not supported by current shader program" << std::endl;
#endif
				return loc;
			}

			template <typename T>
			bool set_attribute_array(const context& ctx, const std::string& name, const T& array) {
			//bool set_attribute_array(const context& ctx, int loc, const T& array) {
				int loc = get_prog_attribute_location(ctx, name);
				if(loc < 0)
					return false;
				if(aam_ptr)
					return aam_ptr->set_attribute_array(ctx, loc, array);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, array);
			}
			template <typename T>
			bool set_attribute_array(const context& ctx, const std::string& name, const T* array_ptr, size_t nr_elements, unsigned stride) {
			//bool set_attribute_array(const context& ctx, int loc, const T* array_ptr, size_t nr_elements, unsigned stride) {
				int loc = get_prog_attribute_location(ctx, name);
				if(loc < 0)
					return false;
				if (aam_ptr)
					return aam_ptr->set_attribute_array(ctx, loc, array_ptr, nr_elements, stride);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, array_ptr, nr_elements, stride);
			}
			bool set_attribute_array(const context& ctx, const std::string& name, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes);
			//bool set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes);
			/// in case that several attributes are stored interleaved, call this function for the first and ref_composed_attribute_array() for all others
			template <typename C, typename T>
			bool set_composed_attribute_array(const context& ctx, const std::string& name, const C* array_ptr, size_t nr_elements, const T& elem) {
			//bool set_composed_attribute_array(const context& ctx, int loc, const C* array_ptr, size_t nr_elements, const T& elem) {
				int loc = get_prog_attribute_location(ctx, name);
				if(loc < 0)
					return false;
				if (aam_ptr)
					return aam_ptr->set_composed_attribute_array(ctx, loc, array_ptr, nr_elements, elem);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, &elem, nr_elements, sizeof(C));
			}
			/// in case that several attributes are stored interleaved, call set_composed_attribute_array() for the first and this function for all others
			template <typename C, typename T>
			bool ref_composed_attribute_array(const context& ctx, const std::string& name, const std::string& name_ref, const C* array_ptr, size_t nr_elements, const T& elem) {
			//bool ref_composed_attribute_array(const context& ctx, int loc, int loc_ref, const C* array_ptr, size_t nr_elements, const T& elem) {
				int loc = get_prog_attribute_location(ctx, name);
				int loc_ref = get_prog_attribute_location(ctx, name_ref);
				if(loc < 0 || loc_ref < 0)
					return false;
				if (aam_ptr)
					return aam_ptr->ref_composed_attribute_array(ctx, loc, loc_ref, array_ptr, nr_elements, elem);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, &elem, nr_elements, sizeof(C));
			}
		public:
			/// default implementation of draw method with support for indexed rendering and different primitive types
			void draw_impl(context& ctx, PrimitiveType pt, size_t start, size_t count, bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
			/// default implementation of instanced draw method with support for indexed rendering and different primitive types
			void draw_impl_instanced(context& ctx, PrimitiveType type, size_t start, size_t count, size_t instance_count, bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
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
			/// build shader program for specific render style
			bool build_program(context& ctx, shader_program& prog, const render_style& rs);
			//! call init() once before using renderer
			/*! creates default render style and builds shader program based on defines that can be
			    configured with ref_defines() before calling init(). Reconfiguring defines after init() causes
				rebuild of shader program in enable() function. */
			virtual bool init(context& ctx);
			/// templated method to set the position attribute from a single position of type T
			template <typename T>
			void set_position(const context& ctx, const T& position) { has_positions = true; ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "position"), position); }
			/// templated method to set the position attribute from a vector of positions of type T
			template <typename T>
			void set_position_array(const context& ctx, const std::vector<T>& positions) { has_positions = true; set_attribute_array(ctx, "position", positions); }
			/// templated method to set the position attribute from a vector of positions of type T
			template <typename T>
			void set_position_array(const context& ctx, const T* positions, size_t nr_elements, unsigned stride_in_bytes = 0) { has_positions = true; set_attribute_array(ctx, "position", positions, nr_elements, stride_in_bytes); }
			/// method to set the position attribute from a vertex buffer object
			void set_position_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes);
			/// template method to set the position attribute from a vertex buffer object, the element type must be given as explicit template parameter
			template <typename T>
			void set_position_array(const context& ctx, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0) { set_position_array(ctx, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(T()), true), vbo, offset_in_bytes, nr_elements, stride_in_bytes); }
			/// templated method to set the color attribute from a single color of type T
			template <typename T>
			void set_color(const context& ctx, const T& color) { has_colors = true; ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "color"), color); }
			/// template method to set the color attribute from a vector of colors of type T
			template <typename T>
			void set_color_array(const context& ctx, const std::vector<T>& colors) { has_colors = true;  set_attribute_array(ctx, "color", colors); }
			/// template method to set the color attribute from a vector of colors of type T
			template <typename T>
			void set_color_array(const context& ctx, const T* colors, size_t nr_elements, unsigned stride_in_bytes = 0) { has_colors = true;  set_attribute_array(ctx, "color", colors, nr_elements, stride_in_bytes); }
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
				if(aam_ptr && aam_ptr->has_index_buffer())
					return true;
				return index_count > 0;
			}
			/// remove previously set indices
			void remove_indices(const context& ctx);
			/*! Returns a pointer to the vertex buffer of the given attribute name as managed by the attribute array manager.
				Returns nullptr if the buffer or attribute array manager does not exist.
				Take caution when manipulating the buffer. */
			const vertex_buffer* get_vertex_buffer_ptr(const context& ctx, const attribute_array_manager& aam, const std::string& attr_name) {
				return aam.get_buffer_ptr(get_prog_attribute_location(ctx, attr_name));
			}
			/*! Returns a pointer to the vertex buffer of type element buffer holding the indices for indexed rendering as managed by the attribute array manager.
				Returns nullptr if the buffer or attribute array manager does not exist.
				Take caution when manipulating the buffer. */
			const vertex_buffer* get_index_buffer_ptr(const attribute_array_manager& aam) {
				return aam.get_buffer_ptr(-1);
			}
			/// call to validate, whether essential position attribute is defined
			virtual bool validate_attributes(const context& ctx) const;
			/// validate attributes and if successful, enable renderer
			bool validate_and_enable(context& ctx);
			//! enables renderer
			/*! if internal program is used, first update defines with update_defines() and rebuild 
			    program if it changed due updating or external modification via ref_defines()*/
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