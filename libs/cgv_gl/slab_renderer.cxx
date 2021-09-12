#include "slab_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		slab_renderer& ref_slab_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static slab_renderer r;
			r.manage_singleton(ctx, "slab_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* slab_renderer::create_render_style() const
		{
			return new slab_render_style();
		}

		slab_render_style::slab_render_style()
		{
			thickness_scale = 1.0f;
			tex_unit = 0;
			tf_tex_unit = 1;
			use_transfer_function = false;
			tf_source_channel = 0;
			tex_idx_offset = 0;
			tex_idx_stride = 1;
			step_size = 0.02f;
			opacity = 1.0f;
			falloff_mix = 1.0f;
			falloff_strength = 1.0f;
			scale = 20.0f;
		}

		slab_renderer::slab_renderer()
		{
			has_extents = false;
			position_is_center = true;
			has_translations = false;
			has_rotations = false;
			has_texture_indices = false;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void slab_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "extent"))
				has_extents = true;
			if (has_attribute(ctx, "translation"))
				has_translations = true;
			if (has_attribute(ctx, "rotation"))
				has_rotations = true;
			if (has_attribute(ctx, "texture_index"))
				has_texture_indices = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void slab_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::disable_attribute_array_manager(ctx, aam);
			has_extents = false;
			has_translations = false;
			has_rotations = false;
			has_texture_indices = false;
		}

		/// set the flag, whether the position is interpreted as the slab center
		void slab_renderer::set_position_is_center(bool _position_is_center)
		{
			position_is_center = _position_is_center;
		}

		bool slab_renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			const slab_render_style& srs = get_style<slab_render_style>();
			bool res = group_renderer::validate_attributes(ctx);
			if (!has_extents) {
				ctx.error("slab_renderer::enable() extent attribute not set");
				res = false;
			}
			return res;
		}
		/// build slab program
		bool slab_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "slab.glpr", true, defines);
		}
		/// 
		bool slab_renderer::enable(context& ctx)
		{
			const slab_render_style& srs = get_style<slab_render_style>();

			if (!group_renderer::enable(ctx))
				return false;
			ref_prog().set_uniform(ctx, "position_is_center", position_is_center);
			ref_prog().set_uniform(ctx, "has_rotations", has_rotations);
			ref_prog().set_uniform(ctx, "has_translations", has_translations);
			ref_prog().set_uniform(ctx, "has_texture_indices", has_texture_indices);
			ref_prog().set_uniform(ctx, "thickness_scale", srs.thickness_scale);
			ref_prog().set_uniform(ctx, "tex", srs.tex_unit);
			ref_prog().set_uniform(ctx, "tf_tex", srs.tf_tex_unit);
			ref_prog().set_uniform(ctx, "use_transfer_function", srs.use_transfer_function);
			ref_prog().set_uniform(ctx, "tf_source_channel", srs.tf_source_channel);
			ref_prog().set_uniform(ctx, "tex_idx_offset", srs.tex_idx_offset);
			ref_prog().set_uniform(ctx, "tex_idx_stride", srs.tex_idx_stride);
			ref_prog().set_uniform(ctx, "step_size", srs.step_size);
			ref_prog().set_uniform(ctx, "opacity_scale", srs.opacity);
			ref_prog().set_uniform(ctx, "falloff_mix", srs.falloff_mix);
			ref_prog().set_uniform(ctx, "falloff_strength", srs.falloff_strength);
			ref_prog().set_uniform(ctx, "scale", srs.scale);

			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			return true;
		}
		///
		bool slab_renderer::disable(context& ctx)
		{
			if (!attributes_persist()) {
				has_extents = false;
				position_is_center = true;
				has_rotations = false;
				has_translations = false;
			}

			glDisable(GL_BLEND);
			glDisable(GL_CULL_FACE);

			return group_renderer::disable(ctx);
		}

		void slab_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}

		bool slab_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<slab_render_style*>(this)) &&
				rh.reflect_member("thickness", thickness_scale) &&
				rh.reflect_member("tex_unit", tex_unit);
		}

		cgv::reflect::extern_reflection_traits<slab_render_style, slab_render_style_reflect> get_reflection_traits(const slab_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<slab_render_style, slab_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct slab_render_style_gui_creator : public gui_creator {
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*) {
				if(value_type != cgv::type::info::type_name<cgv::render::slab_render_style>::get_name())
					return false;
				cgv::render::slab_render_style* srs_ptr = reinterpret_cast<cgv::render::slab_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "thickness scale", srs_ptr->thickness_scale, "value_slider", "min=0.001;step=0.0001;max=10.0;log=true;ticks=true");
				p->add_member_control(b, "texture unit", srs_ptr->tex_unit, "value_slider", "min=0;step=1;max=9;log=false;ticks=true");
				p->add_member_control(b, "transfer function unit", srs_ptr->tf_tex_unit, "value_slider", "min=0;step=1;max=9;log=false;ticks=true");
				p->add_member_control(b, "use transfer function", srs_ptr->use_transfer_function, "check");
				p->add_member_control(b, "tf source channel", srs_ptr->tf_source_channel, "value_slider", "min=0;step=1;max=3;log=false;ticks=true");

				p->add_member_control(b, "step_size", srs_ptr->step_size, "value_slider", "min=0.001;step=0.001;max=0.5;log=true;ticks=true");
				p->add_member_control(b, "opacity scale", srs_ptr->opacity, "value_slider", "min=0.0;step=0.0001;max=1.0;log=true;ticks=true");
				p->add_member_control(b, "falloff mix", srs_ptr->falloff_mix, "value_slider", "min=0.0;step=0.0001;max=1.0;ticks=true");
				p->add_member_control(b, "falloff strength", srs_ptr->falloff_strength, "value_slider", "min=0.0;step=0.0001;max=10.0;log=true;ticks=true");
				p->add_member_control(b, "scale", srs_ptr->scale, "value_slider", "min=0.0;step=0.0001;max=100.0;log=true;ticks=true");

				//p->add_gui("group_render_style", *static_cast<cgv::render::group_render_style*>(srs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

		CGV_API cgv::gui::gui_creator_registration<slab_render_style_gui_creator> slab_rs_gc_reg("slab_render_style_gui_creator");
	}
}
