#include "thread.h"
#include <pthread.h>
#include <iostream>

namespace cgv {
	namespace os {

///create the thread
thread::thread()
{ 
	pthread = new pthread_t();	
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
		if(pthread_create((pthread_t*)pthread,NULL,execute_s,this))
			std::cerr << "error: starting thread" <<std::endl;
		running=true;
	}
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

void thread::stop()
{
	if(running) {
		stop_request=true;
		pthread_join(*(pthread_t*)pthread,NULL);
		stop_request=false;
	}
}

///kill a running thread
void thread::kill()
{
	if (running) {
		pthread_cancel(*(pthread_t*)pthread);
		stop_request=false;
		running=false;
	}
}

///join the current thread
void thread::wait_for_completion()
{
	if (running)
		pthread_join(*(pthread_t*)pthread,NULL);
	
}

///standard destructor (a running thread will be killed)
thread::~thread()
{
	if(running)
		kill();
	delete (pthread_t*)pthread;
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
	return *((const thread_id_type*)pthread);
}
	
	}
}
