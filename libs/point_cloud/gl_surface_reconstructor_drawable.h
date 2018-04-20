#pragma once

#include <cgv/render/drawable.h>
#include <cgv/media/illum/phong_material.hh>

#include "surface_reconstructor.h"

#include "lib_begin.h"


enum DebugMode {
	DBG_NONE, 
	DBG_SR_SYMMETRIZE,
	DGB_SR_MAKE_CONSISTENT
};

enum VertexDebugMode { 
	VDBG_NONE,
	VDBG_ONE_RING, 
	VDBG_LENGTH,
	VDBG_CYCLE
};
enum AccelerationMode {
	GRID,
	KDTREE
};
enum VertexColorMode {
	VCM_NONE,
	VCM_NR_EDGES,
	VCM_NR_TRIANGLES,
	VCM_INFO,
	VCM_TYPE,
};
enum EdgeColorMode {
	ECM_NONE,
	ECM_SYMMETRY,
	ECM_NR_TRIANGLES,
	ECM_INFO
};


class CGV_API gl_surface_reconstructor_drawable :
	public cgv::render::drawable, 
	public point_cloud_types
{
public:
	typedef cgv::media::illum::phong_material::color_type color_type;
protected:
	surface_reconstructor SR;
	std::vector<unsigned int> triangles[3];
	std::vector<unsigned int> edges;
	std::vector<unsigned int> hole;
	bool show_border_edges_only;
	double point_size, line_width, nml_length;
	bool show_box, show_neighbor_graph, show_nmls, show_point_labels, do_symmetrize;
	bool focus_and_not_event;
	unsigned char show_tgls[3];
	bool show_points, illum_points, show_edges, flat_shading, show_face_corners, show_without_orientation;
	int cell_index;
	int vertex_index;
	unsigned int hole_vi;
	int hole_j;
	int max_cycle_length;

	DebugMode debug_mode;
	VertexDebugMode vertex_debug_mode;
	AccelerationMode acceleration_mode;
	VertexColorMode vertex_color_mode;
	EdgeColorMode edge_color_mode;

public:
	gl_surface_reconstructor_drawable();
	gl_surface_reconstructor_drawable(const std::string& file_name);
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	void on_set(void* member_ptr);
	bool read(const std::string& file_name);
	const point_cloud* get_point_cloud() const;
	void set_focus_point(int vi, unsigned int j = 0, bool flip_nml = false);
	void set_view();
	void on_set_receiver();
	void set_point_cloud(point_cloud* _pc);
	const char* get_name() const;
	void render_box(const Box& box);
	void draw_vertex_color(unsigned int vi) const;
	void draw_edge_color(unsigned int vi, unsigned int j, bool is_symm, bool is_start) const;
	void draw_debug_vertex(unsigned int vi);
	void draw_normal(unsigned int vi, unsigned int vj, unsigned int vk) const;

	void draw_boxes();
	void draw_points();
	void draw_normals();
	void draw_graph();
	void draw_face_corners();
	void draw_point_labels();
	void draw_debug();
	void draw_consistent();
	void draw_region_growing();
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);

	void stream_stats(std::ostream& os);
	void stream_help(std::ostream& os);
	bool handle(cgv::gui::event& e);
};

#include <cgv/config/lib_end.h>