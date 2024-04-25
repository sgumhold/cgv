#include "rgbd_point_renderer.h"

namespace rgbd {

void rgbd_point_renderer::update_defines(cgv::render::shader_define_map& defines) 
{
	point_renderer::update_defines(defines);
	cgv::render::shader_code::set_define(defines, "USE_DISTORTION_MAP", use_distortion_map, false);
	cgv::render::shader_code::set_define(defines, "GEOMETRY_LESS_MODE", (int&)geometry_less_mode, 0);
	cgv::render::shader_code::set_define(defines, "USE_MESH_SHADER", use_mesh_shader, true);
}
bool rgbd_point_renderer::build_shader_program(cgv::render::context& ctx, cgv::render::shader_program& prog, const cgv::render::shader_define_map& defines)
{
	if (use_mesh_shader)
		return prog.build_program(ctx, "rgbd_mesh.glpr", true, defines);
	else
		return prog.build_program(ctx, "rgbd_pc.glpr", true, defines);
}
rgbd_point_renderer::rgbd_point_renderer() : distortion_tex("flt32[R,G]") 
{
	distortion_tex.set_mag_filter(cgv::render::TF_NEAREST);
}
void rgbd_point_renderer::configure_invalid_color_handling(bool discard, const cgv::rgba& color)
{
	discard_invalid_color_points = discard;
	invalid_color = color;
}
void rgbd_point_renderer::set_geometry_less_rendering(bool active, GeometryLessMode mode)
{
	geometry_less_rendering = active; 
	geometry_less_mode = mode;
}
bool rgbd_point_renderer::do_geometry_less_rendering() const 
{
	return geometry_less_rendering; 
}
void rgbd_point_renderer::set_color_lookup(bool active) 
{
	lookup_color = active; 
}
void rgbd_point_renderer::set_mesh_shader(bool use)
{
	use_mesh_shader = use;
}
bool rgbd_point_renderer::do_lookup_color() const 
{
	return lookup_color; 
}
void rgbd_point_renderer::set_distortion_map_usage(bool do_use)
{
	use_distortion_map = do_use;
	if (do_use && calib_set)
		calib.depth.compute_distortion_map(distortion_map);
}
void rgbd_point_renderer::set_calibration(const rgbd::rgbd_calibration& _calib)
{
	calib = _calib;
	calib_set = true;
	if (use_distortion_map) {
		calib.depth.compute_distortion_map(distortion_map);
		distortion_map_outofdate = true;
	}
}
bool rgbd_point_renderer::validate_attributes(const cgv::render::context& ctx) const
{
	if (geometry_less_rendering)
		return true;
	else
		return cgv::render::point_renderer::validate_attributes(ctx);
}
bool rgbd_point_renderer::enable(cgv::render::context& ctx)
{
	if (use_mesh_shader) {
		if (!group_renderer::enable(ctx))
			return false;
	}
	else {
		if (!point_renderer::enable(ctx))
			return false;
		ref_prog().set_uniform(ctx, "invalid_color", invalid_color);
		ref_prog().set_uniform(ctx, "discard_invalid_color_points", discard_invalid_color_points);
		ref_prog().set_uniform(ctx, "do_lookup_color", lookup_color);
	}
	ref_prog().set_uniform(ctx, "geometry_less_rendering", geometry_less_rendering);
	set_rgbd_calibration_uniforms(ctx, ref_prog(), calib);
	if (use_distortion_map) {
		if (distortion_map_outofdate) {
			if (distortion_tex.is_created())
				distortion_tex.destruct(ctx);
			cgv::data::data_format df(calib.depth.w, calib.depth.h, cgv::type::info::TI_FLT32, cgv::data::CF_RG);
			cgv::data::data_view dv(&df, distortion_map.data());
			distortion_tex.create(ctx, dv, 0);
			distortion_map_outofdate = false;
		}
		distortion_tex.enable(ctx, 2);
		ref_prog().set_uniform(ctx, "distortion_map", 2);
	}
	return true;
}
bool rgbd_point_renderer::disable(cgv::render::context& ctx)
{
	if (use_distortion_map)
		distortion_tex.disable(ctx);
	return point_renderer::disable(ctx);
}
void rgbd_point_renderer::draw(cgv::render::context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
{
	if (geometry_less_rendering) {
		switch (geometry_less_mode) {
		case GLM_VERTEX :
			draw_impl(ctx, cgv::render::PT_POINTS, 0, calib.depth.w*calib.depth.h);
			break;
		case GLM_MIXED :
			draw_impl_instanced(ctx, cgv::render::PT_POINTS, 0, calib.depth.w, calib.depth.h);
			break;
		case GLM_INSTANCED :
			draw_impl_instanced(ctx, cgv::render::PT_POINTS, 0, 1, calib.depth.w * calib.depth.h);
			break;
		}
	}
	else
		draw_impl(ctx, cgv::render::PT_POINTS, start, count);
}

void rgbd_point_renderer::clear(const cgv::render::context& ctx)
{
	if (distortion_tex.is_created())
		distortion_tex.destruct(ctx);
}

// convenience function to add UI elements
void rgbd_point_renderer::create_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	p.add_member_control(bp, "use_mesh_shader", use_mesh_shader, "check");
	p.add_member_control(bp, "lookup_color", lookup_color, "check");
	p.add_member_control(bp, "discard_invalid_color_points", discard_invalid_color_points, "check");
	p.add_member_control(bp, "invalid_color", invalid_color);
	p.add_member_control(bp, "geometry_less_rendering", geometry_less_rendering, "check");
	p.add_member_control(bp, "geometry_less_mode", geometry_less_mode, "dropdown", "enums='vertex,mixed,instance'");
}

}