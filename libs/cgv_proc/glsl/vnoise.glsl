#version 330

// based on https://www.shadertoy.com/view/XlXBWj

/** use these forward declarations in your shader to use classic 2/3/4d Perlin noise:
// 3D voronoi noise mapping to [0,1]
float vnoise(vec3 x);
// fractal brownian motion version of 3D voronoi noise mapping to [0,1]
float fbm_vnoise(vec3 P, int octaves, float persistence, float lacunarity);
*/

vec3 hash(vec3 p) 
{ 
	return fract(sin(vec3(
		dot(p, vec3(127.1, 311.7, 786.6)), 
		dot(p, vec3(269.5, 183.3, 455.8)), 
		dot(p, vec3(419.2, 371.9, 948.6)))) * 43758.5453);
}

// 3D voronoi noise mapping to [0,1]
float vnoise(vec3 p)
{
	vec3 n = floor(p);
	vec3 f = fract(p);

	float shortestDistance = 1.0;
	for (int x = -1; x < 1; x++) {
		for (int y = -1; y < 1; y++) {
			for (int z = -1; z < 1; z++) {
				vec3 o = vec3(x, y, z);
				vec3 r = (o - f) + 1.0 + sin(hash(n + o) * 50.0) * 0.2;
				float d = dot(r, r);
				if (d < shortestDistance) {
					shortestDistance = d;
				}
			}
		}
	}
	return shortestDistance;
}

// fractal brownian motion version of 3D voronoi noise mapping to [0,1]
float fbm_vnoise(vec3 p, int octaves, float persistence, float lacunarity)
{
	float n = 0.0;
	float f = 0.5, a = 0.5;
	mat2 m = mat2(0.8, 0.6, -0.6, 0.8);
	for (int i = 0; i < octaves; i++) {
		n += vnoise(p * f) * a;
		f *= lacunarity;
		a *= persistence;
		p.xy = m * p.xy;
	}
	return n;
}
