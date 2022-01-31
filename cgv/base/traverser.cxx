#include "traverser.h"
#include <iostream>

namespace cgv {
	namespace base {

traverse_policy::traverse_policy(int _policy, bool _active, int _focus)
	: policy((TraversePolicy)_policy), active(_active), focus(_focus)
{
}
int traverse_policy::get_policy() const
{
	return policy & ~(TP_STOP_ON_SUCCESS|TP_STOP_ON_FAILURE);
}
/// return whether to stop on success
bool traverse_policy::stop_on_success() const
{
	return (policy & TP_STOP_ON_SUCCESS) != 0;
}

/// return whether to stop on failure
bool traverse_policy::stop_on_failure() const
{
	return (policy & TP_STOP_ON_FAILURE) != 0;
}

void traverse_policy::set_policy(int _policy)
{
	policy = (TraversePolicy) _policy;
}
int traverse_policy::get_focused_child() const
{
	return focus;
}
void traverse_policy::set_focused_child(int _focus)
{
	focus = _focus;
}
bool traverse_policy::get_active() const
{
	return active;
}
void traverse_policy::set_active(bool _active)
{
	active = _active;
}


/// called before a node b is processed, return, whether to skip this node. If the node is skipped, the on_leave_node callback is still called
bool traverse_callback_handler::on_enter_node(base_ptr)
{
	return false;
}
/// called when a node b is left, return whether to terminate traversal
bool traverse_callback_handler::on_leave_node(base_ptr)
{
	return false;
}
/// called before the children of a group node g are processed, return whether these should be skipped. If children are skipped, the on_leave_children callback is still called.
bool traverse_callback_handler::on_enter_children(group_ptr)
{
	return false;
}
/// called when the children of a group node g have been left, return whether to terminate traversal
bool traverse_callback_handler::on_leave_children(group_ptr)
{
	return false;
}
/// called before the parent of a node n is processed, return whether this should be skipped. If the parent is skipped, the on_leave_parent callback is still called.
bool traverse_callback_handler::on_enter_parent(node_ptr)
{
	return false;
}
/// called when the parent of a node n has been left, return whether to terminate traversal
bool traverse_callback_handler::on_leave_parent(node_ptr)
{
	return false;
}

/*

private:
	struct queue_entry {
		base_ptr dest;
		base_ptr src;
		int visit_order_loc;
		queue_entry(base_ptr _dest,base_ptr _src = base_ptr(), int _visit_order_loc = 0);
	};
	std::deque<queue_entry> queue;
	bool success;
	bool process_entry(queue_entry& e);



traverser::queue_entry::queue_entry(base_ptr _dest,base_ptr _src, int _visit_order_loc)
{
	dest = _dest;
	src  = _src;
	visit_order_loc = _visit_order_loc;
}

bool traverser::traverse(base_ptr start)
{
	queue.clear();
	queue.push_back(queue_entry(start));
	success = false;
	while (!queue.empty()) {
		std::pair<base_ptr,base_ptr> p;
		if (strategy == TS_DEPTH_FIRST) {
			if (!process_entry(queue.back()))
				queue.pop_back();
		}
		else {
			if (!process_entry(queue.front()))
				queue.pop_front();
		}
	}
	return success;
}

bool traverser::process_entry(queue_entry& e)
{
	// check if we finished processing
	if (e.visit_order_loc >= visit_order.size())
		return true;





	switch (visit_order[e.visit_order_loc]) {
	case 'p' :
	{
		node* n  = dynamic_cast<node*>(e.dest.ref());
		if (!n)
			break;
		base_ptr p = n->get_parent();
		if (p.ref() != e.src.ref())
			add_entry(queue_entry(p, e.dest));
		break;
	}
	case 'c' :
	{
		group* g = dynamic_cast<group*>(e.dest.ref());
		if (!g)
			break;
		a.select(e.dest);
		traverse_policy* tp = a.get_policy();
		int focus = (tp != 0) ? tp->get_focus() : -1;
		if (focus != -1 && focus < (int)g->get_nr_children() && tp->get_policy() != TP_ALL) {
			base_ptr c = g->get_child(focus);
			if (c.ref() != src.ref())
				add_entry(queue_entry(c, e.dest));
		}
		for (int i=0; i < (int)g->get_nr_children(); ++i) if (i != focus) {
			base_ptr c = g->get_child(i);
			if (c.ref() != src.ref())
				add_entry(queue_entry(c, e.dest));

			if (traverse<T1>(c, mp, v, dest)) {
					success = true;
					if (tp && tp->stop_on_success()) {
						if (tp->get_policy() == TP_AUTO_FOCUS)
							tp->set_focus(i);
						return true;
					}
				}
			}
		}
	}
	break;
	case 'n' :
		{
			if (x && (x->*mp)(v)) {
				success = true;
				if (tp && tp->stop_on_success()) {
					if (tp->get_policy() == TP_AUTO_FOCUS)
						tp->set_focus(i);
					return true;
				}
			}
		}
		break;
	default:
		std::cerr << "unknown character " << traversal_order[i] << " in traversal order" << std::endl;
	}


	// s
	a.select(dest);
	traverse_policy* tp = a.get_policy();
	if (tp && !tp->get_active())
		return false;

	bool success = false;
	for (int i=0; i<(int)traversal_order.size(); ++i) {
	}
	return success;
}
*/

/// construct from reference on action and traversal order string
traverser::traverser(action& _a, const std::string& _visit_order, TraverseStrategy _strategy, bool _stop_if_not_implemented, bool _ignore_inactive)
	: a(_a), strategy(_strategy), visit_order(_visit_order), stop_if_not_implemented(_stop_if_not_implemented), ignore_inactive(_ignore_inactive)
{
}

struct use_handler
{
	traverse_callback_handler* tch;
	use_handler(traverse_callback_handler* _tch) : tch(_tch) {}
	inline bool on_enter_node(base_ptr b) { return tch->on_enter_node(b); }
	inline bool on_leave_node(base_ptr b) { return tch->on_leave_node(b); }
	inline bool on_enter_children(group_ptr g) { return tch->on_enter_children(g); }
	inline bool on_leave_children(group_ptr g) { return tch->on_leave_children(g); }
	inline bool on_enter_parent(node_ptr n) { return tch->on_enter_parent(n); }
	inline bool on_leave_parent(node_ptr n) { return tch->on_leave_parent(n); }
};

struct no_handler
{
	no_handler() {}
	inline bool on_enter_node(base_ptr)      { return false; }
	inline bool on_leave_node(base_ptr)      { return false; }
	inline bool on_enter_children(group_ptr) { return false; }
	inline bool on_leave_children(group_ptr) { return false; }
	inline bool on_enter_parent(node_ptr)    { return false; }
	inline bool on_leave_parent(node_ptr)    { return false; }
};


template <typename TCH>
bool traverser::traverse_tmp_1(base_ptr dest, base_ptr src, bool& force_termination, TCH& tch)
{
	if (tch.on_enter_node(dest)) {
		force_termination = true;
		return false;
	}
	a.select(dest);
	if (stop_if_not_implemented && !a.implements_action()) {
		force_termination = true;
		return false;
	}
	traverse_policy* tp = a.get_policy();
	if (!ignore_inactive && tp && !tp->get_active())
		return false;

	bool success = false;
	bool node_success = false;
	bool node_entered = false;
	for (int i=0; !force_termination && i<(int)visit_order.size(); ++i) {
		switch (visit_order[i]) {
		case 'p' :
		{
			node_ptr n  = dest->cast<node>();
			if (n.empty() || n->get_parent().empty())
				break;
			if (!tch.on_enter_parent(n))
				success |= traverse_tmp_2(n->get_parent(), dest, src, tp, force_termination, tch);
			if (tch.on_leave_parent(n))
				force_termination = true;
			break;
		}
		case 'c' :
		{
			group_ptr g = dest->cast<group>();
			if (g.empty())
				break;
			if (!tch.on_enter_children(g)) {
				int focus = (tp != 0) ? tp->get_focused_child() : -1;
				if (focus != -1 && focus < (int)g->get_nr_children() && tp->get_policy() != TP_ALL) {
					success |= traverse_tmp_2(g->get_child(focus), dest, src, tp, force_termination, tch);
				}

				if (!force_termination) {
					for (int i=0; i < (int)g->get_nr_children(); ++i) 
						if (i != focus) {
							bool child_success = traverse_tmp_2(g->get_child(i), dest, src, tp, force_termination, tch);
							success |= child_success;
							if (child_success && tp && tp->get_focused_child() == TP_AUTO_FOCUS)
								tp->set_focused_child(i);
							if (force_termination)
								break;
						}
				}
			}
			if (tch.on_leave_children(g))
				force_termination = true;
			break;
		}
		case 'n' :
		{
			a.select(dest);
			if (a.implements_action()) {
				node_success = a.begin();
				if (!a.has_begin_only()) {
					node_entered = true;
					break;
				}
				if (node_success) {
					success = true;
					if (tp && tp->stop_on_success())
						force_termination = true;
				}
				else {
					if (tp && tp->stop_on_failure())
						force_termination = true;
				}
			}
			break;
		}
		case 'N' :
		{
			if (node_entered) {
				a.select(dest);
				if (a.end() && node_success) {
					success = true;
					if (tp && tp->stop_on_success())
						force_termination = true;
				}
				else {
					if (tp && tp->stop_on_failure())
						force_termination = true;
				}
				node_entered = false;
			}
			else {
				std::cerr << "attempt to call end method before begin in tree traversal ignored!" << std::endl;
			}
		}
		default:
			std::cerr << "unknown character " << visit_order[i] << " in traversal order" << std::endl;
		}
	}

	if (node_entered) {
		a.select(dest);
		if (a.end() && node_success) {
			success = true;
			if (tp && tp->stop_on_success())
				force_termination = true;
		}
		else {
			if (tp && tp->stop_on_failure())
				force_termination = true;
		}
	}
	if (tch.on_leave_node(dest))
		force_termination = true;

	return success;
}

template <typename TCH>
bool traverser::traverse_tmp_2(base_ptr p, base_ptr dest, base_ptr src, traverse_policy* tp, bool& force_termination, TCH& tch)
{
	if (p == src)
		return false;
	if (traverse_tmp_1(p, dest, force_termination, tch)) {
		if (tp && tp->stop_on_success())
			force_termination = true;
		return true;
	}
	else {
		if (tp && tp->stop_on_failure())
			force_termination = true;
		return false;
	}
}


/// traverse a tree starting at given node according to set strategy, order and dest and previously coming from src and return whether traversal yielded success
bool traverser::traverse(base_ptr start, traverse_callback_handler* tch)
{
	bool force_termination = false;
	if (tch) {
		use_handler uh(tch);
		return traverse_tmp_1(start, base_ptr(), force_termination, uh);
	}
	else {
		no_handler nh;
		return traverse_tmp_1(start, base_ptr(), force_termination, nh);
	}
}

	}
}
