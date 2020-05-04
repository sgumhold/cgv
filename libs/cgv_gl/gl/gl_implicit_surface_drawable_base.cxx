#include "gl_implicit_surface_drawable_base.h"
#include <cgv/media/mesh/marching_cubes.h>
#include <cgv/media/mesh/dual_contouring.h>

#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/math/ftransform.h>
#include <cgv_gl/gl/gl.h>

#include <fstream>

using namespace cgv::math;
using namespace cgv::media;

namespace cgv {
	namespace render {
		namespace gl {

gl_implicit_surface_drawable_base::gl_implicit_surface_drawable_base() : box(dvec3(-1.2f,-1.2f,-1.2f),dvec3(1.2f,1.2f,1.2f))
{
	obj_out = 0;
	triangulate = false;
	nr_faces = 0;
	nr_vertices = 0;
#ifdef _DEBUG
	res = 10;
#else
	res = 25;
#endif
	func_ptr = 0;
	show_vertices = false;
	show_wireframe = false;
	show_surface = true;
	contouring_type = DUAL_CONTOURING;
	show_sampling_grid = false;
	show_sampling_locations = false;
	normal_computation_type = FACE_NORMALS;
//	normal_computation_type = GRADIENT_NORMALS;
	brs.culling_mode = CM_FRONTFACE;
	brs.illumination_mode = IM_TWO_SIDED;
	brs.map_color_to_material = CM_COLOR;
	srs.radius = 0.13f;
	srs.surface_color = rgb(1, 1, 0.5f);
	crs.radius = 0.04f;
	crs.surface_color = rgb(0.3f, 0.46f, 0.43f);
	ars.radius_relative_to_length = 0.05f;
	consistency_threshold = 0.01;
	max_nr_iters = 8;
	normal_threshold = 0.73;
	show_box = true;
	show_mesh_normals = false;
	show_gradient_normals = false;
	sm_ptr = 0;
	epsilon = 1e-8;
	grid_epsilon = 0.01;
	ix=iy=iz=0;
	show_mini_box = false;
	sampling_grid_alpha = 0.4f;
	material.set_diffuse_reflectance(rgb(0.1f, 0.6f, 1.0f));
	material.set_specular_reflectance(rgb(0.7f,0.7f,0.7f));
	material.set_roughness(0.6f);
}

/// callback used to save to obj file
bool gl_implicit_surface_drawable_base::save(const std::string& file_name)
{
	std::ofstream os(file_name.c_str());
	if (os.fail())
		return false;
	obj_out = &os;
	normal_index = 0;
	surface_extraction();
	obj_out = 0;
	return true;
}

void gl_implicit_surface_drawable_base::set_function(F* _func_ptr)
{
	func_ptr = _func_ptr;
	post_rebuild();
}


gl_implicit_surface_drawable_base::F* gl_implicit_surface_drawable_base::get_function() const
{
	return func_ptr;
}

void gl_implicit_surface_drawable_base::set_resolution(unsigned int _res)
{
	res = _res;
	post_rebuild();
}

unsigned int gl_implicit_surface_drawable_base::get_resolution() const
{
	return res;
}

void gl_implicit_surface_drawable_base::enable_wireframe(bool do_enable)
{
	if (show_wireframe == do_enable)
		return;
	show_wireframe = do_enable;
	post_redraw();
}

void gl_implicit_surface_drawable_base::enable_sampling_grid(bool do_enable)
{
	if (show_sampling_grid == do_enable)
		return;
	show_sampling_grid = do_enable;
	post_redraw();
}

bool gl_implicit_surface_drawable_base::is_sampling_grid_enabled() const
{
	return show_sampling_grid;
}

void gl_implicit_surface_drawable_base::enable_sampling_locations(bool do_enable)
{
	if (show_sampling_locations == do_enable)
		return;
	show_sampling_locations = do_enable;
	post_redraw();
}

bool gl_implicit_surface_drawable_base::is_sampling_locations_enabled() const
{
	return show_sampling_locations;
}

void gl_implicit_surface_drawable_base::enable_box(bool do_enable)
{
	if (show_box == do_enable)
		return;
	show_box = do_enable;
	post_redraw();
}

bool gl_implicit_surface_drawable_base::is_box_enabled() const
{
	return show_box;
}

void gl_implicit_surface_drawable_base::enable_normals(bool do_enable)
{
	if (show_mesh_normals == do_enable &&
		 show_gradient_normals == do_enable)
		return;
	show_mesh_normals = show_gradient_normals = do_enable;
	post_redraw();
}

bool gl_implicit_surface_drawable_base::are_normals_enabled() const
{
	return show_gradient_normals;
}



bool gl_implicit_surface_drawable_base::is_wireframe_enabled() const
{
	return show_wireframe;
}


void gl_implicit_surface_drawable_base::set_epsilon(double _epsilon)
{
	epsilon = _epsilon;
	post_rebuild();
}

double gl_implicit_surface_drawable_base::get_epsilon() const
{
	return epsilon;
}

void gl_implicit_surface_drawable_base::set_grid_epsilon(double _grid_epsilon)
{
	grid_epsilon = _grid_epsilon;
	post_rebuild();
}

double gl_implicit_surface_drawable_base::get_grid_epsilon() const
{
	return grid_epsilon;
}


void gl_implicit_surface_drawable_base::set_box(const dbox3& _box)
{
	box = _box;
	post_rebuild();
}

const gl_implicit_surface_drawable_base::dbox3& gl_implicit_surface_drawable_base::get_box() const
{
	return box;
}

unsigned int gl_implicit_surface_drawable_base::get_nr_triangles_of_last_extraction() const
{
	return nr_faces;
}

unsigned int gl_implicit_surface_drawable_base::get_nr_vertices_of_last_extraction() const
{
	return nr_vertices;
}


void gl_implicit_surface_drawable_base::add_normal(const dvec3& p, const dvec3& n, std::vector<vec3>& nml_gradient_geometry) const
{
	nml_gradient_geometry.push_back(vec3(p));
	nml_gradient_geometry.push_back(vec3(p+n/res));
}

/// announces a new quad
void gl_implicit_surface_drawable_base::new_polygon(const std::vector<unsigned> &vertex_indices)
{
	unsigned n = (unsigned)vertex_indices.size();
	mesh.start_face();

	// compute face normal
	if (normal_computation_type == FACE_NORMALS) {
		dvec3 ctr;
		dvec3 nml = compute_face_normal(vertex_indices, &ctr);
		if (obj_out) {
			++normal_index;
			(*obj_out) << "vn " << nml(0) << " " << nml(1) << " " << nml(2) << std::endl;
			if (triangulate) {
				for (unsigned i = 0; i < n - 2; ++i) {
					(*obj_out) << "f " << vertex_indices[0] + 1 << "//" << normal_index;
					(*obj_out) << " " << vertex_indices[i + 1] + 1 << "//" << normal_index;
					(*obj_out) << " " << vertex_indices[i + 2] + 1 << "//" << normal_index << std::endl;
				}
			}
			else {
				(*obj_out) << "f";
				for (unsigned i = 0; i < n; ++i) {
					(*obj_out) << " " << vertex_indices[i] + 1 << "//" << normal_index;
				}
				(*obj_out) << std::endl;
			}
		}
		else {
			int ni = mesh.new_normal(nml);
			for (unsigned i = 0; i < n; ++i)
				mesh.new_corner(vertex_indices[i], ni);
			add_normal(ctr, nml, nml_mesh_geometry);
		}
		return;
	}

	if (obj_out) {
		if (triangulate) {
			for (unsigned i = 0; i < n - 2; ++i) {
				(*obj_out) << "f " << vertex_indices[0] + 1 << "//" << vertex_indices[0] + 1;
				(*obj_out) << " " << vertex_indices[i + 1] + 1 << "//" << vertex_indices[i + 1] + 1;
				(*obj_out) << " " << vertex_indices[i + 2] + 1 << "//" << vertex_indices[i + 2] + 1 << std::endl;
			}
		}
		else {
			(*obj_out) << "f";
			for (unsigned i = 0; i < n; ++i) {
				(*obj_out) << " " << vertex_indices[i] + 1 << "//" << vertex_indices[i] + 1;
			}
			(*obj_out) << std::endl;
		}
		return;
	}

	std::vector<int> nis;
	for (unsigned i=0; i<n; ++i) {
		// compute corner normal
		vec3 nml = compute_corner_normal(sm_ptr->vertex_location(vertex_indices[(i+n-1)%n]),
			                  sm_ptr->vertex_location(vertex_indices[i]),
									sm_ptr->vertex_location(vertex_indices[(i+1)%n]),
									sm_ptr->vertex_normal(vertex_indices[i]));

		// check whether we saw it before
		int ni = -1;
		for (int nj : nis)
			if ((nml - mesh.normal(nj)).length() < 1e-6f) {
				ni = nj;
				break;
			}
		// if not add new normal
		if (ni == -1) {
			ni = mesh.new_normal(nml);
			nis.push_back(ni);
		}
		mesh.new_corner(vertex_indices[i], ni);
	}
}

/// allows to augment a newly computed vertex by additional data
void gl_implicit_surface_drawable_base::new_vertex(unsigned int vi)
{
	dvec3 p = sm_ptr->vertex_location(vi);
	mesh.new_position(p);
	if (normal_computation_type != FACE_NORMALS) {
		vec_type grad = func_ptr->evaluate_gradient(p.to_vec());
		dvec3 n(grad.size(), grad);
		n.normalize();
		sm_ptr->vertex_normal(vi) = n;
		if (obj_out)
			(*obj_out) << "v "  << p(0) << " " << p(1) << " " << p(2) << "\n"
			           << "vn " << n(0) << " " << n(1) << " " << n(2) << std::endl;
		else
			add_normal(p,n,nml_gradient_geometry);
	}
	else {
		if (obj_out) {
			dvec3 p = sm_ptr->vertex_location(vi);
			(*obj_out) << "v "  << p(0) << " " << p(1) << " " << p(2) << std::endl;
		}
	}
	++nr_vertices;
}

gl_implicit_surface_drawable_base::dvec3 gl_implicit_surface_drawable_base::compute_face_normal(const std::vector<unsigned int> &vis, dvec3* _c) const
{
	std::vector<const dvec3*> p_pis;
	for (unsigned int i=0; i<vis.size(); ++i)
		p_pis.push_back(&sm_ptr->vertex_location(vis[i]));
	dvec3 c(0,0,0);
	dvec3 n(0,0,0);
	for (unsigned int i=0; i<p_pis.size(); ++i) {
		c += *p_pis[i];
		n += cross(*p_pis[i], *p_pis[(i+1)%p_pis.size()]);
	}
	c *= 1.0/p_pis.size();
	if (_c)
		*_c = c;
	n.normalize();
	return n;
}

/// drop the currently first vertex that has the given global vertex index
void gl_implicit_surface_drawable_base::before_drop_vertex(unsigned int vertex_index)
{
}

gl_implicit_surface_drawable_base::dvec3 gl_implicit_surface_drawable_base::compute_corner_normal(const dvec3& pj, const dvec3& pi, const dvec3& pk, const dvec3& ni)
{
	if (normal_computation_type == FACE_NORMALS)
		return dvec3(0.0);

	if (normal_computation_type == GRADIENT_NORMALS)
		return ni;

	dvec3 n = cross(pk-pi, pj-pi);
	double l  = n.length();
	if ((l > 1e-6) && (dot(n,ni) > l*normal_threshold))
		return ni;
	else {
		dvec3 p = pi;
		if (normal_computation_type == CORNER_GRADIENTS) {
			p = (2.0/3)*pi+(1.0/6)*(pj+pk);
			vec_type grad = func_ptr->evaluate_gradient(p.to_vec());
			n = dvec3(grad.size(),grad);
			n.normalize();
		}
		else {
			if (l < 1e-6)
				return ni;
			n /= l;
		}
		add_normal(p, n, nml_mesh_geometry);
		return n;
	}
}

void gl_implicit_surface_drawable_base::post_rebuild()
{
	outofdate = true;
	post_redraw();
}

void gl_implicit_surface_drawable_base::surface_extraction()
{
	nr_faces = 0;
	nr_vertices = 0;
	switch (contouring_type) {
	case MARCHING_CUBES :
		{
			cgv::media::mesh::marching_cubes<double,double> mc(*func_ptr,this,grid_epsilon,epsilon);
			sm_ptr = &mc;
			mc.extract(0,box,res,res,res,res>40);
			nr_vertices = mc.get_nr_vertices();
			nr_faces = mc.get_nr_faces();
		}
		break;
	case DUAL_CONTOURING :
		{
			cgv::media::mesh::dual_contouring<double,double> dc(*func_ptr,this,consistency_threshold, max_nr_iters, epsilon);
			sm_ptr = &dc;
			dc.extract(0,box,res,res,res,res>40);
			nr_vertices = dc.get_nr_vertices();
			nr_faces = dc.get_nr_faces();
		}
		break;
	}
}

void gl_implicit_surface_drawable_base::extract_mesh()
{
	if (!func_ptr)
		return;

	mesh.clear();
	nml_gradient_geometry.clear();
	nml_mesh_geometry.clear();

	surface_extraction();

	crs.radius_scale = srs.radius_scale = 0.5f * float(box.get_extent().length()/res);
}

bool gl_implicit_surface_drawable_base::init(context& ctx)
{
	ref_box_renderer(ctx, 1);
	ref_sphere_renderer(ctx, 1);
	ref_rounded_cone_renderer(ctx, 1);
	ref_arrow_renderer(ctx, 1);
	return true;
}

void gl_implicit_surface_drawable_base::clear(context& ctx)
{
	ref_box_renderer(ctx, -1);
	ref_rounded_cone_renderer(ctx, -1);
	ref_sphere_renderer(ctx, -1);
	ref_arrow_renderer(ctx, -1);
}

/// overload to draw the content of this drawable
void gl_implicit_surface_drawable_base::draw(context& ctx)
{
	if (outofdate) {
		extract_mesh();
		mri.destruct(ctx);
		if (mesh.get_nr_faces() > 0) {
			mri.construct(ctx, mesh);
			mri.bind(ctx, ctx.ref_surface_shader_program(false), true);
			mri.bind_wireframe(ctx, ref_rounded_cone_renderer(ctx).ref_prog(), true);
		}
		outofdate = false;
	}

	std::vector<box3> boxes;
	std::vector<rgb> box_colors;
	if (show_box) {
		boxes.push_back(box);
		box_colors.push_back(rgb(0.8f, 0.7f, 0.0f));
	}
	if (show_mini_box) {
		dvec3 d = box.get_extent() / double(res - 1);
		dvec3 b0 = box.get_corner(0);
		b0(0) += ix * d(0);
		b0(1) += iy * d(1);
		b0(2) += iz * d(2);
		boxes.push_back(dbox3(b0, b0 + d));
		box_colors.push_back(rgb(0.8f, 0.6f, 1.0f));
	}
	if (!boxes.empty()) {
		auto& br = ref_box_renderer(ctx);
		br.set_render_style(brs);
		br.set_box_array(ctx, boxes);
		br.set_color_array(ctx, box_colors);
		br.render(ctx, 0, boxes.size());
	}

	draw_implicit_surface(ctx);

	if (show_gradient_normals || show_mesh_normals) {
		auto& ar = ref_arrow_renderer(ctx);
		ar.set_render_style(ars);
		if (show_gradient_normals && !nml_gradient_geometry.empty()) {
			ar.set_position_array(ctx, &nml_gradient_geometry.front(), nml_gradient_geometry.size() / 2, 2 * sizeof(vec3));
			ar.set_end_point_array(ctx, &nml_gradient_geometry[1], nml_gradient_geometry.size() / 2, 2 * sizeof(vec3));
			if (ar.validate_and_enable(ctx)) {
				ctx.set_color(rgb(1, 0.5f, 0));
				ar.draw(ctx, 0, nml_gradient_geometry.size() / 2);
				ar.disable(ctx);
			}
		}
		if (show_mesh_normals && !nml_mesh_geometry.empty()) {
			ar.set_position_array(ctx, &nml_mesh_geometry.front(), nml_mesh_geometry.size() / 2, 2 * sizeof(vec3));
			ar.set_end_point_array(ctx, &nml_mesh_geometry[1], nml_mesh_geometry.size() / 2, 2 * sizeof(vec3));
			if (ar.validate_and_enable(ctx)) {
				ctx.set_color(rgb(1, 0, 1));
				ar.draw(ctx, 0, nml_mesh_geometry.size() / 2);
				ar.disable(ctx);
			}
		}
	}
	if (show_sampling_locations) {
		vec3 p = box.get_corner(0);
		vec3 q = box.get_corner(7);
		vec3 d = box.get_extent();
		d /= float(res - 1);
		std::vector<vec3> G;
		std::vector<vec3> G_out;
		for (unsigned int i = 0; i < res; ++i)
			for (unsigned int j = 0; j < res; ++j)
				for (unsigned int k = 0; k < res; ++k) {
					vec3 r(p(0) + i * d(0), p(1) + j * d(1), p(2) + k * d(2));
					if (func_ptr->evaluate(r.to_vec()) > 0)
						G_out.push_back(r);
					else
						G.push_back(r);
				}
		auto& sr = ref_sphere_renderer(ctx);
		sr.set_render_style(srs);
		sr.set_position_array(ctx, G);
		if (sr.validate_and_enable(ctx)) {
			ctx.set_color(rgb(1, 0.3f, 0.3f));
			sr.draw(ctx, 0, G.size());
			sr.disable(ctx);
		}
		sr.set_position_array(ctx, G_out);
		if (sr.validate_and_enable(ctx)) {
			ctx.set_color(rgb(0.3f, 0.3f, 1));
			sr.draw(ctx, 0, G_out.size());
			sr.disable(ctx);
		}
	}
}

void gl_implicit_surface_drawable_base::finish_frame(context& ctx)
{
	if (show_sampling_grid) {
		vec3 p = box.get_corner(0);
		vec3 q = box.get_corner(7);
		vec3 d = box.get_extent();
		d /= float(res - 1);
		std::vector<vec3> G;
		G.clear();
		for (unsigned int i=0; i<res; ++i)
			for (unsigned int j=0; j<res; ++j) {
				G.push_back(vec3(p(0) + i * d(0), p(1) + j * d(1), p(2)));
				G.push_back(vec3(p(0) + i * d(0), p(1) + j * d(1), q(2)));
				G.push_back(vec3(p(0), p(1) + i * d(1), p(2) + j * d(2)));
				G.push_back(vec3(q(0), p(1) + i * d(1), p(2) + j * d(2)));
				G.push_back(vec3(p(0) + i * d(0), p(1), p(2) + j * d(2)));
				G.push_back(vec3(p(0) + i * d(0), q(1), p(2) + j * d(2)));
			}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		auto& cr = ref_rounded_cone_renderer(ctx);
		cr.set_render_style(crs);
		cr.set_position_array(ctx, G);
		if (cr.validate_and_enable(ctx)) {
			ctx.set_color(rgba(0.7f, 0.7f, 0.7f, sampling_grid_alpha));
			cr.draw(ctx, 0, G.size());
			cr.disable(ctx);
		}
		glDisable(GL_BLEND);
	}
}

void gl_implicit_surface_drawable_base::draw_implicit_surface(context& ctx)
{
	if (show_vertices) {
		sphere_renderer& sr = ref_sphere_renderer(ctx);
		sr.set_render_style(srs);
		sr.set_position_array(ctx, mesh.get_positions());
		sr.render(ctx, 0, mesh.get_nr_positions());
	}
	if (show_wireframe) {
		rounded_cone_renderer& cr = ref_rounded_cone_renderer(ctx);
		cr.set_render_style(crs);
		if (cr.enable(ctx)) {
			mri.draw_wireframe(ctx);
			cr.disable(ctx);
		}
	}
	if (show_surface && (mesh.get_nr_faces() > 0)) {
		GLboolean cull_face;
		glGetBooleanv(GL_CULL_FACE, &cull_face);
		ctx.set_material(material);
		glDisable(GL_CULL_FACE);
		ctx.ref_surface_shader_program(false).set_uniform(ctx, "illumination_mode", 2);
		mri.draw_all(ctx);
		if (cull_face)
			glEnable(GL_CULL_FACE);
	}
}


		}
	}
}
