#pragma once

#include <cgv/base/action.h>

namespace cgv {
	namespace base {

/// simple action implementation that adds nodes implementing X to a results vector
template <class X>
class find_action : public action
{
	X* x;
protected:
	std::vector<X*>& result;
public:
	find_action(std::vector<X*>& _result) : action(true,true), result(_result) {}
	/// make the passed object current
	void select(base_ptr p) {
		x = p->get_interface<X>();
	}
	bool begin()
	{
		if (x) {
			result.push_back(x);
			return true;
		}
		else
			return false;
	}
	/// check if the current object implements the interface needed for this action
	bool implements_action() const {
		return x != 0;
	}
};

/// collect all nodes that implement interface X
template <class X>
void find_interface(base_ptr start, std::vector<X*>& result) {
	find_action<X> fa(result);
	traverser(fa).traverse(start);
}

/// traverse the hierarchy to find the i-th instance of type T and set pointer to it but only in case pointer is the nullptr, returns false if pointer was nullptr and no instance of type T was found
template <class X, typename T>
bool ensure_by_find(X* start, T*& pointer, unsigned i = 0)
{
	if (pointer)
		return true;

	std::vector<T*> instances;
	cgv::base::find_interface<T>(base_ptr(start), instances);
	if (instances.empty())
		return false;
	pointer = instances[i < instances.size() ? i : instances.size() - 1];
	return true;
}

	}
}