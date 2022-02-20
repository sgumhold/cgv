#include <cgv/gui/provider.h>
#include <cgv/gui/menu_provider.h>
#include <cgv/gui/control.h>
#include <cgv/gui/shortcut.h>
#include <cgv/utils/convert.h>
#include <cgv/signal/signal.h>
#include <cgv/signal/rebind.h>

using namespace cgv::gui;
using namespace cgv::utils;
using namespace cgv::base;
using namespace cgv::data;


class sub_gui_test : public base, public provider
{
protected:
	int idx;
	std::string text;
	shortcut sc;
	enum Choice { ONE = 1, THREE = 3, FOUR, FIVE } choice;
public:
	sub_gui_test(int i) : idx(i), choice(THREE), sc(KEY_F1)
	{
		text = to_string(i);
	}
	std::string get_type_name() const
	{
		return "sub_gui_test";
	}
	void value_change_cb(control<int>&)
	{
		std::cout << "value of idx of " << this << " changed to " << idx << std::endl;
	}
	void text_change_cb(control<std::string>&)
	{
		std::cout << "text of " << this << " changed to " << text.c_str() << std::endl;
	}
	void choice_change_cb(control<Choice>&)
	{
		std::cout << "choice of " << this << " changed to " << (int) choice << std::endl;

	}
	void button_cb()
	{
		std::cout << "button clicked" << std::endl;
	}
	void shortcut_chance_cb()
	{
		std::cout << "button short cut " << sc << std::endl;
		find_element(get_button_name())->set("shortcut", sc);
		find_element(get_button_name())->set("label", std::string("button (")+to_string(sc)+")");
	}
	std::string get_button_name() const {
		return std::string("button_")+to_string(this);
	}
	/// you must overload this for gui creation
	void create_gui()
	{
		connect_copy(add_button(get_button_name(),"color=0xff0000")->click, rebind(this, &sub_gui_test::button_cb));
		shortcut_chance_cb();
		connect(add_control("idx", idx, "value_slider", "min=0;max=10;color=0xffAA33")->value_change,this,&sub_gui_test::value_change_cb);
		connect(add_control("idx", idx, "slider", "min=0;max=10;step=1")->value_change,this,&sub_gui_test::value_change_cb);
		connect(add_control("text", text)->value_change,this,&sub_gui_test::text_change_cb);
		connect(add_control("choice", choice, "dropdown", "enums='one=1;three=3;four;five';shortcut='Shift-Ctrl-X'")->value_change, this, &sub_gui_test::choice_change_cb);
		connect_copy(add_control("shortcut", sc)->value_change, rebind(this, &sub_gui_test::shortcut_chance_cb));
	}
};

typedef ref_ptr<sub_gui_test> sub_gui_test_ptr;

class gui_test :
	public base,
	public provider,
	public menu_provider
{
protected:
	bool show_first, show_second;
	sub_gui_test_ptr s1, s2;
public:
	gui_test() : s1(new sub_gui_test(4)), s2(new sub_gui_test(8))
	{
		show_first = show_second = true;
	}
	~gui_test()
	{
		std::cout << "destruction of gui test" << std::endl;
	}
	/// overload to return the type name of this object
	std::string get_type_name() const
	{
		return "gui_test";
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const
	{
		return "view/gui test";
	}
	///
	void destruct()
	{
		unregister_object(base_ptr(this));
	}
	void menu_cb()
	{
		find_menu_element("view/gui_test_cb")->set("color", 0xFF000);
	}
	/// you must overload this for menu creation
	void create_menu()
	{
		connect_copy(add_menu_button("view/gui_test_cb", "shortcut='Ctrl-P'")->click,
			rebind(this, &gui_test::menu_cb));
	}
	/// you must overload this for to remove all elements from the menu again
	void destroy_menu()
	{
		remove_menu_element(find_menu_element("view/gui_test_cb"));
	}

	/// you must overload this for gui creation
	void create_gui()
	{
		connect_copy(add_control("show_first", show_first,"toggle")->value_change,
			rebind(static_cast<provider*>(this),&provider::recreate_gui));
		connect_copy(add_control("show_second", show_second,"toggle")->value_change,
			rebind(static_cast<provider*>(this),&provider::post_recreate_gui));
		if (add_tree_node("First Sub GUI", show_first, 1))
			inline_object_gui(s1);
		if (add_tree_node("Second Sub GUI", show_second, 1))
			inline_object_gui(s2);
		connect_copy(add_button("destroy")->click, rebind(this, &gui_test::destruct));
	}
};

#include <cgv/base/register.h>

factory_registration<gui_test> gui_test_fac("gui_test", "menu_text='New/GUI/GUI Test'", true);
