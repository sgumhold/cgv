#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/theme_info.h>

class gui_controls : public cgv::base::node, public cgv::gui::provider {
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

	std::string dir_name;
	std::string save_dir_name;

	cgv::rgb rgb_color = cgv::rgb(0.0f);
	cgv::rgba rgba_color = cgv::rgba(0.0f);

	std::string str;

	bool inactive = false;
	bool colored = false;

public:
	gui_controls() : cgv::base::node("GUI Controls") {
		auto& ti = cgv::gui::theme_info::instance();
	}
	void destruct() {
		unregister_object(base_ptr(this));
	}
	/// overload to return the type name of this object
	std::string get_type_name() const {
		return "gui_controls";
	}
	void on_set(void* member_ptr) {
		if(member_ptr == &inactive || member_ptr == &colored)
			post_recreate_gui();

		//post_redraw();
		update_member(member_ptr);
	}
	std::string is_active() {
		std::string s = ";active=";
		return s + (inactive ? "false" : "true" );
	}
	std::string is_colored() {
		auto& ti = cgv::gui::theme_info::instance();
		std::string hex = ti.selection_hex();
		return colored ? ";color=" + hex : "";
		
		//return colored ? ";color=0xff0000" : "";
	}
	void create_gui() {

		const std::string opt = is_active() + is_colored();

		add_decorator("Heading 1", "heading", "level=0" + opt);
		add_decorator("Heading 2", "heading", "level=1" + opt);
		add_decorator("Heading 3", "heading", "level=2" + opt);
		add_decorator("Heading 4", "heading", "level=3" + opt);
		add_decorator("Separator", "heading", "level=3" + opt);
		add_decorator("", "separator");
		
		add_member_control(this, "Toggle Button", toggle, "toggle", "tooltip='A toggle button'" + opt);
		add_member_control(this, "Checkmark", check, "check", opt);
		add_member_control(this, "Value", value, "value", "min=-10;max=10;step=1" + opt);
		add_member_control(this, "Dial", dial_value, "dial", "min=-1.0;max=1.0;step=0.01;w=40;h=40" + opt);
		add_view("Value View", wheel_value, "", opt);
		add_member_control(this, "Wheel", wheel_value, "wheel", "min=-1.0;max=1.0;step=0.01" + opt);
		add_member_control(this, "Dropdown", demo_enum, "dropdown", "enums='Option 1,Option 2, Option 3'" + opt);
		add_member_control(this, "RGB Color", rgb_color, "", opt);
		add_member_control(this, "RGBA Color", rgba_color, "", opt);
		add_member_control(this, "String", str, "", opt);
		add_view("String View", str, "", opt);

		std::string filter = "filter='Text Files (txt):*.txt|All Files:*.*'";
		add_gui("File", file_name, "file_name", "title='Open Text File';" + filter + ";save=false;w=136;small_icon=true;align_gui=' '" + opt);
		add_gui("save_file_name", save_file_name, "file_name", "title='Save Text ';filter='" + filter + "';save=true;control=false;small_icon=true" + opt);
		add_gui("Directory", dir_name, "directory", "title='Open Text File';" + filter + ";save=false;w=136;small_icon=true;align_gui=' '" + opt);
		add_gui("save Directory", save_dir_name, "directory", "title='Save Text ';filter='" + filter + "';save=true;control=false;small_icon=true" + opt);

		if(begin_tree_node("Tree Node", tree_node, true, opt)) {
			align("\a");
			add_button("Button", opt);
			add_member_control(this, "Value Slider", tree_node.slider_value, "value_slider", "min=0.0;max=1.0;step=0.01;" + opt);
			add_member_control(this, "Value Slider (Ticks)", tree_node.slider_value, "value_slider", "min=0.0;max=1.0;step=0.01;ticks=true" + opt);
			add_member_control(this, "Fill Slider", tree_node.slider_value, "fill_slider", "min=0.0;max=1.0;step=0.01;ticks=true" + opt);
			add_member_control(this, "Vertical Slider", tree_node.slider_value, "vslider", "h=100;min=0.0;max=1.0;step=0.01;ticks=true" + opt);
			add_member_control(this, "Vertical Value Slider", tree_node.slider_value, "vvalue_slider", "h=100;min=0.0;max=1.0;step=0.01;ticks=true" + opt);
			add_member_control(this, "Vertical Fill Slider", tree_node.slider_value, "vfill_slider", "h=100;min=0.0;max=1.0;step=0.01;ticks=true" + opt);
			align("\b");
			end_tree_node(tree_node);
		}
		
		add_member_control(this, "Set Inactive", inactive, "toggle");
		add_member_control(this, "Set Color", colored, "toggle");

		add_decorator("Just", "heading", "level=3");
		add_decorator("some", "heading", "level=3");
		add_decorator("more", "heading", "level=3");
		add_decorator("decorators", "heading", "level=3");
		add_decorator("to", "heading", "level=3");
		add_decorator("fill", "heading", "level=3");
		add_decorator("in", "heading", "level=3");
		add_decorator("some", "heading", "level=3");
		add_decorator("space", "heading", "level=3");
		add_decorator("and", "heading", "level=3");
		add_decorator("force", "heading", "level=3");
		add_decorator("a", "heading", "level=3");
		add_decorator("scrollbar", "heading", "level=3");
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<gui_controls> gui_controls_fac("New/GUI/Controls");
