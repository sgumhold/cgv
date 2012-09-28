/** this source is used to make pthread also work when linked statically */

#ifdef PTW32_STATIC_LIB

#include "pthread.h"

struct thread_control_member
{
	thread_control_member()
	{
		pthread_win32_process_attach_np ();
		pthread_win32_thread_attach_np ();
	}
	~thread_control_member()
	{
		pthread_win32_thread_detach_np ();
		pthread_win32_process_detach_np ();
	}
};

thread_control_member static_thread_control;

#endif /* PTW32_STATIC_LIB */
