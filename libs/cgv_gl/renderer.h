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
			// Make destructor virtual to allow deletion of inherited class instances through pointer to this base class.
			virtual ~render_style();
		};
		
		//! Abstract base class for all renderers that manage a render data and the rendering process.
		/*! Render data is managed through buffer objects and fed into the rendering pipeline via an
		*   attribute array object. Render data management is done either with vertex array pointers
		*   or with an attribute array manager that manages buffers and one attribute array object.
		*   The rendering process is configured via a configuration values stored in a renderer specific
		*   render style, from which shader program defines, default attribute values and uniform values
		*   are derived. Besides standard compiler defines, a custom mechanism similar to macros called 
		*   snippets is provided mostly for compute shader usage and configured in specialized renderers.
		*   
		*   Rendering Process is decomposed into
		*   - validate_attributes() ... check for essential attributes
		*   - enable() ... re-compile shader program if render style configuration induced a change in 
		*                  program defines or snippets
		*   - draw()   ... emit a draw call to execute the rendering pipeline. Multiple calls to draw() are
		*                  possible
		*   - disable()... disable all program, buffers, and attribute array object and clear one shot
		*                  programs and attributes
		* 
		*   Rendering Process convenience functions are
		*   - validate_and_enable() ... call validate_attributes() and in case of success enable()  
		*   - render() ... used for single draw calls and calls validate_and_enable(), draw(), disable() 
		*
		*   External program can be used by setting it with the set_prog() function. For this a pointer
		*   to the external program is stored and used to resolve attribute indices and for successive
		*   draw calls until the first call to disable(), what is also called when the render() function
		*   is used. Thereafter the pointer is cleared and the own shader program set as active again. 
		*   If an external program should be used multiple times, it needs to be set again with set_prog() 
		*   after each call to disable() or render(). The user has to make sure that the external program 
		*   exists till the next disable() or render() call using it.
		*
		*   Shader program attribute indices are determined from the own or external shader program via 
		*   predefinded names, e.g. "position" and "color" for the attributes handled by the renderer 
		*   class, during calls to set_position(), set_position_array(), set_color(), set_color_array().
		*   Thus make sure if you use an external program to set the attributes after calling the 
		*   set_prog() function.
		* 
		*   Attributes persist similar to external program usage only until the next disable() or
		*   render() function call. In compatibility attribute array points are used to transfer 
		*   attribute data to the gpu which can be done assynchronuously and in parallel to draw 
		*   calls. For this to work, the user has to ensure that the CPU side attribute array data 
		*   persists will the last draw() call using it. 
		*   To store attribute data for multiple draw calls persistently in gpu buffers, an external
		*   attribute array manager can be used and enabled or disabled with the 
		*   enable_attribute_manager() and disable_attribute_manager() functions. In core profile a
		*   default attribute array manager is constructed in case no external manager is provided.
		*   In this case, the attributes are still cleared after each calls to diable() or render()
		*   but the CPU side attribute data objects can be destructed before the draw calls. The 
		*   context object ctx allows to check for core profile via the member ctx.core_profile.
		* 
		*   Render styles of concrete renderers inherit the abstract class render_style that has a 
		*   virtual destructor. The renderer class manages a default render style and supports setting
		*   an external render style with the set_render_style() function.
		*/
		class CGV_API renderer
		{
		private:
			/// shader program
			shader_program prog;
			/// shader program
			shader_program* prog_ptr = &prog;
			/// current and modifyable shader program compile options
			shader_compile_options prog_options;
			/// last shader program compile options used to build the current program
			shader_compile_options last_prog_options;
//#ifdef _DEBUG
			/* TODO: FIXME: Find out why _DEBUG is sometimes not set in CMake Ninja builds under Linux, causing crashes
			                due to inconsistent object layout in memory between different modules using the renderers */
			/// count of render calls with current program configuration (used to detect frequent rebuilds);
			/// initialized to the threshold count of 10 to prevent outputting warnings for the very first render procedure
			int current_prog_render_count = 10;
//#endif
			/// otherwise keep track of enabled arrays
			std::set<int> enabled_attribute_arrays;
			/// default render style
			mutable render_style* default_render_style = nullptr;
			/// current render style, can be set by user
			const render_style* rs = nullptr;
			/// pointer to indices in CPU memory
			const void* indices = nullptr;
			/// pointer to index buffer
			const vertex_buffer* index_buffer_ptr = nullptr;
			/// type of indices
			cgv::type::info::TypeId index_type = cgv::type::info::TI_UNDEF;
			/// count of indices
			size_t index_count = 0;
			/// declare default attribute array manager used in core profile
			attribute_array_manager default_aam;
			/// if attribute array manager is set, use it for attribute management
			attribute_array_manager* aam_ptr;
		protected:
			/// check for attribute array manager
			bool has_aam() const { return aam_ptr != 0 && aam_ptr != &default_aam; }
			/// check for attribute
			bool has_attribute(const context& ctx, const std::string& name);
			/// implement this method to create a default render style
			virtual render_style* create_render_style() const = 0;
			/// access to render style
			const render_style* get_style_ptr() const;
			/// return whether attributes persist after a call to disable
			bool attributes_persist() const { return has_aam(); }
			/// overload to update the shader program compile options based on the current render style; only called if internal shader program is used
			virtual void update_shader_program_options(shader_compile_options& options) const {}
			/// overload to change default behaviour and build a custom shader program based on the passed options
			virtual bool build_shader_program(context& ctx, shader_program& prog, const shader_compile_options& options) const;
		public:
			/// access to shader program compile options to update settings not handled by render style
			shader_compile_options& ref_shader_options() { return prog_options; }
			/// derived renderer classes have access to shader program
			shader_program& ref_prog() { return *prog_ptr; }
			/// set external shader program up to next call to disable() or render()
			void set_prog(shader_program& one_shot_prog);
		protected:
			/// implement this method to return the name of the default shader program; return an empty string if the renderer handles program creation on its own
			virtual std::string get_default_prog_name() const = 0;

			/// access to style
			template <typename T>
			const T& get_style() const { return *static_cast<const T*>(get_style_ptr());  }
			/// track whether color attribute is defined
			mutable bool has_colors = false;
			/// track whether position attribute is defined
			mutable bool has_positions = false;

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
				int loc = get_prog_attribute_location(ctx, name);
				int loc_ref = get_prog_attribute_location(ctx, name_ref);
				if(loc < 0 || loc_ref < 0)
					return false;
				if (aam_ptr)
					return aam_ptr->ref_composed_attribute_array(ctx, loc, loc_ref, array_ptr, nr_elements, elem);
				enabled_attribute_arrays.insert(loc);
				return attribute_array_binding::set_global_attribute_array(ctx, loc, &elem, nr_elements, sizeof(C));
			}
			bool remove_attribute_array(const context& ctx, const std::string& name);
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
			/// remove the position attribute
			void remove_position_array(const context& ctx);
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
			/// remove the color attribute
			void remove_color_array(const context& ctx);
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