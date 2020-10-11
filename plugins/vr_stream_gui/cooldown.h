#pragma once

#include <chrono>

namespace trajectory {
namespace util {

	class cooldown {
	  private:
		long long time; // ms
		std::chrono::high_resolution_clock::time_point time_start;

	  public:
		cooldown(long long t);

		void start();

		bool is_done();
		bool is_done_and_reset();

		void set_time(long long t);
	};

} // namespace util
} // namespace trajectory
