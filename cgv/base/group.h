#pragma once

#include <vector>
#include "node.h"

#include "lib_begin.h"

namespace cgv {
	namespace base {

class CGV_API group;

/// ref counted pointer to a node
typedef data::ref_ptr<group,true> group_ptr;

/** The group class is a node with children. */
class CGV_API group : public node
{
protected:
	/// declare group_ptr to be a friend class
	friend class data::ref_ptr<group,true>;
	/// store a list of children
	std::vector<base_ptr> children;
	/// check if the base class is a node and set the parent of the node
	void link(base_ptr b);
	/// check if the base class is a node and clear the parent of the node
	void unlink(base_ptr b);
public:
	/// construct from name
	group(const std::string& name = "");
	/// return the number of children
	unsigned int get_nr_children() const;
	/// return the i-th child
	base_ptr get_child(unsigned int i) const;
	/// append child and return index of appended child
	virtual unsigned int append_child(base_ptr child);
	/// remove all elements of the vector that point to child, return the number of removed children
	virtual unsigned int remove_child(base_ptr child);
	/// remove all children
	virtual void remove_all_children();
	/// insert a child at the given position
	virtual void insert_child(unsigned int i, base_ptr child);
	/// cast upward to group
	data::ref_ptr<group,true> get_group();
	/// overload to return the type name of this object
	std::string get_type_name() const;
};

template <> 
struct cast_helper<group>
{ 
	inline static data::ref_ptr<group,true> cast(base* b) 
	{
		return b->get_group(); 
	} 
};


#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<group>;
CGV_TEMPLATE template class CGV_API std::vector<base_ptr>;
#endif

	}
}

#include <cgv/config/lib_end.h>
