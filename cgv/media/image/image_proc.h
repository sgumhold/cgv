#pragma once

#ifdef SHOW_WAVELET_STATS
#include <cgv/utils/statistics.h>
#endif
#include <cgv/math/permute.h>
namespace cgv {
	namespace media {
		namespace image {

template <typename T_calc, typename T_detail, typename T_store>
void integer_wavelet_transform(T_store* data, size_t nr_inside, size_t nr_outside, size_t step_inside, size_t step_outside, size_t nr_components, unsigned mask, bool do_split = true, int s_scale = 1, int d_scale = 2)
{
#ifdef SHOW_WAVELET_STATS
	cgv::utils::statistics s_stats, d_stats;
#endif
	// perform wavelet transform in x-direction
	size_t c, x, y;
	for (y = 0; y < nr_outside; ++y) {
		T_store* s0_ptr  = data + y * step_outside;
		T_store* d1_ptr  = s0_ptr + step_inside;
		T_store* s0n_ptr = d1_ptr + step_inside;

		T_calc  s0n;
		T_calc  d1p;
		for (c = 0; c < nr_components; ++c) {
			s0n[c] = (*s0_ptr)[c] / s_scale;
			d1p[c] = 0;
		}
		for (x = 0; x < nr_inside; x += 2) {
			// if not right boundary reached, step on with second source color
			if (x + 2 < nr_inside) {
				for (c = 0; c < nr_components; ++c)
					s0n[c] = (*s0n_ptr)[c] / s_scale;
			}
			// predict and compute detail coefficient
			T_calc d1, s0;
			for (c = 0; c < nr_components; ++c) {
				d1[c] = (*d1_ptr)[c] / s_scale;
				s0[c] = (*s0_ptr)[c] / s_scale;
				d1[c] -= (s0[c] + s0n[c] + 1) / 2;
#ifdef SHOW_WAVELET_STATS
				d_stats.update(d1[c]);
#endif
				(*reinterpret_cast<T_detail*>(d1_ptr))[c] = d1[c] / d_scale;
			}

			// update source term and previous detail coefficient
			for (c = 0; c < nr_components; ++c) {
				s0[c] += (d1p[c] + d1[c] + 2) / 4;
#ifdef SHOW_WAVELET_STATS
				s_stats.update(s0[c]);
#endif
				(*reinterpret_cast<T_detail*>(s0_ptr))[c] = s0[c];
				d1p[c] = d1[c];
			}
			// step on pointers
			s0_ptr += 2 * step_inside;
			d1_ptr += 2 * step_inside;
			s0n_ptr += 2 * step_inside;
		}
	}
#ifdef SHOW_WAVELET_STATS
	std::cout << "s_stats=" << s_stats << std::endl;
	std::cout << "d_stats=" << d_stats << std::endl;
#endif
	if (do_split) {
		// compute permutation
		std::vector<int> P;
		P.resize(nr_inside);
		for (x = 0; x < nr_inside; ++x)
			P[x] = ((x & 1) == 0) ? x / 2 : (x / 2 + (nr_inside+1) / 2);
		cgv::math::permute_arrays(data, &P.front(), nr_inside, nr_outside, step_inside, step_outside);
	}
}

template <typename T_calc, typename T_detail, typename T_store>
void integer_inverse_wavelet_transform(T_store* data, size_t nr_inside, size_t nr_outside, size_t step_inside, size_t step_outside, size_t nr_components, unsigned mask, bool do_split = true, int s_scale = 1, int d_scale = 2)
{
	size_t c, x, y;
	if (do_split) {
		// compute permutation
		std::vector<int> P;
		P.resize(nr_inside);
		for (x = 0; x < nr_inside; ++x)
			P[x] = (x < nr_inside / 2) ? 2*x : 2*(x-nr_inside / 2)+1;

		cgv::math::permute_arrays(data, &P.front(), nr_inside, nr_outside, step_inside, step_outside);
	}
#ifdef SHOW_WAVELET_STATS
	cgv::utils::statistics s_stats, d_stats;
#endif

	// perform inverse wavelet transform in x-direction
	for (y = 0; y < nr_outside; ++y) {
		T_detail* s0_ptr  = reinterpret_cast<T_detail*>(data + y * step_outside);
		T_detail* d1_ptr  = s0_ptr + step_inside;
		T_detail* d1p_ptr = 0;
		T_detail* s0n_ptr = d1_ptr + step_inside;

		T_calc  s0p, s0;
		T_calc  d1p, d1;
		for (c = 0; c < nr_components; ++c)
			d1p[c] = 0;

		for (x = 0; x < nr_inside; x += 2) {
			// invert source term update
			for (c = 0; c < nr_components; ++c) {
				d1[c] = (*d1_ptr)[c] * d_scale;
				s0[c] = (*s0_ptr)[c];
				s0[c] -= (d1p[c] + d1[c] + 2) / 4;
#ifdef SHOW_WAVELET_STATS
				s_stats.update(s0[c]);
#endif
				(*reinterpret_cast<T_store*>(s0_ptr))[c] = s_scale * s0[c];
			}
			// invert previous predict step
			if (x > 0) {
				for (c = 0; c < nr_components; ++c) {
					d1p[c] += (s0p[c] + s0[c] + 1) / 2;
#ifdef SHOW_WAVELET_STATS
					d_stats.update(d1p[c]);
#endif
					(*reinterpret_cast<T_store*>(d1p_ptr))[c] = s_scale * d1p[c];
				}
			}
			for (c = 0; c < nr_components; ++c) {
				d1p[c] = d1[c];
				s0p[c] = s0[c];
			}
			d1p_ptr = d1_ptr;
			// step on pointers
			s0_ptr += 2 * step_inside;
			d1_ptr += 2 * step_inside;
			s0n_ptr += 2 * step_inside;
		}
		// invert last predict step
		for (c = 0; c < nr_components; ++c) {
			d1p[c] += (s0p[c] + s0[c] + 1) / 2;
#ifdef SHOW_WAVELET_STATS
			d_stats.update(d1p[c]);
#endif
			(*reinterpret_cast<T_store*>(d1p_ptr))[c] = d1p[c];
		}
	}
#ifdef SHOW_WAVELET_STATS
	std::cout << "inverse s_stats=" << s_stats << std::endl;
	std::cout << "inverse d_stats=" << d_stats << std::endl;
#endif
}

template <typename T_calc, typename T>
void subsample_image(const T* image_ptr, T* subsampled_image, const int W, const int H, const int nr_components)
{
	int w = (W + 1) / 2;
	int h = (H + 1) / 2;
	for (int y = 0; y < h; ++y) {
		bool condense_y = 2 * y + 1 == H;
		for (int x = 0; x < w; ++x) {
			T* target_ptr = subsampled_image + y*w + x;
			const T* src0_ptr = image_ptr + 2 * (y*W + x);
			const T* src1_ptr = src0_ptr + (condense_y ? 0 : W);
			int dx = (2 * x + 1 == W ? 0 : 1);
			for (int c = 0; c < nr_components; ++c)
				(*target_ptr)[c] = (T_calc(src0_ptr[0][c]) + T_calc(src0_ptr[dx][c]) + T_calc(src1_ptr[0][c]) + T_calc(src1_ptr[dx][c])) / 4;
		}
	}
}

template <typename T_calc, typename T>
void subsample_slice(const T* slice0_ptr, const T* slice1_ptr, T* subsampled_slice, const int W, const int H, const int nr_components)
{
	int w = (W + 1) / 2;
	int h = (H + 1) / 2;
	for (int y = 0; y < h; ++y) {
		bool condense_y = 2 * y + 1 == H;
		for (int x = 0; x < w; ++x) {
			T* target_ptr = subsampled_slice + y*w + x;
			const T* src00_ptr = slice0_ptr + 2 * (y*W + x);
			const T* src01_ptr = src00_ptr + (condense_y ? 0 : W);
			const T* src10_ptr = slice1_ptr + 2 * (y*W + x);
			const T* src11_ptr = src10_ptr + (condense_y ? 0 : W);
			int dx = (2 * x + 1 == W ? 0 : 1);
			for (int c = 0; c < nr_components; ++c)
				(*target_ptr)[c] = (
					T_calc(src00_ptr[0][c]) + T_calc(src00_ptr[dx][c]) + T_calc(src01_ptr[0][c]) + T_calc(src01_ptr[dx][c]) + 
					T_calc(src10_ptr[0][c]) + T_calc(src10_ptr[dx][c]) + T_calc(src11_ptr[0][c]) + T_calc(src11_ptr[dx][c])
					) / 8;
		}
	}
}
		}
	}
}
