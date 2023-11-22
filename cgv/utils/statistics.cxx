#include "statistics.h"

#include <iostream>
#include <cmath>

namespace cgv {
	namespace utils {

/** incrementally accumulate statistical information */
/// initialize with no value considered yet
statistics::statistics()
{
	init(); 
}
/// initialize and consider one value
statistics::statistics(const double& v) { init(v); }
/// initialize and consider n equal values
statistics::statistics(const double& v, unsigned int n) { init(v,n); }
/// initialize
void statistics::init() { cnt = 0; }
/// initialize and consider one value
void statistics::init(const double& v) { min = max = sum = v; sms = v*v; cnt = 1; }
/// initialize and consider n equal values
void statistics::init(const double& v, unsigned int n) { min = max = v; sum = n*v; sms = n*v*v; cnt = n; }
/// consider another value
void statistics::update(const double& v) {
	if (cnt) {
		if (v > max) max = v;
		if (v < min) min = v;
		sum += v;
		sms += v*v;
		++cnt;
	}
	else
		init(v);
}
/// consider another value count times
void statistics::update(const double& v, unsigned int n) {
	if (cnt) {
		if (v > max) max = v;
		if (v < min) min = v;
		sum += n*v;
		sms += n*v*v;
		cnt += n;
	}
	else
		init(v,n);
}
/// compute average of the considered values
double statistics::get_average() const { return sum/cnt; }
/// compute variance of the considered values
double statistics::get_variance() const {
	double E = get_average();
	double E2 = sms / cnt;
	return E2 - E*E;
}
/// compute standard deviation of the considered values
double statistics::get_standard_deviation() const {
	return sqrt(get_variance());
}
/// get the sum of the considered variables
double statistics::get_sum() const { return sum; }
/// get the sum of the squares of the considered variables
double statistics::get_sum_of_squares() const { return sms; }
/// get the minimum of the considered variables
double statistics::get_min() const { return min; }
/// get the maximum of the considered variables
double statistics::get_max() const { return max; }
/// get the number of considered variables
unsigned int statistics::get_count() const { return cnt; }

std::ostream& operator << (std::ostream& os, const statistics& s)
{
	return os <<  s.get_min() << ".." << s.get_max() 
				 << ":<" << s.get_sum() << "/" << s.get_count() 
				 << "=" << s.get_average() << "+-" << s.get_standard_deviation() << ">";
}

	}
}