#pragma once

#include <cgv/base/base.h>
#include "point_cloud.h"
#include "neighbor_graph.h"
#include <vector>
#include <cgv/utils/statistics.h>
#include <cgv/reflect/reflection_handler.h>
#include <cgv/data/dynamic_priority_queue.h>

#include "lib_begin.h"

enum Direction { BACKWARD, FORWARD };
enum GrowEventType { CORNER_GROW_EVENT, EDGE_GROW_EVENT };

struct CGV_API grow_event
{
	unsigned int vi;
	unsigned int j;
	unsigned int k;
	int next_grow_event_of_vertex;
	GrowEventType type;
	Direction dir;
	float quality;
	grow_event(unsigned int _vi = 0, unsigned int _j = 0, unsigned int _k = 0, 
		GrowEventType _t = CORNER_GROW_EVENT, Direction _d = BACKWARD) :
			vi(_vi), j(_j), k(_k), type(_t), dir(_d), 
			quality(0), next_grow_event_of_vertex(-1) {}
	bool operator < (const grow_event& ge) const { return quality > ge.quality; }
};

extern CGV_API std::ostream& operator << (std::ostream& os, const grow_event& ge);

struct CGV_API surface_reconstructor : public point_cloud_types
{
	typedef Dir Vec;
	typedef cgv::math::fvec<Crd,2> P2d;

	bool self_reflect(cgv::reflect::reflection_handler& srh);


	point_cloud* pc;
	neighbor_graph* ng;

	bool use_orientation, allow_intersections_in_holes;
	double z_weight;
	double normal_quality_exp, noise_to_sampling_ratio;
	bool use_normal_weight;
	unsigned int max_nr_exhaustive_search;

	/** visual debugging */
	//@{
	float line_width;
	float point_size;
	enum DebugMode {
		DBG_NONE, DBG_SYMMETRIZE, DBG_MAKE_CONSISTENT, DBG_CYCLE_FILTER
	};
	DebugMode debug_mode;
	/// debug vertex
	int debug_vi;
	/// debug neighbor
	int debug_j;

	void draw_colored_point(const Pnt& p, float size, float r, float g, float b) const;
	void draw_colored_point(unsigned int vi, float size, float r, float g, float b) const;

	void draw_colored_corner(unsigned int vi, unsigned int j, float size, float r, float g, float b, float weight=0.6f) const;

	void draw_edge(unsigned int vi, unsigned int vj, float width) const;
	void draw_colored_edge(const Pnt& p, const Pnt& q, float width, float r, float g, float b) const;
	void draw_colored_edge(unsigned int vi, unsigned int vj, float width, float r, float g, float b) const;
	void draw_colored_edge(unsigned int vi, unsigned int vj, float width, 
								  float ri, float gi, float bi, float rj, float gj, float bj) const;
	void draw_colored_triangle(unsigned int vi, unsigned int vj, unsigned int vk, float r, float g, float b) const;
	//@}

	/// standard construction
	surface_reconstructor();
	/// free all alocated memory and reinit
	void init();

	/**@name computation of number of triangles per edge and half edge*/
	//@{
	/// statistics about number of incident triangles per vertex and per edge
	cgv::utils::statistics ntpv, ntpe;
	/// store the number of incident triangles for each vertex
	std::vector<unsigned int> nr_triangles_per_vertex;
	/// store the number of incident triangles per edge for each directed edge
	std::vector<std::vector<unsigned int> > nr_triangles_per_edge;
	/// increment the count of a directed edge
	void increment_directed_edge(int vi, int vj, bool inc = true);
	/// allocate memory for triangle counts and set all triangle counts to zero
	bool init_nr_triangles();
	///
	bool add_triangle_check_edge(unsigned int vi, unsigned int vj) const;
	///
	bool can_add_triangle_manifold(unsigned int vi, unsigned int vj, unsigned int vk) const;
	///
	void count_triangle(unsigned int vi, unsigned int vj, unsigned int vk, bool inc = true);
	/// compute the number of incident triangles for all vertices and all edges
	bool compute_nr_triangles();

	bool nr_triangles_computed() const;
	/// return the computed number of triangles for given vertex
	unsigned int get_nr_triangles(unsigned int vi) const;
	/// return the computed number of triangles for given directed edge
	unsigned int get_nr_triangles(unsigned int vi, unsigned int j) const;
	//@}


	/**@name geometric predicates */
	//@{
	/// check delaunay property of an edge from p to n1
	bool is_locally_delaunay(const P2d& p, const P2d& n0, const P2d& n1, const P2d& n2) const;
	/// compute the direction vector from pi to pj
	Dir compute_tangential_direction(unsigned int vi, unsigned int vj) const;
	/// compute a point inside the corner after the directed edge vi,j with given weight for pi
	Pnt compute_corner_point(unsigned int vi, unsigned int vj, unsigned int vk, float weight = 0.6f) const;
	/// compute a point inside the corner after the directed edge vi,j with given weight for pi
	Pnt compute_corner_point(unsigned int vi, unsigned int j, float weight = 0.6f) const;
	/// compute the normal of a triangle
	Nml compute_triangle_normal(unsigned int vi, unsigned int vj, unsigned int vk) const;
	/// compute the tangential vectors at vi with x directed towards the first neighbor
	bool put_coordinate_system(unsigned int vi, unsigned int j, Vec& x, Vec& y) const;
	/// compute the tangential vectors at vi with x directed towards the first neighbor
	bool put_coordinate_system(unsigned int vi, Vec& x, Vec& y) const;
	/// compute the angle from direction d0 to the tangential direction (vi,vj)
	double compute_tangential_corner_angle(unsigned int vi, const Dir& d0, unsigned int vj) const;
	/// compute corner angle in tangential space
	double compute_tangential_corner_angle(unsigned int vi, unsigned int vj, unsigned int vk) const;
	/// compute corner angle in tangential space
	double compute_tangential_corner_angle(unsigned int vi, unsigned int j) const;
	/// compute corner angle in world space
	double compute_corner_angle(unsigned int vi, unsigned int vj, unsigned int vk) const;
	/// compute corner angle in world space
	double compute_corner_angle(unsigned int vi, unsigned int j) const;
	/// find a corner that contains the edge vi,vj
	unsigned int find_surrounding_corner(unsigned int vi, unsigned int vj) const;
	//@}

	/**@name computation of smoothed normals */
	//@{
	/// keep memory for one copy of normals
	std::vector<Pnt> secondary_points;
	std::vector<Nml> secondary_normals;
	/// copy the original normals to the secondary normals
	void randomize_orientation();
	/// 
	void jitter_points();
	/// copy the original normals to the secondary normals
	void toggle_normals();
	/// copy the original normals to the secondary normals
	void toggle_points();
	/// compute normals from neighbor graph and distance and normal weights
	void smooth_normals();
	/// recompute normals from neighbor graph and distance weights
	void compute_weighted_normals(bool reorient);
	/// recompute normals from neighbor graph and distance and normal weights
	void compute_bilateral_weighted_normals(bool reorient);
	/// consistently orient normals
	void orient_normals();
	//@}

	/**@name access to neighbor graph */
	//@{
	/// look for a corner and return -1 if it is not present in the 1-ring of vi
	int find_corner(unsigned int vi, unsigned int vj, unsigned int vk) const;
	/// check for a corner
	bool is_corner(unsigned int vi, unsigned int vj, unsigned int vk) const;
	//@}

	/**@name additional information per vertex and directed edge*/
	//@{
	/// different vertex types
	enum VertexType { VT_NONE = 0, VT_BORDER, VT_ONE_RING, VT_NM_BORDER };
	/// store per vertex flags
	std::vector<unsigned char> vertex_info;
	unsigned int vertex_counts[4];
	/// store per vertex reference lengths
	std::vector<Crd> vertex_reference_length;
	///
	bool compute_reference_length_from_delaunay_filter;

	/// store directed edge flags in one byte per directed edge
	std::vector<std::vector<unsigned char> > directed_edge_info;

	/// ensure that vertex info is allocated
	void ensure_vertex_info();
	/// clear vertex info
	void clear_vertex_info();
	/// ensure that directed edge info is allocated
	void ensure_directed_edge_info();
	/// clear directed edge info
	void clear_directed_edge_info();
	///
	void clear_face_corner_info();
	///
	void clear_flag();
	///
	void clear_vertex_type_info();
	///
	VertexType compute_vertex_type_info(unsigned int vi) const;
	void compute_vertex_type_info();

	/// vertex border flag
	bool is_border(unsigned int vi) const;
	void mark_as_border(unsigned int vi, bool flag = true);

	/// vertex border flag
	VertexType get_vertex_type(unsigned int vi) const;
	void set_vertex_type(unsigned int vi, VertexType vt);

	/// face corner corners
	bool is_face_corner(unsigned int vi, unsigned int j) const;
	bool is_face_corner(unsigned int vi, unsigned int vj, unsigned int vk) const;
	bool is_corner_or_non_face(unsigned int vi, unsigned int vj, unsigned int vk) const;
	void mark_as_face_corner(unsigned int vi, unsigned int j, bool flag = true);
	void mark_triangle_corner(unsigned int vi, unsigned int vj, unsigned int vk, bool flag = true);
	void mark_triangle(unsigned int vi, unsigned int vj, unsigned int vk, bool flag = true);
	void mark_triangular_faces(const std::vector<unsigned int>& T);

	/// constraints
	bool is_constraint(unsigned int vi, unsigned int j) const;
	void mark_as_constraint(unsigned int vi, unsigned int j, bool flag = true);

	/// simple marking of half edges
	bool is_marked(unsigned int vi, unsigned int j) const;
	void mark(unsigned int vi, unsigned int j, bool flag = true);

	/// reference length
	Crd get_reference_length(unsigned int vi) const;
	void set_reference_length(unsigned int vi, const Crd& l);
	//@}


	/**@name updates*/
	//@{
	void remove_directed_edge(unsigned int vi, unsigned int vj);
	//@}

	/**@name region growing reconstruction*/
	//@{
	/// whether to print debug information on grow events
	bool debug_events;
	double valid_length_scale;
	/// store all grow events
	cgv::data::dynamic_priority_queue<grow_event> grow_events;
	/// store for each vertex the index of its first grow event or -1 if non present
	std::vector<int> first_grow_event;
	/// statistics over the quality of the grow event triangles
	cgv::utils::statistics geqs;
	/// compute geometric quality of a triangle
	float compute_normal_quality(const Nml& n1, const Nml& n2) const;
	/// compute geometric quality of a triangle
	float compute_triangle_quality(unsigned int vi,unsigned int vj, unsigned int vk, unsigned int nr_insert, unsigned int nr_remove) const;
	/// check consistency of corner grow operation
	bool can_extent_fan(unsigned int vi,unsigned int vj, unsigned int vk, Direction dir, unsigned int& nr_insert, unsigned int& nr_remove) const;
	/// check consistency of edge grow operation
	bool can_connect_to_fan(unsigned int vi,unsigned int vj, unsigned int vk, unsigned int& nr_insert, unsigned int& nr_remove) const;
	
	bool is_valid_corner_grow_event(const grow_event& ge, unsigned int& nr_insert, unsigned int& nr_remove) const;
	bool is_valid_edge_grow_event(const grow_event& ge, unsigned int& nr_insert, unsigned int& nr_remove) const;
	bool validate_event(grow_event& ge) const;
	void add_grow_event(const grow_event& ge);
	/// check corner grow event and insert to queue
	bool consider_corner_grow_event(unsigned int vi,unsigned int j, unsigned int k);
	/// check edge grow event and insert to queue
	bool consider_edge_grow_event(unsigned int vi,unsigned int j, unsigned int k, Direction dir);
	/// determine all grow events of the given vi
	void consider_grow_events(unsigned int vi);
	/// build priority queue of events
	void build_grow_queue(const std::vector<unsigned int>& T);
	/// remove the grow events of a given vertex
	void remove_grow_events(unsigned int vi);
	///
	unsigned int insert_directed_edge(unsigned int vi, unsigned int vj);
	///
	void remove_directed_edges(unsigned int vi, unsigned int j, unsigned int k);
	/// extent a triangle fan in the 1-ring of vi
	void extent_fan(unsigned int vi,unsigned int vj, unsigned int vk, Direction dir);
	/// 
	void connect_to_fan(unsigned int vi,unsigned int vj, unsigned int vk);
	/// perform grow event
	void perform_next_grow_event(std::vector<unsigned int>& T);
	/// perform grow events till no more events are left and add the generated triangles to T
	unsigned int grow_all(std::vector<unsigned int>& T);
	//@}


	/**@name neighbor graph filters */
	//@{
	void sort_by_tangential_angle(unsigned int vi);
	void sort_by_tangential_angle();

	bool symmetrize_directed_edge(unsigned int vi, unsigned int j);
	void symmetrize();

	bool delaunay_fan_filter(unsigned int vi, std::vector<Idx>& Mi, std::vector<unsigned char>& Ei) const;
	void delaunay_fan_neighbor_graph_filter();

	void cycle_filter();
	//@}

	/**@name postprocessing */
	//@{

	Crd compute_triangle_area(unsigned int vi, unsigned int vj, unsigned int vk) const;
	bool is_valid_ear(unsigned int vi, unsigned int vj, unsigned int vk) const;
	bool find_min_area_hole_tesselation(const std::vector<unsigned int>& V, 
		std::vector<unsigned int>& tess, Crd& area) const;
	bool greedy_find_min_area_hole_tesselation(const std::vector<unsigned int>& V, 
		std::vector<unsigned int>& tess, Crd& area) const;
	void close_hole_with_tesselation(const std::vector<unsigned int>& _V, 
		const std::vector<unsigned int>& tess, std::vector<unsigned int>& T);

	void close_holes(std::vector<unsigned int>& T);

	enum HoleType { HT_NONE, HT_MANIFOLD, HT_NM, HT_INCONSISTENT };
	///
	HoleType trace_hole(unsigned int vi, int j, std::vector<unsigned int>& V);
	///
	bool step_hole(unsigned int& vi, int& j) const;
	///
	void mark_hole(const std::vector<unsigned int>& V, bool flag = true);
	/// find the next hole
	bool find_next_hole_start(unsigned int& vi, int& j) const;
	bool is_manifold_hole(std::vector<unsigned int>& V) const;

	/// find the next hole
	HoleType find_next_hole(unsigned int& vi, int& j, std::vector<unsigned int>& V);

	Crd compute_fan_quality(unsigned int vi, unsigned int j, unsigned int k) const;
	void remove_triangle(unsigned int vi, unsigned int vj, unsigned int vk, std::vector<unsigned int>& T);
	void resolve_non_manifold_vertex(unsigned int vi, std::vector<unsigned int>& T);
	unsigned int resolve_non_manifold_vertices(std::vector<unsigned int>& T);
	unsigned int resolve_non_manifold_holes(std::vector<unsigned int>& T);

	void analyze_holes();

	struct ear {
		unsigned int i;
		float quality;
		ear(unsigned int _i = 0, float _q = 0) : i(_i), quality(_q) {}
		bool operator < (const ear& e) const { return quality > e.quality; }
	};

	std::vector<unsigned int> hole;
	Nml hole_nml;
	void compute_hole_normal();
	cgv::data::dynamic_priority_queue<ear> hole_queue;
	Crd compute_normal_cosine(const Pnt& pi, const Pnt& pl, const Pnt& pj, const Pnt& pk) const;
	void add_hole_triangle(unsigned int vi, unsigned int vj, unsigned int vk, std::vector<unsigned int>& T);
	Crd compute_hole_triangle_quality(unsigned int vi, unsigned int vj, unsigned int vk) const;

	/// step wise closing a hole
	void build_hole_closing_queue();
	///
	void step_hole_closing(std::vector<unsigned int>& T);
	/// close a hole by ear cutting
	bool close_hole(std::vector<unsigned int>& V);

	bool debug_intersection_tests;
	bool debug_hole;
	bool perform_intersection_tests;
	/// return the volume spanned by the tetrahedron from vi to triangle T
	Crd vol(unsigned int vi, const unsigned int* T) const;
	Crd vol(unsigned int vi, unsigned int vj, unsigned int vk, unsigned int vl) const;
	bool side(unsigned int vi, const unsigned int* T) const;
	bool side(unsigned int vi, unsigned int vj, unsigned int vk, unsigned int vl) const;
	bool reduced_tgl_tgl_intersection_test(unsigned int vi, unsigned int vj1, unsigned int vk1, unsigned int vj2, unsigned int vk2) const;
	/// check if two triangles intersect in space
	bool tgl_tgl_intersection_test(const unsigned int* t1, const unsigned int* t2) const;
	/// check if a triangle can be constructed without generating self intersections
	bool can_create_triangle_without_self_intersections(unsigned int vi,unsigned int vj,unsigned int vk) const;
	//@}

	/**@name consistent information extraction */
	//@{
	void find_consistent_edges(std::vector<unsigned int>& E);
	void find_consistent_triangles(std::vector<unsigned int>* T);
	void find_consistent_quads(std::vector<unsigned int>& T);

	bool make_corner_consistent(unsigned int vi, unsigned int vj, unsigned int vk);
	void make_triangles_consistent(const std::vector<unsigned int>& inconsistent_T, std::vector<unsigned int>& consistent_T);
	//@}

	/**@name write result to file*/
	//@{
	bool read_vrml_1(const std::string& file_name, point_cloud* pc, std::vector<unsigned int>& T);
	bool read_obj(const std::string& file_name, point_cloud* pc, std::vector<unsigned int>& T);
	/// detect output type from extension
	bool write(const std::string& file_name, const std::vector<unsigned int>& T) const;
	/// write in obj format
	bool write_obj(const std::string& file_name, const std::vector<unsigned int>& T) const;
	/// write in ply format
	bool write_ply(const std::string& file_name, const std::vector<unsigned int>& T) const;
	//@}
};

#include <cgv/config/lib_end.h>
