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
			if (!prog.is_created()) {
				prog.create(ctx);
				prog.attach_file(ctx, "pointcloud_clod_filter_points.glcs", cgv::render::ST_COMPUTE, "");
				prog.attach_file(ctx, "pointcloud_clod.glvs", cgv::render::ST_VERTEX, "");
				prog.attach_file(ctx, "pointcloud_clod.glfs", cgv::render::ST_FRAGMENT, "");
				prog.link(ctx);
			}
			return prog.is_linked();
		}

		bool clod_point_renderer::enable(context& ctx)
		{
			const clod_point_render_style& srs = get_style<clod_point_render_style>();

			if (!prog.is_linked())
				return false;
			//TODO set uniforms
			return false;
		}

		bool clod_point_renderer::disable(context& ctx)
		{
			const clod_point_render_style& srs = get_style<clod_point_render_style>();

			if (!attributes_persist()) {
				//TODO reset internal attributes
			}
			return true;
		}


	}
}
