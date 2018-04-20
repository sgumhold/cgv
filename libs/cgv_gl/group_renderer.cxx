#include "group_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {


		group_render_style::group_render_style()
		{
			use_group_color = false;
			use_group_transformation = false;
		}

		group_renderer::group_renderer()
		{
			has_group_indices = false;
			has_group_colors = false;
			has_group_translations = false;
			has_group_rotations = false;
		}

		void group_renderer::set_group_index_attribute(cgv::render::context& ctx, const std::vector<unsigned>& group_indices)
		{
			has_group_indices = true;
			set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "group_index"), group_indices);
		}
		/// method to set the group index attribute
		void group_renderer::set_group_index_attribute(cgv::render::context& ctx, const unsigned* group_indices, size_t nr_elements)
		{
			has_group_indices = true;
			set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "group_index"), group_indices, nr_elements, 0);
		}

		bool group_renderer::validate_attributes(context& ctx) const
		{
			// validate set attributes
			bool res = renderer::validate_attributes(ctx);
			const group_render_style& grs = get_style<group_render_style>();
			if (!has_group_indices && (grs.use_group_color || grs.use_group_transformation)) {
				ctx.error("group_renderer::enable() group_index attribute not set while using group colors or group transformations");
				res = false;
			}
			if (!has_group_colors && grs.use_group_color) {
				ctx.error("group_renderer::enable() group_colors not set");
				res = false;
			}
			if (!has_group_translations && grs.use_group_transformation) {
				ctx.error("group_renderer::enable() group_translations not set");
				res = false;
			}
			if (!has_group_rotations && grs.use_group_transformation) {
				ctx.error("group_renderer::enable() group_rotations not set");
				res = false;
			}
			return res;
		}

		bool group_renderer::enable(cgv::render::context& ctx)
		{
			bool res = renderer::enable(ctx);
			const group_render_style& grs = get_style<group_render_style>();
			if (ref_prog().is_linked()) {
				ref_prog().set_uniform(ctx, "use_group_color", grs.use_group_color);
				ref_prog().set_uniform(ctx, "use_group_transformation", grs.use_group_transformation);
			}
			else
				res = false;
			return res;
		}
		bool group_renderer::disable(cgv::render::context& ctx)
		{
			bool res = renderer::disable(ctx);
			if (!attributes_persist())
				has_group_indices;
			return res;
		}
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			bool group_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
			{
				return
					rh.reflect_member("use_group_color", use_group_color) &&
					rh.reflect_member("use_group_transformation", use_group_transformation);
			}

		}
		cgv::reflect::extern_reflection_traits<cgv::render::group_render_style, cgv::reflect::render::group_render_style> get_reflection_traits(const cgv::render::group_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<cgv::render::group_render_style, cgv::reflect::render::group_render_style>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct group_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::group_render_style>::get_name())
					return false;
				cgv::render::group_render_style* grs_ptr = reinterpret_cast<cgv::render::group_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "use_group_color", grs_ptr->use_group_color);
				p->add_member_control(b, "use_group_transformation", grs_ptr->use_group_transformation);
				return true;
			}
		};

#include "gl/lib_begin.h"

		extern CGV_API cgv::gui::gui_creator_registration<group_render_style_gui_creator> frs_gc_reg("group_render_style_gui_creator");

	}
}