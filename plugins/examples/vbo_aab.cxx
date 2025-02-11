#include <limits>
#include <cgv/base/node.h>
#include <cgv/defines/quote.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/drawable.h>
#include <cgv/render/clipped_view.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/gui/provider.h>
#include <cgv/media/color_scale.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cgv/media/illum/surface_material.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::math;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::illum;

// define maximum index as restart index
#define RESTART_INDEX std::numeric_limits<cgv::type::uint32_type>::max()

class parametric_surface
{
protected:
	const float   eps  = 10.0f*std::numeric_limits<float>::epsilon();
	const float i2eps  = 0.5f / eps;
	const float i4eps2 = 0.25f / (eps*eps);
public:
	virtual cgv::vec3 eval(float u, float v) const = 0;
	virtual cgv::vec3 diff_u(float u, float v) const 
	{
		return i2eps*(eval(u+eps,v) - eval(u-eps,v));
	}
	virtual cgv::vec3 diff_v(float u, float v) const
	{
		return i2eps * (eval(u,v + eps) - eval(u,v - eps));
	}
	virtual bool compute_normal(float u, float v, cgv::vec3& nml) const
	{
		nml = cross(diff_u(u, v), diff_v(u, v));
		return nml.safe_normalize() != 0;
	}
	virtual cgv::vec3 diff_uu(float u, float v) const 
	{
		return i4eps2 * (eval(u + eps, v) - 2.0f*eval(u, v) + eval(u - eps, v));
	}
	virtual cgv::vec3 diff_vv(float u, float v) const 
	{
		return i4eps2 * (eval(u, v + eps) - 2.0f*eval(u, v) + eval(u, v - eps));
	}
	virtual cgv::vec3 diff_uv(float u, float v) const
	{
		return i2eps * (diff_v(u + eps, v) - diff_v(u - eps, v));
	}
	virtual cgv::vec3 diff_vu(float u, float v) const
	{
		return i2eps * (diff_u(u, v + eps) - diff_u(u, v - eps));
	}
	virtual bool compute_fst_fundamental(float u, float v, float& E, float& F, float& G) const
	{
		cgv::vec3 su = diff_u(u, v);
		cgv::vec3 sv = diff_v(u, v);
		E = dot(su, su);
		F = dot(su, sv);
		G = dot(sv, sv);
		return true;
	}
	virtual bool compute_fst_fundamental(float u, float v, cgv::mat2& I) const
	{
		float E, F, G;
		if (!compute_fst_fundamental(u, v, E, F, G))
			return false;
		I = cgv::mat2(cgv::vec2(E, F), cgv::vec2(F, G));
		return true;
	}
	virtual bool compute_snd_fundamental(float u, float v, float& L, float& M, float& N) const
	{
		cgv::vec3 nml;
		if (!compute_normal(u, v, nml))
			return false;
		cgv::vec3 suu = diff_uu(u, v);
		cgv::vec3 suv = diff_uv(u, v);
		cgv::vec3 svv = diff_vv(u, v);
		L = dot(suu, nml);
		M = dot(suv, nml);
		N = dot(svv, nml);
		return true;
	}
	virtual bool compute_snd_fundamental(float u, float v, cgv::mat2& II) const
	{
		float L, M, N;
		if (!compute_snd_fundamental(u, v, L, M, N))
			return false;
		II = cgv::mat2(cgv::vec2(L, M), cgv::vec2(M, N));
		return true;
	}
};

class sphere_surface : public parametric_surface
{
protected:
	const float pi = (float)M_PI;
public:
	cgv::vec3 eval(float u, float v) const {
		float a = 2 * pi * u;
		float b = pi * (v - 0.5f);
		float sa = sin(a);
		float ca = cos(a);
		float sb = sin(b);
		float cb = cos(b);
		return cgv::vec3(ca * cb, sa * cb, sb);
	}
	bool compute_normal(float u, float v, cgv::vec3& nml) const
	{
		nml = eval(u, v);
		return true;
	}
};

class torus_surface : public parametric_surface
{
protected:
	const float pi = (float)M_PI;
	const float r = 0.5f;
	const float R = 2.0f;
public:
	virtual cgv::vec3 eval(float u, float v) const {
		float a = 2*pi * u;
		float b = 2*pi * v;
		float sa = sin(a);
		float ca = cos(a);
		float sb = sin(b);
		float cb = cos(b);
		return cgv::vec3((r*cb+R)*ca, (r * cb + R) * sa, r*sb);
	}
};
/// geometry computation of Klein bottle inspired by https://commons.wikimedia.org/wiki/File:Klein_bottle.svg
class klein_bottle : public parametric_surface
{
protected:
	const float pi = (float)M_PI;
	void prepare(float u, float v, float& a, float& b, float& sa, float& ca, float& sb, float& cb) const
	{
		b = 4 * pi * v;
		sb = sin(b);
		cb = cos(b);
		a = 2 * pi * u;
		sa = sin(a);
		ca = cos(a);
	}
	cgv::vec3 eval(float u, float v, float a, float b, float sa, float ca, float sb, float cb) const {
		return v < 0.25 ? cgv::vec3((2.5 - 1.5 * cb) * ca, (2.5 - 1.5 * cb) * sa, -2.5 * sb) : (
			   v < 0.50 ? cgv::vec3((2.5 - 1.5 * cb) * ca, (2.5 - 1.5 * cb) * sa, 3 * b - 3 * pi) : (
			   v < 0.75 ? cgv::vec3(-2 + (2 + ca) * cb, sa, (2 + ca) * sb + 3 * pi) :
					      cgv::vec3(-2 + 2 * cb - ca, sa, -3 * b + 12 * pi)));
	}
	cgv::vec3 diff_u(float u, float v, float a, float b, float sa, float ca, float sb, float cb) const {
		return v < 0.25 ? cgv::vec3(-pi*(5 - 3 * cb) * sa, pi*(5 - 3 * cb) * ca, 0) : (
			   v < 0.50 ? cgv::vec3(-pi*(5 - 3 * cb) * sa, pi*(5 - 3 * cb) * ca, 0) : (
			   v < 0.75 ? cgv::vec3(-2*pi*cb*sa, 2*pi*ca, -2*pi*sb*sa) :
					      cgv::vec3(2*pi*sa, 2*pi*ca, 0)));
	}
	cgv::vec3 diff_v(float u, float v, float a, float b, float sa, float ca, float sb, float cb) const {
		return v < 0.25 ? cgv::vec3(6*pi*ca*sb, 6*pi*sa*sb, -10*pi*cb) : (
			   v < 0.50 ? cgv::vec3(6*pi*ca*sb, 6*pi*sa*sb, 12*pi) : (
			   v < 0.75 ? cgv::vec3(-4*pi*ca*sb, 0, 4*pi*ca*cb) :
					      cgv::vec3(-8*pi*sb, 0, -12*pi)));
	}
	bool compute_normal(float u, float v, float a, float b, float sa, float ca, float sb, float cb, cgv::vec3& nml) const {
		nml = cross(diff_u(u, v, a, b, sa, ca, sb, cb), diff_v(u, v, a, b, sa, ca, sb, cb));
		return nml.safe_normalize();
	}
	cgv::vec3 diff_uu(float u, float v, float a, float b, float sa, float ca, float sb, float cb) const {
		return v < 0.25 ? cgv::vec3(-pi*pi*(10-6*cb)*ca, -pi*pi*(10-6*cb)*sa, 0) : (
			   v < 0.50 ? cgv::vec3(-pi*pi*(10-6*cb)*ca, -pi*pi*(10-6*cb)*sa, 0) : (
			   v < 0.75 ? cgv::vec3(-4*pi*pi*cb*ca, -4*pi*pi*sa, -4*pi*pi*sb*ca) :
					      cgv::vec3(4*pi*pi*ca, -4*pi*pi*sa, 0)));
	}
	cgv::vec3 diff_uv(float u, float v, float a, float b, float sa, float ca, float sb, float cb) const {
		return v < 0.25 ? cgv::vec3(-12*pi*pi*sa*sb, 12*pi*pi*ca*sb, 0) : (
			   v < 0.50 ? cgv::vec3(-12*pi*pi*sa*sb, 12*pi*pi*ca*sb, 0) : (
			   v < 0.75 ? cgv::vec3(  8*pi*pi*sa*sb, 0, -8*pi*pi*sa*cb) :
					      cgv::vec3(0, 0, 0)));
	}
	cgv::vec3 diff_vv(float u, float v, float a, float b, float sa, float ca, float sb, float cb) const {
		return v < 0.25 ? cgv::vec3(24*pi*pi*ca*cb, 24*pi*pi*sa*cb, 40*pi*pi*sb) : (
			   v < 0.50 ? cgv::vec3(24*pi*pi*ca*cb, 24*pi*pi*sa*cb, 0) : (
			   v < 0.75 ? cgv::vec3(-16*pi*pi*ca*cb, 0, -16*pi*pi*ca*sb) :
					      cgv::vec3(-32*pi*pi*cb, 0, 0)));
	}
public:
	cgv::vec3 eval(float u, float v) const {
		float a, b, sa, ca, sb, cb;
		prepare(u,v, a,b,sa,ca,sb,cb);
		return eval(u,v, a,b,sa,ca,sb,cb);
	}
	cgv::vec3 diff_u(float u, float v) const {
		float a, b, sa, ca, sb, cb;
		prepare(u,v, a,b,sa,ca,sb,cb);
		return diff_u(u,v, a,b,sa,ca,sb,cb);
	}
	cgv::vec3 diff_v(float u, float v) const
	{
		float a, b, sa, ca, sb, cb;
		prepare(u,v, a,b,sa,ca,sb,cb);
		return diff_v(u,v, a,b,sa,ca,sb,cb);
	}
	bool compute_normal(float u, float v, cgv::vec3& nml) const {
		float a, b, sa, ca, sb, cb;
		prepare(u,v, a,b,sa,ca,sb,cb);
		return compute_normal(u,v, a,b,sa,ca,sb,cb, nml);
	}
	cgv::vec3 diff_uu(float u, float v) const {
		float a, b, sa, ca, sb, cb;
		prepare(u, v, a, b, sa, ca, sb, cb);
		return diff_uu(u, v, a, b, sa, ca, sb, cb);
	}
	cgv::vec3 diff_uv(float u, float v) const
	{
		float a, b, sa, ca, sb, cb;
		prepare(u, v, a, b, sa, ca, sb, cb);
		return diff_uv(u, v, a, b, sa, ca, sb, cb);
	}
	cgv::vec3 diff_vv(float u, float v) const
	{
		float a, b, sa, ca, sb, cb;
		prepare(u, v, a, b, sa, ca, sb, cb);
		return diff_vv(u, v, a, b, sa, ca, sb, cb);
	}
	virtual bool compute_fst_fundamental(float u, float v, float& E, float& F, float& G) const
	{
		float a, b, sa, ca, sb, cb;
		prepare(u, v, a, b, sa, ca, sb, cb);
		cgv::vec3 su = diff_u(u, v, a, b, sa, ca, sb, cb);
		cgv::vec3 sv = diff_v(u, v, a, b, sa, ca, sb, cb);
		E = dot(su, su);
		F = dot(su, sv);
		G = dot(sv, sv);
		return true;
	}
	virtual bool compute_snd_fundamental(float u, float v, float& L, float& M, float& N) const
	{
		float a, b, sa, ca, sb, cb;
		prepare(u, v, a, b, sa, ca, sb, cb);
		cgv::vec3 nml;
		if (!compute_normal(u, v, a, b, sa, ca, sb, cb, nml))
			return false;
		cgv::vec3 suu = diff_uu(u, v, a, b, sa, ca, sb, cb);
		cgv::vec3 suv = diff_uv(u, v, a, b, sa, ca, sb, cb);
		cgv::vec3 svv = diff_vv(u, v, a, b, sa, ca, sb, cb);
		L = dot(suu, nml);
		M = dot(suv, nml);
		N = dot(svv, nml);
		return true;
	}
};


/*

This example illustrates how to construct vertex buffer objects containing vertex
attribute arrays as well as element indices for triangle strip and line strip
rendering of surface and wireframe.

The typical workflow is

PREPARATION (implemented in init or init_frame method of drawable)
- compute vertex attribute arrays and element indices in std::vector<T> 
  containers on the GPU. In this example attributes are stored in an 
  interleaved fashion, implemented with the help of a vertex_type struct. 
- create vertex buffers from std::vector<T> containers.
  Careful with the element indices. These have to be stored in a separate
  vertex buffer that is marked to be of type VBT_INDICES.
- choose a shader program. Here the default shader programs provided by
  the context are used.
- bind array pointers at location indices queried from program to vertex buffers
  in an attribute_array_binding struct

RENDERING PHASE (implemented in draw and finish_draw method)
- the following steps should be done for the opaque parts in the draw() and
  for the transparent parts in the finish_draw() method
- enable shader program and configure its uniforms
- enable atribute array binding
- draw elements
- disable atribute array binding
- disable shader program

This example illustrates furthermore
- how to use glPrimitiveRestartIndex()/glEnable(GL_PRIMITIVE_RESTART) to
  render several strips with one draw call
- how to set attribute arrays of an attribute array manager of a renderer 
  to vertex buffer objects. This is done for vertex rendering with a sphere
  shader. The general approach is:

  PREPARATION

  - create your own attribute_array_manager in drawable::init method
  - choose renderer and set attribute_array_manager pointer of renderer
    to your attribute array manager
  - set attributes in renderer which automatically sets them in your 
    attribute array manager
  - set attribute_array_manager pointer of renderer to nullptr

  RENDERING

  - set attribute_array_manager pointer of renderer
    to your attribute array manager
  - enable renderer
  - draw primitives
  - disable renderer
  - set attribute_array_manager pointer of renderer to nullptr
*/
class vbo_aab : public node, public drawable, public provider
{
protected:
	bool generate_new_geometry;
	size_t nr_positions;
	size_t nr_triangle_elements, nr_line_elements;
	/// one vbo for all vertex attributes stored interleaved
	cgv::render::vertex_buffer vbo_attribs;
	/// one vbo for element indices used for triangle strip rendering
	cgv::render::vertex_buffer vbo_elements;
	///
	cgv::render::attribute_array_manager aam_sphere;
	///
	cgv::render::attribute_array_binding aab_surface;
	///
	cgv::render::attribute_array_binding aab_wireframe;
public:
	bool show_surface;
	CullingMode cull_mode;
	ColorMapping color_mapping;
	cgv::rgb  surface_color;
	IlluminationMode illumination_mode;
	surface_material material;

	bool auto_set_radius = true;
	bool auto_set_view = true;
	bool show_wireframe;
	float line_width;
	bool map_color_to_wireframe;
	cgv::rgb  line_color;

	bool show_vertices;
	cgv::render::sphere_render_style sphere_style;

	// mesh generation parameters
	int n, m;
	float a, b;
	float lb, ub;
	cgv::box2 domain = { cgv::vec2(0.0f), cgv::vec2(1.0f) };
	enum SurfaceType {
		ST_SPHERE,
		ST_TORUS,
		ST_DINI,
		ST_KLEIN
	} surface_type = ST_KLEIN;
public:
	vbo_aab() : node("vbo_aab")
	{
		show_surface = true;
		cull_mode = CM_BACKFACE;
		color_mapping = cgv::render::CM_COLOR;
		surface_color = cgv::rgb(0.7f, 0.2f, 1.0f);
		illumination_mode = IM_ONE_SIDED;

		show_wireframe = true;
		line_width = 2.0f;
		line_color = cgv::rgb(0.6f,0.5f,0.4f);
		map_color_to_wireframe = false;

		show_vertices = true;
		a = 1;
		b = 0.2f;
		lb = 0.01f;
		ub = 2.0f;
		n = m = 20;

		generate_new_geometry = true;
	}
	struct vertex_type
	{
		cgv::vec3 position;
		cgv::vec3 normal;
		cgv::rgb  color;
	};
	cgv::box3 construct_vertices(const parametric_surface& S, cgv::box2 dom, int n, int m, int N, int M, std::vector<vertex_type>& V) const
	{
		cgv::box3 box;
		V.resize(N * M);
		auto e = dom.get_extent();
		float in = 1.0f / n;
		float im = 1.0f / m;
		for (int j = 0; j < M; ++j) {
			float v = im * j;
			float b = e(1)*v + dom.get_min_pnt()(1);
			for (int i = 0; i < N; ++i) {
				float u = in * i;
				float a = e(0)*u + dom.get_min_pnt()(0);
				int vi = N * j + i;
				vertex_type& vertex = V[vi];
				vertex.position = S.eval(a,b);
				S.compute_normal(a,b, vertex.normal);
				vertex.color = cgv::rgb(u, v, 0.0f);
				// update bounding box
				box.add_point(vertex.position);
			}
		}
		return box;
	}
	void construct_indices(int N, int M, std::vector<cgv::type::uint32_type>& indices, size_t& nr_triangle_elements, size_t& nr_line_elements) const
	{
		int i, j;
		for (j = 1; j < M; ++j) {
			if (j > 1)
				indices.push_back(RESTART_INDEX);
			for (i = 0; i < N; ++i) {
				indices.push_back(N * j + i);
				indices.push_back(N *(j-1) + i);
			}
		}
		nr_triangle_elements = indices.size();

		// generate u parameter lines
		for (j = 0; j < M; ++j) {
			if (j > 0)
				indices.push_back(RESTART_INDEX);
			for (int i = 0; i < N; ++i)
				indices.push_back(N*j + i);
		}
		for (i = 0; i < N; ++i) {
			indices.push_back(RESTART_INDEX);
			for (int j = 0; j < M; ++j)
				indices.push_back(N*j + i);
			//indices.push_back(N*j);
		}
		nr_line_elements = indices.size() - nr_triangle_elements;
	}
	cgv::box3 generate_surface_geometry(const parametric_surface& S, cgv::render::context& ctx)
	{
		int N = n+1, M = m+1;
		std::vector<vertex_type> V;
		cgv::box3 box =	construct_vertices(S, domain, n, m, N, M, V);
		std::vector<cgv::type::uint32_type> indices;
		construct_indices(N, M, indices, nr_triangle_elements, nr_line_elements);
		construct_vbo_and_aab(ctx, V, indices);
		return box;
	}
	cgv::box3 generate_dini_surface_geometry(cgv::render::context& ctx)
	{
		cgv::box3 box;
		box.invalidate();

		std::vector<vertex_type> V((n + 1)*m);
		std::vector<cgv::type::uint32_type> indices;

		// allocate per vertex colors of type rgb with float components
		int i;
		for (i = 0; i <= n; ++i) {
			float y = (float)i / n;
			float v = (ub - lb)*y + lb;
			if (i > 1)
				indices.push_back(RESTART_INDEX);
			for (int j = 0; j < m; ++j) {
				float x = (float)j / m;
				float u = float(4.0f*M_PI)*x;
				// add new position to the mesh (function returns position index, which is i*m+j in our case)

				int vi = i * m + j;
				vertex_type& vertex = V[vi];
				vertex.position = cgv::vec3(a*cos(u)*sin(v), a*sin(u)*sin(v), a*(cos(v) + log(tan(0.5f*v))) + b * u);

				float t1 = cos(v);
				float t2 = a * (-pow(t1, 0.2e1f) + 0.1e1f);
				float t3 = cos(u);
				float t4 = sin(u);
				float t5 = v / 0.2e1f;
				float t6 = sin(t5);
				t5 = cos(t5);
				float t7 = sin(v);
				float t8 = a / 0.2e1f;
				float t9 = 0.1e1f / t5;
				float t10 = 0.1e1f / t6;
				vertex.normal = cgv::vec3(
					a * (t8 * t3 * t7 - (b * t4 * t1 + t2 * t3) * t6 * t5) * t9 * t10,
					(t8 * t4 * t7 - (-b * t3 * t1 + t2 * t4) * t6 * t5) * a * t9 * t10,
					-t7 * t1 * a * a);
				vertex.normal.normalize();

				/* 
				// uncomment to generate cylinder 
				vertex.position = vec3(cos(0.5f*u), sin(0.5f*u), y);
				vertex.normal = vec3(cos(0.5f*u), sin(0.5f*u), 0);
				*/
				
				// set color
				vertex.color = cgv::rgb(x, y, 0.5f);

				// update bounding box
				box.add_point(vertex.position);

				// add two vertex indices to triangle strips
				if (i > 0) {
					indices.push_back(vi);
					indices.push_back(vi - m);
				}
			}
			if (i > 0) {
				indices.push_back(i*m);
				indices.push_back((i-1)*m);
			}
		}
		nr_triangle_elements = indices.size();

		// generate elements for wireframe rendering
		for (i = 0; i <= n; ++i) {
			if (i > 0)
				indices.push_back(RESTART_INDEX);
			for (int j = 0; j < m; ++j) 
				indices.push_back(i*m + j);
			indices.push_back(i*m);
		}
		for (i = 0; i < m; ++i) {
			indices.push_back(RESTART_INDEX);
			for (int j = 0; j <= n; ++j)
				indices.push_back(j*m + i);
		}
		nr_line_elements = indices.size() - nr_triangle_elements;
		construct_vbo_and_aab(ctx, V, indices);
		return box;
	}
	void construct_vbo_and_aab(
		cgv::render::context& ctx, 
		const std::vector<vertex_type>& V, 
		std::vector<cgv::type::uint32_type>& indices)
	{
		// destruct previously allocated vertex buffers
		vbo_attribs.destruct(ctx);
		vbo_elements.destruct(ctx);
		nr_positions = V.size();

		// transfer data into vertex buffers
		vbo_attribs.create(ctx, V);
		vbo_elements.type = cgv::render::VBT_INDICES; // mark buffer as index buffer
		vbo_elements.create(ctx, indices);

		// configure the attribute manager
		auto& sr = ref_sphere_renderer(ctx);
		sr.enable_attribute_array_manager(ctx, aam_sphere);
			sr.set_position_array(ctx, get_element_type(V.front().position), 
				vbo_attribs, 0, V.size(), sizeof(vertex_type));
			sr.set_color_array(ctx, get_element_type(V.front().color),
				vbo_attribs, 2*sizeof(cgv::vec3), V.size(), sizeof(vertex_type));
		sr.disable_attribute_array_manager(ctx, aam_sphere);

		// configure attribute array binding for surface rendering
		auto& pr = ctx.ref_surface_shader_program(false);
		aab_surface.bind_attribute_array(ctx, pr, "position",
			get_element_type(V.front().position), vbo_attribs, 
			0, nr_positions, sizeof(vertex_type));
		aab_surface.bind_attribute_array(ctx, pr, "normal",
			get_element_type(V.front().normal), vbo_attribs, 
			offsetof(vertex_type, normal), nr_positions, sizeof(vertex_type));
		aab_surface.bind_attribute_array(ctx, pr, "color",
			get_element_type(V.front().color), vbo_attribs, 
			offsetof(vertex_type, color), nr_positions, sizeof(vertex_type));
		aab_surface.set_element_array(ctx, vbo_elements);

		// configure attribute array binding for wireframe rendering
		auto& prw = ctx.ref_default_shader_program(false);
		aab_wireframe.bind_attribute_array(ctx, prw, "position",
			get_element_type(V.front().position), vbo_attribs, 
			0, nr_positions, sizeof(vertex_type));
		aab_wireframe.bind_attribute_array(ctx, prw, "color",
			get_element_type(V.front().color), vbo_attribs, 
			offsetof(vertex_type, color), nr_positions, sizeof(vertex_type));
		aab_wireframe.set_element_array(ctx, vbo_elements);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &surface_type ||
			member_ptr == &n || member_ptr == &m ||
			member_ptr == &lb || member_ptr == &ub ||
			member_ptr == &a || member_ptr == &b ||
			(member_ptr >= &domain && member_ptr < &domain+1)) {
			generate_new_geometry = true;
		}
		if (member_ptr == &sphere_style.radius) {
			auto_set_radius = false;
			update_member(&auto_set_radius);
		}
		update_member(member_ptr);
		post_redraw();
	}
	void create_gui()
	{
		add_decorator("vbo_aab", "heading", "level=2");
		if (begin_tree_node("generate", a, true)) {
			align("\a");
			add_member_control(this, "surface", surface_type, "dropdown", "enums='Sphere,Torus,Dini,Klein'");
			add_gui("domain", domain, "", "options='min=0;max=1;ticks=true'");
			add_member_control(this, "a", a, "value_slider", "min=0.1;max=10;ticks=true;log=true");
			add_member_control(this, "b", b, "value_slider", "min=0.1;max=10;ticks=true;log=true");
			add_member_control(this, "lb", lb, "value_slider", "min=0.001;step=0.0001;max=1;ticks=true;log=true");
			add_member_control(this, "ub", ub, "value_slider", "min=1;max=10;ticks=true;log=true");
			add_member_control(this, "n", n, "value_slider", "min=5;max=100;ticks=true;log=true");
			add_member_control(this, "m", m, "value_slider", "min=5;max=100;ticks=true;log=true");
			add_member_control(this, "set View", auto_set_view, "toggle");
			add_member_control(this, "set Radius", auto_set_radius, "toggle");
			align("\b");
			end_tree_node(a);
		}
		bool show = begin_tree_node("vertex spheres", show_vertices, false, "options='w=140';align=' '");
		add_member_control(this, "show", show_vertices, "toggle", "w=52;shortcut='w'");
		if (show) {
			align("\a");
			add_gui("style", sphere_style);
			align("\b");
			end_tree_node(show_wireframe);
		}

		show = begin_tree_node("wireframe", show_wireframe, true, "options='w=140';align=' '");
		add_member_control(this, "show", show_wireframe, "toggle", "w=52;shortcut='w'");
		if (show) {
			align("\a");
			add_member_control(this, "line width", line_width, "value_slider", "min=1;max=20;ticks=true;log=true");
			add_member_control(this, "line color", line_color);
			add_member_control(this, "map_color_to_wireframe", map_color_to_wireframe, "check");
			align("\b");
			end_tree_node(show_wireframe);
		}

		show = begin_tree_node("surface", show_surface, true, "options='w=140';align=' '");
		add_member_control(this, "show", show_surface, "toggle", "w=52;shortcut='s'");
		if (show) {
			align("\a");
			add_member_control(this, "cull mode", cull_mode, "dropdown", "enums='none,back,front'");
			if (begin_tree_node("color_mapping", color_mapping)) {
				align("\a");
				add_gui("color mapping", color_mapping, "bit_field_control",
					"enums='COLOR_FRONT=1,COLOR_BACK=2,OPACITY_FRONT=4,OPACITY_BACK=8'");
				align("\b");
				end_tree_node(color_mapping);
			}
			add_member_control(this, "surface color", surface_color);
			add_member_control(this, "illumination", illumination_mode, "dropdown", "enums='none,one sided,two sided'");
			if (begin_tree_node("material", material)) {
				align("\a");
				add_gui("material", material);
				align("\b");
				end_tree_node(material);
			}
			align("\b");
			end_tree_node(show_surface);
		}
	}
	bool init(context& ctx)
	{
		aam_sphere.init(ctx);
		aab_surface.create(ctx);
		aab_wireframe.create(ctx);
		ref_sphere_renderer(ctx, 1);
		return true;
	}
	void destruct(context& ctx)
	{
		vbo_attribs.destruct(ctx);
		vbo_elements.destruct(ctx);
		aab_surface.destruct(ctx);
		aab_wireframe.destruct(ctx);
		aam_sphere.destruct(ctx);
		ref_sphere_renderer(ctx, -1);
	}
	void init_frame(context& ctx)
	{
		if (generate_new_geometry) {
			cgv::box3 box;
			switch (surface_type) {
			case ST_SPHERE: box = generate_surface_geometry(sphere_surface(), ctx); break;
			case ST_TORUS:  box = generate_surface_geometry(torus_surface(), ctx); break;
			case ST_DINI: box = generate_dini_surface_geometry(ctx); break;
			case ST_KLEIN:  box = generate_surface_geometry(klein_bottle(), ctx); break;
			}
			generate_new_geometry = false;

			if (auto_set_radius)
				sphere_style.radius = float(0.05*sqrt(box.get_extent().sqr_length() / nr_positions));

			if (auto_set_view) {
				// focus view on new mesh
				clipped_view* view_ptr = dynamic_cast<clipped_view*>(find_view_as_node());
				if (view_ptr) {
					view_ptr->set_scene_extent(box);
					view_ptr->set_focus(box.get_center());
					view_ptr->set_y_extent_at_focus(box.get_extent().length());
				}
			}
		}
	}
	void draw_surface(context& ctx)
	{
		// remember current culling setting
		GLboolean is_culling = glIsEnabled(GL_CULL_FACE);
		GLint cull_face;
		glGetIntegerv(GL_CULL_FACE_MODE, &cull_face);

		// ensure that opengl culling is identical to shader program based culling
		if (cull_mode > 0) {
			glEnable(GL_CULL_FACE);
			glCullFace(cull_mode == CM_BACKFACE ? GL_BACK : GL_FRONT);
		}
		else
			glDisable(GL_CULL_FACE);

		// choose a shader program
		shader_program& prog = ctx.ref_surface_shader_program(false);

		// enable program and configure it based on current settings
		prog.enable(ctx);
		prog.set_uniform(ctx, "culling_mode", (int)cull_mode); // fragment culling mode
		prog.set_uniform(ctx, "map_color_to_material", (int)color_mapping); // fragment color mapping mode
		prog.set_uniform(ctx, "illumination_mode", (int)illumination_mode); // fragment illumination mode

		// the set_uniform method only supports basic types and no structs like the material, where you would have
		// to set each member individually. Therefore the context provides a method that sets all material parameters
		// of the currently enabled program as used here
		ctx.set_material(material);
		// set default surface color for color mapping which only affects 
		// rendering if mesh does not have per vertex colors and color_mapping is on
		prog.set_attribute(ctx, prog.get_color_index(), surface_color);

		aab_surface.enable(ctx);
		glDrawElements(GL_TRIANGLE_STRIP, (GLsizei) nr_triangle_elements, GL_UNSIGNED_INT, 0);
		aab_surface.disable(ctx);

		prog.disable(ctx);

		// recover opengl culling mode
		if (is_culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		glCullFace(cull_face);
	}

	void draw(context& ctx)
	{
		if (show_vertices) {
			sphere_renderer& sr = ref_sphere_renderer(ctx);
			sr.enable_attribute_array_manager(ctx, aam_sphere);
			sr.set_render_style(sphere_style);
			sr.render(ctx, 0, nr_positions);
			sr.disable_attribute_array_manager(ctx, aam_sphere);
		}

		// remember restart setting
		GLboolean is_restarting = glIsEnabled(GL_PRIMITIVE_RESTART);
		GLint restart_index;
		glGetIntegerv(GL_PRIMITIVE_RESTART_INDEX, &restart_index);

		// enable my own restart index
		glPrimitiveRestartIndex(RESTART_INDEX);
		glEnable(GL_PRIMITIVE_RESTART);

		if (show_wireframe) {
			// remember current line width
			GLfloat old_line_width;
			glGetFloatv(GL_LINE_WIDTH, &old_line_width);
			glLineWidth(line_width);

			// choose default shader program
			cgv::render::shader_program& prog = ctx.ref_default_shader_program();

			// enable aab and program (order is not important and could be swapped)
			aab_wireframe.enable(ctx);
			prog.enable(ctx);

				// in case of color mapping, use color attribute array
				if (map_color_to_wireframe)
					aab_wireframe.enable_array(ctx, prog.get_color_index());
				// otherwise turn color attribute array off and 
				// use set_color method of context to set vertex attribute to single value line_color
				else {
					aab_wireframe.disable_array(ctx, prog.get_color_index());
					ctx.set_color(line_color);
				}
				// draw all line strips with one draw call where the last pointer casted argument 
				// is the offset in the element buffer, where the line strips begin
				glDrawElements(GL_LINE_STRIP, (GLsizei) nr_line_elements, GL_UNSIGNED_INT,
					(void*)(sizeof(cgv::type::uint32_type)*nr_triangle_elements));

			// disable program and aab - again order is unimportant
			prog.disable(ctx);
			aab_wireframe.disable(ctx);

			// recover old line width
			glLineWidth(old_line_width);
		}
		// render opaque surfaces here without blending
		if (show_surface && material.get_transparency() < 0.01f)
			draw_surface(ctx);
		// recover restart settings
		glPrimitiveRestartIndex(restart_index);
		if (!is_restarting)
			glDisable(GL_PRIMITIVE_RESTART);
	}
	void finish_draw(context& ctx)
	{
		// remember restart setting
		GLboolean is_restarting = glIsEnabled(GL_PRIMITIVE_RESTART);
		GLint restart_index;
		glGetIntegerv(GL_PRIMITIVE_RESTART_INDEX, &restart_index);

		// enable my own restart index
		glPrimitiveRestartIndex(RESTART_INDEX);
		glEnable(GL_PRIMITIVE_RESTART);

		// render surfaces with transparency here with blending enabled
		if (show_surface && !(ctx.get_current_material()->get_transparency() < 0.01f)) {				
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			draw_surface(ctx);
			glDisable(GL_BLEND);
		}
		// recover restart settings
		glPrimitiveRestartIndex(restart_index);
		if (!is_restarting)
			glDisable(GL_PRIMITIVE_RESTART);

	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<vbo_aab> vbo_aab_fac("New/Media/VBO AAB", 'V');
