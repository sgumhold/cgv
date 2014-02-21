#include "priority.h"
#include <iostream>

#ifdef WIN32
#include <Windows.h>
#endif

namespace cgv {
	namespace os {

ExecutionPriority get_execution_priority()
{
#ifdef WIN32
	int pp = GetPriorityClass(GetCurrentProcess());
	int tp = GetThreadPriority(GetCurrentThread());

	if (pp == NORMAL_PRIORITY_CLASS && tp == THREAD_PRIORITY_NORMAL)
		return EP_NORMAL;

	if (pp == IDLE_PRIORITY_CLASS && tp == THREAD_PRIORITY_IDLE)
		return EP_IDLE;

	if (pp == REALTIME_PRIORITY_CLASS && tp == THREAD_PRIORITY_TIME_CRITICAL)
		return EP_HIGH;
#endif

	return EP_NORMAL;
}


bool set_execution_priority(ExecutionPriority p)
{
#ifdef WIN32
	switch (p) {
	case EP_IDLE :
		if (!SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS)) {
			std::cerr << "could not set idle process priority" << std::endl;
			return false;
		}
		if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE)) {
			std::cerr << "could not set idle thread priority" << std::endl;
			return false;
		}
		break;
	case EP_NORMAL :
		if (!SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS)) {
			std::cerr << "could not set normal process priority" << std::endl;
			return false;
		}
		if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL)) {
			std::cerr << "could not set normal thread priority" << std::endl;
			return false;
		}
		break;
	case EP_HIGH :
		if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
			std::cerr << "could not set realtime process priority" << std::endl;
			return false;
		}
		if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL)) {
			std::cerr << "could not set time critical thread priority" << std::endl;
			return false;
		}
		break;
	}
	return true;
#else
	std::cerr << "set_execution_priority not implemented" << std::endl;
	return false;
#endif
}

	}
}