#include "clod_point_renderer.h"

namespace cgv {
	namespace render {

		clod_point_render_style::clod_point_render_style()
		{
			float clod_factor;
		}

		render_style* clod_point_renderer::create_render_style() const
		{
			return new clod_point_render_style();
		}

		bool clod_point_renderer::init(context& ctx)
		{
			//create shader program
			if (!ref_prog().is_created()) {
				ref_prog().create(ctx);
				add_shader(ctx, "view.glsl", cgv::render::ST_VERTEX);
				add_shader(ctx, "pointcloud_clod.glvs", cgv::render::ST_VERTEX);
				add_shader(ctx, "fragment.glfs", cgv::render::ST_FRAGMENT);
				add_shader(ctx, "pointcloud_clod.glfs", cgv::render::ST_FRAGMENT);
				add_shader(ctx, "pointcloud_clod_filter_points.glcs", cgv::render::ST_COMPUTE);
				ref_prog().link(ctx);
#ifndef NDEBUG
				std::cerr << ref_prog().last_error;
#endif // #ifdef NDEBUG
			}
			return ref_prog().is_linked();
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

		void clod_point_renderer::add_shader(context& ctx, const std::string& sf,const cgv::render::ShaderType st)
		{
#ifndef NDEBUG
			std::cout << "add shader " << sf << '\n';
#endif // #ifdef NDEBUG
			ref_prog().attach_file(ctx, sf, st);
#ifndef NDEBUG
			if (ref_prog().last_error.size() > 0) {
				std::cerr << ref_prog().last_error << '\n';
				ref_prog().last_error = "";
			}	
#endif // #ifdef NDEBUG

		}


	}
}
