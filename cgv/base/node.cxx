#include "node.h"

namespace cgv {
	namespace base {

node::node(const std::string& _name) : named(_name)
{
}
node_ptr node::get_parent() const
{
	return parent;
}
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
void node::set_parent(node_ptr _parent)
{
	parent = _parent;
}
node_ptr node::get_node()
{
	return node_ptr(this);
}
const_node_ptr node::get_node_const()
{
	return const_node_ptr(this);
}
std::string node::get_type_name() const
{
	return "node";
}

	}
}
