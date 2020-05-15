#include "point_cloud.h"
#include <fstream>
#include <future>
#include "ann_tree.h"
#include "SICP.h"

using namespace cgv::pointcloud;

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


SICP::SICP() : sourceCloud(nullptr),targetCloud(nullptr){

}

SICP::~SICP() {

}

void SICP::set_source_cloud(const point_cloud& inputCloud) {
	sourceCloud = &inputCloud;
	closest_points.resize(sourceCloud->get_nr_points());
	neighbor_tree.build(inputCloud);
}

void SICP::set_target_cloud(const point_cloud& inputCloud) {
	targetCloud = &inputCloud;
}

void SICP::get_center_point(const point_cloud &input, Pnt &center_point) {
    center_point.zeros();
    for (int i = 0; i < input.get_nr_points(); i++)
        center_point += input.pnt(i);
    center_point /= (float)input.get_nr_points();
}

void SICP::register_pointcloud(float mu, int max_runs)
{
	for (int i = 0; i < max_runs;++i) {

		//find closest points
		parallel_for(0,sourceCloud->get_nr_points(),[&](size_t i) {
			closest_points.pnt(i) = targetCloud->pnt(neighbor_tree.find_closest(sourceCloud->pnt(i)));
		});

		float mu = parameters.mu;
		for (int o = 0; o < parameters.max_outer_loop; ++o) {

			for (int i = 0; i < parameters.max_inner_loop; ++i) {

			}
		}
	}
	
}


