#include "stopwatch.h"

#include <iostream>
#include <ctime>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace cgv {
	namespace utils {

#ifdef _WIN32
static long long frequency;
static bool queried_perfcounter = false;
#else
constexpr auto S_TO_NS=1000000000LL;
constexpr auto NS_TO_S=(1.0/double(S_TO_NS));
#endif

void stopwatch::init()
{
#ifdef _WIN32
	if (!queried_perfcounter) {
		QueryPerformanceFrequency((LARGE_INTEGER*) &frequency);
		queried_perfcounter = true;
	}
	QueryPerformanceCounter((LARGE_INTEGER*) &start);
#endif
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	start = ((long long)ts.tv_sec)*S_TO_NS + (long long)ts.tv_nsec;
}

//standard constructor starts time measurement
stopwatch::stopwatch(bool silent) : silent(silent)
{
	resultd = 0;
	init();
} 

//start counting time
stopwatch::stopwatch(double *result, bool silent) : silent(silent)
{
	this->resultd = result;
	init();
}

double stopwatch::get_current_time(long long& end) const
{
	double time;
#ifdef _WIN32
	QueryPerformanceCounter((LARGE_INTEGER*) &end);
	time =(end-start)/(double)frequency;
#else
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	end = ((long long)ts.tv_sec)*S_TO_NS + (long long)ts.tv_nsec;
	time = double(end - start)*NS_TO_S;
#endif
	return time;
}

/// return time elpased thus far
double stopwatch::get_elapsed_time() const
{
	long long end;
	return get_current_time(end);
}

/// restart timer and return time elapsed until restart
double stopwatch::restart()
{
	long long end;
	double time = get_current_time(end);
	start = end;
	return time;	
}

// add_time adds the time ellapsed thus far
void stopwatch::add_time()
{
	double time = restart();
	
	if(resultd)
		*resultd += time;	
	else if(!silent)
		std::cout << "elapsed time in seconds: " << time << std::endl;
}

//destructor stops time measurement and puts the result into cout
stopwatch::~stopwatch()
{
	add_time();
}

	}
}

