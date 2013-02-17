#pragma once

#include <cgv/render/drawable.h>
#include <cgv/math/mfunc.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/media/mesh/streaming_mesh.h>
#include <cgv/media/illum/phong_material.hh>

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
	typedef cgv::media::axis_aligned_box<double,3> box_type;
	///
	typedef cgv::math::fvec<double,3> pnt_type;
	///
	typedef cgv::math::fvec<double,3> vec_type;
private:
	unsigned int id;
	bool outofdate;
	cgv::media::mesh::streaming_mesh<double>* sm_ptr;

	std::vector<float> nml_gradient_geometry;
	std::vector<float> nml_mesh_geometry;

protected: //@<
	//@>
	unsigned int res;
	box_type box;
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
	bool wireframe;
	//@>
	bool show_sampling_grid;
	//@>
	bool show_sampling_locations;
	//@>
	bool show_box;
	//@>
	bool show_mini_box;
	//@>
	bool show_gradient_normals;
	//@>
	bool show_mesh_normals;

	//@>
	double epsilon;
	//@>
	double grid_epsilon;

	//@>
	int nr_faces;
	//@>
	int nr_vertices;
	///
	cgv::media::illum::phong_material material;

	void add_normal(const pnt_type& p, const vec_type& n, std::vector<float>& nml_gradient_geometry) const;

	/// marching cubes callback handling for new vertices
	void new_mc_vertex(unsigned int vi);
	/// marching cubes callback handling for new triangles
	void triangle_callback(unsigned int vi, unsigned int vj, 
											 unsigned int vk);

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
	vec_type compute_face_normal(const std::vector<unsigned int> &vis, pnt_type* c = 0) const;
	/// helper function to build a display list with the implicit surface
	virtual void build_display_list();
	/// helper function to tesselate the implicit surface
	void draw_implicit_surface(context& ctx);
	void render_corner_normal(const pnt_type& pk, const pnt_type& pi, const pnt_type& pj, const vec_type& ni);
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

	void set_box(const box_type& _box);
	const box_type& get_box() const;

	unsigned int get_nr_triangles_of_last_extraction() const;
	unsigned int get_nr_vertices_of_last_extraction() const;

	/// use this as callback to ask for a re-tesselation of the implicit surface
	void post_rebuild();

	void draw(context& ctx);

}; // @<

		} // @<
	} // @<
} // @<

#include <cgv/config/lib_end.h> // @<