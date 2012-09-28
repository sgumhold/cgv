#include <cgv/base/group.h>
#include <cgv/base/register.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <iostream>

using namespace cgv::base;
using namespace cgv::gui;

struct eh_group :
	public group,
	public event_handler
{
	int my_kind;
	eh_group(const char* name, int _kind) : group(name), my_kind(_kind) 
	{
		set_policy(TP_AUTO_FOCUS+TP_STOP_ON_SUCCESS);
	}
	bool handle(event& e)
	{
		std::cout << "eh_group(" << get_name() << ")::handle(";
		e.stream_out(std::cout);
		std::cout << ")";
		if (e.get_kind() == my_kind)
			std::cout << " ==> true";
		std::cout << std::endl;
		return e.get_kind() == my_kind;
	}
	void stream_help(std::ostream& os) 
	{
		os << "eh_group(" << get_name() << ")::show_help()" << std::endl;
	}
};

typedef cgv::data::ref_ptr<eh_group> eh_group_ptr;

void show_tree(node_ptr n, int tab = 0)
{
	std::cout << std::string(tab,' ').c_str() << n->get_name() << " of type " << n->get_type_name() << std::endl;
	group_ptr g = n->cast<group>();
	if (g) {
		for (unsigned int i=0; i<g->get_nr_children(); ++i) {
			node_ptr c = g->get_child(i)->cast<node>();
			if (c)
				show_tree(c, tab+3);
		}
	}
}

bool test_tree()
{

	std::cout << "\ntest_tree()\n==============\n" << std::endl;
	group_ptr g(new group("g1"));
	node_ptr n1(new node("n1")), n2(new node("n2"));
	g->append_child(n1);
	g->append_child(n2);
	show_tree(g);
	g->remove_child(n2);
	show_tree(g);

	eh_group_ptr eh_g1(new eh_group("eh_g1:NONE", EID_NONE));
	eh_group_ptr eh_g2(new eh_group("eh_g2:MOUSE", EID_MOUSE));
	eh_group_ptr eh_g3(new eh_group("eh_g3:KEY", EID_KEY));

	g->append_child(eh_g1);

	eh_g1->append_child(eh_g2);
	eh_g1->append_child(eh_g3);

	key_event key;
	event mouse(EID_MOUSE);
	std::cout << "\nsending key event:" << std::endl;
	traverser(make_action<event&>(key, &event_handler::handle)).traverse(g);
	traverser(make_action<std::ostream&>(std::cout, &event_handler::stream_help)).traverse(g);
	std::cout << "\nsending mouse event:" << std::endl;
	traverser(make_action<event&>(mouse, &event_handler::handle)).traverse(g);
	std::cout << "\nsending mouse event:" << std::endl;
	traverser(make_action<event&>(mouse, &event_handler::handle)).traverse(g);
	std::cout << "\nsending key event:" << std::endl;
	traverser(make_action<event&>(key, &event_handler::handle)).traverse(g);

	return true;
}

#include <test/lib_begin.h>

extern CGV_API test_registration test_cb_tree_reg("cgv::base::test_tree", test_tree);