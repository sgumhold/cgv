#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/**
* A trivial stop watch class for time measurement.
* On Win32 the perfomance counter with a resolution of 0,313 microseconds is used, if available.
* If the performance counter is not available and on other systems the std::clock() function with 
* a resolution of 0,055 milliseconds is used.
*
* TODO: Better implementation for Linux (with higher resolution)
* 
* Example 1:
*
* {
*	stopwatch s; 
*	dosomethingtimeconsuming();
* }
* 
* The duration is written into cout.
*
* Example 2:
* double d = 0.0;
*
* {
*	stopwatch s(&d); 
*	dosomethingtimeconsuming();
* }
*
* See also \link Profiler \endlink
*/

class CGV_API stopwatch
{
public:
	//standard constructor starts time measurement
	stopwatch();
	//start counting time
	stopwatch(double *result);
	// add_time adds the time ellapsed thus far
	void add_time();
	//destructor stops time measurement and puts the result into cout
	~stopwatch();
	/// return time elpased thus far
	double get_elapsed_time() const;
	/// restart timer and return time elapsed until restart
	double restart();
private:
	void init();
	double get_current_time(long long& end) const;
	long long start;
	double *resultd;	
};

	}
}

#include <cgv/config/lib_end.h>