#pragma once
#include <unordered_map>
#include <thread>
#include <vector>

	//** concurrency classes *//
	class SenseReversingBarrier
	{
		int remaining = 0;
		int initial_count = 0;
		bool barrier_sense = false;
		std::mutex barrier_mutex;
		std::condition_variable condition;
		std::unordered_map<std::thread::id, bool> thread_sense;
	public:
		inline SenseReversingBarrier(const int i) {
			remaining = i;
			initial_count = i;
		}

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

		void await() {
			await([]() {});
		}
	};
	
	class WorkerPool {

		class PoolAccessException : public std::exception {
			std::string msg;
			const char* what() const throw () {
				return msg.c_str();
			}
		public:
			PoolAccessException(const std::string& msg) : msg(msg) {}
		};

		struct PoolTask {
			//Task* task;
			std::function<void(int)> task;
			std::atomic_int remaining; //threads not finished yet
			PoolTask(const std::function<void(int)>& t, const int num_threads) : task(t), remaining(num_threads) {}
		};

		SenseReversingBarrier barrier_pre;
		//ab::SenseReversingBarrier barrier_post;

		std::unique_ptr<PoolTask> current_task = nullptr;

		std::vector<std::thread> threads;

		bool stop_request = false;

	public:
		WorkerPool(unsigned i) : barrier_pre(i + 1)/*, barrier_post(i+1)*/ {
			construct_threads(i);
		}
		~WorkerPool() {
			join_all();
		}

		//collectiv task execution, calling thread also runs the task
		template<typename F>
		void run(F func) {
			if (is_busy()) {
				throw PoolAccessException("tried to launch a computation on a already busy pool!");
			}
			//no lock required since the calling threads should be the only accessing thread
			current_task = std::make_unique<PoolTask>(std::move(std::function<void(int)>(func)), (int)threads.size() + 1);
			worker_kernel(current_task.get(), (int)threads.size(), false);
		}

		//
		template<typename F>
		void launch(F func) {
			if (is_busy()) {
				throw PoolAccessException("tried to launch a computation on a already busy pool!");
			}
			current_task = std::make_unique<PoolTask>(std::move(std::function<void(int)>(func)), (int)threads.size());
			launchpad_kernel();
		}

		// returns if the pools computation is completed
		void sync() {
			if (!is_busy()) {
				return;
			}
			landing_kernel();
		}

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
		inline void landing_kernel() {
			//non of the pool threads will clear the task pointer so its safe to access
			PoolTask* ptask = current_task.get();

			if (ptask) {
				//wait and clear task afterwards
				{
					//there should only be the thread who called run() in here
					while (ptask->remaining.load(std::memory_order_relaxed) > 0) {
						std::this_thread::yield();
					};
					current_task = nullptr;
				}
			}
		}

		//add i threads to the pool (blocking)
		inline void construct_threads(unsigned i) {
			//create threads
			while (i > 0) {
				bool sense = !barrier_pre.sense();
				threads.emplace_back(&WorkerPool::worker_kernel, this, nullptr, --i, true);
			}
		}
	};


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
	void WorkerPool::worker_kernel(PoolTask* ptask, int thread_id, bool is_pool_member)
	{
		//individual jobs
		bool clear_task = !is_pool_member;

		do {
			//std::printf("%d arrived\n", thread_id);
			barrier_pre.await(); //usually threads wait on this barrier
			//std::printf("%d passed barrier\n", thread_id);
			ptask = current_task.get();
			//terminate on request
			if (stop_request) {
				return;
			}

			//run any existing task
			if (ptask) {
				ptask->task(thread_id);
				int rem = ptask->remaining.fetch_sub(1);

				if (clear_task) {
					//there should only be the thread who called run() in here
					while (ptask->remaining.load(std::memory_order_relaxed) > 0) {
						std::this_thread::yield();
					};
					current_task = nullptr;
				}
			}
		} while (is_pool_member);
	}
