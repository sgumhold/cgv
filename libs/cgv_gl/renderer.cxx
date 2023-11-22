#include "renderer.h"
#include "gl/gl_tools.h"

namespace cgv {
	namespace render {
		render_style::~render_style()
		{
		}
		renderer::renderer()
		{
			// initialize to the threshold count to prevent outputting warnings for the very first render procedure
			current_prog_render_count = 10;
			has_colors = false;
			has_positions = false;
			rs = default_render_style = 0;
			aam_ptr = 0;
			indices = 0;
			index_buffer_ptr = 0;
			index_type = cgv::type::info::TI_UNDEF;
			index_count = 0;
			prog_ptr = &prog;
		}
		void renderer::manage_singleton(context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change)
		{
			switch (ref_count_change) {
			case 1:
				if (ref_count == 0) {
					if (!init(ctx))
						ctx.error(std::string("unable to initialize ") + renderer_name + " singleton");
				}
				++ref_count;
				break;
			case 0:
				break;
			case -1:
				if (ref_count == 0)
					ctx.error(std::string("attempt to decrease reference count of ") + renderer_name + " singleton below 0");
				else {
					if (--ref_count == 0)
						clear(ctx);
				}
				break;
			default:
				ctx.error(std::string("invalid change reference count outside {-1,0,1} for ") + renderer_name + " singleton");
			}
		}
		renderer::~renderer()
		{
			if (default_render_style)
				delete default_render_style;
			default_render_style = 0;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			aam_ptr = &aam;
			if (has_attribute(ctx, "position"))
				has_positions = true;
			if (has_attribute(ctx, "color"))
				has_colors = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			has_positions = false;
			has_colors = false;
			if (ctx.core_profile)
				aam_ptr = &default_aam;
			else
				aam_ptr = 0;
		}
		void renderer::set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr)
		{
			if (_aam_ptr)
				enable_attribute_array_manager(ctx, *_aam_ptr);
			else
				disable_attribute_array_manager(ctx, *aam_ptr);
		}
		bool renderer::set_attribute_array(const context& ctx, const std::string& name, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			int loc = get_prog_attribute_location(ctx, name);
			if(loc < 0)
				return false;
			if (aam_ptr)
				return aam_ptr->set_attribute_array(ctx, loc, element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
			enabled_attribute_arrays.insert(loc);
			return attribute_array_binding::set_global_attribute_array(ctx, loc, element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}
		void renderer::set_position_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_positions = true;
			set_attribute_array(ctx, "position", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}
		void renderer::set_color_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_colors = true;
			set_attribute_array(ctx, "color", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}
		void renderer::remove_indices(const context& ctx)
		{
			if (!has_indices())
				return;
			if (aam_ptr)
				aam_ptr->remove_indices(ctx);
			index_buffer_ptr = 0;
			indices = 0;
			index_count = 0;
			index_type = cgv::type::info::TI_UNDEF;
		}
		bool renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			if (!has_positions) {
				ctx.error("renderer::enable() position attribute not set");
				return false;
			}
			return true;
		}
		/// reference given render style
		void renderer::set_render_style(const render_style& _rs)
		{
			rs = &_rs;
		}

		/// build shader program for specific render style
		bool renderer::build_program(context& ctx, shader_program& prog, const render_style& _rs)
		{
			shader_define_map defines;
			auto* tmp = rs;
			rs = &_rs;
			update_defines(defines);
			bool res = build_shader_program(ctx, prog, defines);
			rs = tmp;
			return res;
		}

		/// access to render style
		const render_style* renderer::get_style_ptr() const
		{
			if (rs)
				return rs;
			if (default_render_style)
				return default_render_style;
			default_render_style = create_render_style();
			return default_render_style;
		}
		/// set external shader program for successive draw call only
		void renderer::set_prog(shader_program& one_shot_prog)
		{
			prog_ptr = &one_shot_prog;
		}

		bool renderer::init(context& ctx)
		{
			if (ctx.core_profile) {
				default_aam.init(ctx);
				if (!aam_ptr)
					aam_ptr = &default_aam;
			}
			if (!default_render_style)
				default_render_style = create_render_style();

			if (!rs)
				rs = default_render_style;

			if (prog_ptr == &prog) {
				update_defines(defines);
				if (!build_shader_program(ctx, prog, defines))
					return false;
				last_defines = defines;
			}
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
			if(prog_ptr == &prog) {
				update_defines(defines);
				if(defines != last_defines) {
					if(prog.is_created())
						prog.destruct(ctx);
					if(!build_shader_program(ctx, prog, defines))
						return false;
#ifndef _DEBUG
				}
#else
					if(current_prog_render_count < 10)
						std::cerr << "Performance warning: shader is rebuild! Consider using multiple instances of the renderer." << std::endl;
					current_prog_render_count = 0;
				}
				++current_prog_render_count;
#endif
				last_defines = defines;
			}
			bool res = ref_prog().enable(ctx);
			if (aam_ptr)
				res = aam_ptr->enable(ctx);
			return res;				
		}

		bool renderer::disable(context& ctx)
		{
			bool res = true;
			if (has_aam())
				res = aam_ptr->disable(ctx);
			else {
				if (ctx.core_profile) {
					default_aam.disable(ctx);
					for (int loc : enabled_attribute_arrays)
						res = default_aam.aab.disable_array(ctx, loc) && res;
				}
				else {
					for (int loc : enabled_attribute_arrays)
						res = attribute_array_binding::disable_global_array(ctx, loc) && res;
				}
				enabled_attribute_arrays.clear();
				has_colors = false;
				has_positions = false;
				index_count = 0;
			}
			res = ref_prog().disable(ctx) && res;
			prog_ptr = &prog;
			return res;
		}
		void renderer::draw_impl(context& ctx, PrimitiveType type, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			if (use_strips && has_indices()) {
				glPrimitiveRestartIndex(strip_restart_index);
				glEnable(GL_PRIMITIVE_RESTART);
			}
			if (type == PT_LINES) {
				if (use_adjacency)
					if (use_strips)
						type = PT_LINE_STRIP_ADJACENCY;
					else
						type = PT_LINES_ADJACENCY;
				else
					if (use_strips)
						type = PT_LINE_STRIP;
			}
			else if (type == PT_TRIANGLES)
				if (use_adjacency)
					if (use_strips)
						type = PT_TRIANGLE_STRIP_ADJACENCY;
					else
						type = PT_TRIANGLES_ADJACENCY;
				else
					if (use_strips)
						type = PT_TRIANGLE_STRIP;

			GLenum pt = gl::map_to_gl(type);
			if (has_indices()) {
				bool aam_has_index_buffer = aam_ptr && aam_ptr->has_index_buffer();
				bool use_index_buffer = index_buffer_ptr && !aam_has_index_buffer;
				bool use_indices = !index_buffer_ptr && !aam_has_index_buffer;
				if (use_index_buffer)
					index_buffer_ptr->bind(ctx, VBT_INDICES);
				const void* offset = reinterpret_cast<const uint8_t*>(use_indices ? indices : 0)
												+ start * cgv::type::info::get_type_size(index_type);
				glDrawElements(pt, (GLsizei)count, gl::map_to_gl(index_type), offset);
				if (use_index_buffer)
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			else
				glDrawArrays(pt, (GLint)start, (GLsizei)count);
		}
		void renderer::draw_impl_instanced(context& ctx, PrimitiveType type, size_t start, size_t count, size_t instance_count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			if (use_strips && has_indices()) {
				glPrimitiveRestartIndex(strip_restart_index);
				glEnable(GL_PRIMITIVE_RESTART);
			}
			if (type == PT_LINES) {
				if (use_adjacency)
					if (use_strips)
						type = PT_LINE_STRIP_ADJACENCY;
					else
						type = PT_LINES_ADJACENCY;
				else
					if (use_strips)
						type = PT_LINE_STRIP;
			}
			else if (type == PT_TRIANGLES)
				if (use_adjacency)
					if (use_strips)
						type = PT_TRIANGLE_STRIP_ADJACENCY;
					else
						type = PT_TRIANGLES_ADJACENCY;
				else
					if (use_strips)
						type = PT_TRIANGLE_STRIP;
			
			GLenum pt = gl::map_to_gl(type);
			if (has_indices()) {
				bool aam_has_index_buffer = aam_ptr && aam_ptr->has_index_buffer();
				bool use_index_buffer = index_buffer_ptr && !aam_has_index_buffer;
				bool use_indices = !index_buffer_ptr && !aam_has_index_buffer;
				if (use_index_buffer)
					index_buffer_ptr->bind(ctx, VBT_INDICES);
				const void* offset = reinterpret_cast<const uint8_t*>(use_indices ? indices : 0)
					+ start * cgv::type::info::get_type_size(index_type);
				glDrawElementsInstanced(pt, (GLsizei)count, gl::map_to_gl(index_type), offset, (GLsizei)instance_count);
				if (use_index_buffer)
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			else
				glDrawArraysInstanced(pt, (GLint)start, (GLsizei)count, (GLsizei)instance_count);
		}
		void renderer::draw(context& ctx, size_t start, size_t count,
			bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_TRIANGLES, start, count, use_strips, use_adjacency, strip_restart_index);
		}
		bool renderer::render(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			if (!validate_and_enable(ctx))
				return false;
			draw(ctx, start, count, use_strips, use_adjacency, strip_restart_index);
			return disable(ctx);
		}
		void renderer::clear(const cgv::render::context& ctx)
		{
			prog.destruct(ctx);
		}
	}
}

