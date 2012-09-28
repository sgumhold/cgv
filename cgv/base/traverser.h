#pragma once

#include <string>
#include "group.h"
#include "action.h"

#include "lib_begin.h"

namespace cgv {
	namespace base {

/// different traversal policies
enum TraversePolicy { 
	TP_ALL,                 /// traverse all children 
	TP_ONLY_FOCUS,          /// traverse only the focused child
	TP_FIRST_FOCUS,         /// first traverse focused and then the remaining children
	TP_AUTO_FOCUS,          /// like previous but change focus to the child, where traversal succeeded
	TP_STOP_ON_SUCCESS = 8, /// this is an optional flag for traversals with methods that return a bool. If the returned bool is true, traversal stops if this flag is set
	TP_STOP_ON_FAILURE = 16 /// this is an optional flag for traversals with methods that return a bool. If the returned bool is false, traversal stops if this flag is set
};

/// nodes should inherit from this policy class to allow selective tree traversals
class CGV_API traverse_policy
{
protected:
	TraversePolicy policy;
	bool active;
	int focus;
public:
	/// construct default traverse policy that visits everything
	traverse_policy(int _policy = TP_ALL+TP_STOP_ON_SUCCESS, bool _active = true, int _focus = -1);
	/// return the policy without the stop on success flag
	int get_policy() const;
	/// return whether to stop on success
	bool stop_on_success() const;
	/// return whether to stop on failure
	bool stop_on_failure() const;
	/// set a new policy, always add stop on success flag if needed
	void set_policy(int _policy);
	/// return the focused child or -1 if none is focused
	int get_focused_child() const;
	/// set the focused child
	void set_focused_child(int _focused_child);
	/// return whether the current node is active
	bool get_active() const;
	/// set the active flag of the current node
	void set_active(bool _active);
};

/// try to grab the focus in the path of this node to the root of the tree
template <class X>
bool grab_focus(X* instance) {
	node* n = dynamic_cast<node*>(instance);
	if (!n)
		return true;
	base_ptr p = n->get_parent();
	if (p.empty())
		return true;
	group_ptr g = p->get_group();
	if (g.empty())
		return true;
	X* pinstance = p->get_interface<X>();
	if (!pinstance)
		return true;
	for (unsigned int i=0; i<g->get_nr_children(); ++i) {
		if (g->get_child(i)->get_interface<X>() == instance) {
			pinstance->set_focused_child(i);
			return grab_focus(pinstance);
		}
	}
	// at this point we did not find the instance in the children of its parent
	return false;
}

/// interface of a handler for traverse callbacks
class CGV_API traverse_callback_handler
{
public:
	/// called before a node b is processed, return, whether to skip this node. If the node is skipped, the on_leave_node callback is still called
	virtual bool on_enter_node(base_ptr b);
	/// called when a node b is left, return whether to terminate traversal
	virtual bool on_leave_node(base_ptr b);
	/// called before the children of a group node g are processed, return whether these should be skipped. If children are skipped, the on_leave_children callback is still called.
	virtual bool on_enter_children(group_ptr g);
	/// called when the children of a group node g have been left, return whether to terminate traversal
	virtual bool on_leave_children(group_ptr g);
	/// called before the parent of a node n is processed, return whether this should be skipped. If the parent is skipped, the on_leave_parent callback is still called.
	virtual bool on_enter_parent(node_ptr n);
	/// called when the parent of a node n has been left, return whether to terminate traversal
	virtual bool on_leave_parent(node_ptr n);
};

/// not yet implemented
enum TraverseStrategy
{
	TS_DEPTH_FIRST,
	TS_BREADTH_FIRST
};

/// class used to traverse a tree structure
class CGV_API traverser
{
protected:
	action& a;
	TraverseStrategy strategy;
	std::string visit_order;
	bool stop_if_not_implemented;
	/// traverse a single object, template over a static traverse callback handler interface
	template <typename TCH>
	bool traverse_tmp_1(base_ptr dest, base_ptr src, bool& force_termination, TCH& tch);
	/// helper method encapsulating common functionality
	template <typename TCH>
	bool traverse_tmp_2(base_ptr p, base_ptr dest, base_ptr src, traverse_policy* tp, bool& force_termination, TCH& tch);
public:
	/// construct from reference on action and traversal order string
	traverser(action& _a, 
			  const std::string& _visit_order = "pnc", 
			  TraverseStrategy _strategy = TS_DEPTH_FIRST, 
			  bool _stop_if_not_implemented = false);
	/// set a different visiting order of node, children and parent
	traverser& set_visit_order(const std::string& _visit_order);
	/// set a different traverse strategy
	traverser& set_strategy(TraverseStrategy _strategy);
	/// traverse a tree starting at given node according to set strategy, order and dest and previously coming from src and return whether traversal yielded success
	bool traverse(base_ptr start, traverse_callback_handler* tch = 0);
};

	}
}

#include <cgv/config/lib_end.h>