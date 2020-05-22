#include "point_cloud.h"
#include <fstream>
#include <future>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <cgv/math/det.h>
#include "ann_tree.h"
#include "SICP.h"

using namespace std;
using namespace cgv::math;

namespace cgv {

	namespace pointcloud {


		float det(const cgv::math::fmat<float, 3, 3>& m) {
			return det_33<float>(m(0, 0), m(0, 1), m(0, 2), m(1, 0), m(1, 1), m(1, 2), m(2, 0), m(2, 1), m(2, 2));
		}

		/// parallelized for loop adapted from https://stackoverflow.com/questions/34221621/optimizing-a-parallel-for-implementation
		template<unsigned THREADS_PER_CORE = 1>
		void parallel_for(size_t start, size_t end, const std::function<void(size_t)> &lambda)
		{
			int thread_count = std::thread::hardware_concurrency()*THREADS_PER_CORE;

			int block_size = (end - start) / thread_count;
			if (block_size*thread_count < end - start)
				++block_size;
			std::vector<std::future<void>> futures;

			for (int thread_index = 0; thread_index < thread_count; thread_index++)
			{
				futures.emplace_back(std::async(std::launch::async, [thread_index, block_size, start, end, &lambda]
				{
					int block_start = start + thread_index * block_size;
					if (block_start >= end) return;
					int block_end = block_start + block_size;
					if (block_end > end) block_end = end;
					for (size_t i = block_start; i < block_end; ++i)
					{
						lambda(i);
					}
				}));
			}

			for (std::future<void> &f : futures)
				f.get();
		}


		SICP::SICP() : sourceCloud(nullptr), targetCloud(nullptr) {
			parameters.max_runs = 20;
			parameters.max_inner_loop = 50;
			parameters.max_outer_loop = 50;
			parameters.mu = 1e-6;
			parameters.p = 0.5f;
			parameters.stop = 1e-4;
			parameters.use_penalty = false;
			parameters.max_mu = 1e5;
			parameters.alpha = 1.2f;
		}

		SICP::~SICP() {

		}

		void SICP::set_source_cloud(point_cloud& inputCloud) {
			sourceCloud = &inputCloud;
		}

		void SICP::set_target_cloud(point_cloud& inputCloud) {
			targetCloud = &inputCloud;
			neighbor_tree.build(inputCloud);
		}

		template<int I>
		inline float shrink_recursion(float mu, float norm, float p, float beta_t) {
			return shrink_recursion<I - 1>(mu, norm, p, 1.f - (p / mu)*powf(norm, p - 2.f)*powf(beta_t, p - 1.f));
		}

		template<>
		inline float shrink_recursion<0>(float mu, float norm, float p, float beta_t) {
			return beta_t;
		}

		template<int I, typename V = cgv::math::fvec<float, 3>>
		inline void shrink(vector<V>& Z, float mu, float p) {
			float alphaA = powf((2.f / mu)*(1.f - p), 1.f / (2.f - p));
			// threshold
			float th = alphaA + (p / mu)*powf(alphaA, 1 / (p - 1));

			for (int i = 0; i < Z.size(); ++i) {
				float n = Z[i].length();

				if (n > th) {
					Z[i] *= shrink_recursion<I>(mu, n, p, (alphaA / n + 1.f) / 2.f);
				}
				else {
					Z[i] = cgv::math::fvec<float, 3>(0, 0, 0);
				}
			}
		}


		void SICP::point_to_point(vec3* X, vec3* Y,size_t size) {
			/// De-mean
			vec3 X_mean, Y_mean;
			X_mean = accumulate(X, X + size, vec3(0, 0, 0))/((float)size);
			Y_mean = accumulate(Y, Y + size, vec3(0, 0, 0))/((float)size);
			
			for_each(X, X + size, [&](Pnt& p) {
				p -= X_mean;
			});
			for_each(Y, Y + size, [&](Pnt& p) {
				p -= Y_mean;
			});
			
			mat3 rotation;
			vec3 translation;
			cgv::math::diag_mat<float> Sigma;
			cgv::math::mat<float> U, V;
			mat3 fA;
			U.zeros();
			V.zeros();
			Sigma.zeros();

			for (int i = 0; i < size; ++i) {
				fA += mat3(Y[i], X[i]);
			}
			cgv::math::mat<float> A(3, 3, &fA(0, 0));
			cgv::math::svd(A, U, Sigma, V);
			mat3 fU(3, 3, &U(0, 0)), fV(3, 3, &V(0, 0));
			
			if (det(fU)*det(fV) < 0.0) {
				cgv::math::diag_mat<float> S(3);

				S(0) = 1.f;S(1) = 1.f; S(2) = -1.f;
				U.transpose();
				cgv::math::mat<float> R = V *S* U;
				rotation = mat3(3, 3, &R(0,0));
			}
			else {
				U.transpose();
				cgv::math::mat<float> R = V * U;
				rotation = Mat(3, 3, &R(0,0));
			}
			translation = Y_mean - rotation * X_mean;
			
			/// Apply transformation
			for (int i = 0; i < size; ++i) {
				X[i] = rotation*X[i];
			}
			/// Re-apply mean
			for_each(X, X + size, [&](vec3& p) {p += X_mean;});
			for_each(Y, Y + size, [&](vec3& p) {p += Y_mean;});
		}

		void SICP::register_pointcloud()
		{
			vector<Pnt> closest_points(sourceCloud->get_nr_points());
			vector<Pnt> Z(sourceCloud->get_nr_points(), Pnt(0, 0, 0));
			vector<Pnt> lagrage_multipliers(sourceCloud->get_nr_points(), Pnt(0, 0, 0));

			vector<Pnt> Xo1 = vector<Pnt>(&sourceCloud->pnt(0), &sourceCloud->pnt(0) + sourceCloud->get_nr_points());
			vector<Pnt> Xo2 = vector<Pnt>(&sourceCloud->pnt(0), &sourceCloud->pnt(0) + sourceCloud->get_nr_points());

			for (int i = 0; i < parameters.max_runs; ++i) {
				// Step 1: find closest points
				/*parallel_for(0, sourceCloud->get_nr_points(), [&](size_t i) {
					closest_points[i] = targetCloud->pnt(neighbor_tree.find_closest(sourceCloud->pnt(i)));
				});*/
				for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
					closest_points[i] = targetCloud->pnt(neighbor_tree.find_closest(sourceCloud->pnt(i)));
				}

				float mu = parameters.mu;
				
				for (int o = 0; o < parameters.max_outer_loop; ++o) {
					float dual = 0;
					for (int i = 0; i < parameters.max_inner_loop; ++i) {
						// update Z
						for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
							Z[i] = sourceCloud->pnt(i) - closest_points[i] + lagrage_multipliers[i] / mu;
						}
						// shrinkage operator usually converges in three iterations (I = 3)
						shrink<3>(Z, mu, parameters.p);

						vector<vec3> U(sourceCloud->get_nr_points());
						for (int i = 0; i < U.size(); ++i) {
							U[i] = closest_points[i] + Z[i] - lagrage_multipliers[i] / mu;
						}
						// ridgid motion estimator
						point_to_point(&sourceCloud->pnt(0), U.data(), sourceCloud->get_nr_points());
						
						dual = -numeric_limits<float>::infinity();
						for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
							dual = max((sourceCloud->pnt(i) - Xo1[i]).length(),dual);
						}
						Xo1 = vector<Pnt>(&sourceCloud->pnt(0), &sourceCloud->pnt(0) + sourceCloud->get_nr_points());
						if (dual < parameters.stop) {
							break;
						}
					}
					// C Update

					vector<float> Pnorms(sourceCloud->get_nr_points());
					for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
						vec3 p = sourceCloud->pnt(i) - closest_points[i] - Z[i];
						Pnorms[i] = p.length();
						if (!parameters.use_penalty) {
							lagrage_multipliers[i] += mu * p;
						}
					}
					//mu Update
					if (mu < parameters.max_mu) mu *= parameters.alpha;
					/// stopping criteria
					float primal = *max_element(Pnorms.begin(),Pnorms.end());
					if (primal < parameters.stop && dual < parameters.stop) break;
				}
				float stop = -numeric_limits<float>::infinity();
				for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
					stop = max((sourceCloud->pnt(0) - Xo2[i]).length(), stop);
				}
				Xo2 = vector<Pnt>(&sourceCloud->pnt(0), &sourceCloud->pnt(0) + sourceCloud->get_nr_points());
				if (stop < parameters.stop) break;
			}

		}


	}
}