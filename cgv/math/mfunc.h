#pragma once

#include <cgv/math/vec.h>

namespace cgv {
	namespace math {

/** interface class for multivariate function with two template arguments:
    - X defines the type of the independent variables and
	 - T the type of range to which the function maps
	 For example a function that maps from n double variables to an int
	 would be declared as mfunc<double,int>
*/
template <typename X, typename T>
class mfunc
{
public:
	/// points must have get_nr_independent_variables() components
	typedef cgv::math::vec<X> pnt_type;
	/// vectors must have get_nr_independent_variables() components
	typedef cgv::math::vec<X> vec_type;
	/// virtual destructor
	virtual ~mfunc() {}
	/// return the number of independent variables that are mapped by the function
	virtual unsigned get_nr_independent_variables() const = 0;
	/// interface for evaluation of the multivariate function
	virtual T evaluate(const pnt_type& p) const = 0;
	/** interface for evaluation of the gradient of the multivariate function.
	    default implementation uses central differences to 
       approximate the gradient, with an epsilon of 1e-5. */
	virtual vec_type evaluate_gradient(const pnt_type& p) const {
		static X epsilon   = (X)1e-5;
		static X inv_2_eps = (X)(0.5/epsilon);
		unsigned n = p.size();
		vec_type g(n);
		pnt_type q(p);
		for (unsigned i=0; i<n; ++i) {
			q(i) += epsilon;
			g(i)  = evaluate(q);
			q(i)  = p(i)-epsilon;
			g(i) -= evaluate(q);
			g(i) *= inv_2_eps;
			q(i)  = p(i);
		}
		return g;
	}
};

/** specialization of a multivariate function to two independent variables,
    which only reimplements the method get_nr_independent_variables. */
template <typename X, typename T>
class v2_func : public mfunc<X,T>
{
public:
	/// returns 2
	unsigned int get_nr_independent_variables() const { return 2; }
};

/** specialization of a multivariate function to three independent variables,
    which only reimplements the method get_nr_independent_variables. */
template <typename X, typename T>
class v3_func : public mfunc<X,T>
{
public:
	/// returns 3
	unsigned int get_nr_independent_variables() const { return 3; }
};

	}
}