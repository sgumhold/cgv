#version 330

/** use these forward declarations in your shader to use 2/3/4d Perlin p_noise:
float pnoise(vec2 x);
float pnoise(vec2 x, float zoom, float time);
float pnoise(vec3 p);
float fbm_pnoise(vec3 p, int octaves, float persistence, float lacunarity);
*/

#define M_PI 3.14159265358979323846
#define screenWidth 1920.0

float prand(vec2 c) { return fract(sin(dot(c.xy, vec2(12.9898, 78.233))) * 43758.5453); }
float prand(vec2 co, float l) { return prand(vec2(prand(co), l)); }
float prand(vec2 co, float l, float t) { return prand(vec2(prand(co, l), t)); }

float p_noise(vec2 p, float freq)
{
	float unit = screenWidth / freq;
	vec2 ij = floor(p / unit);
	vec2 xy = mod(p, unit) / unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5 * (1. - cos(M_PI * xy));
	float a = prand((ij + vec2(0., 0.)));
	float b = prand((ij + vec2(1., 0.)));
	float c = prand((ij + vec2(0., 1.)));
	float d = prand((ij + vec2(1., 1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}
float pnoise(vec2 p, int res) 
{
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
	float f = 4.;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i < 50; i++) {
		n += amp * p_noise(p, f);
		f *= 2.;
		normK += amp;
		amp *= persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n / normK;
	return nf * nf * nf * nf;
}
float pnoise(vec2 p, float dim, float time)
{
	vec2 pos = floor(p * dim);
	vec2 posx = pos + vec2(1.0, 0.0);
	vec2 posy = pos + vec2(0.0, 1.0);
	vec2 posxy = pos + vec2(1.0);

	float c = prand(pos, dim, time);
	float cx = prand(posx, dim, time);
	float cy = prand(posy, dim, time);
	float cxy = prand(posxy, dim, time);

	vec2 d = fract(p * dim);
	d = -0.5 * cos(d * M_PI) + 0.5;

	float ccx = mix(c, cx, d.x);
	float cycxy = mix(cy, cxy, d.x);
	float center = mix(ccx, cycxy, d.y);

	return center * 2.0 - 1.0;
}



/* ----------------------------------------------------------
   3D Perlin Noise
   Based on Stefan Gustavson / Ashima Arts improved noise.
   Cleaned up with explicit gradient vectors.
   Output range is approximately [-1, 1].
   ---------------------------------------------------------- */

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }

// Permutation polynomial: returns pseudo-random integers in [0, 288]
vec4 permute(vec4 x) { return mod289(((x * 34.0) + 1.0) * x); }

// 16 gradient directions (12 unique edges + 4 repeats)
// Normalized so output is roughly [-1, 1]
const float R2 = 1.4142135623730951; // sqrt(2)
const vec3 grad[16] = vec3[](
    vec3(1, 1, 0) / R2, vec3(-1, 1, 0) / R2, vec3(1, -1, 0) / R2, vec3(-1, -1, 0) / R2,
    vec3(1, 0, 1) / R2, vec3(-1, 0, 1) / R2, vec3(1, 0, -1) / R2, vec3(-1, 0, -1) / R2,
    vec3(0, 1, 1) / R2, vec3(0, -1, 1) / R2, vec3(0, 1, -1) / R2, vec3(0, -1, -1) / R2,
    vec3(1, 1, 0) / R2, vec3(0, -1, 1) / R2, vec3(-1, 1, 0) / R2, vec3(0, -1, -1) / R2
    );

float pnoise(vec3 p)
{
    // Integer coordinates of the unit cube
    vec3 pi0 = floor(p);
    vec3 pi1 = pi0 + 1.0;

    // Wrap indices to avoid overflow
    pi0 = mod289(pi0);
    pi1 = mod289(pi1);

    // Fractional part (0 to 1) and fractional part - 1
    vec3 pf0 = fract(p);
    vec3 pf1 = pf0 - 1.0;

    // Hash the 8 corners
    vec4 ix = vec4(pi0.x, pi1.x, pi0.x, pi1.x);
    vec4 iy = vec4(pi0.yy, pi1.yy);
    vec4 iz0 = pi0.zzzz;
    vec4 iz1 = pi1.zzzz;

    vec4 ixy = permute(permute(ix) + iy);
    vec4 ixy0 = permute(ixy + iz0);
    vec4 ixy1 = permute(ixy + iz1);

    // Map hashes to gradient indices 0-15
    int gi000 = int(mod(ixy0.x, 16.0));
    int gi100 = int(mod(ixy0.y, 16.0));
    int gi010 = int(mod(ixy0.z, 16.0));
    int gi110 = int(mod(ixy0.w, 16.0));
    int gi001 = int(mod(ixy1.x, 16.0));
    int gi101 = int(mod(ixy1.y, 16.0));
    int gi011 = int(mod(ixy1.z, 16.0));
    int gi111 = int(mod(ixy1.w, 16.0));

    // Gradient vectors at the 8 corners
    vec3 g000 = grad[gi000];
    vec3 g100 = grad[gi100];
    vec3 g010 = grad[gi010];
    vec3 g110 = grad[gi110];
    vec3 g001 = grad[gi001];
    vec3 g101 = grad[gi101];
    vec3 g011 = grad[gi011];
    vec3 g111 = grad[gi111];

    // Distance from each corner to the sample point, then dot with gradient
    float n000 = dot(g000, pf0);
    float n100 = dot(g100, vec3(pf1.x, pf0.yz));
    float n010 = dot(g010, vec3(pf0.x, pf1.y, pf0.z));
    float n110 = dot(g110, vec3(pf1.xy, pf0.z));
    float n001 = dot(g001, vec3(pf0.xy, pf1.z));
    float n101 = dot(g101, vec3(pf1.x, pf0.y, pf1.z));
    float n011 = dot(g011, vec3(pf0.x, pf1.yz));
    float n111 = dot(g111, pf1);

    // Fade curve: 6t^5 - 15t^4 + 10t^3
    vec3 f = pf0 * pf0 * pf0 * (pf0 * (pf0 * 6.0 - 15.0) + 10.0);

    // Trilinear interpolation
    vec4 n_z = mix(
        vec4(n000, n100, n010, n110),
        vec4(n001, n101, n011, n111),
        f.z
    );
    vec2 n_yz = mix(n_z.xy, n_z.zw, f.y);
    float n_xyz = mix(n_yz.x, n_yz.y, f.x);

    return n_xyz;
}

/* ----------------------------------------------------------
   Fractal Brownian Motion (FBM) using the Perlin function
   ---------------------------------------------------------- */

float fbm_pnoise(vec3 p, int octaves, float persistence, float lacunarity)
{
    float total = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    //float maxValue = 0.0;  // Used to normalize result

    for (int i = 0; i < octaves; ++i)
    {
        total += pnoise(p * frequency) * amplitude;
//        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / (1.0 - pow(persistence, float(octaves)));
//    return total / maxValue;
}
