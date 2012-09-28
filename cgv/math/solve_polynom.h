#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace math {

extern CGV_API int solve_linear(double c[2], double s[1]);
/// write the solutions to c[0]+c[1]*x+c[2]*x*x = 0 into s and return the number of solutions
extern CGV_API int solve_quadric(double c[3], double s[2], bool replicate_multiple_solutions = false);
/// write the solutions to c[0]+c[1]*x+c[2]*x*x+c[3]*x*x*x  = 0 into s and return the number of solutions
extern CGV_API int solve_cubic(double c[4], double s[3], bool replicate_multiple_solutions = false);
/// write the solutions to c[0]+c[1]*x+c[2]*x*x+c[3]*x*x*x+c[4]*x*x*x*x  = 0 into s and return the number of solutions
extern CGV_API int solve_quartic(double c[5], double s[4]);

	}
}

#include <cgv/config/lib_end.h>
