/** use these forward declarations in your shader to use 2/3/4d Perlin p_noise:
float pnoise(vec2 x);
float pnoise(vec2 x, float zoom, float time);
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


