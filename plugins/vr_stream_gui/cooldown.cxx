#include "cooldown.h"

namespace trajectory {
namespace util {
	cooldown::cooldown(long long t) { time = t; }

	void cooldown::start() { time_start = std::chrono::high_resolution_clock::now(); }

	bool cooldown::is_done()
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - time_start)
		               .count();
		return dur > time;
	}

	bool cooldown::is_done_and_reset()
	{
		auto b = is_done();
		time_start = std::chrono::high_resolution_clock::now();
		return b;
	}

	void cooldown::set_time(long long t) { time = t; }
} // namespace util
} // namespace trajectory
