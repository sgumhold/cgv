#include "stopwatch.h"

#include <iostream>
#include <ctime>
#ifdef _WIN32
#define NOMINMAX
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

// add_time adds the time ellapsed thus far
void stopwatch::add_time()
{
	double time;
#ifdef _WIN32
	if(perfcounter_available)
	{
		long long end;
		QueryPerformanceCounter((LARGE_INTEGER*) &end);
		time =(end-start)/ (double)frequency;
		start = end;
	}
	else
	{
#endif
		std::clock_t end = clock();
		clock_t total = end - (std::clock_t&)start; //get elapsed time
		time =  double(total)/CLOCKS_PER_SEC;
		start = end;
#ifdef _WIN32
	}
#endif
	
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

