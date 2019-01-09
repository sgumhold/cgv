#include "renderer.h"

namespace cgv {
	namespace render {

		render_style::~render_style()
		{
		}

		/// default initialization
		attribute_array_manager::attribute_array_manager()
		{
		}
		/// destructor calls destruct
		attribute_array_manager::~attribute_array_manager()
		{
			if (aab.is_created() && aab.ctx_ptr && aab.ctx_ptr->make_current())
				destruct(*aab.ctx_ptr);
		}
		bool attribute_array_manager::set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes)
		{
			return ctx.set_attribute_array_void(&aab, loc, element_type, &vbo, reinterpret_cast<const void*>(offset_in_bytes), nr_elements, stride_in_bytes);
		}
		///
		bool attribute_array_manager::init(context& ctx)
		{
			return aab.create(ctx);
		}
		///
		bool attribute_array_manager::enable(context& ctx)
		{
			return aab.enable(ctx);
		}
		///
		bool attribute_array_manager::disable(context& ctx)
		{
			return aab.disable(ctx);
		}
		///
		void attribute_array_manager::destruct(const context& ctx)
		{
			for (auto& p : vbos) {
				p.second->destruct(ctx);
				delete p.second;
				p.second = 0;
			}
			vbos.clear();
			aab.destruct(ctx);
		}

		renderer::renderer()
		{
			has_colors = false;
			has_positions = false;
			rs = default_render_style = 0;
			aam_ptr = 0;
		}
		/// destructor deletes default renderer style
		renderer::~renderer()
		{
			if (default_render_style)
				delete default_render_style;
			default_render_style = 0;
		}
		/// provide an attribute manager that is used in successive calls to attribute array setting methods and in the enable and disable method, if a nullptr is provided attributes are managed through deprecated VertexAttributePointers - in this case a call to disable deattaches all attribute arrays which have to be set before the next enable call again
		void renderer::set_attribute_array_manager(attribute_array_manager* _aam_ptr)
		{
			aam_ptr = _aam_ptr;
		}

		bool renderer::set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes)
		{
			if (aam_ptr)
				return aam_ptr->set_attribute_array(ctx, loc, element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
			enabled_attribute_arrays.insert(loc);
			return attribute_array_binding::set_global_attribute_array(ctx, loc, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}
		void renderer::set_position_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes)
		{
			has_positions = true;
			set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "position"), element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}
		void renderer::set_color_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, size_t stride_in_bytes)
		{
			has_colors = true;
			set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "color"), element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}

		bool renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			if (!has_positions) {
				ctx.error("box_renderer::enable() position attribute not set");
				return false;
			}
			return true;
		}
		/// reference given render style
		void renderer::set_render_style(const render_style& _rs)
		{
			rs = &_rs;
		}
		bool renderer::init(context& ctx)
		{
			if (!default_render_style) {
				default_render_style = create_render_style();
			}
			if (!rs)
				rs = default_render_style;
			return default_render_style != 0;
		}

		/// validate attributes and if successful, enable renderer
		bool renderer::validate_and_enable(context& ctx)
		{
			if (validate_attributes(ctx))
				return enable(ctx);
			return false;
		}

		bool renderer::enable(context& ctx)
		{
			bool res = ref_prog().enable(ctx);
			if (aam_ptr)
				res = aam_ptr->enable(ctx);
			return res;				
		}

		bool renderer::disable(context& ctx)
		{
			bool res = true;
			if (aam_ptr)
				res = aam_ptr->disable(ctx);
			else {
				for (int loc : enabled_attribute_arrays)
					res = attribute_array_binding::disable_global_array(ctx, loc) && res;
				enabled_attribute_arrays.clear();
				has_colors = false;
				has_positions = false;
			}
			return ref_prog().disable(ctx) && res;
		}

		void renderer::clear(const cgv::render::context& ctx)
		{
			prog.destruct(ctx);
		}
	}
}

