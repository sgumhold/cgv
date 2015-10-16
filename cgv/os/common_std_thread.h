#include <thread>
#include <mutex>
#include <condition_variable>

typedef std::mutex Mutex;

struct Condition
{
	std::unique_lock<std::mutex> ul;
	std::condition_variable cv;
	Condition(Mutex& m) : ul(m, std::defer_lock) 
	{
	}
};

