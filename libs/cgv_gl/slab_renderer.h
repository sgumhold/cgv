#pragma once

#include "group_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		class CGV_API slab_renderer;

		//! reference to a singleton slab renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API slab_renderer& ref_slab_renderer(context& ctx, int ref_count_change = 0);

		/*!	Style to control the look of slabs. The slab renderer uses texture arrays for rendering.
			A texture index can be specified for each slab, which is used to access the texture array.
			Ech slab texture must therefore have the same resolution.
			For animated slabs store the timesteps for each slab in a non-interleaved fashion in the
			texture array. Set the tex_idx_stride to the number of animation frames. The tex_idx_offset
			can be used to select the rendered time step.
		*/
		struct CGV_API slab_render_style : public group_render_style
		{
			/// multiplied to the thickness, initialized to 1
			float thickness_scale;
			/// unit used to access the slab texture values, initialized to 0, must point to a 2d texture array
			int tex_unit;
			/// unit used to access the volume transfer function texture, initialized to 1, must be a 1d texture
			int tf_tex_unit;
			/// whether to use a transfer function or interpret the luminance as alpha, initialized to false
			bool use_transfer_function;
			///
			int tf_source_channel;
			/// used to offset the texture index of each slab when specified
			int tex_idx_offset;
			/// multiplied to the texture index of each slab
			int tex_idx_stride;
			/// constant step size for volume ray integration over all slabs
			float step_size;
			/// multiplied to the input opacity value during rendering to change the overall opacity of the slabs
			float opacity;
			/// overall influence of the falloff
			float falloff_mix;
			/// strength of the opacity falloff in the slabs normal direction
			float falloff_strength;
			/// compensates for the overall slab scale
			float scale;
			/// construct with default values
			slab_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API slab_renderer : public group_renderer
		{
		protected:
			/// store whether extent array has been specified
			bool has_extents;
			/// whether array with per slab translations has been specified
			bool has_translations;
			/// whether array with per slab rotations has been specified
			bool has_rotations;
			/// whether array with per slab texture index has been specified
			bool has_texture_indices;
			/// whether position is slab center, if not it is lower left bottom corner
			bool position_is_center;
			/// store whether thickness array has been specified
			bool has_thicknesses;
			/// overload to allow instantiation of box_renderer
			render_style* create_render_style() const;
			/// build slab program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
		public:
			/// initializes position_is_center to true 
			slab_renderer();
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// set the flag, whether the position is interpreted as the slab center, true by default
			void set_position_is_center(bool _position_is_center);
			/// 
			bool enable(context& ctx);
			/// specify a single extent for all slabs
			template <typename T>
			void set_extent(const context& ctx, const T& extent) { has_extents = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "extent"), extent); }
			/// extent array specifies slab extends in case of position_is_center=true, otherwise the maximum point of each slab
			template <typename T>
			void set_extent_array(const context& ctx, const std::vector<T>& extents) { has_extents = true; set_attribute_array(ctx,  "extent", extents); }
			/// extent array specifies slab extends in case of position_is_center=true, otherwise the maximum point of each slab
			template <typename T>
			void set_extent_array(const context& ctx, const T* extents, size_t nr_elements, unsigned stride_in_bytes = 0) { has_extents = true;  set_attribute_array(ctx, "extent", extents, nr_elements, stride_in_bytes); }
			/// specify box array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_box_array(const context& ctx, const std::vector<cgv::media::axis_aligned_box<T, 3> >& box) {
				set_composed_attribute_array(ctx, "position", &box.front(), box.size(), box[0].get_min_pnt());
				ref_composed_attribute_array(ctx, "extent", "position", &box.front(), box.size(), box[0].get_max_pnt());
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
			}
			/// specify box array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_box_array(const context& ctx, const cgv::media::axis_aligned_box<T, 3>* box, size_t count) {
				set_composed_attribute_array(ctx, "position", box, count, box[0].get_min_pnt());
				ref_composed_attribute_array(ctx, "extent", "position", box, count, box[0].get_max_pnt());
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
			}
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const std::vector<T>& translations) { has_translations = true; set_attribute_array(ctx, "translation", translations); }
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const T* translations, size_t nr_elements, unsigned stride) { has_translations = true; set_attribute_array(ctx, "translation", translations, nr_elements, stride); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const std::vector<T>& rotations) { has_rotations = true; set_attribute_array(ctx, "rotation", rotations); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const T* rotations, size_t nr_elements, unsigned stride) { has_rotations = true; set_attribute_array(ctx, "rotation", rotations, nr_elements, stride); }
			/// extent array specifies slab extends in case of position_is_center=true, otherwise the maximum point of each slab
			template <typename T>
			void set_texture_index_array(const context& ctx, const std::vector<T>& texture_indices) { has_texture_indices = true; set_attribute_array(ctx, "texture_index", texture_indices); }
			/// extent array specifies slab extends in case of position_is_center=true, otherwise the maximum point of each slab
			template <typename T>
			void set_texture_index_array(const context& ctx, const T* texture_indices, size_t nr_elements, unsigned stride_in_bytes = 0) { has_texture_indices = true;  set_attribute_array(ctx, "texture_index", texture_indices, nr_elements, stride_in_bytes); }
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool disable(context& ctx);
			///
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};

		struct CGV_API slab_render_style_reflect : public slab_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<slab_render_style, slab_render_style_reflect> get_reflection_traits(const slab_render_style&);
	}
}

#include <cgv/config/lib_end.h>
