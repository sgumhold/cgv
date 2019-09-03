#include <iostream>
#include "common_std_thread.h"

namespace cgv {
	namespace os {


///create the thread
thread::thread()
{ 
	thread_ptr = 0;	
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
		std::thread*& std_thread_ptr = reinterpret_cast<std::thread*&>(thread_ptr);
		//if (std_thread_ptr)
			//delete std_thread_ptr;
		std_thread_ptr = new std::thread(&cgv::os::thread::execute_s, this);
		running=true;
	}
}

/// sleep till the signal from the given condition_mutex is sent
void thread::wait_for_signal(condition_mutex& cm)
{
	Condition& c = *((Condition*&) cm.pcond);
	c.cv.wait(c.ul);
}

/// sleep till the signal from the given condition_mutex is sent or the timeout is reached, lock the mutex first and unlock after waiting
bool thread::wait_for_signal_or_timeout(condition_mutex& cm, unsigned millisec)
{
	Condition& c = *((Condition*&) cm.pcond);
	std::chrono::milliseconds dura(millisec);
	return c.cv.wait_for(c.ul, dura) == std::cv_status::no_timeout;
}

/// prefered approach to wait for signal and implemented as { cm.lock(); wait_for_signal(cm); cm.unlock(); } 
void thread::wait_for_signal_with_lock(condition_mutex& cm)
{
	cm.lock(); 
	wait_for_signal(cm); 
	cm.unlock();
}

/// prefered approach to wait for signal or the timeout is reached and implemented as { cm.lock(); wait_for_signal_or_timeout(cm,millisec); cm.unlock(); } 
bool thread::wait_for_signal_or_timeout_with_lock(condition_mutex& cm, unsigned millisec)
{
	cm.lock(); 
	bool res =  wait_for_signal_or_timeout(cm, millisec); 
	cm.unlock();
	return res;
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
	std::chrono::milliseconds dura(millisec);
    std::this_thread::sleep_for(dura);
}

void thread::stop()
{
	if(running) {
		stop_request=true;
		std::thread* std_thread_ptr = reinterpret_cast<std::thread*>(thread_ptr);
		std_thread_ptr->join();
		stop_request=false;
	}
}

///kill a running thread
void thread::kill()
{
	if (running) {
		std::thread*& std_thread_ptr = reinterpret_cast<std::thread*&>(thread_ptr);
		std_thread_ptr->detach();
		delete std_thread_ptr;
		std_thread_ptr = 0;
		stop_request=false;
		running=false;
	}
}

///join the current thread
void thread::wait_for_completion()
{
	if (running) {
		std::thread& t = *((std::thread*&) thread_ptr);
		t.join();
	}
}

///standard destructor (a running thread will be killed)
thread::~thread()
{
	if(running)
		kill();
	if (thread_ptr) {
		std::thread* std_thread_ptr = reinterpret_cast<std::thread*>(thread_ptr);
		delete std_thread_ptr;
		std_thread_ptr = 0;
	}
}

/// return the id of the currently executed thread
thread_id_type thread::get_current_thread_id()
{
	std::thread::id id = std::this_thread::get_id();
	return (long long&) id;
}

/// return id of this thread
thread_id_type thread::get_id() const
{
	std::thread* std_thread_ptr = reinterpret_cast<std::thread*>(thread_ptr);
	std::thread::id id = std_thread_ptr->get_id();
	return (long long&) id;
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
