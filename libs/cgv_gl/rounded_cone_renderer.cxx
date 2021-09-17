#include "rounded_cone_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		rounded_cone_renderer& ref_rounded_cone_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static rounded_cone_renderer r;
			r.manage_singleton(ctx, "rounded_cone_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* rounded_cone_renderer::create_render_style() const
		{
			return new rounded_cone_render_style();
		}

		rounded_cone_render_style::rounded_cone_render_style()
		{
			radius = 1.0f;
			radius_scale = 1.0f;
			
			enable_texturing = false;
			texture_blend_mode = TBM_MIX;
			texture_blend_factor = 1.0f;
			texture_tile_from_center = false;
			texture_offset = vec2(0.0f);
			texture_tiling = vec2(1.0f);
			texture_use_reference_length = false;
			texture_reference_length = 1.0f;

			enable_ambient_occlusion = false;
			ao_offset = 0.04f;
			ao_distance = 0.8f;
			ao_strength = 1.0f;

			tex_offset = vec3(0.0f);
			tex_scaling = vec3(1.0f);
			tex_coord_scaling = vec3(1.0f);
			texel_size = 1.0f;

			cone_angle_factor = 1.0f;
			sample_dirs.resize(3);
			sample_dirs[0] = vec3(0.0f, 1.0f, 0.0f);
			sample_dirs[1] = vec3(0.0f, 1.0f, 0.0f);
			sample_dirs[2] = vec3(0.0f, 1.0f, 0.0f);
		}

		rounded_cone_renderer::rounded_cone_renderer()
		{
			has_radii = false;
			shader_defines = shader_define_map();
			albedo_texture = nullptr;
			density_texture = nullptr;
		}

		/// call this before setting attribute arrays to manage attribute array in given manager
		void rounded_cone_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "radius"))
				has_radii = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void rounded_cone_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::disable_attribute_array_manager(ctx, aam);
			has_radii = false;
		}

		bool rounded_cone_renderer::validate_attributes(const context& ctx) const
		{
			const rounded_cone_render_style& rcrs = get_style<rounded_cone_render_style>();
			bool res = surface_renderer::validate_attributes(ctx);
			return res;
		}
		void rounded_cone_renderer::update_defines(shader_define_map& defines)
		{
			const rounded_cone_render_style& rcrs = get_style<rounded_cone_render_style>();
			shader_code::set_define(defines, "ENABLE_TEXTURING", rcrs.enable_texturing, false);
			shader_code::set_define(defines, "ENABLE_TEXTURING", rcrs.texture_blend_mode, rounded_cone_render_style::TBM_MIX);
			shader_code::set_define(defines, "TEXTURE_TILE_FROM_CENTER", rcrs.texture_tile_from_center, false);
			shader_code::set_define(defines, "TEXTURE_USE_REFERENCE_LENGTH", rcrs.texture_use_reference_length, false);
			shader_code::set_define(defines, "ENABLE_AMBIENT_OCCLUSION", rcrs.enable_ambient_occlusion, false);
		}
		bool rounded_cone_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "rounded_cone.glpr", true, defines);
		}
		bool rounded_cone_renderer::set_albedo_texture(texture* tex)
		{
			if(!tex || tex->get_nr_dimensions() != 2)
				return false;
			albedo_texture = tex;
			return true;
		}
		bool rounded_cone_renderer::set_density_texture(texture* tex)
		{
			if(!tex || tex->get_nr_dimensions() != 3)
				return false;
			density_texture = tex;
			return true;
		}
		/// 
		bool rounded_cone_renderer::enable(context& ctx)
		{
			if (!surface_renderer::enable(ctx))
				return false;

			if(!ref_prog().is_linked())
				return false;

			const rounded_cone_render_style& rcrs = get_style<rounded_cone_render_style>();
			if(!has_radii)
				ref_prog().set_attribute(ctx, "radius", rcrs.radius);

			ref_prog().set_uniform(ctx, "radius_scale", rcrs.radius_scale);
			
			if(rcrs.enable_texturing && !albedo_texture)
				return false;

			if(rcrs.enable_ambient_occlusion && !density_texture)
				return false;

			if(rcrs.enable_ambient_occlusion) {
				ref_prog().set_uniform(ctx, "ao_offset", rcrs.ao_offset);
				ref_prog().set_uniform(ctx, "ao_distance", rcrs.ao_distance);
				ref_prog().set_uniform(ctx, "ao_strength", rcrs.ao_strength);
				ref_prog().set_uniform(ctx, "density_tex_offset", rcrs.tex_offset);
				ref_prog().set_uniform(ctx, "density_tex_scaling", rcrs.tex_scaling);
				ref_prog().set_uniform(ctx, "tex_coord_scaling", rcrs.tex_coord_scaling);
				ref_prog().set_uniform(ctx, "texel_size", rcrs.texel_size);
				ref_prog().set_uniform(ctx, "cone_angle_factor", rcrs.cone_angle_factor);
				ref_prog().set_uniform_array(ctx, "sample_dirs", rcrs.sample_dirs);
			}

			if(albedo_texture) {
				ref_prog().set_uniform(ctx, "texture_blend_factor", rcrs.texture_blend_factor);
				ref_prog().set_uniform(ctx, "texture_offset", rcrs.texture_offset);
				ref_prog().set_uniform(ctx, "texture_tiling", rcrs.texture_tiling);
				ref_prog().set_uniform(ctx, "texture_reference_length", rcrs.texture_reference_length);
				albedo_texture->enable(ctx, 0);
			}
			if(density_texture) density_texture->enable(ctx, 1);

			return true;
		}
		///
		bool rounded_cone_renderer::disable(context& ctx)
		{
			if(albedo_texture) albedo_texture->disable(ctx);
			if(density_texture) density_texture->disable(ctx);

			if(!attributes_persist()) {
				has_radii = false;
			}
			return surface_renderer::disable(ctx);
		}

		bool rounded_cone_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<surface_render_style*>(this)) &&
				rh.reflect_member("radius", radius);
		}

		void rounded_cone_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_LINES, start, count, use_strips, use_adjacency, strip_restart_index);
		}

		cgv::reflect::extern_reflection_traits<rounded_cone_render_style, rounded_cone_render_style_reflect> get_reflection_traits(const rounded_cone_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<rounded_cone_render_style, rounded_cone_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct rounded_cone_render_style_gui_creator : public gui_creator {
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*) {
				if(value_type != cgv::type::info::type_name<cgv::render::rounded_cone_render_style>::get_name())
					return false;
				cgv::render::rounded_cone_render_style* rcrs_ptr = reinterpret_cast<cgv::render::rounded_cone_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "default radius", rcrs_ptr->radius, "value_slider", "min=0.001;step=0.0001;max=10.0;log=true;ticks=true");
				p->add_member_control(b, "radius scale", rcrs_ptr->radius_scale, "value_slider", "min=0.01;step=0.0001;max=100.0;log=true;ticks=true");

				if(p->begin_tree_node("texturing", rcrs_ptr->enable_texturing, false)) {
					p->align("\a");
					p->add_member_control(b, "enable", rcrs_ptr->enable_texturing, "check");
					p->add_member_control(b, "blend mode", rcrs_ptr->texture_blend_mode, "dropdown", "enums='mix,tint,multiply,inverse multiply,add'");
					p->add_member_control(b, "blend factor", rcrs_ptr->texture_blend_factor, "value_slider", "min=0.0;step=0.0001;max=1.0;ticks=true");
					//p->add_member_control(b, "texcoord offset", rcrs_ptr->texcoord_offset, "value_slider", "min=-1.0;step=0.0001;max=1.0;ticks=true");
					//p->add_member_control(b, "texcoord scale", rcrs_ptr->texcoord_scale, "value_slider", "min=-10.0;step=0.0001;max=10.0;ticks=true");

					p->add_member_control(b, "tile from center", rcrs_ptr->texture_tile_from_center, "check");

					p->add_member_control(b, "offset", rcrs_ptr->texture_offset[0], "value", "w=95;min=-1;max=1;step=0.001", " ");
					p->add_member_control(b, "", rcrs_ptr->texture_offset[1], "value", "w=95;min=-1;max=1;step=0.001");
					p->add_member_control(b, "", rcrs_ptr->texture_offset[0], "slider", "w=95;min=-1;max=1;step=0.001;ticks=true", " ");
					p->add_member_control(b, "", rcrs_ptr->texture_offset[1], "slider", "w=95;min=-1;max=1;step=0.001;ticks=true");

					p->add_member_control(b, "tiling", rcrs_ptr->texture_tiling[0], "value", "w=95;min=-5;max=5;step=0.001", " ");
					p->add_member_control(b, "", rcrs_ptr->texture_tiling[1], "value", "w=95;min=-5;max=5;step=0.001");
					p->add_member_control(b, "", rcrs_ptr->texture_tiling[0], "slider", "w=95;min=-5;max=5;step=0.001;ticks=true", " ");
					p->add_member_control(b, "", rcrs_ptr->texture_tiling[1], "slider", "w=95;min=-5;max=5;step=0.001;ticks=true");
					
					p->add_member_control(b, "use reference length", rcrs_ptr->texture_use_reference_length, "check");
					p->add_member_control(b, "reference length", rcrs_ptr->texture_reference_length, "value_slider", "min=0.0;step=0.0001;max=5.0;log=true;ticks=true");

					p->align("\b");
					p->end_tree_node(rcrs_ptr->enable_texturing);
				}

				if(p->begin_tree_node("ambient occlusion", rcrs_ptr->enable_ambient_occlusion, false)) {
					p->align("\a");
					p->add_member_control(b, "enable", rcrs_ptr->enable_ambient_occlusion, "check");
					p->add_member_control(b, "offset", rcrs_ptr->ao_offset, "value_slider", "min=0.0;step=0.0001;max=0.2;log=true;ticks=true");
					p->add_member_control(b, "distance", rcrs_ptr->ao_distance, "value_slider", "min=0.0;step=0.0001;max=1.0;log=true;ticks=true");
					p->add_member_control(b, "strength", rcrs_ptr->ao_strength, "value_slider", "min=0.0;step=0.0001;max=10.0;log=true;ticks=true");
					p->align("\b");
				}
				
				p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(rcrs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

		CGV_API cgv::gui::gui_creator_registration<rounded_cone_render_style_gui_creator> rounded_cone_rs_gc_reg("rounded_cone_render_style_gui_creator");
	}
}
