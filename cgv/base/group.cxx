#include "group.h"
#include <iostream>

namespace cgv {
	namespace base {

group::group(const std::string& _name) : node(_name)
{
}
void group::link(base_ptr child)
{
	node_ptr n = child->get_node();
	if (n.empty())
		return;
	n->set_parent(node_ptr(this));
}
void group::unlink(base_ptr child)
{
	node_ptr n = child->get_node();
	if (n.empty())
		return;
	n->set_parent(node_ptr());
}
unsigned int group::get_nr_children() const
{
	return (unsigned int) children.size();
}
base_ptr group::get_child(unsigned int i) const
{
	if (i >= children.size()) {
		std::cerr << "group " << get_name() << ":" << get_type_name() << " -> attempt to access child at " << i << " what is after last valid location " << children.size()-1 << std::endl;
		return base_ptr();
	}
	return children[i];
}
unsigned int group::append_child(base_ptr child)
{
	children.push_back(child);
	link(child);
	return get_nr_children()-1;
}
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
void group::remove_all_children()
{
	for (unsigned int i=0; i<children.size(); ++i)
		unlink(children[i]);
	children.clear();
}
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
group_ptr group::get_group()
{
	return group_ptr(this);
}
const_group_ptr group::get_group_const()
{
	return const_group_ptr(this);
}
std::string group::get_type_name() const
{
	return "group";
}

	}
}
