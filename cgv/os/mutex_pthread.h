#include <pthread.h>
#include <iostream>
#include <errno.h>

namespace cgv {
	namespace os {

unsigned& ref_debug_lock_counter()
{
	static unsigned counter = 0;
	return counter;
}

mutex& ref_debug_mutex()
{
	static mutex m;
	return m;
}

/// return the global locking counter that is used for mutex debugging
unsigned mutex::get_debug_lock_counter()
{
	return ref_debug_lock_counter();
}

///construct a mutex
mutex::mutex()
{
	pthread_mutex_init (&(pthread_mutex_t&)pmutex, NULL);
}

///destruct a mutex
mutex::~mutex()
{
	pthread_mutex_destroy (&(pthread_mutex_t&)pmutex);
}

///lock the mutex (if the mutex is still locked, the caller is blocked until the
///mutex becomes available)
void mutex::lock()
{
	pthread_mutex_lock (&(pthread_mutex_t&)pmutex);	
}

///unlock the mutex
void mutex::unlock()
{
	pthread_mutex_unlock (&(pthread_mutex_t&)pmutex);	
}

/// same as lock but with printing debug information
void mutex::debug_lock(const std::string& info)
{
	ref_debug_mutex().lock();
	std::cout << info << ": wait for lock [" << this << ":" << get_debug_lock_counter() << "]" << std::endl;
	++ref_debug_lock_counter();
	ref_debug_mutex().unlock();
	lock();
	ref_debug_mutex().lock();
	std::cout << info << ": received lock [" << this << "]" << std::endl;
	ref_debug_mutex().unlock();
}
/// same unlock but with printing debug information
void mutex::debug_unlock(const std::string& info)
{
	unlock();
	ref_debug_mutex().lock();
	std::cout << info << ": unlock        [" << this << "]" << std::endl;
	ref_debug_mutex().unlock();
}


///try to lock the mutex (return false if the mutex is still locked)
bool mutex::try_lock()
{
	return pthread_mutex_trylock(&(pthread_mutex_t&)pmutex) != EBUSY;
}

///construct a mutex
condition_mutex::condition_mutex()
{
	pthread_cond_init(&(pthread_cond_t&)pcond, NULL);
}

///destruct a mutex
condition_mutex::~condition_mutex()
{
	pthread_cond_destroy(&(pthread_cond_t&)pcond);
}

/// send the signal to unblock a thread waiting for the condition represented by this condition_mutex
void condition_mutex::send_signal()
{
	pthread_cond_signal(&(pthread_cond_t&)pcond);
}

/// prefered approach to send the signal and implemented as {lock();send_signal();unlock();}
void condition_mutex::send_signal_with_lock()
{
	lock();
	send_signal();
	unlock();
}

/// broadcast signal to unblock several threads waiting for the condition represented by this condition_mutex
void condition_mutex::broadcast_signal()
{
	pthread_cond_broadcast(&(pthread_cond_t&)pcond);
}

/// prefered approach to broadcast the signal and implemented as {lock();broadcast_signal();unlock();}
void condition_mutex::broadcast_signal_with_lock()
{
	lock();
	broadcast_signal();
	unlock();
}

	}
}

