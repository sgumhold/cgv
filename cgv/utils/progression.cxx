#include "progression.h"

#include <iostream>

namespace cgv {
	/// namespace that holds tools that dont fit any other namespace
	namespace utils {

/// create empty progression
progression::progression() 
{
	enumerations = -1; 
}

/// create from total enumerations and number of times to print progression
progression::progression(const std::string& process, size_t total, int count) 
{
	init(process, total, count); 
}

/// reinitialize
void progression::init(const std::string& process, size_t total, int count) 
{
	if (count > 0) {
		std::cout << process << ":"; std::cout.flush();
		percent      = 0;
		enumerations = 0;
		next_shown   = next_step = (total-1.1)/count;
		percent_step = 100.0f/count;
	}
	else
		enumerations = -1;
}

/// next iteration
void progression::step()
{
	if (enumerations < 0) 
		return;
	if (++enumerations > next_shown) {
		next_shown += next_step;
		percent   += percent_step;
		std::cout << " " << percent << "%";
		if (percent > 99.9f)
			std::cout << std::endl;
		else
			std::cout.flush();
	}
}

	}
}