#pragma once

#include <vector>
#include <string>
#include <cgv/data/ref_ptr.h>

#include "lib_begin.h"

namespace cgv {
	namespace math {

class CGV_API sparse_les;

/// reference counted pointer type for sparse les solver
typedef cgv::data::ref_ptr<sparse_les,true> sparse_les_ptr;

/// capability options for a sparse linear solver
enum SparseLesCaps
{
	SLC_SYMMETRIC = 1,
	SLC_UNSYMMETRIC = 2,
	SLC_NZE_OPTIONAL = 4,
	SLC_ALL = 7
};

/// factory class for sparse linear system solvers
class CGV_API sparse_les_factory : public cgv::data::ref_counted
{
public:
	/// make constructor virtual
	virtual ~sparse_les_factory();
	/// return the supported capabilities of the solver
	virtual SparseLesCaps get_caps() const = 0;
	/// return the name of the solver
	virtual std::string get_solver_name() const = 0;
	//! create an instance of the solver.
	/*! The parameters to the creation function are 
	    - n      ... the number of unknowns, 
		- nr_rhs ... the number of right hand sides
		- nr_nze ... the estimated number of non zero elements in the 
		             sparse matrix. For solvers with the SLC_NZE_OPTIONAL
					 cap flag set, one can skip this argument. */
	virtual sparse_les_ptr create(int n, int nr_rhs, int nr_nze = -1) = 0;
};

/// reference counted pointer type for sparse les solver factories
typedef cgv::data::ref_ptr<sparse_les_factory,true> sparse_les_factory_ptr;


/** interface for implementations of solvers for sparse linear systems of the form A * x = b with a square matrix A. */
class CGV_API sparse_les : public cgv::data::ref_counted
{
public:
	/**@name static interface */
	//@{
	/// register a factory for a new type of linear equation solver
	static void register_solver_factory(sparse_les_factory_ptr sls_fac);
	/// return the list of registered solvers
	static const std::vector<sparse_les_factory_ptr>& get_solver_factories();
	/// check the registered solver types for one with the given name
	static sparse_les_ptr create_by_name(const std::string& solver_name, int n, int nr_rhs, int nr_nze = -1);
	/// check the registered solver types for one with the given capabilities and create an instance of it
	static sparse_les_ptr create_by_cap(SparseLesCaps caps, int n, int nr_rhs, int nr_nze = -1);
	//@}
	/**@name problem setup */
	//@{
	/// set entry in row r and column c in the sparse matrix A
	virtual void set_mat_entry(int r, int c, double val) = 0;
	/// set i-th entry in a single right hand side b
	virtual void set_b_entry(int i, double val);
	/// set i-th entry in the j-th right hand side
	virtual void set_b_entry(int i, int j, double val) = 0;
	/// return reference to i-th entry in a single right hand side b
	virtual double& ref_b_entry(int i);
	/// set i-th entry in j-th right hand side
	virtual double& ref_b_entry(int i, int j) = 0;
	//@}
	/// try to solve system -  in case of success analyze residuals if demanded by the optional parameter
	virtual bool solve(bool analyze_residual = false) = 0;
	/**@name access to solution*/
	//@{
	/// return the i-th component of a single solution vector
	virtual double get_x_entry(int i) const;
	/// return the i-th component of the j-th solution vector
	virtual double get_x_entry(int i, int j) const = 0;
	//@}
};

/// implementation of factory class for sparse linear system solvers
template <class T>
class sparse_les_factory_impl : public sparse_les_factory
{
protected:
	std::string solver_name;
	SparseLesCaps caps;
public:
	sparse_les_factory_impl(const std::string& _solver_name, SparseLesCaps _caps) :
	  solver_name(_solver_name), caps(_caps) {}
	/// return the supported capabilities of the solver
	SparseLesCaps get_caps() const { return caps; }
	/// return the name of the solver
	std::string get_solver_name() const { return solver_name; }
	/// create an instance of the solver.
	sparse_les_ptr create(int n, int nr_rhs, int nr_nze) {
		return sparse_les_ptr(new T(n, nr_rhs, nr_nze));
	}
};


/// helper template to register a sparse les solver
template <class T>
struct register_sparse_les_factory
{
	register_sparse_les_factory(const std::string& _solver_name, SparseLesCaps _caps) {
		sparse_les::register_solver_factory(sparse_les_factory_ptr(
			new sparse_les_factory_impl<T>(_solver_name, _caps)));
	}
};

	}
}

#include <cgv/config/lib_end.h>