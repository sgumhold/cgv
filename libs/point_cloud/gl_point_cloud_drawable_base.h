#pragma once

#include <cgv/render/drawable.h>
#include <cgv/media/illum/phong_material.hh>

#include "point_cloud.h"
#include "neighbor_graph.h"
#include "normal_estimator.h"

#include "lib_begin.h"

/** drawable for a point cloud that manages a neighbor graph and a normal estimator and supports rendering of point cloud and bounding box. */
struct CGV_API gl_point_cloud_drawable_base :
	public cgv::render::drawable, public point_cloud_types
{
public:
	typedef cgv::media::illum::phong_material::color_type color_type;
protected:
	point_cloud pc;
	neighbor_graph ng;
	normal_estimator ne;

	std::string data_path;
	std::string file_name;

	float point_size, line_width, nml_length;
	bool show_points, show_nmls, show_clrs, show_box;
	bool smooth_points;
	bool illum_points, show_neighbor_graph;

	unsigned k;
	bool do_symmetrize;
	
	bool reorient_normals;

	cgv::media::illum::phong_material base_material;
	color_type base_color;
	color_type nml_color;
	color_type box_color;

public:
	void clear();

	void build_neighbor_graph();

	void compute_normals();
	void recompute_normals();
	void orient_normals();
	void orient_normals_to_view_point();

	gl_point_cloud_drawable_base();
	std::string get_type_name() const;
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	void on_set(void* member_ptr);

	bool read(const std::string& file_name);
	bool append(const std::string& file_name);
	bool write(const std::string& file_name);
	
	//void draw_vertex_color(unsigned int vi) const;
	void draw_edge_color(unsigned int vi, unsigned int j, bool is_symm, bool is_start) const;
	//void draw_debug_vertex(unsigned int vi);

	void draw_box(cgv::render::context& ctx, const Box& box);
	void draw_box(cgv::render::context& ctx);
	void draw_points(cgv::render::context& ctx);
	void draw_normals(cgv::render::context& ctx);
	void draw_graph(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
};

#include <cgv/config/lib_end.h>