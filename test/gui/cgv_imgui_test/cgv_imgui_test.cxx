#include <libs/cgv_imgui/cgv_imgui.h>
#include <cgv/base/node.h>
#include <libs/cgv_gl/gl/gl.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/window.h>

// declare a new plugin with support for drawing, event handling and a gui
struct cgv_imgui_test : 
	public cgv::base::node, 
	public cgv::imgui::cgv_imgui, 
	public cgv::gui::provider
{
	float angle;
	cgv_imgui_test() : node("cgv imgui test"), cgv_imgui(false)
	{
		angle = 0;
		connect(cgv::gui::get_animation_trigger().shoot, this, &cgv_imgui_test::timer_event);
	}
	~cgv_imgui_test()
	{
	}
	/// return type name
	std::string get_type_name() const { return "cgv_imgui_test"; }
	/// in case of animated cursor, continuously post redraw
	void timer_event(double, double)
	{
		angle += 0.1f;
		if (angle > 6.28f)
			angle -= 6.28f;
		post_redraw();
	}
	void stream_help(std::ostream &os)
	{
		os << "events handled by imgui" << std::endl;
	}
	void create_gui()
	{
		add_decorator("cgv_imgui_test", "heading");
		add_member_control(this, "use_offline_rendering", use_offline_rendering);
		add_member_control(this, "clear_color", clear_color);
		add_member_control(this, "demo_window", show_demo_window, "check");
		add_member_control(this, "other_window", show_another_window, "check");
	}
};


// register plugin and export symbol to ensure that library of plugin is not discarded
#include <cgv/base/register.h>

extern cgv::base::object_registration<cgv_imgui_test> cit_reg("");