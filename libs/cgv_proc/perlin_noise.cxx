#include "perlin_noise.h"

#define STB_PERLIN_IMPLEMENTATION // force include to generate implementation of perlin noise functions
#include "stb_perlin.h"
#undef STB_PERLIN_IMPLEMENTATION

namespace cgv {
namespace proc {

float perlin_noise_3d(float x, float y, float z) {
	return stb_perlin_noise3(x, y, z, 0, 0, 0);
}

float perlin_noise_3d(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap) {
	return stb_perlin_noise3(x, y, z, x_wrap, y_wrap, z_wrap);
}

float perlin_noise_3d(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap, uint8_t seed) {
	return stb_perlin_noise3_seed(x, y, z, x_wrap, y_wrap, z_wrap, static_cast<int>(seed));
}

float perlin_fbm_noise3_3d(float x, float y, float z, float lacunarity, float gain, int octaves) {
	return stb_perlin_fbm_noise3(x, y, z, lacunarity, gain, octaves);
}

float perlin_ridge_noise_3d(float x, float y, float z, float lacunarity, float gain, float offset, int octaves) {
	return stb_perlin_ridge_noise3(x, y, z, lacunarity, gain, offset, octaves);
}

float perlin_turbulence_noise_3d(float x, float y, float z, float lacunarity, float gain, int octaves) {
	return stb_perlin_turbulence_noise3(x, y, z, lacunarity, gain, octaves);
}

} // namespace proc
} // namespace cgv
