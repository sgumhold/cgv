#pragma once

#include <cgv/base/node.h>
#include <delaunay/delaunay_mesh_with_hierarchy.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/provider.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;

class gl_delaunay_mesh_draw :
	public node,
	public drawable,
	public provider
{
public:
	/// the used coordinate type
	typedef double coord_type;
	/// the type of the triangle mesh
	typedef delaunay_mesh_with_hierarchy<> tm_type;
protected:
	void clear_mesh();
	void clear_all();
	void insert_triangle();
	void insert_point();
	void add_voronoi_points();
	/// whether to show the corner data structure
	bool show_corner_ds;
	/// whether to show original points
	bool show_points;
	/// whether to show the voronoi vertices
	bool show_voronoi_vertices;
	/// whether to show the voronoi edges
	bool show_voronoi_edges;
	/// whether to show the delaunay triangles
	bool show_delaunay_triangles;
	/// whether to label the vertices
	bool show_vertex_labels;
	///
	bool show_reconstruction;
	///
	bool debug_next_insertion;
	bool debug_nearest_neighbor;
	bool debug_point_location;
	bool debug_flipable;
	bool debug_trace;

	bool debug_largest_circle;
	unsigned ti_lc;
	tm_type::point_type c_lc;
	double r_lc;
	unsigned nr_to_be_added;
	void add_circle_center();
	void compute_largest_circle();
	///
	bool set_random_seed;
	///
	double primitive_scale;
	///
	unsigned int hierarchy_factor;
	///
	tm_type::SamplingType sample_sampling;
	///
	tm_type::DistributionType sample_distribution;
	///
	tm_type::GeneratorType sample_generator;
	///
	tm_type::ShapeType sample_shape;
	///
	tm_type::SamplingStrategy sample_strategy;
	///
	unsigned int sample_size;
	///
	bool sample_shuffle;
	/// triangle mesh
	tm_type *tm, *tm_bak;
	/// next vertex index
	unsigned int vi;
	/// constraint point index
	unsigned int vc;
	/// first vertex to colorize differently
	int first_other_vi;
	/// currently used random seed
	unsigned int s;
private:
	// data that is used in several places during drawing
	std::vector<tm_type::trace_hit> trace_hits;
	std::vector<tm_type::point_type> vv;
	tm_type::point_location_info pli;
	unsigned int vnn;
public:
	/// construct with initialized mesh
	gl_delaunay_mesh_draw();
	/// initialize mesh to random point set
	void init_mesh(unsigned int n);
	/// measure the time to compute the delaunay triangulation of n random points
	void measure_time(unsigned int debug_point = -1);
	/// overload methods of drawable
	std::string get_type_name() const;
	void draw_edge(unsigned int ci) const;	
	void draw_triangle(unsigned int ci) const;
	void compute_draw_data();
	void draw_text_elements(context& ctx) const;
	void draw_point_elements() const;
	void draw_line_elements() const;
	void clear_draw_data();
	bool init(context& ctx);
	void draw(context& ctx);
	void create_gui();
};