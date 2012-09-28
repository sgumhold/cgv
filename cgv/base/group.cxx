#include "group.h"
#include <iostream>

namespace cgv {
	namespace base {

/// construct from name
group::group(const std::string& _name) : node(_name)
{
}

/// check if the base class is a node and set the parent of the node
void group::link(base_ptr child)
{
	node_ptr n = child->get_node();
	if (n.empty())
		return;
	n->set_parent(node_ptr(this));
}

/// check if the base class is a node and clear the parent of the node
void group::unlink(base_ptr child)
{
	node_ptr n = child->get_node();
	if (n.empty())
		return;
	n->set_parent(node_ptr());
}

/// return the number of children
unsigned int group::get_nr_children() const
{
	return (unsigned int) children.size();
}

/// return the i-th child
base_ptr group::get_child(unsigned int i) const
{
	if (i >= children.size()) {
		std::cerr << "group " << get_name() << ":" << get_type_name() << " -> attempt to access child at " << i << " what is after last valid location " << children.size()-1 << std::endl;
		return base_ptr();
	}
	return children[i];
}
/// append child and return index of appended child
unsigned int group::append_child(base_ptr child)
{
	children.push_back(child);
	link(child);
	return get_nr_children()-1;
}
/// remove all elements of the vector that point to child, return the number of removed children
unsigned int group::remove_child(base_ptr child)
{
	unsigned int i, nr_removed = 0;
	for (i=0; i<children.size(); ++i) {
		if (children[i] == child) {
			++nr_removed;
			children.erase(children.begin()+i);
			--i;
		}
	}
	unlink(child);
	return nr_removed;
}

/// remove all children
void group::remove_all_children()
{
	for (unsigned int i=0; i<children.size(); ++i)
		unlink(children[i]);
	children.clear();
}

/// insert a child at the given position
void group::insert_child(unsigned int i, base_ptr child)
{
	if (i > children.size()) {
		std::cerr << "attempt to insert child at " << i << " what is after last valid location " << children.size() << std::endl;
		return;
	}
	if (i == children.size())
		children.push_back(child);
	else
		children.insert(children.begin()+i, child);
	link(child);
}

/// cast upward to group
data::ref_ptr<group,true> group::get_group()
{
	return group_ptr(this);
}

/// overload to return the type name of this object
std::string group::get_type_name() const
{
	return "group";
}

	}
}
