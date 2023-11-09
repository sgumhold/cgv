#include "concurrency.h"

namespace cgv {
namespace pointcloud {
namespace utility {

	
	SenseReversingBarrier::SenseReversingBarrier(const int i)
	{
		remaining = i;
		initial_count = i;
		barrier_sense = false;
	}

	WorkerPool::WorkerPool(unsigned i) : barrier_pre(i + 1), current_task(nullptr)
	{
		construct_threads(i);
	}

	WorkerPool::~WorkerPool()
	{
		join_all();
	}

	void WorkerPool::join_all()
	{
		{
			{
				stop_request = true;
				current_task = nullptr;
			}
			worker_kernel(nullptr, -1, false);
		}
		for (auto& thread : threads) {
			thread.join();
		}
		threads.clear();
		stop_request = false;
	}

	// returns if the pools computation is completed
	void WorkerPool::sync()
	{
		if (!is_busy()) {
			return;
		}
		landing_kernel();
	}

	void WorkerPool::construct_threads(unsigned i)
	{
		// create threads
		while (i > 0) {
			bool sense = !barrier_pre.sense();
			threads.emplace_back(&WorkerPool::worker_kernel, this, nullptr, --i, true);
		}
	}

	void WorkerPool::landing_kernel()
	{
		// non of the pool threads will clear the task pointer so its safe to access
		PoolTask* ptask = current_task.get();

		if (ptask) {
			// wait and clear task afterwards
			{
				// there should only be the thread who called run() in here
				while (ptask->remaining.load(std::memory_order_relaxed) > 0) {
					std::this_thread::yield();
				};
				current_task = nullptr;
			}
		}
	}

} // namespace utility
} // namespace pointcloud
} // namespace cgv