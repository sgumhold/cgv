#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/render/render_types.h>

class gui_controls : public cgv::base::node, public cgv::render::render_types, public cgv::gui::provider {
protected:
	struct {
		float slider_value = 0.0f;
	} tree_node;

	bool toggle = false;
	bool check = false;
	int value = 0;
	
	float dial_value = 0.0f;
	float wheel_value = 0.0f;

	enum DemoEnum {
		DE_A,
		DE_B,
		DE_C,
	};
	DemoEnum demo_enum = DE_A;

	std::string file_name;
	std::string save_file_name;

	rgb color;

	bool inactive = false;

public:
	gui_controls() : cgv::base::node("GUI Controls") {}
	void destruct() {
		unregister_object(base_ptr(this));
	}
	/// overload to return the type name of this object
	std::string get_type_name() const {
		return "gui_controlsss";
	}
	void on_set(void* member_ptr) {
		if(member_ptr == &inactive)
			post_recreate_gui();

		//post_redraw();
		update_member(member_ptr);
	}
	std::string active() {
		std::string s = ";active=";
		return s + (inactive ? "false" : "true" );
	}
	void create_gui() {
		add_decorator("Heading 1", "heading", "level=0" + active());
		add_decorator("Heading 2", "heading", "level=1" + active());
		add_decorator("Heading 3", "heading", "level=2" + active());
		add_decorator("Heading 4", "heading", "level=3" + active());
		add_decorator("Separator", "heading", "level=3" + active());
		add_decorator("", "separator");

		add_member_control(this, "Toggle Button", toggle, "toggle", active());
		add_member_control(this, "Checkmark", check, "check", active());
		add_member_control(this, "Value", value, "value", "min=-10;max=10;step=1" + active());
		add_member_control(this, "Dial", dial_value, "dial", "min=-1.0;max=1.0;step=0.01;w=40;h=40" + active());
		add_member_control(this, "Wheel", wheel_value, "wheel", "min=-1.0;max=1.0;step=0.01" + active());
		add_member_control(this, "Dropdown", demo_enum, "dropdown", "enums='Option 1,Option 2, Option 3'" + active());
		add_member_control(this, "Color", color);

		std::string filter = "Text Files (txt):*.txt|All Files:*.*";
		add_gui("File", file_name, "file_name", "title='Open Text File';" + filter + ";save=false;w=136;small_icon=true;align_gui=' '");
		add_gui("save_file_name", save_file_name, "file_name", "title='Save Text ';filter='" + filter + "';save=true;control=false;small_icon=true");

		if(begin_tree_node("Tree Node", tree_node, true, active())) {
			align("\a");
			add_button("Button", active());
			add_member_control(this, "Slider", tree_node.slider_value, "value_slider", "min=0.0;max=1.0;step=0.01;" + active());
			add_member_control(this, "Slider Ticks", tree_node.slider_value, "value_slider", "min=0.0;max=1.0;step=0.01;ticks=true" + active());
			align("\b");
			end_tree_node(tree_node);
		}

		add_member_control(this, "Set Inactive", inactive, "toggle");
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
//cgv::base::factory_registration<gui_controls> gui_controls_fac("New/GUI/Controls");
cgv::base::object_registration<gui_controls> gui_controls_reg("GUI Controls");
