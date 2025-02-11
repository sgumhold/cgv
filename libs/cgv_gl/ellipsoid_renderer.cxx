#include "ellipsoid_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		ellipsoid_renderer& ref_ellipsoid_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static ellipsoid_renderer r;
			r.manage_singleton(ctx, "ellipsoid_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* ellipsoid_renderer::create_render_style() const
		{
			return new ellipsoid_render_style();
		}

		ellipsoid_render_style::ellipsoid_render_style()
		{
			size_scale = 1;
			size = 1;
		}

		ellipsoid_renderer::ellipsoid_renderer()
		{
			has_sizes = false;
			has_orientations = false;
			cull_per_primitive = false;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void ellipsoid_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "size"))
				has_sizes = true;
			if (has_attribute(ctx, "orientation"))
				has_orientations = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void ellipsoid_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::disable_attribute_array_manager(ctx, aam);
			has_sizes = false;
			has_orientations = false;
		}
		void ellipsoid_renderer::remove_size_array(const context& ctx) {
			has_sizes = false;
			remove_attribute_array(ctx, "size");
		}
		void ellipsoid_renderer::remove_orientation_array(const context& ctx) {
			has_orientations = false;
			remove_attribute_array(ctx, "orientation");
		}
		bool ellipsoid_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "ellipsoid.glpr", true, defines);
		}
		bool ellipsoid_renderer::validate_attributes(const context& ctx) const
		{
			const ellipsoid_render_style& rs = get_style<ellipsoid_render_style>();
			return surface_renderer::validate_attributes(ctx);
		}
		bool ellipsoid_renderer::enable(context& ctx)
		{
			const ellipsoid_render_style& rs = get_style<ellipsoid_render_style>();

			if (!surface_renderer::enable(ctx))
				return false;

			if (!ref_prog().is_linked())
				return false;
			
			if (!has_sizes)
				ref_prog().set_attribute(ctx, "size", rs.size);

			if (!has_orientations)
				ref_prog().set_attribute(ctx, "orientation", cgv::math::quaternion<float>());

			ref_prog().set_uniform(ctx, "size_scale", rs.size_scale);
			return true;
		}

		bool ellipsoid_renderer::disable(context& ctx)
		{
			if (!attributes_persist()) {
				has_sizes = false;
				has_orientations = false;
			}
			return surface_renderer::disable(ctx);
		}

		void ellipsoid_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}

		bool ellipsoid_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<surface_render_style*>(this)) &&
				rh.reflect_member("size_scale", size_scale);
		}

		cgv::reflect::extern_reflection_traits<ellipsoid_render_style, ellipsoid_render_style_reflect> get_reflection_traits(const ellipsoid_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<ellipsoid_render_style, ellipsoid_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct ellipsoid_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::ellipsoid_render_style>::get_name())
					return false;
				cgv::render::ellipsoid_render_style* rs_ptr = reinterpret_cast<cgv::render::ellipsoid_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "Default Size", rs_ptr->size, "value_slider", "min=0.01;max=100;log=true;ticks=true");
				p->add_member_control(b, "Size Scale", rs_ptr->size_scale, "value_slider", "min=0.01;max=100;log=true;ticks=true");
				
				p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(rs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<ellipsoid_render_style_gui_creator> ellipsoid_rs_gc_reg("ellipsoid_render_style_gui_creator");

	}
}