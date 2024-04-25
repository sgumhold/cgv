#pragma once

#include <cgv/type/traits/min.h>
#include <cgv/type/traits/max.h>
#include <iostream>
#include <algorithm>
#include <vector>

namespace cgv {
	namespace math {

/** The interval template represents a closed interval of two numbers, i.e. [a,b].
    All arithmetic operators are overloaded for the interval template. This allows to
	compute bounds for an arbitrary expression. For example if the variable x is from
	the interval [-1,1] then the expression x*x is from the interval [0,1], which can
	be computed via
\codebegin
	interval<int> x(-1,1);
	interval<int> e = x*x;
	std::cout << e << std::endl;
\codeend
    Furthermore, the interval template supports further interval based operations like
	intersection and extension of an interval.
*/
template <typename T>
class interval
{	
public:
	// the interval lower bound [x,_]
	T lower_bound;
	// the interval upper bound [_,x]
	T upper_bound;

	/**@name interval operations*/
	//@{
	/// construct empty interval
	interval() : lower_bound(1), upper_bound(0) {}
	/** Construct interval over all valid values of the value type, where the parameter is
	    only a dummy to distinguish this constructor from the standard constructor.
	    This constructor relies on cgv::type::traits::min and cgv::type::traits::max. Thus if
		you want to build intervals over non standard types, you need to ensure that these two
		traits are instantiated for your type when using this constructor. */
	interval(bool) : lower_bound(type::traits::min_fct<T>::get_value()), upper_bound(type::traits::max_fct<T>::get_value()) {}
	/// copy constructor
	interval(const interval<T>& I) : lower_bound(I.lower_bound), upper_bound(I.upper_bound) {}
	/// Contruct interval from bounds, sort bounds if necessary. To construct empty interval call the standard constructor.
	interval(const T& lb, const T& ub) : lower_bound(std::min(lb, ub)), upper_bound(std::max(lb, ub)) {}
	/// check if interval is empty
	bool empty() const { return upper_bound < lower_bound; }
	/// set to empty interval
	void clear() { lower_bound = T(1); upper_bound = T(0); }
	/// check if given value is contained in interval
	bool contains(const T& v) const { return lower_bound <= v && v <= upper_bound; }
	/// return the center value, which is only valid if the interval is not empty
	T center() const { return (lower_bound + upper_bound) / T(2); }
	/// return the size of the interval
	T size() const { return empty() ? T(0) : upper_bound - lower_bound; }
	/// set interval to intersection with given interval and return reference to this interval
	interval<T>& intersect(const interval<T>& I) {
		lower_bound = std::max(lower_bound, I.lower_bound);
		upper_bound = std::min(upper_bound, I.upper_bound);
		return *this;
	}
	/// return intersection interval
	interval<T> intersection(const interval<T>& I) const { interval J(*this); return J.intersect(I); } 
	/// extend interval such that it includes the given value
	interval<T>& extend(const T& v) { 
		if(empty()) {
			lower_bound = v;
			upper_bound = v;
		} else {
			lower_bound = std::min(lower_bound, v);
			upper_bound = std::max(upper_bound, v);
		}
		return *this;
	}
	/// extend interval such that it includes the n given values
	interval<T>& extend(const T vs[], size_t n) {
		for(size_t i = 0; i < n; ++i)
			extend(vs[i]);
		return *this;
	}
	/// extend interval such that it includes the given values
	interval<T>& extend(const std::vector<T>& vs) {
		for(size_t i = 0; i < vs.size(); ++i)
			extend(vs[i]);
		return *this;
	}
	/// return extension of interval so that it includes the given value
	interval<T>& extension(const T& v) { interval<T> I(*this); return I.extend(v); }
	/// extend interval such that it includes the given interval
	interval<T>& extend(const interval<T>& I) {
		if (!I.empty()) {
			extend(I.lower_bound);
			extend(I.upper_bound);
		}
		return *this;
	}
	/// return extension of interval so that it includes the given interval
	interval<T>& extension(const interval<T>& J) { interval<T> I(*this); return I.extend(J); }
	//@}
	/**@name scalar operators */
	//@{
	/// scale the interval
	interval<T>& operator *= (const T& s) { lower_bound *= s; upper_bound *= s; return *this; } 
	/// return scaled interval
	interval<T> operator * (const T& s) const { interval I(*this); return I *= s; } 
	/// divide the interval
	interval<T>& operator /= (const T& s) { return *this *= T(1) / s; } 
	/// return divided interval
	interval<T> operator / (const T& s) const { return *this * (T(1) / s); } 
	/// right shift interval by adding scalar to both bounds
	interval<T>& operator += (const T& s) { lower_bound += s; upper_bound += s; return *this; } 
	/// return right shifted interval
	interval<T> operator + (const T& s) const { interval I(*this); return I += s; } 
	/// left shift interval by subtracting scalar from both bounds
	interval<T>& operator -= (const T& s) { lower_bound -= s; upper_bound -= s; return *this; } 
	/// return left shifted interval
	interval<T> operator - (const T& s) const { interval I(*this); return I -= s; } 
	//@}

	/**@name interval operators */
	//@{
	/// compute the interval of all values of all products of values from the two intervals
	interval<T>& operator *= (const interval<T>& I) { 
		if (empty() || I.empty())
			*this = interval<T>();
		else 
			extend(lower_bound * I.lower_bound, lower_bound * I.upper_bound,
				   upper_bound * I.lower_bound, upper_bound * I.upper_bound);
		return *this; 
	} 
	/// return the interval of all values of products of values from the two intervals
	interval<T> operator * (const interval<T>& J) const { interval I(*this); return I *= J; } 
	/** Compute the interval of all values of all quotients of values from the two intervals. 
	    Notice that if zero is contained in the second interval, the result is the complete
		interval of all valid values and the same precondition holds as for the constructor
		with the bool dummy parameter interval(bool). */
	interval<T>& operator /= (const interval<T>& I) { 
		if (empty() || I.empty())
			*this = interval<T>();
		else {
			if (I.contains(T(0)))
				*this = interval<T>(true);
			else
				extend(lower_bound / I.lower_bound, lower_bound/I.upper_bound,
					   upper_bound / I.lower_bound, upper_bound/I.upper_bound);
		}
		return *this;
	} 
	/// return the interval of all values of quotients of values from the two intervals. See also comments on the /= operator().
	interval<T> operator / (const interval<T>& J) const { interval I(*this); return I /= J; } 
	/// unary minus operator reflects interval at zero value
	interval<T> operator - () const { if (empty()) return *this; else return interval<T>(-upper_bound, -lower_bound); }
	/// compute interval of all sums of values from the two intervals
	interval<T>& operator += (const interval<T>& I) { 
		if (empty() || I.empty())
			*this = interval<T>();
		else {
			extend(lower_bound + I.lower_bound, lower_bound + I.upper_bound,
				   upper_bound + I.lower_bound, upper_bound + I.upper_bound);
		}
		return *this; 
	} 
	/// return interval of all sums of values from the two intervals
	interval<T> operator + (const interval<T>& J) const { interval I(*this); return I += J; } 
	/// compute interval of all differences of values from the two intervals
	interval<T>& operator -= (const interval<T>& I) { return *this += -I; }
	/// return interval of all differences of values from the two intervals
	interval<T> operator - (const interval<T>& J) const { interval I(*this); return I -= J; } 
	//@}

	/**@name comparison operators*/
	//@{
	/// check for equality of two intervals
	bool operator == (const interval<T>& I) const {
		return empty() && I.empty() ||
			lower_bound == I.lower_bound && upper_bound == I.upper_bound;
	}
	/// check for inequality of two intervals
	bool operator != (const interval<T>& I) const {
		return !(*this == I);
	}
	/// only returns true if both intervals are not empty and the operator holds for all values in both intervals
	bool operator < (const interval<T>& I) const {
		return !empty() && !I.empty() && upper_bound < I.lower_bound;
	}
	/// only returns true if both intervals are not empty and the operator holds for all values in both intervals
	bool operator > (const interval<T>& I) const {
		return !empty() && !I.empty() && lower_bound > I.upper_bound;
	}
	//@}

};

template <typename T> inline interval<T> operator + (const T& s, const interval<T>& I) { return  I+s; }
template <typename T> inline interval<T> operator - (const T& s, const interval<T>& I) { return -I+s; }
template <typename T> inline interval<T> operator * (const T& s, const interval<T>& I) { return  I*s; }
template <typename T> inline interval<T> operator / (const T& s, const interval<T>& I) { return interval<T>(s,s)/I; }

template <typename T>
std::ostream& operator << (std::ostream& os, const interval<T>& I) {
	return os << '[' << I.lower_bound << ',' << I.upper_bound << ']';
}

	}
}