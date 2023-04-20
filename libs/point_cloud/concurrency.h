#pragma once
#include <unordered_map>
#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "lib_begin.h"

namespace cgv {
namespace pointcloud {
namespace utility {



	//** concurrency classes *//
class CGV_API SenseReversingBarrier
	{
		int remaining;
		int initial_count;
		bool barrier_sense;
		std::mutex barrier_mutex;
		std::condition_variable condition;
		std::unordered_map<std::thread::id, bool> thread_sense;
	public:
		SenseReversingBarrier(const int i);

		inline bool sense() const {
			return barrier_sense;
		}

		// post_once is run by one thread whith following condition,
		//	-- run after all threads reached the phase synchronization point
		//  -- before any thread is notified in the current phase
		template <typename F>
		void await(F post_once) {
			bool sense;
			std::thread::id id = std::this_thread::get_id();
			std::unique_lock<std::mutex> lock(barrier_mutex);
			auto sen = thread_sense.find(id);
			if (sen != thread_sense.end()) {
				sense = sen->second;
			}
			else {
				sense = !barrier_sense;
			}

			--remaining;
			if (remaining == 0) {
				remaining = initial_count; //one addition for the calling thread
				barrier_sense = !barrier_sense;
				post_once();
				condition.notify_all();
			}
			else {
				while (sense != barrier_sense)
					condition.wait(lock);
			}
			thread_sense[id] = !sense;
		}

		inline void await() {
			await([]() {});
		}
	};

	class CGV_API WorkerPool
	{

	public:
		class PoolAccessException : public std::exception {
			std::string msg;
			inline const char* what() const throw () {
				return msg.c_str();
			}
		public:
			inline PoolAccessException(const std::string& msg) : msg(msg) {}
		};

		struct PoolTask {
			std::function<void(int)> task;
			std::atomic_int remaining; //threads not finished yet
			PoolTask(const std::function<void(int)>& t, const int num_threads)
				: task(t), remaining(num_threads)
			{
			}
		};
	private:
		SenseReversingBarrier barrier_pre;
		//ab::SenseReversingBarrier barrier_post;

		std::unique_ptr<PoolTask> current_task;

		std::vector<std::thread> threads;

		bool stop_request = false;

	public:
		WorkerPool(unsigned i);

		~WorkerPool();

		//collectiv task execution, calling thread also runs the task
		template<typename F> void run(F func);

		//
		template<typename F> void launch(F func);

		// returns if the pools computation is completed
		void sync();

		inline bool is_busy() const noexcept {
			return (current_task != nullptr);
		}

	protected:
		//join i threads (blocking)
		void join_all();

		// this kernel runs in parallel
		void worker_kernel(PoolTask* ptask, int thread_id, bool is_pool_member = true);

		// launches pool workers without participating, executed by a guest thread
		inline void launchpad_kernel() {
			barrier_pre.await();
		}
		// wait for results, executed by one guest thread
		void landing_kernel();

		//add i threads to the pool (blocking)
		void construct_threads(unsigned i);
	};

	template <typename TASK>
	struct TaskPool {
		std::vector<TASK> pool;
		std::atomic_int next_task = 0;

		std::function<void(TASK*)> func;

		void operator()() {
			while (true) {
				TASK* task;
				//get task
				{
					int task_id = next_task.fetch_add(1);
					if (task_id >= pool.size())
						return;
					task = &pool[task_id];
				}
				func(task);
			}
		}
	};


	// template definitions
	template <typename F>
	void WorkerPool::run(F func)
	{
		if (is_busy()) {
			throw PoolAccessException("tried to launch a computation on a already busy pool!");
		}
		// no lock required since the calling threads should be the only accessing thread
		current_task = std::make_unique<PoolTask>(std::move(std::function<void(int)>(func)), (int)threads.size() + 1);
		worker_kernel(current_task.get(), (int)threads.size(), false);
	}


	template <typename F>
	void WorkerPool::launch(F func)
	{
		if (is_busy()) {
			throw PoolAccessException("tried to launch a computation on a already busy pool!");
		}
		current_task = std::make_unique<PoolTask>(std::move(std::function<void(int)>(func)), (int)threads.size());
		launchpad_kernel();
	}


	inline void WorkerPool::worker_kernel(WorkerPool::PoolTask* ptask, int thread_id, bool is_pool_member)
	{
		// individual jobs
		bool clear_task = !is_pool_member;

		do {
			// std::printf("%d arrived\n", thread_id);
			barrier_pre.await(); // usually threads wait on this barrier
			// std::printf("%d passed barrier\n", thread_id);
			ptask = current_task.get();
			// terminate on request
			if (stop_request) {
				return;
			}

			// run any existing task
			if (ptask) {
				ptask->task(thread_id);
				int rem = ptask->remaining.fetch_sub(1);

				if (clear_task) {
					// there should only be the thread who called run() in here
					while (ptask->remaining.load(std::memory_order_relaxed) > 0) {
						std::this_thread::yield();
					};
					current_task = nullptr;
				}
			}
		} while (is_pool_member);
	}

	} //utility namespace
} //pointcloud namespace
} //cgv namespaces

#include <cgv/config/lib_end.h>