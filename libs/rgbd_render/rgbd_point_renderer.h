#pragma once

#include "rgbd_render.h"
#include <cgv_gl/point_renderer.h>
#include <rgbd_capture/rgbd_device.h>
#include <cgv/gui/provider.h>

#include "lib_begin.h"

namespace rgbd {

    //! Renderer implementation to render rgbd frames as points. 
	/*! The rgbd_point_renderer is derived from the point_renderer and uses the 
	    same point_render_style. Depth and color frames can either be converted
		to integer points and colors with the rgbd::construct_rgbd_render_data_with_color()
		or rgbd::construct_rgbd_render_data() functions and passed to the renderer
		via the position and color arrays. Alternatively, the frames can be converted
		to textures with the rgbd::create_or_update_texture_from_frame() function
		and passed to the renderer by setting the uniform samplers "depth_image" and
		"color_image" after the renderer has been enabled typically with the 
		validate_and_enable() function. In case you configure the renderer to use
		a distortion map, do not use texture unit 2 for depth and color images as this
		is used by renderer for the distortion map. Configure with set_geometry_less_rendering()
		and set_color_lookup() whether to use the depth and or color image. Query the
		current rendering approach with the do_geometry_less_rendering() and do_lookup_color()
		functions and provide render data to position and color attributes.
	*/
	class CGV_API rgbd_point_renderer : public cgv::render::point_renderer
	{
		bool calib_set = false;
		bool distortion_map_outofdate = true;
	protected:
		enum GeometryLessMode {
			GLM_VERTEX,
			GLM_MIXED,
			GLM_INSTANCED
		} geometry_less_mode = GLM_MIXED;
		// members that define shader uniforms
		rgbd::rgbd_calibration calib;
		bool use_mesh_shader = true;
		bool use_distortion_map = false;
		bool geometry_less_rendering = true;
		bool lookup_color = true;
		bool discard_invalid_color_points = false;
		cgv::rgba invalid_color = cgv::rgba(1, 0, 1, 1);
		// cpu and gpu storage of undistortion map
		std::vector<cgv::vec2> distortion_map;
		cgv::render::texture distortion_tex;
		// internal renderer functions
		void update_defines(cgv::render::shader_define_map& defines);
		bool build_shader_program(cgv::render::context& ctx, cgv::render::shader_program& prog, const cgv::render::shader_define_map& defines);
	public:
		rgbd_point_renderer();
		// configuration functions
		void configure_invalid_color_handling(bool discard, const cgv::rgba& color);
		void set_geometry_less_rendering(bool active, GeometryLessMode mode = GLM_VERTEX);
		bool do_geometry_less_rendering() const;
		void set_color_lookup(bool active);
		void set_mesh_shader(bool use);
		bool do_lookup_color() const;
		void set_distortion_map_usage(bool do_use = true);
		void set_calibration(const rgbd::rgbd_calibration& _calib);
		// renderer interface
		bool validate_attributes(const cgv::render::context& ctx) const;
		bool enable(cgv::render::context& ctx);
		bool disable(cgv::render::context& ctx);
		void draw(cgv::render::context& ctx, size_t start, size_t count, bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		void clear(const cgv::render::context& ctx);
		// convenience function to add UI elements
		void create_gui(cgv::base::base* bp, cgv::gui::provider& p);
	};
}

#include <cgv/config/lib_end.h>