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
	/// try to lock the mutex (return false if the mutex is still locked)
	bool try_lock();
	/// lock the mutex (if the mutex is already locked, the caller is blocked until the mutex becomes available)
	void lock();
	/// unlock the mutex
	void unlock();
	/// same as lock but with printing debug information
	void debug_lock(const std::string& info);
	/// same unlock but with printing debug information
	void debug_unlock(const std::string& info);
	/// return the global locking counter that is used for mutex debugging
	static unsigned get_debug_lock_counter();
};

class thread;

/**
*A mutex that can wake up other threads by signals sent when a condition is fulfilled 
*/
struct CGV_API condition_mutex : public mutex
{
protected:
	void* pcond;
	friend class thread;
public:
	///construct a mutex
	condition_mutex();
	///destruct a mutex
	~condition_mutex();
	/// send the signal to unblock a thread waiting for the condition represented by this condition_mutex
	void send_signal();
	/// prefered approach to send the signal and implemented as {lock();send_signal();unlock();}
	void send_signal_with_lock();
	/// broadcast signal to unblock several threads waiting for the condition represented by this condition_mutex
	void broadcast_signal();
	/// prefered approach to broadcast the signal and implemented as {lock();broadcast_signal();unlock();}
	void broadcast_signal_with_lock();
};

	}
}

#include <cgv/config/lib_end.h>