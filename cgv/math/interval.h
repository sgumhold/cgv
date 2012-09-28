#pragma once

#include <cgv/type/traits/min.h>
#include <cgv/type/traits/max.h>
#include <iostream>

namespace cgv {
	namespace math {

/** the interval template represents a close interval of two numbers, i.e. [a,b].
    All arithmetic operators are overloaded for the interval template. This allows to
	compute bounds for an arbitrary expression. For example if the variable x is from
	the interval [-1,1] then the expression x*x is from the interval [0,1], what can
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
protected:
	T lb, ub;
public:
	/**@name interval operations*/
	//@{
	/// construct empty interval
	interval() : lb(1), ub(0) {}
	/** construct interval over all valid values of the value type, where the parameter is
	    only a dummy to distinguish this constructor from the standard constructor.
	    This constructor relies on cgv::type::traits::min and cgv::type::traits::max. Thus if
		you want to build intervals over non standard types, you need to ensure that these two
		traits are instantiated for your type when using this constructor. */
	interval(bool) : lb(type::traits::min_fct<T>::get_value()), ub(type::traits::max_fct<T>::get_value()) {}
	/// copy constructor
	interval(const interval<T>& I) : lb(I.get_lower_bound()), ub(I.get_upper_bound()) {}
	/// contruct interval from bounds, sort bounds if necessary. To construct empty interval call the standard constructor
	interval(const T& _lb, const T& _ub) : lb(std::min(_lb,_ub)), ub(std::max(_lb,_ub)) {}
	/// check if interval is empty
	bool is_empty() const { return ub < lb; }
	/// set to empty interval
	void clear() { ub = 0; lb = 1; }
	/// check if given value is contained in interval
	bool contains(const T& v) const { return lb <= v && v <= ub; }
	/// set the lower bound
	void set_lower_bound(const T& _lb) { lb = _lb; }
	/// set the upper bound
	void set_upper_bound(const T& _ub) { ub = _ub; }
	/// return the lower bound
	const T& get_lower_bound() const { return lb; }
	/// return the upper bound
	const T& get_upper_bound() const { return ub; }
	/// return the center value, which is only valid if the interval is not empty
	T get_center() const { return (lb+ub)/2; }
	/// return the size of the interval
	T get_size() const { return is_empty() ? 0 : ub-lb; }
	/// set interval to intersection with given interval and return reference to this interval
	interval<T>& intersect(const interval<T>& I) {
		lb = std::max(lb, I.get_lower_bound());
		ub = std::min(ub, I.get_upper_bound());
		return *this;
	}
	/// return intersection interval
	interval<T> intersection(const interval<T>& I) const { interval J(*this); return J.intersect(I); } 
	/// extend interval such that it includes the given value
	interval<T>& extend(const T& v) { 
		if (is_empty()) 
			lb=ub=v; 
		else { 
			lb = std::min(lb,v);
			ub = std::max(ub,v);
		}
		return *this;
	}
	/// return extension of interval that it includes the given value
	interval<T>& extension(const T& v) { interval<T> I(*this); return I.extend(v); }
	/// extend interval such that it includes the given interval
	interval<T>& extend(const interval<T>& I) { 
		if (!I.is_empty()) {
			extend(I.get_lower_bound()); 
			extend(I.get_upper_bound()); 
		}
		return *this;
	}
	/// extend by four values
	interval<T>& extend(const T& v0, const T& v1, const T& v2, const T& v3) {
		extend(v0); extend(v1); extend(v2); extend(v3);
		return *this;
	}
	/// return extension of interval that it includes the given interval
	interval<T>& extension(const interval<T>& J) { interval<T> I(*this); return I.extend(J); }
	//@}

	/**@name scalar operators */
	//@{
	/// scale the interval
	interval<T>& operator *= (const T& s) { lb *= s; ub *= s; return *this; } 
	/// return scaled the interval
	interval<T> operator * (const T& s) const { interval I(*this); return I *= s; } 
	/// divide the interval
	interval<T>& operator /= (const T& s) { return *this *= 1/s; } 
	/// return divided the interval
	interval<T> operator / (const T& s) const { return *this * (1/s); } 
	/// right shift interval by adding scalar to both bounds
	interval<T>& operator += (const T& s) { lb += s; ub += s; return *this; } 
	/// return right shifted interval
	interval<T> operator + (const T& s) const { interval I(*this); return I += s; } 
	/// left shift interval by subtracting scalar from both bounds
	interval<T>& operator -= (const T& s) { lb -= s; ub -= s; return *this; } 
	/// return left shifted interval
	interval<T> operator - (const T& s) const { interval I(*this); return I -= s; } 
	//@}

	/**@name interval operators */
	//@{
	/// compute the interval of all values of all products of values from the two intervals
	interval<T>& operator *= (const interval<T>& I) { 
		if (is_empty() || I.is_empty())
			*this = interval<T>();
		else 
			extend(lb*I.get_lower_bound(), lb*I.get_upper_bound(),
				   ub*I.get_lower_bound(), ub*I.get_upper_bound());
		return *this; 
	} 
	/// return the interval of all values of products of values from the two intervals
	interval<T> operator * (const interval<T>& J) const { interval I(*this); return I *= J; } 
	/** compute the interval of all values of all quotients of values from the two intervals. 
	    Notice that if zero is contained in the second interval, the result is the complete
		interval of all valid values and the same precondition holds as for the constructor
		with the bool dummy parameter interval(bool). */
	interval<T>& operator /= (const interval<T>& I) { 
		if (is_empty() || I.is_empty())
			*this = interval<T>();
		else {
			if (I.contains(0))
				*this = interval<T>(true);
			else
				extend(lb/I.get_lower_bound(), lb/I.get_upper_bound(),
					   ub/I.get_lower_bound(), ub/I.get_upper_bound());
		}
		return *this;
	} 
	/// return the interval of all values of quotients of values from the two intervals. See also comments on the /= operator().
	interval<T> operator / (const interval<T>& J) const { interval I(*this); return I /= J; } 
	/// unary minus operator reflects interval at zero value
	interval<T> operator - () const { if (is_empty()) return *this; else return interval<T>(-ub,-lb); }
	/// compute interval of all sums of values from the two intervals
	interval<T>& operator += (const interval<T>& I) { 
		if (is_empty() || I.is_empty())
			*this = interval<T>();
		else {
			extend(lb+I.get_lower_bound(), lb+I.get_upper_bound(),
				   ub+I.get_lower_bound(), ub+I.get_upper_bound());
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
		return is_empty() && I.is_empty() ||
			lb == I.get_lower_bound() && ub == I.get_upper_bound();
	}
	/// check for inequality of two intervals
	bool operator != (const interval<T>& I) const {
		return !(*this == I);
	}
	/// only returns true if both intervals are not empty and the operator holds for all values in both intervals
	bool operator < (const interval<T>& I) const {
		return !is_empty() && !I.is_empty() && ub < I.get_lower_bound();
	}
	/// only returns true if both intervals are not empty and the operator holds for all values in both intervals
	bool operator > (const interval<T>& I) const {
		return !is_empty() && !I.is_empty() && lb > I.get_upper_bound();
	}
	//@}

};

template <typename T> inline interval<T> operator + (const T& s, const interval<T>& I) { return  I+s; }
template <typename T> inline interval<T> operator - (const T& s, const interval<T>& I) { return -I+s; }
template <typename T> inline interval<T> operator * (const T& s, const interval<T>& I) { return  I*s; }
template <typename T> inline interval<T> operator / (const T& s, const interval<T>& I) { return interval<T>(s,s)/I; }

template <typename T>
std::ostream& operator << (std::ostream& os, const interval<T>& I) {
	return os << '[' << I.get_lower_bound() << ',' << I.get_upper_bound() << ']';
}

	}
}