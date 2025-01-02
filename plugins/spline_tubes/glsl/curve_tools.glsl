#version 330 core

//////
//
// Shader module interface definition
//

///***** begin interface of curve_tools.glsl **********************************/
// --- link dependencies ---------------
/* transform.glsl */
// --- structs -------------------------
// Representation of a 3D axis-aligned box
struct AABox3 {
	vec3 pmin, pmax;
};
// --- functions -----------------------
float _pi();
float _inv_pi();
float _inf();
float _neg_inf();
float _eps();
float _sqrt_eps();
vec3  _inf3();
vec3  _neg_inf3();
AABox3 _initbox3();
mat4  _h2b();
mat4  _b2h();
mat4  _b2m();
mat4  _m2b();
mat3  _b2m_2();
mat3  _m2b_2();
mat2  _b2m_1();
mat2  _m2b_1();
void   aabox3_add_point (inout AABox3 box, const vec3 point);
vec3   aabox3_center (const AABox3 box);
vec3   aabox3_extent (const AABox3 box);
vec3   aabox3_half_extent (const AABox3 box);
vec2   aabox3_project_onto_dir (const AABox3 box, const vec3 dir);
float  eval_bezier (const vec2 bezier, const float t);
vec2   eval_bezier (const mat2 bezier, const float t);
vec3   eval_bezier (const mat2x3 bezier, const float t);
vec4   eval_bezier (const mat2x4 bezier, const float t);
float  eval_bezier (const vec3 bezier, const float t);
vec2   eval_bezier (const mat3x2 bezier, const float t);
vec3   eval_bezier (const mat3 bezier, const float t);
vec4   eval_bezier (const mat3x4 bezier, const float t);
float  eval_bezier (const vec4 bezier, const float t);
vec2   eval_bezier (const mat4x2 bezier, const float t);
vec3   eval_bezier (const mat4x3 bezier, const float t);
vec4   eval_bezier (const mat4 bezier, const float t);
vec4   to_bezier (const vec4 hermite);
mat4x2 to_bezier (const mat4x2 hermite);
mat4x3 to_bezier (const mat4x3 hermite);
mat4   to_bezier (const mat4 hermite);
vec4   to_hermite (const vec4 bezier);
mat4x2 to_hermite (const mat4x2 bezier);
mat4x3 to_hermite (const mat4x3 bezier);
mat4   to_hermite (const mat4 bezier);
vec4   to_monomial (const vec4 bezier);
mat4x2 to_monomial (const mat4x2 bezier);
mat4x3 to_monomial (const mat4x3 bezier);
mat4   to_monomial (const mat4 bezier);
vec3   to_monomial (const vec3 bezier);
mat3x2 to_monomial (const mat3x2 bezier);
mat3   to_monomial (const mat3 bezier);
mat3x4 to_monomial (const mat3x4 bezier);
vec2   to_monomial (const vec2 bezier);
mat2   to_monomial (const mat2 bezier);
mat2x3 to_monomial (const mat2x3 bezier);
mat2x4 to_monomial (const mat2x4 bezier);
vec4   transform_bezier_h (const vec4 bezier, const mat2 trans_h);
mat4x2 transform_bezier_h (const mat4x2 bezier, const mat3 trans_h);
mat4x3 transform_bezier_h (const mat4x3 bezier, const mat4 trans_h);
vec3   transform_bezier_h (const vec3 bezier, const mat2 trans_h);
mat3x2 transform_bezier_h (const mat3x2 bezier, const mat3 trans_h);
mat3   transform_bezier_h (const mat3 bezier, const mat4 trans_h);
vec3   derive_bezier (const vec4 bezier);
mat3x2 derive_bezier (const mat4x2 bezier);
mat3   derive_bezier (const mat4x3 bezier);
mat3x4 derive_bezier (const mat4 bezier);
vec2   derive_bezier (const vec3 bezier);
mat2   derive_bezier (const mat3x2 bezier);
mat2x3 derive_bezier (const mat3 bezier);
mat2x4 derive_bezier (const mat3x4 bezier);
float  derive_bezier (const vec2 bezier);
vec2   derive_bezier (const mat2x2 bezier);
vec3   derive_bezier (const mat2x3 bezier);
vec4   derive_bezier (const mat2x4 bezier);
vec3   derive_monomial (const vec4 monomial);
mat3x2 derive_monomial (const mat4x2 monomial);
mat3   derive_monomial (const mat4x3 monomial);
mat3x4 derive_monomial (const mat4 monomial);
vec2   derive_monomial (const vec3 monomial);
mat2   derive_monomial (const mat3x2 monomial);
mat2x3 derive_monomial (const mat3 monomial);
mat2x4 derive_monomial (const mat3x4 monomial);
float  derive_monomial (const vec2 monomial);
vec2   derive_monomial (const mat2 monomial);
vec3   derive_monomial (const mat2x3 monomial);
vec4   derive_monomial (const mat2x4 monomial);
vec3   solve_quadratic (const float _negb, const float _2a, const float D, const float _2c);
vec3   solve_quadratic (const vec3 monomial);
mat2x3 solve_quadratic (const mat3x2 monomial);
mat3   solve_quadratic (const mat3 monomial);
mat4x3 solve_quadratic (const mat3x4 monomial);
vec2   solve_linear (const vec2 monomial);
mat2   solve_linear (const mat2 monomial);
mat3x2 solve_linear (const mat2x3 monomial);
mat4x2 solve_linear (const mat2x4 monomial);
void   split_bezier (out vec4 b0, out vec4 b1, const vec4 bezier);
void   split_bezier (out mat4x2 b0, out mat4x2 b1, const mat4x2 bezier);
void   split_bezier (out mat4x3 b0, out mat4x3 b1, const mat4x3 bezier);
void   split_bezier (out mat4 b0, out mat4 b1, const mat4 bezier);
void   split_bezier (out vec4 b_half, const vec4 bezier, const bool second);
void   split_bezier (out mat4x2 b_half, const mat4x2 bezier, const bool second);
void   split_bezier (out mat4x3 b_half, const mat4x3 bezier, const bool second);
void   split_bezier (out mat4 b_half, const mat4 bezier, const bool second);
vec3   bitangent (const mat4x3 bezier, const mat3 dbezier, const float t, const vec3 eye);
vec3   bitangent (const mat4x3 bezier, const float t, const vec3 eye);
vec3   bitangent (const mat4x3 bezier, const int end, const vec3 eye);
vec3   bitangent0(const mat4x3 bezier, const vec3 eye);
vec3   bitangent1(const mat4x3 bezier, const vec3 eye);
vec3   bitangent (const mat3 bezier, const mat2x3 dbezier, const float t, const vec3 eye);
vec3   bitangent (const mat3 bezier, const float t, const vec3 eye);
vec3   bitangent (const mat3 bezier, const int end, const vec3 eye);
vec3   bitangent0(const mat3 bezier, const vec3 eye);
vec3   bitangent1(const mat3 bezier, const vec3 eye);
AABox3 curve_bounds (const mat4x3 bezier, const float tmin, const float tmax);
AABox3 curve_bounds (const mat3 bezier, const float tmin, const float tmax);
AABox3 curve_bounds (const mat4x3 bezier, const vec4 radius, const float tmin, const float tmax);
AABox3 curve_bounds (const mat3 bezier, const vec3 radius, const float tmin, const float tmax);
AABox3 ribbon_bounds (const mat4x3 bezier, const mat3 dbezier, const vec4 radius, const vec3 eye);
AABox3 ribbon_bounds (out mat2x3 bitangets, const mat4x3 bezier, const mat3 dbezier, const vec4 radius,
                      const vec3 eye);
AABox3 ribbon_bounds (const mat3 bezier, const mat2x3 dbezier, const vec3 radius, const vec3 eye);
AABox3 ribbon_bounds (out mat2x3 bitangets, const mat3 bezier, const mat2x3 dbezier, const vec3 radius,
                      const vec3 eye);
vec3   curve_prinicipal_dir (const mat4x3 bezier);
vec3   curve_prinicipal_dir (const mat3 bezier);
mat3   curve_prinicipal_dirs (const mat4x3 bezier);
mat3   curve_prinicipal_dirs (const mat3 bezier);
mat4   curve_obb_trans (const mat4x3 curve, const vec4 radius);
mat4   curve_obb_trans (const vec3 ref_dir, const mat4x3 curve, const vec4 radius);
mat4   curve_obb_trans (const mat3 curve, const vec3 radius);
mat4   curve_obb_trans (const vec3 ref_dir, const mat3 curve, const vec3 radius);
mat4   curve_system_trans (const mat4x3 curve);
mat4   curve_system_trans (const mat3 curve);
float  nonlinearness (const float p0, const float p1, const float p2, const float t1);
float  nonlinearness (const vec2 p0, const vec2 p1, const vec2 p2, const float t1);
float  nonlinearness (const vec3 p0, const vec3 p1, const vec3 p2, const float t1);
float  nonlinearness (const vec4 p0, const vec4 p1, const vec4 p2, const float t1);
float  nonlinearness (const vec4 bezier);
float  nonlinearness (const mat4x2 bezier);
float  nonlinearness (const mat4x3 bezier);
float  nonlinearness (const mat4 bezier);
///***** end interface of curve_tools.glsl ************************************/



//////
//
// Private imports
//

///***** begin interface of transform.glsl ************************************/
vec3   ortho_vec (const vec3 v);
void   make_orthonormal_basis (out vec3 e0, out vec3 e1, const vec3 ref);
mat2x3 make_orthonormal_basis (const vec3 ref);
void   make_orthonormal_system_x (out mat3 M, const vec3 ref);
mat3   make_orthonormal_system_x (const vec3 ref);
void   make_orthonormal_system_y (out mat3 M, const vec3 ref);
mat3   make_orthonormal_system_y (const vec3 ref);
void   make_orthonormal_system_z (out mat3 M, const vec3 ref);
mat3   make_orthonormal_system_z (const vec3 ref);
void   make_local_frame (out mat4 M, const vec3 x, const vec3 y, const vec3 z, const vec3 o);
mat4   make_local_frame (const vec3 x, const vec3 y, const vec3 z, const vec3 o);
///***** end interface of transforms.glsl *************************************/



//////
//
// Constants
//

// The number Pi.
const float pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117;

// The inverse of the number Pi.
const float pi_inv = 0.31830988618379067153776752674502872406891929148091289749533468811779359526845307018023113407739;

// Positive infinity.
const float inf = 1./0.;

// Negative infinity.
const float neg_inf = -inf;

// A 3D vector with all components initialized to positive infinity.
const vec3 inf3 = vec3(inf);

// A 3D vector with all components initialized to negative infinity.
const vec3 neg_inf3 = vec3(neg_inf);

// An initialized 0-volume 3D axis-aligned bounding box.
const AABox3 initbox3 = AABox3(inf3, neg_inf3);

// The IEEE 754 single precision epsilon
const float eps = 1.19209290e-07;

// The square root of the IEEE 754 single precision epsilon
const float sqrt_eps = .000345266977;

// Cubic Hermite to Bezier basis transform
const mat4 h2b = mat4(
	vec4( 1,    0,     0,     0),
	vec4( 1,    1/3.,  0,     0),
	vec4( 0,    0,    -1/3.,  1),
	vec4( 0,    0,     0,     1)
);
// Cubic Bezier to Hermite basis transform
const mat4 b2h = mat4(
	vec4( 1,    0,     0,     0),
	vec4(-3,    3,     0,     0),
	vec4( 0,    0,    -3,     3),
	vec4( 0,    0,     0,     1)
);

// Cubic Bezier to monomial basis transform
const mat4 b2m = mat4(
	vec4(-1,    3,    -3,     1),
	vec4( 3,   -6,     3,     0),
	vec4(-3,    3,     0,     0),
	vec4( 1,    0,     0,     0)
);
// Cubic monomial to Bezier basis transform
const mat4 m2b = mat4(
	vec4( 0,    0,     0,     1),
	vec4( 0,    0,     1/3.,  1),
	vec4( 0,    1/3.,  2/3.,  1),
	vec4( 1,    1,     1,     1)
);

// Quadratic Bezier to monomial basis transform
const mat3 b2m_2 = mat3(
	vec3( 1,   -2,     1),
	vec3(-2,    2,     0),
	vec3( 1,    0,     0)
);
// Quadratic monomial to Bezier basis transform
const mat3 m2b_2 = mat3(
	vec3( 0,    0,     1),
	vec3( 0,    .5,    1),
	vec3( 1,    1,     1)
);

// Linear Bezier to monomial basis transform
const mat2 b2m_1 = mat2(
	vec2(-1,    1),
	vec2( 1,    0)
);
// Linear monomial to Bezier basis transform
const mat2 m2b_1 = mat2(
	vec2( 0,    1),
	vec2( 1,    1)
);



//////
//
// Functions
//

////
// Constant accessors (for use by external modules).

float _pi() { return pi; }
float _inv_pi() { return pi_inv; }
float _inf() { return inf; }
float _neg_inf() { return neg_inf; }
float _eps() { return eps; }
float _sqrt_eps() { return sqrt_eps; }
vec3 _inf3() { return inf3; }
vec3 _neg_inf3() { return neg_inf3; }
AABox3 _initbox3() { return initbox3; }
mat4 _h2b() { return h2b; }
mat4 _b2h() { return b2h; }
mat4 _b2m() { return b2m; }
mat4 _m2b() { return m2b; }
mat3 _b2m_2() { return b2m_2; }
mat3 _m2b_2() { return m2b_2; }
mat2 _b2m_1() { return b2m_1; }
mat2 _m2b_1() { return m2b_1; }


////
// Axis-aligned box utilities

// Add the given 3D point to the axis-aligned box.
void aabox3_add_point (inout AABox3 box, const vec3 point) {
	box.pmin = min(box.pmin, point);
	box.pmax = max(box.pmax, point);
}

// Compute the center point of the given 3D axis-aligned box.
vec3 aabox3_center (const AABox3 box) {
	return .5*(box.pmin + box.pmax);
}

// Compute the extent of the given 3D axis-aligned box.
vec3 aabox3_extent (const AABox3 box) {
	return box.pmax - box.pmin;
}

// Compute the half-extent of the given 3D axis-aligned box.
vec3 aabox3_half_extent (const AABox3 box) {
	return .5*(box.pmax - box.pmin);
}

// Finds the projection of the given 3D axis-aligned box onto the given 3D direction, returning an ordered pair of values
// corresponding to the projections of the frontmost and backmost (relative to the given direction) points of the box,
// respectively.
vec2 aabox3_project_onto_dir (const AABox3 box, const vec3 dir)
{
	vec3 he = aabox3_half_extent(box), center = aabox3_center(box);
	float center_p = dot(dir, center),
	      radius = he[0]*abs(dir[0]) + he[1]*abs(dir[1]) + he[2]*abs(dir[2]);
	return vec2(center_p-radius, center_p+radius);
}


////
// Curve evaluators

// Evaluate the given scalar linear bezier curve.
float eval_bezier (const vec2 bezier, const float t) {
	// De-Casteljau level 0 (already equals final result)
	return mix(bezier[0], bezier[1], t);
}

// Evaluate the given 2D linear bezier curve.
vec2 eval_bezier (const mat2 bezier, const float t) {
	// De-Casteljau level 0 (already equals final result)
	return mix(bezier[0], bezier[1], t);
}

// Evaluate the given 3D linear bezier curve.
vec3 eval_bezier (const mat2x3 bezier, const float t) {
	// De-Casteljau level 0 (already equals final result)
	return mix(bezier[0], bezier[1], t);
}

// Evaluate the given 4D linear bezier curve.
vec4 eval_bezier (const mat2x4 bezier, const float t) {
	// De-Casteljau level 0 (already equals final result)
	return mix(bezier[0], bezier[1], t);
}

// Evaluate the given scalar quadratic bezier curve.
float eval_bezier (const vec3 bezier, const float t)
{
	// De-Casteljau level 0
	vec2 v = vec2(mix(bezier[0], bezier[1], t), mix(bezier[1], bezier[2], t));

	// De-Casteljau level 1 (final result)
	return mix(v[0], v[1], t);
}

// Evaluate the given 2D quadratic bezier curve.
vec2 eval_bezier (const mat3x2 bezier, const float t)
{
	// De-Casteljau level 0
	mat2 v = mat2(mix(bezier[0], bezier[1], t), mix(bezier[1], bezier[2], t));

	// De-Casteljau level 1 (final result)
	return mix(v[0], v[1], t);
}

// Evaluate the given 3D quadratic bezier curve.
vec3 eval_bezier (const mat3 bezier, const float t)
{
	// De-Casteljau level 0
	mat2x3 v = mat2x3(mix(bezier[0], bezier[1], t), mix(bezier[1], bezier[2], t));

	// De-Casteljau level 1 (final result)
	return mix(v[0], v[1], t);
}

// Evaluate the given 4D quadratic bezier curve.
vec4 eval_bezier (const mat3x4 bezier, const float t)
{
	// De-Casteljau level 0
	mat2x4 v = mat2x4(mix(bezier[0], bezier[1], t), mix(bezier[1], bezier[2], t));

	// De-Casteljau level 1 (final result)
	return mix(v[0], v[1], t);
}

// Evaluate the given scalar cubic bezier curve.
float eval_bezier (const vec4 bezier, const float t)
{
	// De-Casteljau level 0
	vec3 v = vec3(
		mix(bezier[0], bezier[1], t), mix(bezier[1], bezier[2], t), mix(bezier[2], bezier[3], t)
	);

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], t);
	v[1] = mix(v[1], v[2], t);

	// De-Casteljau level 2 (final result)
	return mix(v[0], v[1], t);
}

// Evaluate the given 2D cubic bezier curve.
vec2 eval_bezier (const mat4x2 bezier, const float t)
{
	// De-Casteljau level 0
	mat3x2 v = mat3x2(
		mix(bezier[0], bezier[1], t), mix(bezier[1], bezier[2], t), mix(bezier[2], bezier[3], t)
	);

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], t);
	v[1] = mix(v[1], v[2], t);

	// De-Casteljau level 2 (final result)
	return mix(v[0], v[1], t);
}

// Evaluate the given 3D cubic bezier curve.
vec3 eval_bezier (const mat4x3 bezier, const float t)
{
	// De-Casteljau level 0
	mat3 v = mat3(
		mix(bezier[0], bezier[1], t), mix(bezier[1], bezier[2], t), mix(bezier[2], bezier[3], t)
	);

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], t);
	v[1] = mix(v[1], v[2], t);

	// De-Casteljau level 2 (final result)
	return mix(v[0], v[1], t);
}

// Evaluate the given 4D cubic bezier curve.
vec4 eval_bezier (const mat4 bezier, const float t)
{
	// De-Casteljau level 0
	mat3x4 v = mat3x4(
		mix(bezier[0], bezier[1], t), mix(bezier[1], bezier[2], t), mix(bezier[2], bezier[3], t)
	);

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], t);
	v[1] = mix(v[1], v[2], t);

	// De-Casteljau level 2 (final result)
	return mix(v[0], v[1], t);
}


////
// Curve transformations

/*--- Hermite to Bezier basis transforms ------------------------------------------------*/
// Transform the given scalar Hermite control points into Bezier basis.
vec4 to_bezier (const vec4 hermite) {
	return hermite * h2b;
}

// Transform the given 2D Hermite control points into Bezier basis.
mat4x2 to_bezier (const mat4x2 hermite) {
	return hermite * h2b;
}

// Transform the given 3D Hermite control points into Bezier basis.
mat4x3 to_bezier (const mat4x3 hermite) {
	return hermite * h2b;
}

// Transform the given 4D Hermite control points into Bezier basis.
mat4 to_bezier (const mat4 hermite) {
	return hermite * h2b;
}

/*--- Bezier to Hermite basis transforms ------------------------------------------------*/
// Transform the given scalar cubic Bezier control points into Hermite basis.
vec4 to_hermite (const vec4 bezier) {
	return bezier * b2h;
}

// Transform the given 2D cubic Bezier control points into Hermite basis.
mat4x2 to_hermite (const mat4x2 bezier) {
	return bezier * b2h;
}

// Transform the given 3D cubic Bezier control points into Hermite basis.
mat4x3 to_hermite (const mat4x3 bezier) {
	return bezier * b2h;
}

// Transform the given 4D cubic Bezier control points into Hermite basis.
mat4 to_hermite (const mat4 bezier) {
	return bezier * b2h;
}

/*--- Bezier to monomial basis transforms ------------------------------------------------*/
// Transform the given scalar cubic Bezier control points into monomial basis.
vec4 to_monomial (const vec4 bezier) {
	return bezier * b2m;
}

// Transform the given 2D cubic Bezier control points into monomial basis.
mat4x2 to_monomial (const mat4x2 bezier) {
	return bezier * b2m;
}

// Transform the given 3D cubic Bezier control points into monomial basis.
mat4x3 to_monomial (const mat4x3 bezier) {
	return bezier * b2m;
}

// Transform the given 4D cubic Bezier control points into monomial basis.
mat4 to_monomial (const mat4 bezier) {
	return bezier * b2m;
}

// Transform the given scalar quadratic Bezier control points into monomial basis.
vec3 to_monomial (const vec3 bezier) {
	return bezier * b2m_2;
}

// Transform the given 2D quadratic Bezier control points into monomial basis.
mat3x2 to_monomial (const mat3x2 bezier) {
	return bezier * b2m_2;
}

// Transform the given 3D quadratic Bezier control points into monomial basis.
mat3 to_monomial (const mat3 bezier) {
	return bezier * b2m_2;
}

// Transform the given 4D quadratic Bezier control points into monomial basis.
mat3x4 to_monomial (const mat3x4 bezier) {
	return bezier * b2m_2;
}

// Transform the given scalar linear Bezier control points into monomial basis.
vec2 to_monomial (const vec2 bezier) {
	return bezier * b2m_1;
}

// Transform the given 2D linear Bezier control points into monomial basis.
mat2 to_monomial (const mat2 bezier) {
	return bezier * b2m_1;
}

// Transform the given 3D linear Bezier control points into monomial basis.
mat2x3 to_monomial (const mat2x3 bezier) {
	return bezier * b2m_1;
}

// Transform the given 4D linear Bezier control points into monomial basis.
mat2x4 to_monomial (const mat2x4 bezier) {
	return bezier * b2m_1;
}

/*--- Arbitrary Bezier-basis control point transformations ------------------------------*/
// Transform the given scalar cubic bezier control points with the given homogenous affine 1D transformation.
vec4 transform_bezier_h (const vec4 bezier, const mat2 trans_h) {
	return vec4(trans_h * mat4x2(
		vec2(bezier[0], 1), vec2(bezier[1], 1), vec2(bezier[2], 1), vec2(bezier[3], 1)
	));
}

// Transform the given 2D cubic bezier control points with the given homogenous affine 2D transformation.
mat4x2 transform_bezier_h (const mat4x2 bezier, const mat3 trans_h) {
	return mat4x2(trans_h * mat4x3(
		vec3(bezier[0], 1), vec3(bezier[1], 1), vec3(bezier[2], 1), vec3(bezier[3], 1)
	));
}

// Transform the given 3D cubic bezier control points with the given homogenous affine 3D transformation.
mat4x3 transform_bezier_h (const mat4x3 bezier, const mat4 trans_h) {
	return mat4x3(trans_h * mat4(
		vec4(bezier[0], 1), vec4(bezier[1], 1), vec4(bezier[2], 1), vec4(bezier[3], 1)
	));
}

// Transform the given scalar quadratic bezier control points with the given homogenous affine 1D transformation.
vec3 transform_bezier_h (const vec3 bezier, const mat2 trans_h) {
	return vec3(trans_h * mat3x2(
		vec2(bezier[0], 1), vec2(bezier[1], 1), vec2(bezier[2], 1)
	));
}

// Transform the given 2D quadratic bezier control points with the given homogenous affine 2D transformation.
mat3x2 transform_bezier_h (const mat3x2 bezier, const mat3 trans_h) {
	return mat3x2(trans_h * mat3(
		vec3(bezier[0], 1), vec3(bezier[1], 1), vec3(bezier[2], 1)
	));
}

// Transform the given 3D quadratic bezier control points with the given homogenous affine 3D transformation.
mat3 transform_bezier_h (const mat3 bezier, const mat4 trans_h) {
	return mat3(trans_h * mat3x4(
		vec4(bezier[0], 1), vec4(bezier[1], 1), vec4(bezier[2], 1)
	));
}


////
// Curve derivatives

/*--- Bezier-basis derivatives -----------------------------------------------------------*/
// Compute derivative of the given scalar cubic Bezier curve.
vec3 derive_bezier (const vec4 bezier) {
	float b10 = bezier[1]-bezier[0], b21 = bezier[2]-bezier[1], b32 = bezier[3]-bezier[2];
	return vec3(b10+b10+b10, b21+b21+b21, b32+b32+b32);
}

// Compute derivative of the given 2D cubic Bezier curve.
mat3x2 derive_bezier (const mat4x2 bezier) {
	vec2 b10 = bezier[1]-bezier[0], b21 = bezier[2]-bezier[1], b32 = bezier[3]-bezier[2];
	return mat3x2(b10+b10+b10, b21+b21+b21, b32+b32+b32);
}

// Compute derivative of the given 3D cubic Bezier curve.
mat3 derive_bezier (const mat4x3 bezier) {
	vec3 b10 = bezier[1]-bezier[0], b21 = bezier[2]-bezier[1], b32 = bezier[3]-bezier[2];
	return mat3(b10+b10+b10, b21+b21+b21, b32+b32+b32);
}

// Compute derivative of the given 4D cubic Bezier curve.
mat3x4 derive_bezier (const mat4 bezier) {
	vec4 b10 = bezier[1]-bezier[0], b21 = bezier[2]-bezier[1], b32 = bezier[3]-bezier[2];
	return mat3x4(b10+b10+b10, b21+b21+b21, b32+b32+b32);
}

// Compute derivative of the given scalar quadratic Bezier curve.
vec2 derive_bezier (const vec3 bezier) {
	float b10 = bezier[1]-bezier[0], b21 = bezier[2]-bezier[1];
	return vec2(b10+b10, b21+b21);
}

// Compute derivative of the given 2D quadratic Bezier curve.
mat2 derive_bezier (const mat3x2 bezier) {
	vec2 b10 = bezier[1]-bezier[0], b21 = bezier[2]-bezier[1];
	return mat2(b10+b10, b21+b21);
}

// Compute derivative of the given 3D quadratic Bezier curve.
mat2x3 derive_bezier (const mat3 bezier) {
	vec3 b10 = bezier[1]-bezier[0], b21 = bezier[2]-bezier[1];
	return mat2x3(b10+b10, b21+b21);
}

// Compute derivative of the given 4D quadratic Bezier curve.
mat2x4 derive_bezier (const mat3x4 bezier) {
	vec4 b10 = bezier[1]-bezier[0], b21 = bezier[2]-bezier[1];
	return mat2x4(b10+b10, b21+b21);
}

// Compute derivative of the given scalar linear Bezier curve.
float derive_bezier (const vec2 bezier) {
	return bezier[1] - bezier[0];
}

// Compute derivative of the given 2D linear Bezier curve.
vec2 derive_bezier (const mat2x2 bezier) {
	return bezier[1] - bezier[0];
}

// Compute derivative of the given 3D linear Bezier curve.
vec3 derive_bezier (const mat2x3 bezier) {
	return bezier[1] - bezier[0];
}

// Compute derivative of the given 4D linear Bezier curve.
vec4 derive_bezier (const mat2x4 bezier) {
	return bezier[1] - bezier[0];
}

/*--- Monomial-basis derivatives -----------------------------------------------------------*/
// Compute derivative of the given scalar cubic monomial curve.
vec3 derive_monomial (const vec4 monomial) {
	return vec3(monomial[0]+monomial[0]+monomial[0], monomial[1]+monomial[1], monomial[2]);
}

// Compute derivative of the given 2D cubic monomial curve.
mat3x2 derive_monomial (const mat4x2 monomial) {
	return mat3x2(monomial[0]+monomial[0]+monomial[0], monomial[1]+monomial[1], monomial[2]);
}

// Compute derivative of the given 3D cubic monomial curve.
mat3 derive_monomial (const mat4x3 monomial) {
	return mat3(monomial[0]+monomial[0]+monomial[0], monomial[1]+monomial[1], monomial[2]);
}

// Compute derivative of the given 4D cubic monomial curve.
mat3x4 derive_monomial (const mat4 monomial) {
	return mat3x4(monomial[0]+monomial[0]+monomial[0], monomial[1]+monomial[1], monomial[2]);
}

// Compute derivative of the given scalar quadratic monomial curve.
vec2 derive_monomial (const vec3 monomial) {
	return vec2(monomial[0]+monomial[0], monomial[1]);
}

// Compute derivative of the given 2D quadratic monomial curve.
mat2 derive_monomial (const mat3x2 monomial) {
	return mat2(monomial[0]+monomial[0], monomial[1]);
}

// Compute derivative of the given 3D quadratic monomial curve.
mat2x3 derive_monomial (const mat3 monomial) {
	return mat2x3(monomial[0]+monomial[0], monomial[1]);
}

// Compute derivative of the given 4D quadratic monomial curve.
mat2x4 derive_monomial (const mat3x4 monomial) {
	return mat2x4(monomial[0]+monomial[0], monomial[1]);
}

// Compute derivative of the given scalar linear monomial curve.
float derive_monomial (const vec2 monomial) {
	return monomial[0];
}

// Compute derivative of the given 2D linear monomial curve.
vec2 derive_monomial (const mat2 monomial) {
	return monomial[0];
}

// Compute derivative of the given 3D linear monomial curve.
vec3 derive_monomial (const mat2x3 monomial) {
	return monomial[0];
}

// Compute derivative of the given 4D linear monomial curve.
vec4 derive_monomial (const mat2x4 monomial) {
	return monomial[0];
}


////
// Root finders

// Solve for the linear root
vec3 solve_quadratic (const float _negb, const float _2a, const float D, const float _2c)
{
	if (_2a != 0)
	{
		// Quadratic
		if (D > 0) {
			float sqrtD = sqrt(D);
			return vec3(2, (_negb-sqrtD)/_2a, (_negb+sqrtD)/_2a);
		}
		else if (D == 0)
			return vec3(1, _negb/_2a, 0);
	}
	else if (_negb != 0)
		// Linear
		return vec3(1, _2c/(2*_negb), 0);

	// No solution
	return vec3(0);
}

// Solve the quadratic formula for the given monomial scalar curve, returning the real solutions per dimension
// in the corresponding column of the result matrix, with the very first row indicating the number of
// real solutions for that dimension
vec3 solve_quadratic (const vec3 monomial) {
	// Compute common terms and discriminant and delegate
	float _negb = -monomial[1], _2a = monomial[0]*2, _2c = monomial[2]*2, D = _negb*_negb - _2a*_2c;
	return solve_quadratic(_negb, _2a, D, _2c);
}

// Solve the quadratic formula for the given monomial 2D curve, returning the real solutions per dimension
// in the corresponding column of the result matrix, with the very first row indicating the number of
// real solutions for that dimension
mat2x3 solve_quadratic (const mat3x2 monomial)
{
	// Result matrix
	mat2x3 result;

	// Common terms and discriminant
	vec2 _negb = -monomial[1], _2a = monomial[0]*2, _2c = monomial[2]*2, D = _negb*_negb - _2a*_2c;

	// Solve each component polynomial individually
	for (int i=0; i<2; i++)
		result[i] = solve_quadratic(_negb[i], _2a[i], D[i], _2c[i]);
	return result;
}

// Solve the quadratic formula for the given monomial 3D curve, returning the real solutions per dimension
// in the corresponding column of the result matrix, with the very first row indicating the number of
// real solutions for that dimension
mat3 solve_quadratic (const mat3 monomial)
{
	// Result matrix
	mat3 result;

	// Common terms and discriminant
	vec3 _negb = -monomial[1], _2a = monomial[0]*2, _2c = monomial[2]*2, D = _negb*_negb - _2a*_2c;

	// Solve each component polynomial individually
	for (int i=0; i<3; i++)
		result[i] = solve_quadratic(_negb[i], _2a[i], D[i], _2c[i]);
	return result;
}

// Solve the quadratic formula for the given monomial 4D curve, returning the real solutions per dimension
// in the corresponding column of the result matrix, with the very first row indicating the number of
// real solutions for that dimension
mat4x3 solve_quadratic (const mat3x4 monomial)
{
	// Result matrix
	mat4x3 result;

	// Common terms and discriminant
	vec4 _negb = -monomial[1], _2a = monomial[0]*2, _2c = monomial[2]*2, D = _negb*_negb - _2a*_2c;

	// Solve each component polynomial individually
	for (int i=0; i<4; i++)
		result[i] = solve_quadratic(_negb[i], _2a[i], D[i], _2c[i]);
	return result;
}

// Calculate the root of the given scalar linear function and return it in the second element of the result vector. The first
// element will contain 1 if the root exists, and 0 otherwise.
vec2 solve_linear (const vec2 monomial) {
	return monomial[0] != 0 ? vec2(1, -monomial[1]/monomial[0]) : vec2(0, 0);
}

// Calculate the roots of the component polynomials for the given linear monomial 2D curve, returning them per dimension in the
// corresponding column of the result matrix, with the very first row indicating whether a root exists in that dimension (1)
// or not (0), and the second row containing the actual per-dimension roots.
mat2 solve_linear (const mat2 monomial)
{
	// Solve each component polynomial individually
	mat2 result;
	for (int i=0; i<2; i++)
		result[i] = solve_linear(vec2(monomial[0][i], monomial[1][i]));
	return result;
}

// Calculate the roots of the component polynomials for the given linear monomial 3D curve, returning them per dimension in the
// corresponding column of the result matrix, with the very first row indicating whether a root exists in that dimension (1)
// or not (0), and the second row containing the actual per-dimension roots.
mat3x2 solve_linear (const mat2x3 monomial)
{
	// Solve each component polynomial individually
	mat3x2 result;
	for (int i=0; i<3; i++)
		result[i] = solve_linear(vec2(monomial[0][i], monomial[1][i]));
	return result;
}

// Calculate the roots of the component polynomials for the given linear monomial 4D curve, returning them per dimension in the
// corresponding column of the result matrix, with the very first row indicating whether a root exists in that dimension (1)
// or not (0), and the second row containing the actual per-dimension roots.
mat4x2 solve_linear (const mat2x4 monomial)
{
	// Solve each component polynomial individually
	mat4x2 result;
	for (int i=0; i<4; i++)
		result[i] = solve_linear(vec2(monomial[0][i], monomial[1][i]));
	return result;
}


////
// Bezier curve tools

// Compute the control points of two scalar cubic Bezier curves that each cover one half of the given scalar cubic bezier
// curve.
void split_bezier (out vec4 b0, out vec4 b1, const vec4 bezier)
{
	// De-Casteljau level 0
	vec3 v = vec3(
		mix(bezier[0], bezier[1], .5), mix(bezier[1], bezier[2], .5), mix(bezier[2], bezier[3], .5)
	);
	float cp1_0 = v[0];

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], .5);
	v[1] = mix(v[1], v[2], .5);

	// De-Casteljau level 2 (final result)
	float joint = mix(v[0], v[1], .5);
	b0 = vec4(bezier[0], cp1_0, v[0], joint);
	b1 = vec4(joint, v[1],  v[2], bezier[3]);
}

// Compute the control points of two 2D cubic Bezier curves that each cover one half of the given 2D cubic bezier curve.
void split_bezier (out mat4x2 b0, out mat4x2 b1, const mat4x2 bezier)
{
	// De-Casteljau level 0
	mat3x2 v = mat3x2(
		mix(bezier[0], bezier[1], .5), mix(bezier[1], bezier[2], .5), mix(bezier[2], bezier[3], .5)
	);
	vec2 cp1_0 = v[0];

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], .5);
	v[1] = mix(v[1], v[2], .5);

	// De-Casteljau level 2 (final result)
	vec2 joint = mix(v[0], v[1], .5);
	b0 = mat4x2(bezier[0], cp1_0, v[0], joint);
	b1 = mat4x2(joint, v[1],  v[2], bezier[3]);
}

// Compute the control points of two 3D cubic Bezier curves that each cover one half of the given 3D cubic bezier curve.
void split_bezier (out mat4x3 b0, out mat4x3 b1, const mat4x3 bezier)
{
	// De-Casteljau level 0
	mat3 v = mat3(
		mix(bezier[0], bezier[1], .5), mix(bezier[1], bezier[2], .5), mix(bezier[2], bezier[3], .5)
	);
	vec3 cp1_0 = v[0];

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], .5);
	v[1] = mix(v[1], v[2], .5);

	// De-Casteljau level 2 (final result)
	vec3 joint = mix(v[0], v[1], .5);
	b0 = mat4x3(bezier[0], cp1_0, v[0], joint);
	b1 = mat4x3(joint, v[1],  v[2], bezier[3]);
}

// Compute the control points of two 4D cubic Bezier curves that each cover one half of the given 4D cubic bezier curve.
void split_bezier (out mat4 b0, out mat4 b1, const mat4 bezier)
{
	// De-Casteljau level 0
	mat3x4 v = mat3x4(
		mix(bezier[0], bezier[1], .5), mix(bezier[1], bezier[2], .5), mix(bezier[2], bezier[3], .5)
	);
	vec4 cp1_0 = v[0];

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], .5);
	v[1] = mix(v[1], v[2], .5);

	// De-Casteljau level 2 (final result)
	vec4 joint = mix(v[0], v[1], .5);
	b0 = mat4(bezier[0], cp1_0, v[0], joint);
	b1 = mat4(joint, v[1],  v[2], bezier[3]);
}

// Compute the control points of the scalar cubic Bezier curve that covers either exactly the first or second half of the
// given scalar cubic bezier curve
void split_bezier (out vec4 b_half, const vec4 bezier, const bool second)
{
	// De-Casteljau level 0
	vec3 v = vec3(
		mix(bezier[0], bezier[1], .5), mix(bezier[1], bezier[2], .5), mix(bezier[2], bezier[3], .5)
	);
	float cp1_0 = v[0];

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], .5);
	v[1] = mix(v[1], v[2], .5);

	// De-Casteljau level 2 (final result)
	float joint = mix(v[0], v[1], .5);
	b_half = second ? vec4(joint, v[1],  v[2], bezier[3]) : vec4(bezier[0], cp1_0, v[0], joint);
}

// Compute the control points of the 2D cubic Bezier curve that covers either exactly the first or second half of the
// given 2D cubic bezier curve
void split_bezier (out mat4x2 b_half, const mat4x2 bezier, const bool second)
{
	// De-Casteljau level 0
	mat3x2 v = mat3x2(
		mix(bezier[0], bezier[1], .5), mix(bezier[1], bezier[2], .5), mix(bezier[2], bezier[3], .5)
	);
	vec2 cp1_0 = v[0];

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], .5);
	v[1] = mix(v[1], v[2], .5);

	// De-Casteljau level 2 (final result)
	vec2 joint = mix(v[0], v[1], .5);
	b_half = second ? mat4x2(joint, v[1],  v[2], bezier[3]) : mat4x2(bezier[0], cp1_0, v[0], joint);;
}

// Compute the control points of the 3D cubic Bezier curve that covers either exactly the first or second half of the
// given 3D cubic bezier curve
void split_bezier (out mat4x3 b_half, const mat4x3 bezier, const bool second)
{
	// De-Casteljau level 0
	mat3 v = mat3(
		mix(bezier[0], bezier[1], .5), mix(bezier[1], bezier[2], .5), mix(bezier[2], bezier[3], .5)
	);
	vec3 cp1_0 = v[0];

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], .5);
	v[1] = mix(v[1], v[2], .5);

	// De-Casteljau level 2 (final result)
	vec3 joint = mix(v[0], v[1], .5);
	b_half = second ? mat4x3(joint, v[1],  v[2], bezier[3]) : mat4x3(bezier[0], cp1_0, v[0], joint);;
}

// Compute the control points of the 4D cubic Bezier curve that covers either exactly the first or second half of the
// given 4D cubic bezier curve
void split_bezier (out mat4 b_half, const mat4 bezier, const bool second)
{
	// De-Casteljau level 0
	mat3x4 v = mat3x4(
		mix(bezier[0], bezier[1], .5), mix(bezier[1], bezier[2], .5), mix(bezier[2], bezier[3], .5)
	);
	vec4 cp1_0 = v[0];

	// De-Casteljau level 1 (reuse local storage from above)
	v[0] = mix(v[0], v[1], .5);
	v[1] = mix(v[1], v[2], .5);

	// De-Casteljau level 2 (final result)
	vec4 joint = mix(v[0], v[1], .5);
	b_half = second ? mat4(joint, v[1],  v[2], bezier[3]) : mat4(bezier[0], cp1_0, v[0], joint);;
}


////
// 3D curve tools

// Find a view-aligned bitangent at the given point on the given 3D cubic Bezier curve using the pre-computed curve derivative.
vec3 bitangent (const mat4x3 bezier, const mat3 dbezier, const float t, const vec3 eye) {
	return normalize(cross(eye-eval_bezier(bezier, t), eval_bezier(dbezier, t)));
}

// Find a view-aligned bitangent at the given point on the given 3D cubic Bezier curve (computes the curve derivative internally).
vec3 bitangent (const mat4x3 bezier, const float t, const vec3 eye) {
	return normalize(cross(eye-eval_bezier(bezier, t), eval_bezier(derive_bezier(bezier), t)));
}

// Find a view-aligned bitangent at the given 3D cubic Bezier curve end point (end:0 for t=0 or end:1 for t=1).
vec3 bitangent (const mat4x3 bezier, const int end, const vec3 eye) {
	return normalize(end==0 ?   cross(eye-bezier[0], bezier[1]-bezier[0])
	                          : cross(eye-bezier[3], bezier[3]-bezier[2]));
}

// Find a view-aligned bitangent at t=0 for the given 3D cubic Bezier curve.
vec3 bitangent0 (const mat4x3 bezier, const vec3 eye) {
	return normalize(cross(eye-bezier[0], bezier[1]-bezier[0]));
}

// Find a view-aligned bitangent at t=1 for the given 3D cubic Bezier curve.
vec3 bitangent1 (const mat4x3 bezier, const vec3 eye) {
	return normalize(cross(eye-bezier[3], bezier[3]-bezier[2]));
}

// Find a view-aligned bitangent at the given point on the given 3D quadratic Bezier curve using the pre-computed curve derivative.
vec3 bitangent (const mat3 bezier, const mat2x3 dbezier, const float t, const vec3 eye) {
	return normalize(cross(eye-eval_bezier(bezier, t), eval_bezier(dbezier, t)));
}

// Find a view-aligned bitangent at the given point on the given 3D quadratic Bezier curve (computes the curve derivative internally).
vec3 bitangent (const mat3 bezier, const float t, const vec3 eye) {
	return normalize(cross(eye-eval_bezier(bezier, t), eval_bezier(derive_bezier(bezier), t)));
}

// Find a view-aligned bitangent at the given 3D quadratic Bezier curve end point (end:0 for t=0 or end:1 for t=1).
vec3 bitangent (const mat3 bezier, const int end, const vec3 eye) {
	return normalize(end==0 ?   cross(eye-bezier[0], bezier[1]-bezier[0])
	                          : cross(eye-bezier[2], bezier[2]-bezier[1]));
}

// Find a view-aligned bitangent at t=0 for the given 3D quadratic Bezier curve.
vec3 bitangent0 (const mat3 bezier, const vec3 eye) {
	return normalize(cross(eye-bezier[0], bezier[1]-bezier[0]));
}

// Find a view-aligned bitangent at t=1 for the given 3D quadratic Bezier curve.
vec3 bitangent1 (const mat3 bezier, const vec3 eye) {
	return normalize(cross(eye-bezier[2], bezier[2]-bezier[1]));
}

// Find a tight-fitting, axis-aligned bounding box around the given 3D cubic Bezier curve.
AABox3 curve_bounds (const mat4x3 bezier, const float tmin, const float tmax)
{
	// Determine curve extrema
	mat3 dmonomial = to_monomial(derive_bezier(bezier));
	mat3 roots = solve_quadratic(dmonomial);

	// Fit bounds to curve extrema
	// - initialize with just the curve start/end points
	AABox3 bounds = initbox3;
	aabox3_add_point(bounds, bezier[0]);
	aabox3_add_point(bounds, bezier[3]);
	// - consider all inflection points within range
	for (int c=0; c<3; c++)
	{
		vec3 sol_c = roots[c];
		int num_sols = int(sol_c.x);
		for (int i=1; i<=num_sols; i++) {
			float t_sol = sol_c[i];
			if (t_sol >= tmin && t_sol <= tmax) {
				vec3 pcur = eval_bezier(bezier, t_sol);
				bounds.pmin[c] = min(bounds.pmin[c], pcur[c]);
				bounds.pmax[c] = max(bounds.pmax[c], pcur[c]);
			}
		}
	}
	return bounds;
}

// Find a tight-fitting, axis-aligned bounding box around the given 3D quadratic Bezier curve.
AABox3 curve_bounds (const mat3 bezier, const float tmin, const float tmax)
{
	// Determine curve extrema
	mat2x3 dmonomial = to_monomial(derive_bezier(bezier));
	mat3x2 roots = solve_linear(dmonomial);

	// Fit bounds to curve extrema
	// - initialize with just the curve start/end points
	AABox3 bounds = initbox3;
	aabox3_add_point(bounds, bezier[0]);
	aabox3_add_point(bounds, bezier[2]);
	// - consider all inflection points within range
	for (int c=0; c<3; c++)
	{
		if (roots[c].x == 1) {
			float t_sol = roots[c].y;
			if (t_sol >= tmin && t_sol <= tmax) {
				vec3 pcur = eval_bezier(bezier, t_sol);
				bounds.pmin[c] = min(bounds.pmin[c], pcur[c]);
				bounds.pmax[c] = max(bounds.pmax[c], pcur[c]);
			}
		}
	}
	return bounds;
}

// Find a tight-fitting, axis-aligned bounding box around the given 3D cubic Bezier curve with varying radius.
AABox3 curve_bounds (const mat4x3 bezier, const vec4 radius, const float tmin, const float tmax)
{
	// Scatter radius curve over all spatial components
	mat4x3 curve_r = mat4x3(vec3(radius[0]), vec3(radius[1]), vec3(radius[2]), vec3(radius[3]));

	// Calculate bounds with radius added in both positive and negative directions
	AABox3 bounds = curve_bounds(bezier-curve_r, tmin, tmax);
	aabox3_add_point(bounds, curve_bounds(bezier+curve_r, tmin, tmax).pmax);
	return bounds;
}

// Find a tight-fitting, axis-aligned bounding box around the given 3D quadratic Bezier curve with varying radius.
AABox3 curve_bounds (const mat3 bezier, const vec3 radius, const float tmin, const float tmax)
{
	// Scatter radius curve over all spatial components
	mat3 curve_r = mat3(vec3(radius[0]), vec3(radius[1]), vec3(radius[2]));

	// Calculate bounds with radius added in both positive and negative directions
	AABox3 bounds = curve_bounds(bezier-curve_r, tmin, tmax);
	aabox3_add_point(bounds, curve_bounds(bezier+curve_r, tmin, tmax).pmax);
	return bounds;
}

// Find a tight-fitting, axis-aligned bounding box around the view-aligned ribbon spawned by the given 3D cubic
// Bezeir curve and associated varying radius.
AABox3 ribbon_bounds (const mat4x3 bezier, const mat3 dbezier, const vec4 radius, const vec3 eye)
{
	// Scatter radius curve over all spatial components
	mat4x3 curve_r = mat4x3(vec3(radius[0]), vec3(radius[1]), vec3(radius[2]), vec3(radius[3]));

	// Determine extrema
	mat3 sol[2] = mat3[2](
		// axis minus radius
		solve_quadratic(to_monomial(derive_bezier(bezier-curve_r))),
		// axis plus radius
		solve_quadratic(to_monomial(derive_bezier(bezier+curve_r)))
	);

	// Fit bounds to curve extrema
	// - initialize with just the curve start/end points
	AABox3 bounds = initbox3;
	{ vec3 bt0r = bitangent0(bezier, eye) * radius[0],
	       bt1r = bitangent1(bezier, eye) * radius[3];
	  aabox3_add_point(bounds, bezier[0]-bt0r); aabox3_add_point(bounds, bezier[0]+bt0r);
	  aabox3_add_point(bounds, bezier[3]-bt1r); aabox3_add_point(bounds, bezier[3]+bt1r); }
	// - incorporate any inflection points within range
	for (int i=0; i<2; i++) {
		for (int c=0; c<3; c++) {
			vec3 sol_c = sol[i][c];
			int num_sols = int(sol_c.x);
			for (int s=1; s<=num_sols; s++) {
				if (sol_c[s] >= 0 && sol_c[s] <= 1) {
					vec3 pcur = eval_bezier(bezier, sol_c[s]),
					     btr = bitangent(bezier, dbezier, sol_c[s], eye)*eval_bezier(radius, sol_c[s]),
					     btmr = pcur-btr, btpr = pcur+btr;
					bounds.pmin[c] = min(bounds.pmin[c], min(btmr[c], btpr[c]));
					bounds.pmax[c] = max(bounds.pmax[c], max(btmr[c], btpr[c]));
				}
			}
		}
	}
	return bounds;
}

// Find a tight-fitting, axis-aligned bounding box around the view-aligned ribbon spawned by the given 3D cubic
// Bezeir curve and associated varying radius, outputting the unit bitangents at tmin/tmax for future use.
AABox3 ribbon_bounds (
	out mat2x3 bitangets, const mat4x3 bezier, const mat3 dbezier, const vec4 radius, const vec3 eye
){
	// Scatter radius curve over all spatial components
	mat4x3 curve_r = mat4x3(vec3(radius[0]), vec3(radius[1]), vec3(radius[2]), vec3(radius[3]));

	// Determine extrema
	mat3 sol[2] = mat3[2](
		// axis minus radius
		solve_quadratic(to_monomial(derive_bezier(bezier-curve_r))),
		// axis plus radius
		solve_quadratic(to_monomial(derive_bezier(bezier+curve_r)))
	);

	// Fit bounds to curve extrema
	// - initialize with just the curve start/end points
	bitangets[0] = bitangent0(bezier, eye);
	bitangets[1] = bitangent1(bezier, eye);
	AABox3 bounds = initbox3;
	{ vec3 bt0r = bitangets[0] * radius[0],
	       bt1r = bitangets[1] * radius[3];
	  aabox3_add_point(bounds, bezier[0]-bt0r); aabox3_add_point(bounds, bezier[0]+bt0r);
	  aabox3_add_point(bounds, bezier[3]-bt1r); aabox3_add_point(bounds, bezier[3]+bt1r); }
	// - incorporate any inflection points within range
	for (int i=0; i<2; i++) {
		for (int c=0; c<3; c++) {
			vec3 sol_c = sol[i][c];
			int num_sols = int(sol_c.x);
			for (int s=1; s<=num_sols; s++) {
				if (sol_c[s] >= 0 && sol_c[s] <= 1) {
					vec3 pcur = eval_bezier(bezier, sol_c[s]),
					     btr = bitangent(bezier, dbezier, sol_c[s], eye)*eval_bezier(radius, sol_c[s]),
					     btmr = pcur-btr, btpr = pcur+btr;
					bounds.pmin[c] = min(bounds.pmin[c], min(btmr[c], btpr[c]));
					bounds.pmax[c] = max(bounds.pmax[c], max(btmr[c], btpr[c]));
				}
			}
		}
	}
	return bounds;
}

// Find a tight-fitting, axis-aligned bounding box around the view-aligned ribbon spawned by the given 3D quadratic
// Bezeir curve and associated varying radius.
AABox3 ribbon_bounds (const mat3 bezier, const mat2x3 dbezier, const vec3 radius, const vec3 eye)
{
	// Scatter radius curve over all spatial components
	mat3 curve_r = mat3(vec3(radius[0]), vec3(radius[1]), vec3(radius[2]));

	// Determine extrema
	mat3x2 sol[2] = mat3x2[2](
		// axis minus radius
		solve_linear(to_monomial(derive_bezier(bezier-curve_r))),
		// axis plus radius
		solve_linear(to_monomial(derive_bezier(bezier+curve_r)))
	);

	// Fit bounds to curve extrema
	// - initialize with just the curve start/end points
	AABox3 bounds = initbox3;
	{ vec3 bt0r = bitangent0(bezier, eye) * radius[0],
	       bt1r = bitangent1(bezier, eye) * radius[2];
	  aabox3_add_point(bounds, bezier[0]-bt0r); aabox3_add_point(bounds, bezier[0]+bt0r);
	  aabox3_add_point(bounds, bezier[2]-bt1r); aabox3_add_point(bounds, bezier[2]+bt1r); }
	// - incorporate any inflection points within range
	for (int i=0; i<2; i++) {
		for (int c=0; c<3; c++) {
			vec2 sol_c = sol[i][c];
			if (sol_c.x == 1) {
				if (sol_c.y >= 0 && sol_c.y <= 1) {
					vec3 pcur = eval_bezier(bezier, sol_c.y),
					     btr = bitangent(bezier, dbezier, sol_c.y, eye)*eval_bezier(radius, sol_c.y),
					     btmr = pcur-btr, btpr = pcur+btr;
					bounds.pmin[c] = min(bounds.pmin[c], min(btmr[c], btpr[c]));
					bounds.pmax[c] = max(bounds.pmax[c], max(btmr[c], btpr[c]));
				}
			}
		}
	}
	return bounds;
}

// Find a tight-fitting, axis-aligned bounding box around the view-aligned ribbon spawned by the given 3D quadratic
// Bezeir curve and associated varying radius, outputting the unit bitangents at tmin/tmax for future use.
AABox3 ribbon_bounds (
	out mat2x3 bitangets, const mat3 bezier, const mat2x3 dbezier, const vec3 radius, const vec3 eye
){
	// Scatter radius curve over all spatial components
	mat3 curve_r = mat3(vec3(radius[0]), vec3(radius[1]), vec3(radius[2]));

	// Determine extrema
	mat3x2 sol[2] = mat3x2[2](
		// axis minus radius
		solve_linear(to_monomial(derive_bezier(bezier-curve_r))),
		// axis plus radius
		solve_linear(to_monomial(derive_bezier(bezier+curve_r)))
	);

	// Fit bounds to curve extrema
	// - initialize with just the curve start/end points
	bitangets[0] = bitangent0(bezier, eye);
	bitangets[1] = bitangent1(bezier, eye);
	AABox3 bounds = initbox3;
	{ vec3 bt0r = bitangets[0] * radius[0],
	       bt1r = bitangets[1] * radius[2];
	  aabox3_add_point(bounds, bezier[0]-bt0r); aabox3_add_point(bounds, bezier[0]+bt0r);
	  aabox3_add_point(bounds, bezier[2]-bt1r); aabox3_add_point(bounds, bezier[2]+bt1r); }
	// - incorporate any inflection points within range
	for (int i=0; i<2; i++) {
		for (int c=0; c<3; c++) {
			vec2 sol_c = sol[i][c];
			if (sol_c.x == 1) {
				if (sol_c.y >= 0 && sol_c.y <= 1) {
					vec3 pcur = eval_bezier(bezier, sol_c.y),
					     btr = bitangent(bezier, dbezier, sol_c.y, eye)*eval_bezier(radius, sol_c.y),
					     btmr = pcur-btr, btpr = pcur+btr;
					bounds.pmin[c] = min(bounds.pmin[c], min(btmr[c], btpr[c]));
					bounds.pmax[c] = max(bounds.pmax[c], max(btmr[c], btpr[c]));
				}
			}
		}
	}
	return bounds;
}

// Determine the direction of the largest extend of the given 3D cubic Bezier curve
vec3 curve_prinicipal_dir (const mat4x3 bezier)
{
	vec3 m0 = bezier[1] - bezier[0], m1 = bezier[2] - bezier[3],
	     diff = bezier[3] - bezier[0];
	float m0l = dot(m0, m0), m1l = dot(m1, m1);
	vec3 ref; {
		if (dot(diff, diff) > 0)
			ref = normalize(diff);
		else
			ref = m1l > m0l ?
			  (m1l > 0 ? normalize(m1) : vec3(1, 0, 0))
			: (m0l > 0 ? normalize(m0) : vec3(1, 0, 0));
	}
	return ref;
}

// Determine the direction of the largest extend of the given 3D quadratic Bezier curve
vec3 curve_prinicipal_dir (const mat3 bezier)
{
	vec3 m = bezier[1] - bezier[0],
	     diff = bezier[2] - bezier[0];
	float ml = dot(m, m);
	vec3 ref; {
		if (dot(diff, diff) > 0)
			ref = normalize(diff);
		else
			ref = ml > 0 ? normalize(m) : vec3(1, 0, 0);
	}
	return ref;
}

// Determine orthonormal basis vectors approximatly aligned with the prinicipal extent directions of the given 3D cubic Bezier
// curve
mat3 curve_prinicipal_dirs (const mat4x3 bezier)
{
	vec3 m0 = bezier[1] - bezier[0], m1 = bezier[3] - bezier[2];
	float m0l = dot(m0, m0), m1l = dot(m1, m1);
	mat3 ret;
	/* e0 = */ {
		vec3 candidate = bezier[3] - bezier[0];
		if (dot(candidate, candidate) > 0)
			ret[0] = normalize(candidate);
		else
			ret[0] = m1l > m0l ?
			  (m1l > 0 ? normalize(ortho_vec(m1)) : vec3(1, 0, 0))
			: (m0l > 0 ? normalize(ortho_vec(m0)) : vec3(1, 0, 0));
	};
	/* e1 = */ {
		vec3 proj0 = m0 - dot(ret[0], m0)*ret[0],  proj1 = m1 - dot(ret[0], m1)*ret[0];
		float proj0l = dot(proj0, proj0), proj1l = dot(proj1, proj1);
		ret[1] = normalize(
			proj1l > proj0l ?   (proj1l > eps ? proj1 : ortho_vec(ret[0]))
			                  : (proj0l > eps ? proj0 : ortho_vec(ret[0]))
		);
	};
	ret[2] = cross(ret[0], ret[1]);
	return ret;
}

// Determine orthonormal basis vectors approximatly aligned with the prinicipal extent directions of the given 3D quadratic
// Bezier curve
mat3 curve_prinicipal_dirs (const mat3 bezier)
{
	vec3 m = bezier[1] - bezier[0];
	float ml = dot(m, m);
	mat3 ret;
	/* e0 = */ {
		vec3 candidate = bezier[2] - bezier[0];
		if (dot(candidate, candidate) > 0)
			ret[0] = normalize(candidate);
		else
			ret[0] = ml > 0 ? normalize(ortho_vec(m)) : vec3(1, 0, 0);
	};
	/* e1 = */ {
		vec3 proj = m - dot(ret[0], m)*ret[0];
		float projl = dot(proj, proj);
		ret[1] = normalize(projl > eps ? proj : ortho_vec(ret[0]));
	};
	ret[2] = cross(ret[0], ret[1]);
	return ret;
}

// Calculate an affine transformation that transforms the positive 3D unit cube into a tight-fitting oriented box around the
// given 3D cubic Bezier curve with varying radius
mat4 curve_obb_trans (const mat4x3 curve, const vec4 radius)
{
	// Determine curve orientation
	mat3 basis = curve_prinicipal_dirs(curve);

	// Determine curve extents in local oriented coordinate system
	AABox3 dimbox = curve_bounds(transpose(basis)*curve, radius, 0, 1);

	// Determine curve bbox model transformation
	vec3 ext = aabox3_extent(dimbox);
	return mat4(basis) * mat4(
		vec4(ext.x, 0, 0, 0), vec4(0, ext.y, 0, 0),
		vec4(0, 0, ext.z, 0), vec4(dimbox.pmin, 1)
	);
}

// Calculate an affine transformation that transforms the positive 3D unit cube into a tight-fitting oriented box around the
// given 3D cubic Bezier curve with varying radius, fixing one axis of the local coordinate system to the given vector.
mat4 curve_obb_trans (const vec3 ref_dir, const mat4x3 curve, const vec4 radius)
{
	// Determine curve orientation
	mat3 basis;
	make_orthonormal_basis(basis[0], basis[1], ref_dir);
	basis[2] = ref_dir;

	// Determine curve extents in local oriented coordinate system
	AABox3 dimbox = curve_bounds(transpose(basis)*curve, radius, 0, 1);

	// Determine curve bbox model transformation
	vec3 ext = aabox3_extent(dimbox);
	return mat4(basis) * mat4(
		vec4(ext.x, 0, 0, 0), vec4(0, ext.y, 0, 0),
		vec4(0, 0, ext.z, 0), vec4(dimbox.pmin, 1)
	);
}

// Calculate an affine transformation that transforms the positive 3D unit cube into a tight-fitting oriented box around the
// given 3D quadratic Bezier curve with varying radius
mat4 curve_obb_trans (const mat3 curve, const vec3 radius)
{
	// Determine curve orientation
	mat3 basis = curve_prinicipal_dirs(curve);

	// Determine curve extents in local oriented coordinate system
	AABox3 dimbox = curve_bounds(transpose(basis)*curve, radius, 0, 1);

	// Determine curve bbox model transformation
	vec3 ext = aabox3_extent(dimbox);
	return mat4(basis) * mat4(
		vec4(ext.x, 0, 0, 0), vec4(0, ext.y, 0, 0),
		vec4(0, 0, ext.z, 0), vec4(dimbox.pmin, 1)
	);
}

// Calculate an affine transformation that transforms the positive 3D unit cube into a tight-fitting oriented box around the
// given 3D quadratic Bezier curve with varying radius, fixing one axis of the local coordinate system to the given vector.
mat4 curve_obb_trans (const vec3 ref_dir, const mat3 curve, const vec3 radius)
{
	// Determine curve orientation
	mat3 basis;
	make_orthonormal_basis(basis[0], basis[1], ref_dir);
	basis[2] = ref_dir;

	// Determine curve extents in local oriented coordinate system
	AABox3 dimbox = curve_bounds(transpose(basis)*curve, radius, 0, 1);

	// Determine curve bbox model transformation
	vec3 ext = aabox3_extent(dimbox);
	return mat4(basis) * mat4(
		vec4(ext.x, 0, 0, 0), vec4(0, ext.y, 0, 0),
		vec4(0, 0, ext.z, 0), vec4(dimbox.pmin, 1)
	);
}

// Calculate an affine transformation into an oriented local space around the given 3D cubic Bezier curve where every curve
// point between t=[0...1] lies in the positive octant, with the coordinate system origin as close to the curve point at t=0 as
// possible
mat4 curve_system_trans (const mat4x3 curve)
{
	// Determine curve orientation and extends
	mat3 basis = curve_prinicipal_dirs(curve);
	AABox3 dimbox = curve_bounds(curve, 0, 1);

	// Compose transformation matrix
	return make_local_frame(basis[0], basis[1], basis[2], dimbox.pmin);
}

// Calculate an affine transformation into an oriented local space around the given 3D quadratic Bezier curve where every curve
// point between t=[0...1] lies in the positive octant, with the coordinate system origin as close to the curve point at t=0 as
// possible
mat4 curve_system_trans (const mat3 curve)
{
	// Determine curve orientation and extends
	mat3 basis = curve_prinicipal_dirs(curve);
	AABox3 dimbox = curve_bounds(curve, 0, 1);

	// Compose transformation matrix
	return make_local_frame(basis[0], basis[1], basis[2], dimbox.pmin);
}


////
// Curve measures

// Compute the non-linearness of 3 values, ranging from [0..∞] (zero means completely linear). Note that this measure also
// takes into account non-linearness in the parametrization, so even perfectly straight sequences won't have 0 non-linearness
// unless they are also perfectly length-parametrized.
float nonlinearness (const float p0, const float p1, const float p2, const float t1) {
	return nonlinearness(vec2(p0, 0), vec2(p1, t1), vec2(p2, 1), .5);
}

// Compute the non-linearness of 3 2D points, ranging from [0..∞] (zero means completely linear). Note that this measure also
// takes into account non-linearness in the parametrization, so even perfectly straight lines won't have 0 non-linearness
// unless the 3 points are also perfectly length-parametrized along that line.
float nonlinearness (const vec2 p0, const vec2 p1, const vec2 p2, const float t1) {
	vec2 tangent_ref = p2 - p0, slope = (p1-p0)/t1 - tangent_ref;
	return dot(slope, slope) / dot(tangent_ref, tangent_ref);
}

// Compute the non-linearness of 3 3D points, ranging from [0..∞] (zero means completely linear). Note that this measure also
// takes into account non-linearness in the parametrization, so even perfectly straight lines won't have 0 non-linearness
// unless the 3 points are also perfectly length-parametrized along that line.
float nonlinearness (const vec3 p0, const vec3 p1, const vec3 p2, const float t1) {
	vec3 tangent_ref = p2 - p0, slope = (p1-p0)/t1 - tangent_ref;
	return dot(slope, slope) / dot(tangent_ref, tangent_ref);
}

// Compute the non-linearness of 3 4D points, ranging from [0..∞] (zero means completely linear). Note that this measure also
// takes into account non-linearness in the parametrization, so even perfectly straight lines won't have 0 non-linearness
// unless the 3 points are also perfectly length-parametrized along that line.
float nonlinearness (const vec4 p0, const vec4 p1, const vec4 p2, const float t1) {
	vec4 tangent_ref = p2 - p0, slope = (p1-p0)/t1 - tangent_ref;
	return dot(slope, slope) / dot(tangent_ref, tangent_ref);
}

// Compute the non-linearness of the given scalar cubic Bezier curve.
float nonlinearness (const vec4 bezier) {
	return nonlinearness(bezier[0], bezier[1], bezier[2], .5) + nonlinearness(bezier[1], bezier[2], bezier[3], .5);
}

// Compute the non-linearness of the given 2D cubic Bezier curve.
float nonlinearness (const mat4x2 bezier) {
	return nonlinearness(bezier[0], bezier[1], bezier[2], .5) + nonlinearness(bezier[1], bezier[2], bezier[3], .5);
}

// Compute the non-linearness of the given 3D cubic Bezier curve.
float nonlinearness (const mat4x3 bezier) {
	return nonlinearness(bezier[0], bezier[1], bezier[2], .5) + nonlinearness(bezier[1], bezier[2], bezier[3], .5);
}

// Compute the non-linearness of the given 4D cubic Bezier curve.
float nonlinearness (const mat4 bezier) {
	return nonlinearness(bezier[0], bezier[1], bezier[2], .5) + nonlinearness(bezier[1], bezier[2], bezier[3], .5);
}
