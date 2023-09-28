#pragma once

#include "rgbd_render.h"
#include <cgv_gl/point_renderer.h>
#include <rgbd_capture/rgbd_device.h>
#include <cgv/gui/provider.h>

#include "lib_begin.h"

namespace rgbd {

/** interface to provided access to rgbd devices. This is independent of device driver. 
    Different plugins can implement the rgbd_driver and rgbd_device classes and seemlessly
	integrate into the rgbd_input class. */
	/// rgbd point renderer can render point cloud from depth and or color images
	class CGV_API rgbd_point_renderer : public cgv::render::point_renderer
	{
		bool calib_set = false;
		bool undistortion_map_outofdate = true;
	protected:
		// members that define shader uniforms
		rgbd::rgbd_calibration calib;
		bool use_undistortion_map = false;
		bool geometry_less_rendering = true;
		bool lookup_color = true;
		bool discard_invalid_color_points = false;
		rgba invalid_color = rgba(1, 0, 1, 1);
		// cpu and gpu storage of undistortion map
		std::vector<vec2> undistortion_map;
		cgv::render::texture undistortion_tex;
		// internal renderer functions
		void update_defines(cgv::render::shader_define_map& defines);
		bool build_shader_program(cgv::render::context& ctx, cgv::render::shader_program& prog, const cgv::render::shader_define_map& defines);
	public:
		rgbd_point_renderer();
		// configuration functions
		void configure_invalid_color_handling(bool discard, const rgba& color);
		void set_geomtry_less_rendering(bool active);
		bool do_geometry_less_rendering() const;
		void set_color_lookup(bool active);
		bool do_lookup_color() const;
		void set_undistortion_map_usage(bool do_use = true);
		void set_calibration(const rgbd::rgbd_calibration& _calib);
		// renderer interface
		bool validate_attributes(const cgv::render::context& ctx) const;
		bool enable(cgv::render::context& ctx);
		bool disable(cgv::render::context& ctx);
		void draw(cgv::render::context& ctx, size_t start, size_t count, bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		// convenience function to add UI elements
		void create_gui(cgv::base::base* bp, cgv::gui::provider& p);
	};
}

#include <cgv/config/lib_end.h>