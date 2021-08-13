#pragma once

#include "line_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		class CGV_API normal_renderer;

		//! reference to a singleton normal renderer that can be shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API normal_renderer& ref_normal_renderer(context& ctx, int ref_count_change = 0);


		struct CGV_API normal_render_style : public line_render_style
		{
			float normal_length;
			normal_render_style();
		};

		/// renderer that supports rendering point normals
		class CGV_API normal_renderer : public line_renderer
		{
		protected:
			/// scaling of normal length
			float normal_scale;
			/// overload to allow instantiation of box_wire_renderer
			render_style* create_render_style() const;
			/// build normal program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
			bool validate_attributes(const context& ctx) const;
		public:
			normal_renderer();
			/// the normal scale is multiplied to the normal length of the normal render style
			void set_normal_scale(float _normal_scale);
			/// enable normal renderer
			bool enable(context& ctx);
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};

		struct CGV_API normal_render_style_reflect : public normal_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<normal_render_style, normal_render_style_reflect> get_reflection_traits(const cgv::render::normal_render_style&);
	}
}

#include <cgv/config/lib_end.h>