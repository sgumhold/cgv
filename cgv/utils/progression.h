#pragma once

#include <string>
#include <cgv/type/standard_types.h>

#include "lib_begin.h"

namespace cgv {
	/// namespace that holds tools that dont fit any other namespace
	namespace utils {

/** progression provides a simple possibility to show progression of process in console. */
struct CGV_API progression
{
	double next_shown;
	double next_step;
	double percent;
	double percent_step;
	cgv::type::int64_type enumerations;
public:
	/// create empty progression
	progression();
	/// create from total enumerations and number of times to print progression
	progression(const std::string& process, size_t total, int count);
	/// reinitialize
	void init(const std::string& process, size_t total, int count);
	/// next iteration
	void step();
};

	}
}

#include <cgv/config/lib_end.h>