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
static BOOL perfcounter_available;
static bool queried_perfcounter = false;
#endif

void stopwatch::init()
{
#ifdef _WIN32
	if (!queried_perfcounter) {
		perfcounter_available = QueryPerformanceFrequency((LARGE_INTEGER*) &frequency);
		queried_perfcounter = true;
	}
	if (perfcounter_available)
		QueryPerformanceCounter((LARGE_INTEGER*) &start);
	else
#endif			
		(std::clock_t&) start = std::clock();
}

//standard constructor starts time measurement
stopwatch::stopwatch()
{
	resultd = 0;
	init();
} 

//start counting time
stopwatch::stopwatch(double *result)
{
	this->resultd = result;
	init();
}

double stopwatch::get_current_time(long long& end) const
{
	double time;
#ifdef _WIN32
	if(perfcounter_available)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &end);
		time =(end-start)/ (double)frequency;
	}
	else
	{
#endif
		end = clock();
		std::clock_t total = (std::clock_t)(end - start); //get elapsed time
		time =  double(total)/CLOCKS_PER_SEC;
#ifdef _WIN32
	}
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
	else
		std::cout << "elapsed time in seconds: " << time << std::endl;
}

//destructor stops time measurement and puts the result into cout
stopwatch::~stopwatch()
{
	add_time();
}

	}
}

