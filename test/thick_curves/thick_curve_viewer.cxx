#include <cgv/defines/quote.h>
#include <cgv/utils/file.h>
#include <cgv/utils/stopwatch.h>
#include <cgv/base/group.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/clipped_view.h>
#include <cgv/math/fray.h>
#include <cgv/math/functions.h>
#include <cgv/math/intersection.h>
#include <cgv_gl/spline_tube_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv_gl/surfel_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cg_gizmo/transformation_gizmo.h>
#include "fpoly.h"
#include "cyPolynomial.h"

struct camera_manager
{
	std::string path = QUOTE_SYMBOL_VALUE(INPUT_DIR) "/cameras.bin";
	struct camera_info
	{
		cgv::render::clipped_view view;
		cgv::dmat4 M, P, MPW;
		cgv::uvec2 resolution;
	};
	std::vector<camera_info> cameras;
	int max_index() const { return int(cameras.empty() ? 0 : cameras.size() - 1); }
	void add_camera(const cgv::render::context& ctx, cgv::render::clipped_view* view_ptr) {
		camera_info I;
		I.view = *view_ptr;
		I.M = ctx.get_modelview_matrix();
		I.P = ctx.get_projection_matrix();
		I.MPW = ctx.get_modelview_projection_window_matrix();
		I.resolution = { ctx.get_width(), ctx.get_height() };
		cameras.emplace_back(I);
	}
	void sample_rays(const camera_info& I, std::vector<cgv::ray3>& rays, unsigned subsampling = 1) {
		cgv::uvec2 res = I.resolution / subsampling;
		cgv::dmat4 iMP = inverse(I.P * I.M);
		cgv::dvec4 p_clip = cgv::dvec4(1.0);
		p_clip[2] = 0.0;
		cgv::dvec3 eye = I.view.get_eye();
		for (unsigned y = 0; y < res[1]; ++y) {
			p_clip[1] = 1.0 - (y+0.5)*(2.0 / res[1]);
			for (unsigned x = 0; x < res[0]; ++x) {
				p_clip[0] = -1.0 + (x + 0.5) * (2.0 / res[0]);
				cgv::dvec4 p_world = iMP * p_clip;
				cgv::vec3 ray_dir = reinterpret_cast<cgv::dvec3&>(p_world) / p_world[3] - eye;
				ray_dir.normalize();
				rays.push_back(cgv::ray3(eye, ray_dir));
			}
		}
	}
	bool write(const std::string& file_name) const {
		FILE* fp = fopen(file_name.c_str(), "wb");
		if (!fp)
			return false;
		uint32_t cnt = uint32_t(cameras.size());
		if (!fwrite(&cnt, sizeof(uint32_t), 1, fp))
			return false;
		if (!fwrite(cameras.data(), sizeof(camera_info), cameras.size(), fp))
			return false;
		fclose(fp);
		return true;
	}
	bool read(const std::string& file_name) {
		FILE* fp = fopen(file_name.c_str(), "rb");
		if (!fp)
			return false;
		uint32_t cnt;
		if (!fread(&cnt, sizeof(uint32_t), 1, fp))
			return false;
		cameras.resize(cnt);
		if (!fread(cameras.data(), sizeof(camera_info), cameras.size(), fp))
			return false;
		fclose(fp);
		return true;
	}
};

cgv::mat4 transform_to_ray_coordinates(const cgv::ray3& r, const cgv::mat4& B_world)
{
	cgv::mat3 R;
	R.col(0) = r.direction;
	R.col(1) = normalize(ortho(r.direction));
	R.col(2) = cross(r.direction, R.col(1));
	cgv::mat4 B = B_world;
	cgv::vec3& b0 = reinterpret_cast<cgv::vec3&>(B.col(0));
	cgv::vec3& b1 = reinterpret_cast<cgv::vec3&>(B.col(1));
	cgv::vec3& b2 = reinterpret_cast<cgv::vec3&>(B.col(2));
	cgv::vec3& b3 = reinterpret_cast<cgv::vec3&>(B.col(3));
	b0 = (b0 - r.origin) * R;
	b1 = (b1 - r.origin) * R;
	b2 = (b2 - r.origin) * R;
	b3 = (b3 - r.origin) * R;
	return B;
}
const cgv::mat4& monom_from_bernstein_matrix()
{
	static cgv::mat4 T = { {1, 0, 0, 0}, { -3,3,0,0 }, { 3,-6,3,0 }, { -1,3,-3,1 } };
	return T;
}

template <typename T>
void matrix_to_polynomials(cgv::math::fmat<T, 4, 4> M, cy::Polynomial<T, 3>& cx,
	cy::Polynomial<T, 3>& cy, cy::Polynomial<T, 3>& cz, cy::Polynomial<T, 3>& ra)
{
	cx = { M(0,0), M(0,1), M(0,2), M(0,3) };
	cy = { M(1,0), M(1,1), M(1,2), M(1,3) };
	cz = { M(2,0), M(2,1), M(2,2), M(2,3) };
	ra = { M(3,0), M(3,1), M(3,2), M(3,3) };
}

template <typename T>
void matrix_to_polynomials(cgv::math::fmat<T,4,4> M, cgv::math::fpoly_mon<T, 3>& cx,
	cgv::math::fpoly_mon<T, 3>& cy, cgv::math::fpoly_mon<T, 3>& cz, cgv::math::fpoly_mon<T, 3>& ra)
{
	cx = { M(0,0), M(0,1), M(0,2), M(0,3) };
	cy = { M(1,0), M(1,1), M(1,2), M(1,3) };
	cz = { M(2,0), M(2,1), M(2,2), M(2,3) };
	ra = { M(3,0), M(3,1), M(3,2), M(3,3) };
}

template <typename T>
void consider_ray_local_sphere_intersection(const cgv::vec4& sphere, T t0, T* s, T* t, int& cnt)
{
	T x = sphere.x();
	T y = sphere.y();
	T z = sphere.z();
	T r = sphere.w();
	T q = r * r - y * y - z * z;
	if (q >= 0) {
		s[cnt] = x - sqrt(q);
		t[cnt++] = t0;
	}
}

bool compute_ray_sphere_intersection(const cgv::ray3& r, const cgv::mat4& B_world, std::vector<cgv::mat4x2>& intersections)
{
	bool found = false;
	cgv::mat4 B = transform_to_ray_coordinates(r, B_world);
	for (unsigned i = 0; i < 4; ++i) {
		float ra = B(3, i);
		float x = B(0, i);
		float y = B(1, i);
		float z = B(2, i);
		float d = ra * ra - y * y - z * z;
		if (d > 0.f) {
			float o = sqrt(d);
			float t = 0.333f * i;
			float s0 = x - o;
			float s1 = x + o;
			cgv::vec3 p0 = r.position(s0);
			cgv::vec3 p1 = r.position(s1);
			cgv::vec3 ctr = reinterpret_cast<const cgv::vec3&>(B_world.col(i));
			cgv::vec3 n0 = normalize(r.position(s0) - ctr);
			cgv::vec3 n1 = normalize(r.position(s1) - ctr);
			found = true;
			intersections.push_back({ cgv::vec4(p0,s0), cgv::vec4(n0,t) });
			intersections.push_back({ cgv::vec4(p1,s1), cgv::vec4(n1,t) });
		}
	}
	return found;
}
bool compute_ray_control_geometry_intersection(const cgv::ray3& r, const cgv::mat4& B_world, std::vector<cgv::mat4x2>& intersections)
{
	bool found = false;
	cgv::mat4 B = transform_to_ray_coordinates(r, B_world);
	for (unsigned i = 0; i < 3; ++i) {
		cgv::vec3 p0 = reinterpret_cast<cgv::vec3&>(B.col(i));
		cgv::vec3 p1 = reinterpret_cast<cgv::vec3&>(B.col(i+1));
		float r0 = B(3, i);
		float r1 = B(3, i+1);
		cy::Polynomial<double, 1> cx = { p0[0], p1[0]-p0[0] };
		cy::Polynomial<double, 1> cy = { p0[1], p1[1]-p0[1] };
		cy::Polynomial<double, 1> cz = { p0[2], p1[2]-p0[2] };
		cy::Polynomial<double, 1> ra = { r0, r1-r0 };
		// polynomial representing ortogonal distance to ray
		cy::Polynomial<double, 2> D = cy * cy + cz * cz;
		cy::Polynomial<double, 0> dcx = cx.Derivative();
		cy::Polynomial<double, 2> q = ra * ra - D;
		cy::Polynomial<double, 2> lhs = dcx * dcx * q;
		cy::Polynomial<double, 1> hdD = D.Derivative(); hdD *= 0.5;
		cy::Polynomial<double, 1> rhs_sqrt = ra * ra.Derivative() - hdD;
		cy::Polynomial<double, 2> p = lhs - rhs_sqrt * rhs_sqrt;
		double s[4], t[4];
		int cnt = p.Roots(t);
		consider_ray_local_sphere_intersection(cgv::vec4(p0,r0), 0.0, s, t, cnt);
		consider_ray_local_sphere_intersection(cgv::vec4(p1,r1), 1.0, s, t, cnt);
		int i_min = -1;
		for (int j = 0; j < cnt; ++j) {
			if (t[j] >= 0. && t[j] <= 1.) {
				s[j] = cx.Eval(t[j]) - sqrt(q.Eval(t[j]));
				if (s[j] > 0.0 && (i_min == -1 || s[j] < s[i_min]))
					i_min = j;
			}
		}

		if (i_min != -1) {
			cgv::vec3 b0 = reinterpret_cast<const cgv::vec3&>(B_world.col(i));
			cgv::vec3 b1 = reinterpret_cast<const cgv::vec3&>(B_world.col(i + 1));
			cgv::vec3 cp = lerp(b0,b1,(float)t[i_min]);
			cgv::vec3 ip = r.position(float(s[i_min]));
			cgv::mat4x2 I = { cgv::vec4(ip,s[i_min]), cgv::vec4(normalize(ip-cp),t[i_min]) };
			if (intersections.empty())
				intersections.push_back(I);
			else if (intersections.front()(3,0) > s[i_min])
				intersections.front() = I;
			found = true;
		}
	}
	return found;
}

template <typename T, int solver = 0>
bool compute_ray_sphere_tube_intersection(const cgv::ray3& r, const cgv::mat4& B_world, 
	std::vector<cgv::mat4x2>& intersections, T eps)
{
	cgv::mat4 B = transform_to_ray_coordinates(r, B_world);
	cgv::math::fmat<T, 4, 4> M = B * monom_from_bernstein_matrix();
	T t[12], s[12];
	int cnt = 0;
	if (solver == 0) {
		cy::Polynomial<T, 3> cx, cy, cz, ra;
		matrix_to_polynomials(M, cx, cy, cz, ra);
		// polynomial representing ortogonal distance to ray
		cy::Polynomial<T, 6> D = cy * cy + cz * cz;
		cy::Polynomial<T, 2> dcx = cx.Derivative();
		cy::Polynomial<T, 6> q = ra * ra - D;
		cy::Polynomial<T, 10> lhs = dcx * dcx * q;
		cy::Polynomial<T, 5> hdD = D.Derivative(); hdD *= 0.5;
		cy::Polynomial<T, 5> rhs_sqrt = ra * ra.Derivative() - hdD;
		cy::Polynomial<T, 10> p = lhs - rhs_sqrt * rhs_sqrt;
		cnt = p.Roots(t, 0.0, 1.0, eps);
		for (int i = 0; i < cnt; ++i)
			s[i] = cx.Eval(t[i]) - sqrt(q.Eval(t[i]));
	}
	else {
		cgv::math::fpoly_mon<T, 3> cx, cy, cz, ra;
		matrix_to_polynomials(M, cx, cy, cz, ra);
		// polynomial representing ortogonal distance to ray
		cgv::math::fpoly_mon<T, 6> D = cy * cy + cz * cz;
		cgv::math::fpoly_mon<T, 2> dcx = cx.derive();
		cgv::math::fpoly_mon<T, 6> q = ra * ra - D;
		cgv::math::fpoly_mon<T, 10> lhs = dcx * dcx * q;
		cgv::math::fpoly_mon<T, 5> hdD = D.derive(); hdD *= 0.5;
		cgv::math::fpoly_mon<T, 5> rhs_sqrt = ra * ra.derive() - hdD;
		cgv::math::fpoly_mon<T, 10> p = lhs - rhs_sqrt * rhs_sqrt;
		cnt = p.compute_roots(0.0, 1.0, t, eps);
		for (int i = 0; i < cnt; ++i)
			s[i] = cx.eval(t[i]) - sqrt(q.eval(t[i]));
	}
	consider_ray_local_sphere_intersection(B.col(0), T(0), s, t, cnt);
	consider_ray_local_sphere_intersection(B.col(3), T(1), s, t, cnt);
	int i_min = -1;
	for (int i = 0; i < cnt; ++i)
		if (s[i] > 0.0 && (i_min == -1 || s[i] < s[i_min]))
			i_min = i;
	if (i_min == -1)
		return false;
	cgv::vec3 b0 = B_world.col(0).down();
	cgv::vec3 b1 = B_world.col(1).down();
	cgv::vec3 b2 = B_world.col(2).down();
	cgv::vec3 b3 = B_world.col(3).down();
	float tt = float(t[i_min]);
	cgv::vec3 b10 = lerp(b0, b1, tt);
	cgv::vec3 b11 = lerp(b1, b2, tt);
	cgv::vec3 b12 = lerp(b2, b3, tt);
	cgv::vec3 b20 = lerp(b10, b11, tt);
	cgv::vec3 b21 = lerp(b11, b12, tt);
	cgv::vec3 cp = lerp(b20,b21,tt);
	cgv::vec3 ip = r.position((float)s[i_min]);
	cgv::mat4x2 I = { cgv::vec4(ip,s[i_min]), cgv::vec4(normalize(ip - cp),t[i_min]) };
	intersections.push_back(I);
	return true;
}
template <typename T, int solver = 0>
bool compute_ray_circle_tube_intersection(const cgv::ray3& r, const cgv::mat4& B_world, std::vector<cgv::mat4x2>& intersections, const T& eps)
{
	cy::Polynomial<T, 3> ra0;
	cgv::math::fpoly_mon<T, 3> ra1;
	cgv::mat4 B = transform_to_ray_coordinates(r, B_world);
	cgv::math::fmat<T, 4, 4> M = B * monom_from_bernstein_matrix();
	T t[12], s[12];
	int cnt = 0;
	if (solver == 0) {
		cy::Polynomial<T, 3> cx, cy, cz;
		matrix_to_polynomials(M, cx, cy, cz, ra0);
		// polynomial representing ortogonal distance to ray
		cy::Polynomial<T, 6> D = cy * cy + cz * cz;
		cy::Polynomial<T, 2> dcx = cx.Derivative();
		cy::Polynomial<T, 6> q = ra0 * ra0 - D;
		cy::Polynomial<T, 10> lhs = dcx * dcx * q;
		cy::Polynomial<T, 5> hdD = D.Derivative(); hdD *= T(0.5);
		cy::Polynomial<T, 10> p = lhs - hdD * hdD;
		cnt = p.Roots(t, T(0.0), T(1.0), eps);
		for (int i = 0; i < cnt; ++i) {
			cgv::math::fvec<T, 3> dc;
			cgv::math::fvec<T, 3> c(cx.EvalWithDeriv(dc[0], t[i]), cy.EvalWithDeriv(dc[1], t[i]), cz.EvalWithDeriv(dc[2], t[i]));
			s[i] = dot(c, dc) / dc[0];
		}
	}
	else {
		cgv::math::fpoly_mon<T, 3> cx, cy, cz;
		matrix_to_polynomials(M, cx, cy, cz, ra1);
		// polynomial representing ortogonal distance to ray
		cgv::math::fpoly_mon<T, 6> D = cy * cy + cz * cz;
		cgv::math::fpoly_mon<T, 2> dcx = cx.derive();
		cgv::math::fpoly_mon<T, 6> q = ra1 * ra1 - D;
		cgv::math::fpoly_mon<T, 10> lhs = dcx * dcx * q;
		cgv::math::fpoly_mon<T, 5> hdD = D.derive(); hdD *= T(0.5);
		cgv::math::fpoly_mon<T, 10> p = lhs - hdD * hdD;
		cnt = p.compute_roots(T(0.0), T(1.0), t, eps);
		for (int i = 0; i < cnt; ++i) {
			cgv::math::fvec<T, 3> dc;
			cgv::math::fvec<T, 3> c(cx.eval_with_derivative(t[i], dc[0]), cy.eval_with_derivative(t[i], dc[1]), cz.eval_with_derivative(t[i], dc[2]));
			s[i] = dot(c, dc) / dc[0];
		}
	}
	int i_min = -1;
	for (int i = 0; i < cnt; ++i) {
		if (s[i] > 0.0 && (i_min == -1 || s[i] < s[i_min]))
			i_min = i;
	}
	if (i_min == -1)
		return false;
	cgv::vec3 b0 = B_world.col(0).down();
	cgv::vec3 b1 = B_world.col(1).down();
	cgv::vec3 b2 = B_world.col(2).down();
	cgv::vec3 b3 = B_world.col(3).down();
	float tt = float(t[i_min]);
	cgv::vec3 b10 = lerp(b0, b1, tt);
	cgv::vec3 b11 = lerp(b1, b2, tt);
	cgv::vec3 b12 = lerp(b2, b3, tt);
	cgv::vec3 b20 = lerp(b10, b11, tt);
	cgv::vec3 b21 = lerp(b11, b12, tt);
	cgv::vec3 cp = lerp(b20, b21, tt);
	cgv::vec3 ta = normalize(b21 - b20);
	cgv::vec3 ip = r.position((float)s[i_min]);
	cgv::vec3 no = normalize(ip - cp);
	T rad, drad;
	if (solver == 0)
		rad = ra0.EvalWithDeriv(drad, t[i_min]);
	else
		rad = ra1.eval_with_derivative(t[i_min], drad);
	cgv::vec3 nml = normalize(no - (float)drad * ta);
	cgv::mat4x2 I = { cgv::vec4(ip,s[i_min]), cgv::vec4(nml,t[i_min]) };
	intersections.push_back(I);
	return true;
}
template <typename T, int solver = 0>
bool compute_ray_view_aligned_ribbon_intersection(const cgv::ray3& r, const cgv::mat4& B_world, std::vector<cgv::mat4x2>& intersections, const T& eps)
{
	cgv::mat4 B = transform_to_ray_coordinates(r, B_world);
	cgv::math::fmat<T, 4, 4> M = B * monom_from_bernstein_matrix();
	T t[8], s[8];
	bool valid[8];
	int cnt = 0;
	if (solver == 0) {
		cy::Polynomial<T, 3> cx, cy, cz, ra;
		matrix_to_polynomials(M, cx, cy, cz, ra);
		// polynomial representing ortogonal distance to ray
		cy::Polynomial<T, 2> dcx = cx.Derivative(), dcy = cy.Derivative(), dcz = cz.Derivative();
		cy::Polynomial<T, 8> p = (cx * dcx + cy * dcy + cz * dcz) * cx - (cx * cx + cy * cy + cz * cz) * dcx;
		cnt = p.Roots(t, 0.0, 1.0, eps);
		for (int i = 0; i < cnt; ++i) {
			cgv::math::fvec<T,3> dc;
			cgv::math::fvec<T,3> c(cx.EvalWithDeriv(dc[0], t[i]), cy.EvalWithDeriv(dc[1], t[i]), cz.EvalWithDeriv(dc[2], t[i]));
			cgv::math::fvec<T,3> d2 = cross(c, dc);
			cgv::math::fvec<T,3> m2 = cross(c, d2);
			cgv::math::fvec<T,3> d1 = { 1,0,0 };
			cgv::math::fvec<T,3> d12 = cross(d1, d2);
			s[i] = dot(m2, d12) / dot(d12, d12);
			valid[i] = (c - cgv::math::fvec<T, 3>(s[i],0,0)).length() <= ra.Eval(t[i]);
		}
	}
	else {
		cgv::math::fpoly_mon<T, 3> cx, cy, cz, ra;
		matrix_to_polynomials(M, cx, cy, cz, ra);
		// polynomial representing ortogonal distance to ray
		cgv::math::fpoly_mon<T, 2> dcx = cx.derive(), dcy = cy.derive(), dcz = cz.derive();
		cgv::math::fpoly_mon<T, 8> p = p = (cx * dcx + cy * dcy + cz * dcz) * cx - (cx * cx + cy * cy + cz * cz) * dcx;
		cnt = p.compute_roots(0.0, 1.0, t, eps);
		for (int i = 0; i < cnt; ++i) {
			cgv::math::fvec<T, 3> dc;
			cgv::math::fvec<T, 3> c(cx.eval_with_derivative(t[i], dc[0]), cy.eval_with_derivative(t[i], dc[1]), cz.eval_with_derivative(t[i], dc[2]));
			cgv::math::fvec<T, 3> d2 = cross(c, dc);
			cgv::math::fvec<T, 3> m2 = cross(c, d2);
			cgv::math::fvec<T, 3> d1 = { 1,0,0 };
			cgv::math::fvec<T, 3> d12 = cross(d1, d2);
			s[i] = dot(m2, d12) / dot(d12, d12);
			valid[i] = (c - cgv::math::fvec<T, 3>(s[i], 0, 0)).length() <= ra.eval(t[i]);
		}
	}
	int i_min = -1;
	for (int i = 0; i < cnt; ++i) {
		if (valid[i]) {
			if (s[i] > 0.0 && (i_min == -1 || s[i] < s[i_min]))
				i_min = i;
		}
	}
	if (i_min == -1)
		return false;
	cgv::vec3 b0 = B_world.col(0).down();
	cgv::vec3 b1 = B_world.col(1).down();
	cgv::vec3 b2 = B_world.col(2).down();
	cgv::vec3 b3 = B_world.col(3).down();
	float tt = float(t[i_min]);
	cgv::vec3 b10 = lerp(b0, b1, tt);
	cgv::vec3 b11 = lerp(b1, b2, tt);
	cgv::vec3 b12 = lerp(b2, b3, tt);
	cgv::vec3 b20 = lerp(b10, b11, tt);
	cgv::vec3 b21 = lerp(b11, b12, tt);
	cgv::vec3 cp = lerp(b20, b21, tt);
	cgv::vec3 ta = normalize(b21-b20);
	cgv::vec3 bi = cross(cp - r.origin, ta);
	cgv::vec3 ip = r.position((float)s[i_min]);
	cgv::vec3 nml = normalize(cross(ta,bi));
	cgv::mat4x2 I = { cgv::vec4(ip,s[i_min]), cgv::vec4(nml,t[i_min]) };
	intersections.push_back(I);
	return true;
}


#include <random>

std::vector<double> random_vector(int N, std::default_random_engine& e, std::uniform_real_distribution<double>& u)
{
	std::vector<double> v;
	for (int i = 0; i < N; ++i)
		v.push_back(u(e));
	return v;
}

template <int N, int M>
bool random_test_remainder(int n)
{
	std::default_random_engine e;
	std::uniform_real_distribution<double> u(-5, 5);
	for (int i = 0; i < n; ++i) {
		std::vector<double> f = random_vector(N, e, u);
		std::vector<double> g = random_vector(M, e, u);		
		cgv::math::fpoly_mon<double, N> divident(f.data());
		cgv::math::fpoly_mon<double, M> divisor(g.data());
		cgv::math::fpoly_mon<double, N-M> quotient;
		cgv::math::fpoly_mon<double, M-1> remainder;
		divident.div(divisor, quotient, remainder);
		cgv::math::fpoly_mon<double, N> reconstruction = quotient * divisor;
		for (int j=0; j<M; ++j)
			reconstruction[j] += remainder[j];
		if ((reconstruction - divident).length() > 1e-8) {
			std::cout << "ERROR: divident = " << divident << ", divisor = " << divisor << ", quotient = " << quotient << ", remainder = " << remainder << ", reconstruction = " << reconstruction << std::endl;
			return false;
		}
	}
	return true;
}

bool test_remainder(int n = 100)
{
	return
		random_test_remainder<4, 1>(n) &&
		random_test_remainder<4, 2>(n) &&
		random_test_remainder<4, 3>(n) &&
		random_test_remainder<4, 4>(n) &&
		random_test_remainder<5, 1>(n) &&
		random_test_remainder<5, 2>(n) &&
		random_test_remainder<5, 3>(n) &&
		random_test_remainder<5, 4>(n) &&
		random_test_remainder<5, 5>(n) &&
		random_test_remainder<6, 1>(n) &&
		random_test_remainder<6, 2>(n) &&
		random_test_remainder<6, 3>(n) &&
		random_test_remainder<6, 4>(n) &&
		random_test_remainder<6, 5>(n) &&
		random_test_remainder<6, 6>(n);
}

template<typename T, int G = 6>
void test_schur_chain(T eps, T eps_val, int n = 100)
{
	std::default_random_engine e;
	std::uniform_real_distribution<T> u(T(0.001), T(0.999));
	for (int i = 0; i < n; ++i) {
		std::vector<T> v;
		for (int j = 0; j < G; ++j)
			v.push_back(u(e));
		int cnt = G;
		for (int j = 0; j < G; ++j)
			if (u(e) > 0.5) {
				v[j] += 1.0;
				--cnt;
			}
		cgv::math::fpoly_mon<T, G> p(v);
		cgv::math::sturm_chain_mon<T, G> sc(p);
		auto I = sc.eliminate_roots(0.0, 1.0);
		std::vector<T> w;
		for (auto iv : I)
			w.push_back(sc.find_root(iv[0], iv[1], eps));

		for (auto r : w) {
			int j;
			T min_delta = 1000.0;
			for (j = 0; j < v.size(); ++j) {
				T delta = std::abs(r - v[j]);
				if (delta <= eps_val)
					break;
				else if (delta < min_delta)
					min_delta = delta;
			}
			if (j < v.size())
				v.erase(v.begin() + j);
			else {
				std::cout << "ERROR: did not find root " << r << " in";
				for (auto vv : v)
					std::cout << " " << vv;
				std::cout << " [" << eps_val << " <-> " << min_delta<< "]" << std::endl;
			}
		}
		///int sturm_cnt = sc.estimate_nr_roots_on_interval(0.0, 1.0);
		if (w.size() + v.size() == G) {
		}
		else {
			std::cout << "ERROR: found the roots";
			for (auto ww : w)
				std::cout << " " << ww;
			std::cout << " and the non roots";
			for (auto vv : v)
				std::cout << " " << vv;

			std::cout << std::endl;
		}
	}

}

struct thick_line_data 
{
	std::vector<uint32_t>  indices  = { 0,1,1,2 };
	std::vector<cgv::vec4> spheres  = { {-2.f,0.f,0.f,.2f}, {0.f,0.f,0.f,.1f}, {0.f,0.f,-2.f,.3f} };
	std::vector<cgv::vec4> tangents = { {0.f,4.f,0.f,-.2f}, {0.f,-4.f,0.f,.2f}, {0.f,4.f,0.f,.4f } };
	std::vector<cgv::rgb>  colors   = { {1.f,0.f,0.f}, {0.f,1.f,0.f}, {0.f,0.f,1.f} };

	std::vector<cgv::mat4> bezier_tubes;

	void compute_bezier_tubes() {
		bezier_tubes.clear();
		for (size_t i = 0; i < indices.size(); i += 2) {
			// transform hermit to bezier representation in ray coordinates resulting in Bezier matrix with radius in 4th comp
			cgv::mat4 B;
			uint32_t j = indices[i], k = indices[i+1];
			B.col(0) = spheres[j];
			B.col(1) = spheres[j] + (1.0f/3)*tangents[j];
			B.col(2) = spheres[k] - (1.0f/3)*tangents[k];
			B.col(3) = spheres[k];
			bezier_tubes.push_back(B);
		}
	}
	thick_line_data() {
		compute_bezier_tubes();
	}
};

class thick_line_viewer :
	public cgv::base::group,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
protected:
	cgv::render::clipped_view* view_ptr = 0;
	cgv::gui::transformation_gizmo_ptr tg_ptr;
	cgv::render::spline_tube_render_style trs;
	cgv::render::sphere_render_style srs;
	cgv::render::cone_render_style crs;
	cgv::render::arrow_render_style ars;
	cgv::render::arrow_render_style ars2;
	cgv::render::surfel_render_style urs;

	bool use_radii = true;
	bool use_tangents = true;
	bool use_colors = true;

	bool show_tube = true;
	bool show_control_geometry = false;
	bool show_cameras = true;
	bool show_rays = true;
	bool show_intersections = true;

	thick_line_data lines;

	unsigned subsampling = 32;
	float ray_offset = 1.0f;
	float ray_length = 2.0f;
	std::vector<cgv::ray3> rays;
	enum class primitive_type {
		spheres, control_geometry, sphere_tube, circle_tube, view_aligned_ribbon
	} primitive = primitive_type::view_aligned_ribbon;
	enum class precision_type {
		flt32, flt64
	} precision = precision_type::flt64;
	enum class solver_type {
		recursive, sturm
	} solver = solver_type::sturm;
	enum class strategy_type {
		direct, filtered
	} strategy = strategy_type::direct;
	float eps32 = 6e-4f;
	double eps64 = 6e-7;
	std::vector<cgv::vec3> intersections;
	std::vector<cgv::vec3> intersection_normals;
	float ray_color_offset = 0.5f;
	float ray_color_scale = 0.3f;
	std::vector<cgv::rgb> intersection_colors;

	void sample_rays() {
		rays.clear();
		cam_man.sample_rays(cam_man.cameras[camera_index], rays, subsampling);
		post_redraw();
	}
	void intersect_rays() {
		intersections.clear();
		intersection_normals.clear();
		intersection_colors.clear();
		if (rays.empty())
			return;
		lines.compute_bezier_tubes();
		std::vector<cgv::mat4x2> Is;
		cgv::utils::stopwatch watch(true);
		int iter = -1;
		for (const auto& r : rays) {
			//if (++iter % 100 == 0)
			//	std::cout << iter << std::endl;
			cgv::mat4x2 best(-1.f);
			for (unsigned i = 0; i < nr_tubes; ++i) {
				Is.clear();
				bool found = false;
				switch (primitive) {
				case primitive_type::spheres:
					found = compute_ray_sphere_intersection(r, lines.bezier_tubes[i], Is);
					break;
				case primitive_type::control_geometry:
					found = compute_ray_control_geometry_intersection(r, lines.bezier_tubes[i], Is);
					break;
				case primitive_type::sphere_tube:
					switch (precision) {
					case precision_type::flt32:
						switch (solver) {
						case solver_type::recursive:
							found = compute_ray_sphere_tube_intersection<float, 0>(r, lines.bezier_tubes[i], Is, eps32);
							break;
						case solver_type::sturm:
							found = compute_ray_sphere_tube_intersection<float, 1>(r, lines.bezier_tubes[i], Is, eps32);
							break;
						}
						break;
					case precision_type::flt64:
						switch (solver) {
						case solver_type::recursive:
							found = compute_ray_sphere_tube_intersection<double, 0>(r, lines.bezier_tubes[i], Is, eps64);
							break;
						case solver_type::sturm:
							found = compute_ray_sphere_tube_intersection<double, 1>(r, lines.bezier_tubes[i], Is, eps64);
							break;
						}
						break;
					}
					break;
				case primitive_type::circle_tube:
					switch (precision) {
					case precision_type::flt32:
						switch (solver) {
						case solver_type::recursive:
							found = compute_ray_circle_tube_intersection<float, 0>(r, lines.bezier_tubes[i], Is, eps32);
							break;
						case solver_type::sturm:
							found = compute_ray_circle_tube_intersection<float, 1>(r, lines.bezier_tubes[i], Is, eps32);
							break;
						}
						break;
					case precision_type::flt64:
						switch (solver) {
						case solver_type::recursive:
							found = compute_ray_circle_tube_intersection<double, 0>(r, lines.bezier_tubes[i], Is, eps64);
							break;
						case solver_type::sturm:
							found = compute_ray_circle_tube_intersection<double, 1>(r, lines.bezier_tubes[i], Is, eps64);
							break;
						}
						break;
					}
					break;
				case primitive_type::view_aligned_ribbon:
					switch (precision) {
					case precision_type::flt32:
						switch (solver) {
						case solver_type::recursive:
							found = compute_ray_view_aligned_ribbon_intersection<float, 0>(r, lines.bezier_tubes[i], Is, eps32);
							break;
						case solver_type::sturm:
							found = compute_ray_view_aligned_ribbon_intersection<float, 1>(r, lines.bezier_tubes[i], Is, eps32);
							break;
						}
						break;
					case precision_type::flt64:
						switch (solver) {
						case solver_type::recursive:
							found = compute_ray_view_aligned_ribbon_intersection<double, 0>(r, lines.bezier_tubes[i], Is, eps64);
							break;
						case solver_type::sturm:
							found = compute_ray_view_aligned_ribbon_intersection<double, 1>(r, lines.bezier_tubes[i], Is, eps64);
							break;
						}
						break;
					}
					break;
				}
				if (found) {
					if (best(3, 0) == -1.f || (best(3, 0) > Is.front()(3, 0)))
						best = Is.front();
				}
			}
			if (best(3, 0) != -1) {
				intersections.push_back(reinterpret_cast<const cgv::vec3&>(best.col(0)));
				intersection_normals.push_back(reinterpret_cast<const cgv::vec3&>(best.col(1)));
				intersection_colors.push_back(cgv::rgb(cgv::math::clamp(ray_color_scale * (best(3, 0) - ray_color_offset), 0.f, 1.f), best(3, 1), 0.f));
			}
			post_redraw();
		}
		static const char* primitive_names[] = { "sphere", "geometry", "sphere tube", "circle tube", "ribbon" };
		static const char* solver_names[] = { "recursive", "Sturm" };
		static const char* precision_names[] = { "float", "double" };
		static const char* strategy_names[] = { "direct", "filtered" };
		std::cout << "elapsed time [" << primitive_names[(int&)primitive] << "," << solver_names[(int&)solver]
			<< "," << precision_names[(int&)precision] << "]: " << watch.get_elapsed_time() << std::endl;
		post_recreate_gui();
	}
private:
	camera_manager cam_man;
	int camera_index = 0;
	bool record_next_camera = false;
	unsigned nr_tubes;

	int current_control_point = -1;
	cgv::dmat4 last_MPW;
	int get_current_sphere_index(int ccp, int* tangent_index_ptr) const
	{
		int si, ti = 0;
		if ((ccp % 3) == 0)
			si = ccp / 3;
		else {
			int segment_index = ccp / 3;
			ti = 3 - 2 * (ccp % 3);
			si = lines.indices[2 * segment_index + ccp % 3 - 1];
		}
		if (tangent_index_ptr)
			*tangent_index_ptr = ti;
		return si;
	}
	cgv::vec4 get_control_sphere(int ccp) const {
		int ti, si = get_current_sphere_index(ccp, &ti);
		return (ti == 0) ? lines.spheres[si] : lines.spheres[si] + 1 / 3.f * ti * lines.tangents[si];
	}
	void callback(cgv::gui::GizmoAction action, cgv::gui::transformation_gizmo::Mode mode)
	{
		int tangent_index, sphere_index = get_current_sphere_index(current_control_point, &tangent_index);
		switch (mode) {
		case cgv::gui::transformation_gizmo::Mode::kTranslation:
			if (tangent_index == 0)
				lines.spheres[sphere_index].down() = tg_ptr->get_position();
			else
				lines.tangents[sphere_index].down() =
					(3.f*tangent_index) * (tg_ptr->get_position() - lines.spheres[sphere_index].down());
			lines.compute_bezier_tubes();
			post_redraw();
			break;
		case cgv::gui::transformation_gizmo::Mode::kScale: {
			cgv::vec3 scale = tg_ptr->get_scale();
			float radius = (scale[0]+scale[1]+scale[2])/3;
			if (tangent_index == 0)
				lines.spheres[sphere_index][3] = radius;
			else
				lines.tangents[sphere_index][3] =
					(3.f * tangent_index) * radius - lines.spheres[sphere_index][3];
			lines.compute_bezier_tubes();
			tg_ptr->set_scale(cgv::vec3(radius));
			post_redraw();
			break;
		}
		}
	}
	void begin_gizmo()
	{
		if (current_control_point == -1)
			tg_ptr->hide();
		else {
			if (!tg_ptr->is_visible())
				tg_ptr->show();
			cgv::vec4 control_sphere = get_control_sphere(current_control_point);
			tg_ptr->set_position(control_sphere.down());
			tg_ptr->set_scale(control_sphere[3]);
		}
	}
	void set_mode(cgv::gui::transformation_gizmo::Mode mode)
	{
		if (tg_ptr->get_mode() == mode)
			return;
		tg_ptr->set_mode(mode);
		begin_gizmo();
		post_redraw();
	}
	void select_control_point(int i)
	{
		if (i == current_control_point)
			return;
		current_control_point = i;
		begin_gizmo();
		post_redraw();
	}
public:
	thick_line_viewer() : cgv::base::group("Thick Line Viewer") {
		//test_remainder(100);
		//std::cout << "test degree 6:" << std::endl;
		//test_schur_chain<double,6>(0.0000001f,0.00001f,10000);
		//std::cout << "test degree 8:" << std::endl;
		//test_schur_chain<double,8>(0.0000001f,0.00001f, 10000);
		//std::cout << "test degree 10:" << std::endl;
		//test_schur_chain<double,10>(0.0000001f,0.00001f, 10000);
		//std::cout << "performed tests with n=100" << std::endl;
		urs.orient_splats = true;
		urs.measure_point_size_in_pixel = false;
		urs.point_size = 3;
		urs.blend_points = false;
		urs.blend_width_in_pixel = 0.f;
		urs.illumination_mode = cgv::render::IM_TWO_SIDED;
		trs.radius = 0.2f;
		srs.radius = 0.1f;
		crs.radius = 0.02f;
		crs.rounded_caps = true;
		ars.radius_relative_to_length = 0.0f;
		ars.radius_lower_bound = 0.05f;
		ars2.radius_relative_to_length = 0.0f;
		ars2.radius_lower_bound = 0.005f;
		nr_tubes = unsigned(lines.indices.size() / 2);
		if (cgv::utils::file::exists(cam_man.path))
			cam_man.read(cam_man.path);
		tg_ptr = create_and_append_child<cgv::gui::transformation_gizmo>("Gizmo");
		tg_ptr->on_change = [this](cgv::gui::GizmoAction action, cgv::gui::transformation_gizmo::Mode mode) {
			this->callback(action, mode); };
		tg_ptr->hide();
	}
	std::string get_type_name() const { return "Thick Line Viewer"; }
	void clear(cgv::render::context& ctx) {
		ref_spline_tube_renderer(ctx, -1);
		cgv::render::ref_sphere_renderer(ctx, -1);
		cgv::render::ref_cone_renderer(ctx, -1);
		cgv::render::ref_arrow_renderer(ctx, -1);
		cgv::render::ref_surfel_renderer(ctx, -1);
	}
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return
			rh.reflect_member("show_tube", show_tube) &&
			rh.reflect_member("show_cameras", show_cameras) &&
			rh.reflect_member("show_rays", show_rays) &&
			rh.reflect_member("show_intersections", show_intersections) &&
			rh.reflect_member("use_radii", use_radii) &&
			rh.reflect_member("use_tangents", use_tangents) &&
			rh.reflect_member("use_colors", use_colors) &&
			rh.reflect_member("subsampling", subsampling) &&
			rh.reflect_member("ray_offset", ray_offset) &&
			rh.reflect_member("ray_length", ray_length) &&
			rh.reflect_member("primitive", (int&)primitive) &&
			rh.reflect_member("primitive", ray_color_offset) &&
			rh.reflect_member("primitive", ray_color_scale) &&
			rh.reflect_member("show_control_geometry", show_control_geometry);
	}
	void stream_help(std::ostream& os) {
		os << "Thick Line Viewer: toggle <T>ubes, <C>ameras, control <G>eometry, <P>rimitive, <R>ays\n";
		os << "         <S>ample rays, compute <I>ntersections, <+|-> in|decrease subsampling\n";
		os << "         Camera control: <Left|Right> select, <X> record current, <Y> restore selected\n";
		os << "                         <c-S> save cameras to, <c-O> load cameras from 'INPUT_DIR/cameras.bin'" << std::endl;
	}
	void stream_stats(std::ostream& os) {}

	bool handle(cgv::gui::event& e) { 
		if (e.get_kind() == cgv::gui::EID_MOUSE) {
			auto& me = reinterpret_cast<cgv::gui::mouse_event&>(e);
			if (me.get_modifiers() != cgv::gui::EM_SHIFT)
				return false;
			if (me.get_action() != cgv::gui::MA_PRESS || me.get_button() != cgv::gui::MB_LEFT_BUTTON)
				return false;
			cgv::ray3 ray = get_world_ray(me.get_x(),me.get_y(),view_ptr,last_MPW);
			for (int ci = 0; ci < 3 * lines.indices.size() / 2 + 1; ++ci) {
				cgv::vec4 cs = get_control_sphere(ci);
				cgv::vec2 ts;
				if (cgv::math::ray_sphere_intersection(ray, cs.down(), cs[3], ts) > 0) {
					if (current_control_point == ci) {
						if (tg_ptr->get_mode() == cgv::gui::transformation_gizmo::Mode::kScale)
							select_control_point(-1);
						else
							tg_ptr->set_mode(cgv::gui::transformation_gizmo::Mode::kScale);
					}
					else {
						tg_ptr->set_mode(cgv::gui::transformation_gizmo::Mode::kTranslation);
						select_control_point(ci);
					}
					break;
				}
			}
			return true;
		}
		if (e.get_kind() == cgv::gui::EID_KEY) {
			auto& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() == cgv::gui::KA_RELEASE)
				return false;
			switch (ke.get_char()) {
			case '+':
				if (subsampling < 128) {
					subsampling *= 2;
					on_set(&subsampling);
				}
				return true;
			case '-':
				if (subsampling > 1) {
					subsampling /= 2;
					on_set(&subsampling);
				}
				return true;
			}
			switch (ke.get_key()) {
			case cgv::gui::KEY_Up:
				if (camera_index == cam_man.max_index())
					camera_index = 0;
				else
					++camera_index;
				on_set(&camera_index);
				return true;
			case cgv::gui::KEY_Down:
				if (camera_index == 0)
					camera_index = cam_man.max_index();
				else
					--camera_index;
				on_set(&camera_index);
				return true;
			case cgv::gui::KEY_Right:
				if (++current_control_point > 3*lines.indices.size()/2)
					current_control_point = -1;
				on_set(&current_control_point);
				return true;
			case cgv::gui::KEY_Left:
				if (current_control_point == -1)
					current_control_point = int(3*lines.indices.size()/2);
				else 
					--current_control_point;
				on_set(&current_control_point);
				return true;
			case cgv::gui::KEY_Space:
				if (tg_ptr->is_visible()) {
					if (tg_ptr->get_mode() == cgv::gui::transformation_gizmo::Mode::kScale)
						set_mode(cgv::gui::transformation_gizmo::Mode::kTranslation);
					else
						set_mode(cgv::gui::transformation_gizmo::Mode::kScale);
					post_redraw();
					return true;
				}
				break;
			case '0':
				if (solver != solver_type::recursive) {
					solver = solver_type::recursive;
					on_set(&solver);
				}
				return true;
			case '1':
				if (solver != solver_type::sturm) {
					solver = solver_type::sturm;
					on_set(&solver);
				}
				return true;
			case 'T':
				show_tube = !show_tube;
				on_set(&show_tube);
				break;
			case 'C':
				show_cameras = !show_cameras;
				on_set(&show_cameras);
				break;
			case 'G':
				show_control_geometry = !show_control_geometry;
				on_set(&show_control_geometry);
				break;
			case 'P':
				if (primitive == primitive_type::view_aligned_ribbon)
					primitive = primitive_type::spheres;
				else
					++(int&)primitive;
				on_set(&primitive);
				break;
			case 'R':
				show_rays = !show_rays;
				on_set(&show_rays);
				return true;
			case 'O':
				if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
					if (cam_man.read(cam_man.path)) {
						if (find_control(camera_index))
							find_control(camera_index)->set("max", cam_man.max_index());
						if (camera_index > cam_man.max_index()) {
							camera_index = 0;
							on_set(&camera_index);
						}
						post_redraw();
					}
					return true;
				}
				break;
			case 'S':
				if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
					cam_man.write(cam_man.path);
					return true;
				}
				else {
					if (camera_index <= cam_man.max_index()) {
						sample_rays();
						post_redraw();
					}
					return true;
				}
				break;
			case 'I':
				if (!rays.empty())
					intersect_rays();
				post_redraw();
				break;
			case 'X':
				record_next_camera = true;
				post_redraw();
				break;
			case 'Y':
				if (camera_index < cam_man.cameras.size()) {
					*view_ptr = cam_man.cameras[camera_index].view;
					post_redraw();
				}
				break;
			}
			return false;
		}
		return false;
	}
	void on_set(void* member_ptr) {
		if (!rays.empty() && (member_ptr == &ray_length || member_ptr == &ray_offset || member_ptr == &subsampling))
			sample_rays();
		if (!intersections.empty() &&
			((member_ptr == &primitive) || (member_ptr == &precision) || (member_ptr == &solver) || (member_ptr == &strategy) || (member_ptr == &ray_color_offset) || (member_ptr == &ray_color_scale))) {
			intersect_rays();
		}
		if (member_ptr == &current_control_point)
			begin_gizmo();
		update_member(member_ptr);
		post_redraw();
	}

	bool init(cgv::render::context& ctx) {
		cgv::render::ref_spline_tube_renderer(ctx, 1);
		cgv::render::ref_sphere_renderer(ctx, 1);
		cgv::render::ref_cone_renderer(ctx, 1);
		cgv::render::ref_arrow_renderer(ctx, 1);
		cgv::render::ref_surfel_renderer(ctx, 1);
		ctx.set_bg_clr_idx(0);
		return true;
	}
	void init_frame(cgv::render::context& ctx) {
		if (!view_ptr) {
			if (view_ptr = dynamic_cast<cgv::render::clipped_view*>(find_view_as_node())) {
				view_ptr->set_eye_keep_view_angle(cgv::dvec3(2.5f, 0.f, 2.5f));
			}
		}
		if (record_next_camera) {
			cam_man.add_camera(ctx, view_ptr);
			find_control(camera_index)->set("max", camera_index = cam_man.max_index());
			on_set(&camera_index);
			record_next_camera = false;
		}
	}
	void draw_rays(cgv::render::context& ctx) {
		if (rays.empty())
			return;

		std::vector<cgv::vec3> positions;
		std::vector<cgv::vec3> directions;
		std::vector<cgv::rgb> colors;

		for (const auto& r : rays) {
			positions.push_back(r.origin + ray_offset * r.direction);
			directions.push_back(ray_length * r.direction);
			colors.push_back(cgv::rgb(0.1f, 0.3f, 0.5f));
		}
		auto& ar = cgv::render::ref_arrow_renderer(ctx);
		ar.set_render_style(ars2);
		ar.set_position_array(ctx, positions);
		ar.set_direction_array(ctx, directions);
		ar.set_color_array(ctx, colors);
		ar.render(ctx, 0, positions.size());
	}
	void draw_cameras(cgv::render::context& ctx) {
		if (cam_man.cameras.empty())
			return;
		auto& sr = cgv::render::ref_sphere_renderer(ctx);
		sr.set_render_style(srs);
		std::vector<cgv::vec3> positions;
		std::vector<cgv::vec3> directions;
		std::vector<cgv::rgb> colors;
		for (const auto& C : cam_man.cameras) {
			positions.push_back(C.view.get_eye());
			colors.push_back(cgv::rgb(.3f, .3f, .7f));
		}
		for (const auto& C : cam_man.cameras) {
			positions.push_back(C.view.get_focus());
			colors.push_back(cgv::rgb(.3f, .7f, .3f));
		}
		size_t N = cam_man.cameras.size();
		for (size_t i = 0; i < N; ++i)
			directions.push_back(positions[i+N] - positions[i]);
		colors[camera_index] = cgv::rgb(1, 0, 0.3f);
		colors[camera_index+N] = cgv::rgb(1, 0.3f, 0);
		sr.set_position_array(ctx, positions);
		sr.set_color_array(ctx, colors);
		sr.render(ctx, 0, positions.size());

		auto& ar = cgv::render::ref_arrow_renderer(ctx);
		ar.set_render_style(ars);
		ar.set_position_array(ctx, positions);
		ar.set_direction_array(ctx, directions);
		ar.set_color_array(ctx, colors);
		ar.render(ctx, 0, directions.size());
		
		// construct wire frame of view frustum
		positions.clear();
		colors.clear();
		std::vector<uint32_t> indices;
		uint32_t o = 0;
		for (const auto& C : cam_man.cameras) {
			cgv::dvec4 h_near = C.P*C.M*cgv::dvec4(C.view.get_eye() + 0.2 * (C.view.get_focus() - C.view.get_eye()),1.0);
			double z_near = h_near[2] / h_near[3];
			cgv::dvec4 h_far = C.P*C.M*cgv::dvec4(C.view.get_eye() + 2.5 * (C.view.get_focus() - C.view.get_eye()),1.0);
			double z_far = h_far[2] / h_far[3];
			cgv::dmat4 iMP = inverse(C.P*C.M);
			for (int i = 0; i < 8; ++i) {
				cgv::dvec4 p_clip = cgv::dvec4(1.0);
				if ((i & 1) == 0)
					p_clip[0] = -1.0;
				if ((i & 2) == 0)
					p_clip[1] = -1.0;
				p_clip[2] = ((i & 4) == 0) ? z_near : z_far;
				cgv::dvec4 p_world = iMP * p_clip;
				positions.push_back(reinterpret_cast<cgv::dvec3&>(p_world) / p_world[3]);
				colors.push_back(cgv::rgb(.5f, .5f, .5f));
			}
			indices.push_back(o+0); indices.push_back(o+1);
			indices.push_back(o+1); indices.push_back(o+3);
			indices.push_back(o+3); indices.push_back(o+2);
			indices.push_back(o+2); indices.push_back(o+0);

			indices.push_back(o + 0); indices.push_back(o + 4);
			indices.push_back(o + 1); indices.push_back(o + 5);
			indices.push_back(o + 3); indices.push_back(o + 7);
			indices.push_back(o + 2); indices.push_back(o + 6);

			indices.push_back(o + 4); indices.push_back(o + 5);
			indices.push_back(o + 5); indices.push_back(o + 7);
			indices.push_back(o + 7); indices.push_back(o + 6);
			indices.push_back(o + 6); indices.push_back(o + 4);
			o = uint32_t(positions.size());
		}
		auto& cr = cgv::render::ref_cone_renderer(ctx);
		cr.set_render_style(crs);
		cr.set_position_array(ctx, positions);
		cr.remove_radius_array(ctx);
		cr.set_color_array(ctx, colors);
		cr.set_indices(ctx, indices);
		cr.render(ctx, 0, indices.size());
	}
	void draw_tubes(cgv::render::context & ctx) {
		if (nr_tubes == 0)
			return;
		cgv::render::spline_tube_renderer& str = ref_spline_tube_renderer(ctx);
		str.set_render_style(trs);
		if (use_radii)
			str.set_sphere_array(ctx, lines.spheres);
		else {
			str.set_position_array(ctx, reinterpret_cast<const cgv::vec3*>(lines.spheres.data()), lines.spheres.size(), sizeof(cgv::vec4));
			str.remove_radius_array(ctx);
		}

		if (use_tangents)
			str.set_tangent_array(ctx, lines.tangents);
		else
			str.remove_tangent_array(ctx);
		
		if (use_colors)
			str.set_color_array(ctx, lines.colors);
		else
			str.remove_color_array(ctx);
		str.set_indices(ctx, lines.indices);
		str.render(ctx, 0, 2*nr_tubes);
	}
	void draw_control_geometry(cgv::render::context& ctx) {
		auto& cr = cgv::render::ref_cone_renderer(ctx);
		cr.set_render_style(crs);
		cr.set_sphere_array(ctx, &lines.bezier_tubes.data()->col(0), 4 * lines.bezier_tubes.size());
		std::vector<uint32_t> indices;
		for (size_t i = 0; i < nr_tubes; ++i) {
			indices.push_back(uint32_t(4 * i));
			indices.push_back(uint32_t(4 * i + 1));
			indices.push_back(uint32_t(4 * i + 1));
			indices.push_back(uint32_t(4 * i + 2));
			indices.push_back(uint32_t(4 * i + 2));
			indices.push_back(uint32_t(4 * i + 3));
		}
		cr.set_indices(ctx, indices);
		cr.render(ctx, 0, indices.size());
	}
	void draw_intersections(cgv::render::context& ctx) {
		if (intersections.empty())
			return;
		auto& sr = cgv::render::ref_surfel_renderer(ctx);
		sr.set_render_style(urs);
		sr.set_position_array(ctx, intersections);
		sr.set_normal_array(ctx, intersection_normals);
		sr.set_color_array(ctx, intersection_colors);
		sr.render(ctx, 0, intersections.size());
	}
	void draw(cgv::render::context& ctx) {
		last_MPW = ctx.get_modelview_projection_window_matrix();
		if (show_tube)
			draw_tubes(ctx);
		if (show_cameras)
			draw_cameras(ctx);
		if (show_rays)
			draw_rays(ctx);
		if (show_control_geometry)
			draw_control_geometry(ctx);
		if (show_intersections)
			draw_intersections(ctx);
	}
	void create_gui() {
		add_decorator("Thick Line View", "heading", "level=2");
		add_member_control(this, "Control Point", current_control_point, "value_slider", "min=-1;ticks=true")->
			set("max", 3*lines.indices.size()/2);
		add_member_control(this, "Nr Tubes", nr_tubes, "value_slider", "min=0;ticks=true")->
			set("max", lines.indices.size() / 2);
		add_member_control(this, "Show Tubes", show_tube, "check");
		add_member_control(this, "Show Control Geometry", show_control_geometry, "check");
		add_member_control(this, "Show Cameras", show_cameras, "check");
		connect_copy(add_button("Sample Rays")->click, cgv::signal::rebind(this, &thick_line_viewer::sample_rays));
		add_member_control(this, "Show Rays", show_rays, "check");
		add_member_control(this, "Primitive", primitive, "dropdown", "enums='spheres,control geometry,sphere tube,circle tube,view aligned ribbon'");
		add_member_control(this, "Precision", precision, "dropdown", "enums='float,double'");
		add_member_control(this, "Solver", solver, "dropdown", "enums='recursive,Sturm'");
		add_member_control(this, "Strategy", strategy, "dropdown", "enums='direct,filtered'");
		add_member_control(this, "Eps32", eps32, "value_slider", "min=0.0000001;step=0.000000001;max=0.01;log=true;ticks=true");
		add_member_control(this, "Eps64", eps64, "value_slider", "min=0.000000001;step=0.00000000001;max=0.01;log=true;ticks=true");

		connect_copy(add_button("Intersect Rays")->click, cgv::signal::rebind(this, &thick_line_viewer::intersect_rays));
		add_member_control(this, "Show Intersections", show_intersections, "check");
		add_member_control(this, "Ray Color Offset", ray_color_offset, "value_slider", "min=0.01;max=20;ticks=true;log=true");
		add_member_control(this, "Ray Color Scale", ray_color_scale, "value_slider", "min=0.01;max=2;ticks=true;log=true");
		add_member_control(this, "Subsampling", (cgv::type::DummyEnum&)subsampling, "dropdown", "enums='1=1,2=2,4=4,8=8,16=16,32=32,64=64,128=128'");
		add_member_control(this, "Ray Offset", ray_offset, "value_slider", "min=0.1;max=20;ticks=true;log=true");
		add_member_control(this, "Ray Length", ray_length, "value_slider", "min=0.1;max=20;ticks=true;log=true");
		add_member_control(this, "Camera Index", camera_index, "value_slider", "min=0;ticks=true")->
			set("max", cam_man.max_index());
		add_member_control(this, "Use Radii", use_radii, "check");
		add_member_control(this, "Use Tangents", use_tangents, "check");
		add_member_control(this, "Use Colors", use_colors, "check");
		if (begin_tree_node("Sphere Rendering", srs)) {
			align("\a");
			add_gui("sphere_style", srs);
			align("\b");
			end_tree_node(srs);
		}
		if (begin_tree_node("Cone Rendering", crs)) {
			align("\a");
			add_gui("cone_style", crs);
			align("\b");
			end_tree_node(crs);
		}
		if (begin_tree_node("Arrow Rendering", ars)) {
			align("\a");
			add_gui("arrow_style", ars);
			align("\b");
			end_tree_node(ars);
		}
		if (begin_tree_node("Ray Rendering", ars2)) {
			align("\a");
			add_gui("ray_style", ars2);
			align("\b");
			end_tree_node(ars2);
		}
		if (begin_tree_node("Surfel Rendering", urs)) {
			align("\a");
			add_gui("surfel_style", urs);
			align("\b");
			end_tree_node(urs);
		}
		if (begin_tree_node("Spline Tube Rendering", trs)) {
			align("\a");
			add_gui("spline_tubes_style", trs);
			align("\b");
			end_tree_node(trs);
		}
	}
};


#include <cgv/base/register.h>

extern cgv::base::object_registration<thick_line_viewer> thick_line_viewer_reg("thick_line_viewer");

#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("stereo_view_interactor;viewer");
#endif

