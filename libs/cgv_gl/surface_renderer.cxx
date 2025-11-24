#include "surface_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/gl/gl_context.h>

namespace cgv {
	namespace render {

		/// overload to update the shader program compile options based on the current render style; only called if internal shader program is used
		void surface_renderer::update_shader_program_options(shader_compile_options& options) const
		{
			group_renderer::update_shader_program_options(options);
			const auto& srs = get_style<surface_render_style>();
			options.define_macro("MAX_NR_LIGHTS", srs.max_nr_lights);
		}
		/// overload to disable context from setting color in shader program
		bool surface_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_compile_options& options) const
		{
			bool res = group_renderer::build_shader_program(ctx, prog, options);
			prog.allow_context_to_set_color(false);
			return res;
		}

		/// call this before setting attribute arrays to manage attribute array in given manager
		void surface_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "normal"))
				has_normals = true;
			if (has_attribute(ctx, "texcoord"))
				has_texcoords = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void surface_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::disable_attribute_array_manager(ctx, aam);
			has_normals = false;
			has_texcoords = false;
		}
		void surface_renderer::remove_normal_array(const context& ctx) {
			has_normals = false;
			remove_attribute_array(ctx, "normal");
		}
		void surface_renderer::remove_texcoord_array(const context& ctx) {
			has_texcoords = false;
			remove_attribute_array(ctx, "texcoord");
		}
		/// method to set the normal attribute from a vertex buffer object, the element type must be given as explicit template parameter
		void surface_renderer::set_normal_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_normals = true;
			set_attribute_array(ctx, "normal", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);

		}
		/// template method to set the texcoord attribute from a vertex buffer object, the element type must be given as explicit template parameter
		void surface_renderer::set_texcoord_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_texcoords = true;
			set_attribute_array(ctx, "texcoord", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);

		}
		bool surface_renderer::enable(context& ctx)
		{
			bool res = group_renderer::enable(ctx);
			const surface_render_style& srs = get_style<surface_render_style>();

			if (cull_per_primitive) {
				ctx.push_cull_state();
				ctx.set_cull_state(srs.culling_mode);
			}
			if (ref_prog().is_linked()) {
				if (!has_colors) {
					cgv::rgba col;
					(cgv::rgb&)col = srs.surface_color;
					col.opacity() = srs.surface_opacity;
					ref_prog().set_attribute(ctx, "color", col);
				}
				ctx.set_material(srs.material);
				ref_prog().set_uniform(ctx, "map_color_to_material", int(srs.map_color_to_material));
				ref_prog().set_uniform(ctx, "culling_mode", int(srs.culling_mode));
				ref_prog().set_uniform(ctx, "illumination_mode", int(srs.illumination_mode));
			}
			else
				res = false;
			return res;
		}

		bool surface_renderer::disable(context& ctx)
		{
			const surface_render_style& srs = get_style<surface_render_style>();
			if (cull_per_primitive) {
				ctx.pop_cull_state();
			}
			if (!attributes_persist()) {
				has_normals = false;
				has_texcoords = false;
			}
			return group_renderer::disable(ctx);
		}
		bool surface_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base<group_render_style>(*this) &&
				rh.reflect_member("culling_mode", culling_mode) &&
				rh.reflect_member("illumination_mode", illumination_mode) &&
				rh.reflect_member("map_color_to_material", map_color_to_material) &&
				rh.reflect_member("surface_color", surface_color) &&
				rh.reflect_member("max_nr_lights", max_nr_lights) &&
				rh.reflect_member("material", material);
		}

		cgv::reflect::extern_reflection_traits<surface_render_style, surface_render_style_reflect> get_reflection_traits(const surface_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<surface_render_style, surface_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct surface_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::surface_render_style>::get_name())
					return false;
				cgv::render::surface_render_style* srs_ptr = reinterpret_cast<cgv::render::surface_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
				if (p->begin_tree_node("Color Mapping", srs_ptr->map_color_to_material, false, "level=3")) {
					p->align("\a");
					p->add_gui("Map Color to Material", srs_ptr->map_color_to_material, "bit_field_control",
						"enums='Color Front=1,Color Back=2,Opacity Front=4,Opacity Back=8'");
					p->align("\b");
					p->end_tree_node(srs_ptr->map_color_to_material);
				}
				p->add_member_control(b, "Illumination Mode", srs_ptr->illumination_mode, "dropdown", "enums='Off,One-Sided,Two-Sided'");
				p->add_member_control(b, "Max Nr Light", srs_ptr->max_nr_lights, "value_slider", "min=1;max=8;ticks=true");
				p->add_member_control(b, "Culling Mode", srs_ptr->culling_mode, "dropdown", "enums='Off,Backface,Frontface'");
				if (p->begin_tree_node("Color and Materials", srs_ptr->surface_color, false, "level=3")) {
					p->align("\a");
					p->add_member_control(b, "Surface Color", srs_ptr->surface_color);
					p->add_member_control(b, "Surface Opacity", srs_ptr->surface_opacity, "value_slider", "min=0.0;step=0.01;max=1.0;log=false;ticks=true");
					if (p->begin_tree_node("Material", srs_ptr->material, false, "level=3")) {
						p->align("\a");
						p->add_gui("front_material", srs_ptr->material);
						p->align("\b");
						p->end_tree_node(srs_ptr->material);
					}
					p->align("\b");
					p->end_tree_node(srs_ptr->surface_color);
				}
				if (p->begin_tree_node("Use of Group Information", srs_ptr->illumination_mode, false, "level=3")) {
					p->align("\a");
					p->add_gui("group render style", *static_cast<cgv::render::group_render_style*>(srs_ptr));
					p->align("\b");
					p->end_tree_node(srs_ptr->illumination_mode);
				}
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<surface_render_style_gui_creator> surface_rs_gc_reg("surface_render_style_gui_creator");

	}
}