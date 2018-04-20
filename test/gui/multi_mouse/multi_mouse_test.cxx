#include <sample_application.h>
#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <libs/cgv_gl/gl/gl.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/window.h>

using namespace std;
using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;

// declare a new plugin with support for drawing, event handling and a gui
struct multi_mouse_test : 
	public node, 
	public drawable, 
	public event_handler, 
	public multi_mouse_handler,
	public cgv::signal::tacker,
	public sample_application
{
	// default constructor 
	multi_mouse_test() : node("Multi Mouse Demo") 
	{
		connect(get_animation_trigger().shoot, this, &multi_mouse_test::timer_event);
	}
	// unregister handler in destructor
	~multi_mouse_test() 
	{
		unregister_multi_mouse_handler(this);
	}
	/// return type name
	string get_type_name() const { return "multi_mouse_test"; }
	/// configure viewer after registration
	void on_register()
	{
		window_ptr wp = get_root()->get_interface<window>();
		wp->multi_set("x=20;y=20;menu=false;gui=false;show_stats=false;state='fullscreen';cursor='invisible'");
		init_mice(wp->get<int>("w"),wp->get<int>("h"));
		register_multi_mouse_handler(this);
	}
	/// in case of animated cursor, continuously post redraw
	void timer_event(double, double)
	{
		if (c.is_created() && c.get_nr_steps() > 0)
			post_redraw();
	}
	/// called when a new mouse device is attached (attach=true) or an existing is detached (attach=false)
	void on_mouse_device_change(bool attach, void* id)
	{
		process_mouse_change_event(attach,id,get_context()->get_width(),get_context()->get_height());
		post_redraw();
	}
	void stream_help(ostream &os)
	{
		os << "move several mice" << endl;
	}
	bool handle(event& e)
	{
		// check for mouse events
		if (e.get_kind() != EID_MOUSE)
			return false;
		// check for multi mouse events
		if ( (e.get_flags() & EF_MULTI) == 0 )
			return false;
		// cast to multi mouse event
		const multi_mouse_event& mme = static_cast<const multi_mouse_event&>(e);
		// call method of sample application
		handle_mouse_event(mme);
		// ensure that everything is redrawn
		post_redraw();
		return true;
	}
	// draw the drawings and the mouse pointers
	void draw(context& ctx)
	{		
		// disable lighting which is enabled by default in cgv framework
		glDisable(GL_LIGHTING);
		// define projection and modelview to generate mouse coordinates
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,ctx.get_width(),ctx.get_height(),0,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		draw_scene(get_animation_trigger().get_current_time());
	}
};

// window specific declaration of CGV_API for external declaration of registration object
#if defined(CGV_GUI_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#ifdef MULTI_MOUSE_TEST_EXPORTS
#	define CGV_EXPORTS
#endif
#include <cgv/config/lib_begin.h>

// register plugin and export symbol to ensure that library of plugin is not discarded
#include <cgv/base/register.h>

extern CGV_API object_registration<multi_mouse_test> mmt_reg("");