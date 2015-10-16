#pragma once

#include <vector>

namespace cgv {
	namespace data {

struct union_find : public std::vector<unsigned int>
{
	/// construct with given number of elements
	union_find(unsigned int n) { init(n); }
	/// init such that each element is a representative
	void init(unsigned int n) { 
		resize(n); 
		for (unsigned int i=0; i<n; ++i) 
			at(i) = i; 
	}
	/// find representative with path compression
	unsigned int find(unsigned int i)
	{
		// find representative j
		unsigned int j = i;
		while (at(j) != j)
			j = at(j);
		// compress path
		while (i != j) {
			unsigned int tmp = at(i);
			at(i) = j;
			i = tmp;
		}
		return j;
	}
	/// union of two groups
	void unify(unsigned int i, unsigned int j)
	{
		at(find(i)) = j;
	}
};

	}
}