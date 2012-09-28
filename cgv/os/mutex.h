#pragma once

#include <string>

#include "lib_begin.h"

namespace cgv {
	namespace os {

/**
*A simple mutex (mutual exclusion) for solving thread synchronisation problems.
*/
struct CGV_API mutex
{
protected:
	void* pmutex;
public:
	///construct a mutex
	mutex();
	///destruct a mutex
	~mutex();
	///try to lock the mutex (return false if the mutex is still locked)
	bool try_lock();
	///lock the mutex (if the mutex is still locked, the caller is blocked until the
	///mutex becomes available)
	void lock();
	///unlock the mutex
	void unlock();
	/// same as lock but with printing debug information
	void debug_lock(const std::string& info);
	/// same unlock but with printing debug information
	void debug_unlock(const std::string& info);
	/// return the global locking counter that is used for mutex debugging
	static unsigned get_debug_lock_counter();
};

	}
}

