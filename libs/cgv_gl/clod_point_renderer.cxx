#include "clod_point_renderer.h"

namespace cgv {
	namespace render {

		clod_point_render_style::clod_point_render_style()
		{
			float clod_factor;
		}

		void clod_point_renderer::draw_and_compute_impl(context& ctx, PrimitiveType type, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			renderer::draw_impl(ctx, type, start, count, use_strips, use_adjacency, strip_restart_index);
			//TODO run compute shader
			reduce_prog.enable(ctx);
			glDispatchCompute( positions.size(), 1, 1);
			reduce_prog.disable(ctx);
			;
		}

		render_style* clod_point_renderer::create_render_style() const
		{
			return new clod_point_render_style();
		}

		bool clod_point_renderer::init(context& ctx)
		{
			if (!reduce_prog.is_created()) {
				reduce_prog.create(ctx);
				add_shader(ctx, reduce_prog, "pointcloud_clod_filter_points.glcs", cgv::render::ST_COMPUTE);
				reduce_prog.link(ctx);
#ifndef NDEBUG
				std::cerr << reduce_prog.last_error;
#endif // #ifdef NDEBUG
			}

			//create shader program
			if (!ref_prog().is_created()) {
				ref_prog().create(ctx);
				add_shader(ctx, ref_prog(), "view.glsl", cgv::render::ST_VERTEX);
				add_shader(ctx, ref_prog(), "pointcloud_clod.glvs", cgv::render::ST_VERTEX);
				add_shader(ctx, ref_prog(), "fragment.glfs", cgv::render::ST_FRAGMENT);
				add_shader(ctx, ref_prog(), "pointcloud_clod.glfs", cgv::render::ST_FRAGMENT);
				ref_prog().link(ctx);
#ifndef NDEBUG
				std::cerr << ref_prog().last_error;
#endif // #ifdef NDEBUG
			}
			return ref_prog().is_linked() && reduce_prog.is_linked();
		}

		bool clod_point_renderer::enable(context& ctx)
		{
			if (!renderer::enable(ctx)) {
				return false;
			}

			if (!ref_prog().is_linked()) {
				return false;
			}

			const clod_point_render_style& srs = get_style<clod_point_render_style>();
			//TODO set uniforms
			vec2 screenSize(ctx.get_width(), ctx.get_height());
			vec4 pivot = ctx.get_modelview_matrix().col(3);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "CLOD"), CLOD);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "scale"), scale);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "spacing"), spacing);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "pivot"), pivot);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "screenSize"), screenSize);
			ref_prog().set_uniform(ctx, ref_prog().get_uniform_location(ctx, "transform"), ctx.get_modelview_projection_device_matrix());
				
			reduce_prog.set_uniform(ctx, ref_prog().get_uniform_location(ctx, "transform"), ctx.get_modelview_projection_device_matrix());
			reduce_prog.set_uniform(ctx, ref_prog().get_uniform_location(ctx, "CLOD"), CLOD);
			reduce_prog.set_uniform(ctx, ref_prog().get_uniform_location(ctx, "scale"), scale);
			reduce_prog.set_uniform(ctx, ref_prog().get_uniform_location(ctx, "spacing"), spacing);
			reduce_prog.set_uniform(ctx, ref_prog().get_uniform_location(ctx, "pivot"), pivot);
			reduce_prog.set_uniform(ctx, ref_prog().get_uniform_location(ctx, "screenSize"), screenSize);
			return true;
		}

		bool clod_point_renderer::disable(context& ctx)
		{
			const clod_point_render_style& srs = get_style<clod_point_render_style>();

			if (!attributes_persist()) {
				//TODO reset internal attributes
			}
			return true;
		}

		void clod_point_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_and_compute_impl(ctx, cgv::render::PT_POINTS, start, count, use_strips, use_adjacency, strip_restart_index);
		}

		void clod_point_renderer::add_shader(context& ctx, shader_program& prog, const std::string& sf,const cgv::render::ShaderType st)
		{
#ifndef NDEBUG
			std::cout << "add shader " << sf << '\n';
#endif // #ifdef NDEBUG
			prog.attach_file(ctx, sf, st);
#ifndef NDEBUG
			if (prog.last_error.size() > 0) {
				std::cerr << prog.last_error << '\n';
				prog.last_error = "";
			}	
#endif // #ifdef NDEBUG

		}


	}
}
