#pragma once

#include <stdint.h>

#include "lib_begin.h"

namespace cgv {
namespace proc {

/// @brief Computes a random value at the coordinate(x, y, z).
/// Adjacent random values are continuous but the noise fluctuates
/// its randomness with period 1, i.e. takes on wholly unrelated values
/// at integer points. Specifically, this implements Ken Perlin's
/// revised noise function from 2002.
/// 
/// @param x The x coordinate at which to evaluate the noise function.
/// @param y The y coordinate at which to evaluate the noise function.
/// @param z The z coordinate at which to evaluate the noise function.
/// @return The noise value in the range [-1,1].
extern CGV_API float perlin_noise_3d(float x, float y, float z);

/// @brief Computes a random value at the coordinate(x, y, z).
/// See perlin_noise_3d(x, y, z).
/// 
/// The "wrap" parameters can be used to create wraparound noise that
/// wraps at powers of two. The numbers MUST be powers of two. Specify
/// 0 to mean "don't care". (The noise always wraps every 256 due
/// details of the implementation, even if you ask for larger or no
/// wrapping.)
/// 
/// @param x The x coordinate at which to evaluate the noise function.
/// @param y The y coordinate at which to evaluate the noise function.
/// @param z The z coordinate at which to evaluate the noise function.
/// @param x_wrap The x wrap period.
/// @param y_wrap The y wrap period.
/// @param z_wrap The z wrap period.
/// @return The noise value in the range [-1,1].
extern CGV_API float perlin_noise_3d(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap);

/// @brief Computes a random value at the coordinate(x, y, z).
/// See perlin_noise_3d(x, y, z, x_wrap, y_wrap, z_wrap).
/// 
/// The 'seed' parameter selects from multiple different variations of the
/// noise function.
/// 
/// @param x The x coordinate at which to evaluate the noise function.
/// @param y The y coordinate at which to evaluate the noise function.
/// @param z The z coordinate at which to evaluate the noise function.
/// @param x_wrap The x wrap period.
/// @param y_wrap The y wrap period.
/// @param z_wrap The z wrap period.
/// @param seed The random seed.
/// @return The noise value in the range [-1,1].
extern CGV_API float perlin_noise_3d(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap, uint8_t seed);

/// @brief Computes a random value at the coordinate(x, y, z).
/// Computes fractal noise from multiple noise layers based on fractional Brownian Motion.
/// 
/// Will call the noise function 'octaves' times, so this parameter will affect runtime.
/// 
/// @param x The x coordinate at which to evaluate the noise function.
/// @param y The y coordinate at which to evaluate the noise function.
/// @param z The z coordinate at which to evaluate the noise function.
/// @param lacunarity The spacing between successive octaves (use exactly 2.0 for wrapping output).
/// @param gain The relative weighting applied to each successive octave (use 0.5 as default).
/// @param octaves The number of noise layers to sum (use 6 as default).
/// @return The noise value in the range [-1,1].
extern CGV_API float perlin_fbm_noise3_3d(float x, float y, float z, float lacunarity, float gain, int octaves);

/// @brief Computes a random value at the coordinate(x, y, z).
/// Computes fractal noise from multiple noise layers based on fractional Brownian Motion.
/// This variant will produce ridge-like patterns in the resulting noise.
/// 
/// Will call the noise function 'octaves' times, so this parameter will affect runtime.
/// 
/// @param x The x coordinate at which to evaluate the noise function.
/// @param y The y coordinate at which to evaluate the noise function.
/// @param z The z coordinate at which to evaluate the noise function.
/// @param lacunarity The spacing between successive octaves (use exactly 2.0 for wrapping output).
/// @param gain The relative weighting applied to each successive octave (use 0.5 as default).
/// @param offset Used to invert the ridges (use 1.0 as default, may need to be larger, not sure).
/// @param octaves The number of noise layers to sum (use 6 as default).
/// @return The noise value in the range [-1,1].
extern CGV_API float perlin_ridge_noise_3d(float x, float y, float z, float lacunarity, float gain, float offset, int octaves);

/// @brief Computes a random value at the coordinate(x, y, z).
/// Computes fractal noise from multiple noise layers based on fractional Brownian Motion.
/// This variant will produce so-called turbulence noise.
/// 
/// Will call the noise function 'octaves' times, so this parameter will affect runtime.
/// 
/// @param x The x coordinate at which to evaluate the noise function.
/// @param y The y coordinate at which to evaluate the noise function.
/// @param z The z coordinate at which to evaluate the noise function.
/// @param lacunarity The spacing between successive octaves (use exactly 2.0 for wrapping output).
/// @param gain The relative weighting applied to each successive octave (use 0.5 as default).
/// @param octaves The number of noise layers to sum (use 6 as default).
/// @return The noise value in the range [-1,1].
extern CGV_API float perlin_turbulence_noise_3d(float x, float y, float z, float lacunarity, float gain, int octaves);

} // namespace proc
} // namespace cgv

#include <cgv/config/lib_end.h>
