#version 430

/*
The following interface is implemented in this shader:
//***** begin interface of quadratic_bezier2d.glsl ***********************************
vec4 compute_quadratic_bezier_bounding_box(in vec2 p0, in vec2 p1, in vec2 p2);
vec2 ud_quadratic_bezier(in vec2 pos, in vec2 A, in vec2 B, in vec2 C);
//***** end interface of quadratic_bezier2d.glsl ***********************************
*/

// https://www.shadertoy.com/view/lsyfWc
vec4 compute_quadratic_bezier_bounding_box(in vec2 p0, in vec2 p1, in vec2 p2) {
    // extremes
    vec2 mi = min(p0,p2);
    vec2 ma = max(p0,p2);

    // maxima/minima point, if p1 is outside the current bbox/hull
    if( p1.x<mi.x || p1.x>ma.x || p1.y<mi.y || p1.y>ma.y )
    {
        // p = (1-t)^2*p0 + 2(1-t)t*p1 + t^2*p2
        // dp/dt = 2(t-1)*p0 + 2(1-2t)*p1 + 2t*p2 = t*(2*p0-4*p1+2*p2) + 2*(p1-p0)
        // dp/dt = 0 -> t*(p0-2*p1+p2) = (p0-p1);

        vec2 t = clamp((p0-p1)/(p0-2.0*p1+p2),0.0,1.0);
        vec2 s = 1.0 - t;
        vec2 q = s*s*p0 + 2.0*s*t*p1 + t*t*p2;
        
        mi = min(mi,q);
        ma = max(ma,q);
    }
    
    return vec4( mi, ma );
}

float dot2(in vec2 v) {
	return dot(v,v);
}

// compute the unsigned distance from a position to a quadratic bezier segment (https://www.shadertoy.com/view/MlKcDD)
vec2 ud_quadratic_bezier(in vec2 pos, in vec2 A, in vec2 B, in vec2 C) {
	vec2 a = B - A;
	vec2 b = A - 2.0*B + C;
	vec2 c = a * 2.0;
	vec2 d = A - pos;

	float kk = 1.0 / dot(b, b);
	float kx = kk * dot(a, b);
	float ky = kk * (2.0*dot(a, a) + dot(d, b)) / 3.0;
	float kz = kk * dot(d, a);

	vec2 res;

	float p = ky - kx * kx;
	float p3 = p * p * p;
	float q = kx * (2.0*kx*kx - 3.0*ky) + kz;
	float h = q * q + 4.0*p3;

	if(h >= 0.0) {
		h = sqrt(h);
		vec2 x = (vec2(h, -h) - q) / 2.0;
		vec2 uv = sign(x)*pow(abs(x), vec2(1.0 / 3.0));
		float t = clamp(uv.x + uv.y - kx, 0.0, 1.0);

		// 1 root
		res = vec2(dot2(d + (c + b * t)*t), t);
	} else {
		float z = sqrt(-p);
		float v = acos(q / (p*z*2.0)) / 3.0;
		float m = cos(v);
		float n = sin(v) * 1.732050808;
		vec3 t = clamp(vec3(m + m, -n - m, n - m) * z - kx, 0.0, 1.0);

		// 3 roots, but only need two
		float dis = dot2(d + (c + b * t.x) * t.x);
		res = vec2(dis, t.x);

		dis = dot2(d + (c + b * t.y) * t.y);
		if(dis < res.x) res = vec2(dis, t.y);

		// the third root cannot be the closest. See https://www.shadertoy.com/view/4dsfRS
        // res = min(res,dot2(d+(c+b*t.z)*t.z));
	}

	res.x = sqrt(res.x);
	return res;
}
