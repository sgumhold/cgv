#include <cgv/base/named.h>
#include <cgv/gui/application.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/media/color.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/file.h>
#include <cgv/base/register.h>

using namespace cgv::base;
using namespace cgv::render;
using namespace cgv::gui;
using namespace cgv::media;
using namespace cgv::utils;
using namespace cgv::utils::file;
using namespace std;

std::string get_base_name(base_ptr b)
{
	if (b->cast<named>() && !b->cast<named>()->get_name().empty())
		return b->cast<named>()->get_name();
	else
		return b->get_type_name();
}

class object_browser : public base, public provider
{
private:
	std::vector<base_ptr> object_stack;
public:
	object_browser()                    { }
	std::string get_type_name() const   { return "object_browser"; }
	std::string get_parent_type() const { return "tree_group"; }

	void button_callback(button& B)
	{
		std::cout << "clicked button " << B.get_name() << std::endl;
	}

	void select_cb(base_ptr b, bool select)
	{
		if (select)
			std::cout << "select ";
		else
			std::cout << "unselect ";
		std::cout << get_base_name(b) << std::endl;
	}
	void open_cb(gui_group_ptr g, bool opened)
	{
		if (opened)
			std::cout << "opened ";
		else
			std::cout << "closed ";
		std::cout << get_base_name(g) << std::endl;
	}

	void create_object_gui(base_ptr o, gui_group_ptr ggp)
	{
		gui_group_ptr new_ggp = ggp->add_group(get_base_name(o)+"\t"+
			                                   o->get_type_name()+"\t"+
											   to_string((void*)&(*o)),"", "", "");
		// check for cycles
		bool cyclic = (o == this) || (o == parent_group);
		for (unsigned i=0; !cyclic && i<object_stack.size(); ++i) {
/*
		if (i > 0)
				std::cout << "::";
			std::cout << object_stack[i]->get_type_name();
*/
			if (object_stack[i] == o)
				cyclic = true;
		}
//		std::cout << get_base_name(o) << std::endl;

		new_ggp->set("color",cyclic?0xbbbbbb:0xffffbb);
		if (!cyclic) {
			object_stack.push_back(o);
			create_object_member_gui(o, new_ggp);
			object_stack.pop_back();
		}
		else
			std::cout << "detected cycle" << std::endl;
	}

	void create_object_member_gui(base_ptr o, gui_group_ptr ggp)
	{
		if (o->get_group()) {
			group_ptr g = o->get_group();
			for (unsigned i=0; i<g->get_nr_children(); ++i)
				create_object_gui(g->get_child(i), ggp);
		}

		std::string prop_decls(o->get_property_declarations());
		std::vector<token> toks;
		bite_all(tokenizer(prop_decls).set_ws(";"),toks);
		// for each assignment
		for (unsigned int i=0; i<toks.size(); ++i) {
			std::vector<token> ts;
			bite_all(tokenizer(toks[i]).set_ws(":"),ts);
			if (ts.size() != 2)
				continue;
			button_ptr B = ggp->add_button(to_string(ts[0])+"\t"+to_string(ts[1])+"\t"+o->get<std::string>(to_string(ts[0])),"","");
			B->set("color", 0xffbbbb);
			connect(B->click, this, &object_browser::button_callback);
		}
	}
	void create_gui()
	{
		parent_group->multi_set("column_heading_0='name';column_width_0=150;column_heading_1='type';column_width_1=150;column_heading_2='value';column_width_2=150");
		connect(parent_group->on_selection_change,this,&object_browser::select_cb);
		connect(parent_group->on_open_state_change,this,&object_browser::open_cb);
		unsigned n = application::get_nr_windows();
		for (unsigned i=0; i<n; ++i)
			create_object_gui(application::get_window(i), parent_group);
	}
};

#if defined(CGV_GUI_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#ifdef CGV_GUI_META_EXPORTS
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>

extern CGV_API cgv::base::factory_registration<object_browser> obt_1("object_browser", "menu_text=\"new/object browser\";shortcut='Ctrl-Alt-O");

