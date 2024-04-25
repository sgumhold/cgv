#pragma once

#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/view.h>

#include "point_cloud.h"

#include <cgv_gl/surfel_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/box_wire_renderer.h>

#include "lib_begin.h"


/** drawable for a point cloud that manages a neighbor graph and a normal estimator and supports rendering of point cloud and bounding box. */
class CGV_API gl_point_cloud_drawable : public cgv::render::drawable, public point_cloud_types
{
protected:
	using rgba = cgv::rgba;

	point_cloud pc;

	cgv::render::surfel_render_style surfel_style;
	//cgv::render::arrow_render_style normal_style;
	cgv::render::arrow_render_style normal_style;
	cgv::render::surface_render_style box_style;
	cgv::render::line_render_style box_wire_style;

	cgv::render::surfel_renderer s_renderer;
	cgv::render::arrow_renderer a_renderer;
	cgv::render::box_renderer b_renderer;
	cgv::render::box_wire_renderer bw_renderer;

	bool show_points;
	bool show_box;
	bool show_boxes;
	bool show_nmls;
	bool sort_points;
	bool use_component_colors;
	bool use_component_transformations;
	rgba box_color;
	
	std::vector<Clr>* use_these_point_colors;
	std::vector<cgv::type::uint8_type>* use_these_point_color_indices;
	std::vector<RGBA>* use_these_point_palette;
	std::vector<RGBA>* use_these_component_colors;

	// reduction to subset 
	unsigned show_point_step;
	std::size_t show_point_begin, show_point_end;
	unsigned nr_draw_calls;
	cgv::render::view* view_ptr;
	mutable std::string last_error;
	bool ensure_view_pointer();
	void set_arrays(cgv::render::context& ctx, size_t offset = 0, size_t count = -1);
	bool ensure_file_name(std::string& _file_name, const std::string* data_path_ptr = 0) const;
public:
	gl_point_cloud_drawable();

	bool read(std::string& file_name, const std::string* data_path_ptr = 0);
	bool append(std::string& file_name, bool add_component = true, const std::string* data_path_ptr = 0);
	bool write(const std::string& file_name);
	
	void render_boxes(cgv::render::context& ctx, cgv::render::group_renderer& R, cgv::render::group_render_style& RS);
	void draw_box(cgv::render::context& ctx, const Box& box, const rgba& clr);
	void draw_boxes(cgv::render::context& ctx);
	void draw_points(cgv::render::context& ctx);
	void draw_normals(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void clear(cgv::render::context& ctx);
};

#include <cgv/config/lib_end.h>