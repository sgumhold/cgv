#include "point_cloud.h"
#include <fstream>
#include <future>
#include <vector>
#include <cmath>
#include "ann_tree.h"
#include "SICP.h"

using namespace std;

namespace cgv {

	namespace pointcloud {


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
			parameters.mu = 1e-8;
			parameters.p = 0.5f;
		}

		SICP::~SICP() {

		}

		void SICP::set_source_cloud(const point_cloud& inputCloud) {
			sourceCloud = &inputCloud;
		}

		void SICP::set_target_cloud(const point_cloud& inputCloud) {
			targetCloud = &inputCloud;
			neighbor_tree.build(inputCloud);
		}

		void SICP::get_center_point(const point_cloud &input, Pnt &center_point) {
			center_point.zeros();
			for (int i = 0; i < input.get_nr_points(); i++)
				center_point += input.pnt(i);
			center_point /= (float)input.get_nr_points();
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

		void SICP::register_pointcloud()
		{
			vector<Pnt> closest_points(sourceCloud->get_nr_points());
			vector<Pnt> Z(sourceCloud->get_nr_points(), Pnt(0, 0, 0));
			vector<Pnt> lagrage_multipliers(sourceCloud->get_nr_points(), Pnt(0, 0, 0));

			for (int i = 0; i < parameters.max_runs; ++i) {
				// Step 1: find closest points
				/*parallel_for(0, sourceCloud->get_nr_points(), [&](size_t i) {
					closest_points[i] = targetCloud->pnt(neighbor_tree.find_closest(sourceCloud->pnt(i)));
				});*/
				for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
					closest_points[i] = targetCloud->pnt(neighbor_tree.find_closest(sourceCloud->pnt(i)));
				}

				float mu = parameters.mu;
				vector<float> weights(sourceCloud->get_nr_points());
				for (int o = 0; o < parameters.max_outer_loop; ++o) {
					for (int i = 0; o < parameters.max_inner_loop; ++i) {
						for (int i = 0; i < sourceCloud->get_nr_points(); ++i) {
							Z[i] = sourceCloud->pnt(i) - closest_points[i] + lagrage_multipliers[i] / mu;
						}
						// shrinkage operator usually converges in three iterations (I = 3)
						shrink<3>(Z, mu, parameters.p);
					}
				}
			}

		}


	}
}