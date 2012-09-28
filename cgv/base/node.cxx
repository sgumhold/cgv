#include "node.h"

namespace cgv {
	namespace base {


/// construct from name
node::node(const std::string& _name) : named(_name)
{
}

/// return the parent node
node_ptr node::get_parent() const
{
	return parent;
}

/// return the root node by traversing parents until no more parent is available
base_ptr node::get_root() const
{
	node_ptr np(const_cast<node*>(this));
	do {
		base_ptr bp = np->get_parent();
		if (!bp)
			return np;
		np = bp->get_node();
		if (!np)
			return bp;
	} 
	while (true);
}

/// set a new parent node
void node::set_parent(node_ptr _parent)
{
	parent = _parent;
}

/// cast upward to node
node_ptr node::get_node()
{
	return node_ptr(this);
}


/// overload to return the type name of this object
std::string node::get_type_name() const
{
	return "node";
}

	}
}
