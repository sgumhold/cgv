#pragma once

#include "named.h"
#include <cgv/data/ref_ptr.h>
#include "lib_begin.h"

namespace cgv {
	namespace base {

class CGV_API node;

/// ref counted pointer to a node
typedef data::ref_ptr<node,true> node_ptr;

/** The node class keeps a pointer to its parent */
class CGV_API node : public named
{
protected:
	/// declare node_ptr to be a friend class
	friend class data::ref_ptr<node,true>;
	/// store a pointer to the parent node
	node_ptr parent;
public:
	/// construct from name
	node(const std::string& name = "");
	/// return the parent node
	node_ptr get_parent() const;
	/// return the root node by traversing parents until no more parent is available
	base_ptr get_root() const;
	/// set a new parent node
	void set_parent(node_ptr _parent);
	/// cast upward to node
	data::ref_ptr<node,true> get_node();
	/// overload to return the type name of this object
	std::string get_type_name() const;
};

template <> 
struct cast_helper<node>
{ 
	inline static data::ref_ptr<node,true> cast(base* b) 
	{ 
		return b->get_node(); 
	} 
};

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<node>;
#endif

	}
}

#include <cgv/config/lib_end.h>
