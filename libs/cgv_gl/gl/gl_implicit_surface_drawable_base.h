#pragma once

#include <cgv/render/drawable.h>
#include "mesh_render_info.h"
#include <cgv/math/mfunc.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/media/mesh/streaming_mesh.h>
#include <cgv/media/illum/surface_material.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv_gl/cone_renderer.h>

#include "lib_begin.h" // @<

namespace cgv { // @<
	namespace render { // @<
		namespace gl { // @<

/// type of contouring method @>
enum ContouringType { MARCHING_CUBES, DUAL_CONTOURING };
/// normal computation type @>
enum NormalComputationType { GRADIENT_NORMALS, FACE_NORMALS, CORNER_NORMALS, CORNER_GRADIENTS };

/** drawable that visualizes implicit surfaces by contouring them with marching cubes or
    dual contouring. @> */
class CGV_API gl_implicit_surface_drawable_base : 
	public drawable, 
	public cgv::media::mesh::streaming_mesh_callback_handler
{
public: //@<
	/// type of the function describing the implicit surface
	typedef cgv::math::v3_func<double,double> F;
	/// type of axis aligned box used to define the tesselation domain
	typedef cgv::media::axis_aligned_box<double,3> dbox3;
	///
	typedef cgv::math::vec<double> vec_type;
private:
	bool outofdate;
	cgv::media::mesh::streaming_mesh<double>* sm_ptr;

	cgv::media::mesh::simple_mesh<float> mesh;
	mesh_render_info mri;

	std::vector<vec3> nml_gradient_geometry;
	std::vector<vec3> nml_mesh_geometry;

protected: //@<
	//@>
	unsigned int res;
	dbox3 box;
	F* func_ptr;
	//@>
	ContouringType contouring_type;
	//@>
	double normal_threshold;
	//@>
	double consistency_threshold;
	//@>
	unsigned int max_nr_iters;
	//@>
	NormalComputationType normal_computation_type;
	//@>
	unsigned int ix;
	//@>
	unsigned int iy;
	//@>
	unsigned int iz;
	//@>
	bool show_vertices;
	bool show_wireframe;
	bool show_surface;
	//@>
	bool show_sampling_locations;
	//@>
	bool show_sampling_grid;
	float sampling_grid_alpha;
	//@>
	bool show_box;
	//@>
	bool show_mini_box;
	//@>
	bool show_gradient_normals;
	//@>
	bool show_mesh_normals;

	box_render_style brs;
	sphere_render_style srs;
	cone_render_style crs;
	arrow_render_style ars;

	//@>
	double epsilon;
	//@>
	double grid_epsilon;

	//@>
	int nr_faces;
	//@>
	int nr_vertices;
	///
	cgv::media::illum::surface_material material;

	void add_normal(const dvec3& p, const dvec3& n, std::vector<vec3>& nml_gradient_geometry) const;

	/// allows to augment a newly computed vertex by additional data
	void new_vertex(unsigned int vi);
	/// announces a new quad
	void new_polygon(const std::vector<unsigned int>& vertex_indices);
	/// drop the currently first vertex that has the given global vertex index
	void before_drop_vertex(unsigned int vertex_index);
	///
	bool triangulate;
	/// of this output stream is defined, use it to write currently extracted surface to it
	std::ostream* obj_out;
	/// normal index for face normals
	unsigned normal_index;
	/// function used to save to obj file
	bool save(const std::string& file_name);
	/// call the selected surface extraction method
	virtual void surface_extraction();
	/// compute the normal of a face
	dvec3 compute_face_normal(const std::vector<unsigned int> &vis, dvec3* c = 0) const;
	/// helper function to extract mesh from implicit surface
	virtual void extract_mesh();
	/// helper function to tesselate the implicit surface
	void draw_implicit_surface(context& ctx);
	dvec3 compute_corner_normal(const dvec3& pk, const dvec3& pi, const dvec3& pj, const dvec3& ni);
public:
	/// standard constructor does not initialize the function pointer such that nothing is drawn
	gl_implicit_surface_drawable_base();

	void set_function(F* _func_ptr);
	F* get_function() const;

	void set_resolution(unsigned int _res);
	unsigned int get_resolution() const;

	void enable_wireframe(bool do_enable = true);
	bool is_wireframe_enabled() const;

	void enable_sampling_grid(bool do_enable = true);
	bool is_sampling_grid_enabled() const;

	void enable_sampling_locations(bool do_enable = true);
	bool is_sampling_locations_enabled() const;

	void enable_box(bool do_enable = true);
	bool is_box_enabled() const;

	void enable_normals(bool do_enable = true);
	bool are_normals_enabled() const;

	void set_epsilon(double _epsilon);
	double get_epsilon() const;

	void set_grid_epsilon(double _grid_epsilon);
	double get_grid_epsilon() const;

	void set_box(const dbox3& _box);
	const dbox3& get_box() const;

	unsigned int get_nr_triangles_of_last_extraction() const;
	unsigned int get_nr_vertices_of_last_extraction() const;

	/// use this as callback to ask for a re-tesselation of the implicit surface
	void post_rebuild();
	bool init(context& ctx);
	void clear(context& ctx);
	void draw(context& ctx);
	void finish_frame(context& ctx);

}; // @<

		} // @<
	} // @<
} // @<

#include <cgv/config/lib_end.h> // @<