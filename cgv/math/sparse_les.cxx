#include "sparse_les.h"
#include <string>

namespace cgv {
	namespace math {

/// make constructor virtual
sparse_les_factory::~sparse_les_factory()
{
}

std::vector<sparse_les_factory_ptr>& ref_solver_factories()
{
	static std::vector<sparse_les_factory_ptr> facs;
	return facs;
}

/// register a factory for a new type of linear equation solver
void sparse_les::register_solver_factory(sparse_les_factory_ptr sls_fac)
{
	ref_solver_factories().push_back(sls_fac);
}
/// return the list of registered solvers
const std::vector<sparse_les_factory_ptr>& sparse_les::get_solver_factories()
{
	return ref_solver_factories();
}

/// check the registered solver types for one with the given name
sparse_les_ptr sparse_les::create_by_name(const std::string& solver_name, int n, int nr_rhs, int nr_nze)
{
	std::vector<sparse_les_factory_ptr>& F = ref_solver_factories();
	for (std::vector<sparse_les_factory_ptr>::const_iterator fi = F.begin(); fi != F.end(); ++fi) {
		if ((*fi)->get_solver_name() == solver_name)
			return (*fi)->create(n, nr_rhs, nr_nze);
	}
	return sparse_les_ptr();
}

/// check the registered solver types for one with the given capabilities and create an instance of it
sparse_les_ptr sparse_les::create_by_cap(SparseLesCaps caps, int n, int nr_rhs, int nr_nze)
{
	std::vector<sparse_les_factory_ptr>& F = ref_solver_factories();
	for (std::vector<sparse_les_factory_ptr>::const_iterator fi = F.begin(); fi != F.end(); ++fi) {
		if (((*fi)->get_caps() & caps) == caps)
			return (*fi)->create(n, nr_rhs, nr_nze);
	}
	return sparse_les_ptr();
}

/// set i-th entry in a single right hand side b
void sparse_les::set_b_entry(int i, double val)
{
	set_b_entry(i,0,val);
}
/// return reference to i-th entry in a single right hand side b
double& sparse_les::ref_b_entry(int i)
{
	return ref_b_entry(i, 0);
}
/// return the i-th component of a single solution vector
double sparse_les::get_x_entry(int i) const
{
	return get_x_entry(i,0);
}

	}
}