#pragma once

#include "mutex.h"
#include <iostream>

#include "lib_begin.h"

namespace cgv {
	namespace os {

/// type used to identify a thread
typedef unsigned long long thread_id_type;

/** Thread class implementation that uses pthreads internally. 
To create and run your own thread, follow this example:
\begincode
class mythread : thread
{
	
	void run()
	{
		//no external stop request? 
		while(no_stop_request())
		{
			std::cout << "abc" << std::endl;
		}
	}
	
};

mythread t1;

t1.start();
...
t1.stop();
\endcode
*/
class CGV_API thread
{
protected:
	void *pthread;
	bool stop_request;
	bool running;
	bool delete_after_termination;
	static void* execute_s(void* args);
	/// executes the run method 
	void execute();
public:
	///create the thread
	thread();
	///standard destructor (a running thread will be killed)
	virtual ~thread();
	///start the implemented run() method (asynchronly) and destruct the thread object
	void start(bool _delete_after_termination = false);
	///thread function to override
	virtual void run()=0;	
	/// try to stop the thread execution via indicating a stop request. The existence of a stop request
	/// can be recognized by the no_stop_request() method. This test should be done periodically within
	/// the implementation of the run() method, e.g. to leave the execution loop in a clean way.
	void stop();
	/// kill a running thread
	void kill();
	/** the thread is interpreted as a slave thread and started from another master thread. This
	    method is called from the master thread in order to wait for termination of the slave thread. */
	void wait_for_completion();
	///return true if thread is running
	inline bool is_running() { return running; }
	/// check if there is a stop request 
	inline bool have_stop_request() { return stop_request; }
	/// return the id of the currently executed thread
	static thread_id_type get_current_thread_id();
	/// return id of this thread
	thread_id_type get_id() const;
};
	
	}
}

#include <cgv/config/lib_end.h>