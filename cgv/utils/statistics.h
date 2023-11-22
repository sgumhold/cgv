#pragma once

#include <iostream>
#include <cmath>

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/** incrementally accumulate statistical information */
class CGV_API statistics
{
public:
	/// initialize with no value considered yet
	statistics();
	/// initialize and consider one value
	statistics(const double& v);
	/// initialize and consider n equal values
	statistics(const double& v, unsigned int n);
	/// initialize
	void init();
	/// initialize and consider one value
	void init(const double& v);
	/// initialize and consider n equal values
	void init(const double& v, unsigned int n);
	/// consider another value
	void update(const double& v);
	/// consider another value count times
	void update(const double& v, unsigned int n);
	/// compute average of the considered values
	double get_average() const;
	/// compute variance of the considered values
	double get_variance() const;
	/// compute standard deviation of the considered values
	double get_standard_deviation() const;
	/// get the sum of the considered variables
	double get_sum() const;
	/// get the sum of the squares of the considered variables
	double get_sum_of_squares() const;
	/// get the minimum of the considered variables
	double get_min() const;
	/// get the maximum of the considered variables
	double get_max() const;
	/// get the number of considered variables
	unsigned int get_count() const;
protected:
	double min, max, sum, sms;
	unsigned int cnt;
};


extern CGV_API std::ostream& operator << (std::ostream& os, const statistics& s);

	}
}

#include <cgv/config/lib_end.h>