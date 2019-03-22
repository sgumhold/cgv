#include "thread.h"
#include <pthread.h>
#include <iostream>
#ifdef _WIN32
#include <concrt.h>
#else
#include <unistd.h>
#endif

namespace cgv {
	namespace os {

///create the thread
thread::thread()
{ 
	thread_ptr = new pthread_t();	
	stop_request = false;
	running = false;
	delete_after_termination = false;
}

///start the implemented run() method (asynchronly)
void thread::start(bool _delete_after_termination)
{
	if(!running) {
		delete_after_termination = _delete_after_termination;
		stop_request=false;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		if(pthread_create((pthread_t*)thread_ptr,&attr,execute_s,this))
			std::cerr << "error: starting thread" <<std::endl;
		pthread_attr_destroy(&attr);
		running=true;
	}
}

/// sleep till the signal from the given condition_mutex is sent
void thread::wait_for_signal(condition_mutex& cm)
{
	pthread_cond_wait(&(pthread_cond_t&)cm.pcond, &(pthread_mutex_t&)cm.pmutex);
}

/// prefered approach to wait for signal and implemented as { cm.lock(); wait_for_signal(cm); cm.unlock(); } 
void thread::wait_for_signal_with_lock(condition_mutex& cm)
{
	cm.lock(); 
	wait_for_signal(cm); 
	cm.unlock();
}

void* thread::execute_s(void* args)
{
	thread* t = (thread*)args;
	t->execute(); 
	if (t->delete_after_termination)
		delete t;
	return NULL; 
}


void thread::execute()
{
	run();
	running = false; 
}

/// wait the given number of milliseconds
void thread::wait(unsigned millisec)
{
#ifdef _WIN32
	Concurrency::wait(millisec);
#else
	usleep(1000*millisec);
#endif
}

void thread::stop()
{
	if(running) {
		stop_request=true;
		pthread_join(*(pthread_t*)thread_ptr,NULL);
		stop_request=false;
	}
}

///kill a running thread
void thread::kill()
{
	if (running) {
		pthread_cancel(*(pthread_t*)thread_ptr);
		stop_request=false;
		running=false;
	}
}

///join the current thread
void thread::wait_for_completion()
{
	if (running)
		pthread_join(*(pthread_t*)thread_ptr,NULL);
	
}

///standard destructor (a running thread will be killed)
thread::~thread()
{
	if(running)
		kill();
	delete (pthread_t*)thread_ptr;
}

thread_id_type to_id(const pthread_t& pt)
{
	return (const thread_id_type&) pt;
}

/// return the id of the currently executed thread
thread_id_type thread::get_current_thread_id()
{
	return (const thread_id_type&) pthread_self();
}

/// return id of this thread
thread_id_type thread::get_id() const
{
	return *((const thread_id_type*)thread_ptr);
}

class function_thread : public thread
{
protected:
	void (*func)(thread_id_type);
public:
	function_thread(void (*_func)(thread_id_type)) 
	{
		func = _func;
	}
	void run()
	{
		func(get_id());
	}
};

/// start a function in a newly constructed thread, which is deleted automatically on termination
thread* start_in_thread(void (*func)(thread_id_type), bool _delete_after_termination)
{
	thread* ft = new function_thread(func);
	ft->start(_delete_after_termination);
	return ft;
}

	}
}
