#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/utils/ostream_printf.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;

class simple_cube : 
	public base,    // base class of all to be registered classes
	public tacker,  // necessary to allow attachment of signals
	public event_handler, // necessary to receive events
	public drawable // registers for drawing with opengl
{
protected:
	/// rotation angle around y-axis in degrees
	double angle;
	/// rotation speed in degrees per second
	double speed;
	/// recursion depth
	unsigned int rec_depth;
public:
	/// initialize rotation angle
	simple_cube() : angle(0), speed(90), rec_depth(4)
	{
		connect(get_animation_trigger().shoot, this, &simple_cube::timer_event);
	}
	/// return the type name of the class derived from base
	std::string get_type_name() const 
	{
		return "simple_cube"; 
	}
	/// show statistic information
	void stream_stats(std::ostream& os)
	{
		cgv::utils::oprintf(os, "simple_cube: depth = %d, angle = %.1f, speed = %.1f\n", rec_depth, angle, speed);
	}
	/// show help information
	void stream_help(std::ostream& os)
	{
		os << "simple_cube:\a\n"
		   << "   change recursion depth: <up/down arrow>\n"
		   << "   change speed: <left/right arrow>\n"
		   << "   reset angle:  <space>\b\n";
	}
	/// overload to handle events, return true if event was processed
	bool handle(event& e)
	{
		if (e.get_kind() == EID_KEY) {
			key_event& ke = static_cast<key_event&>(e);
			if (ke.get_action() == KA_PRESS) {
				switch (ke.get_key()) {
				case KEY_Space:
					angle = 0;
					post_redraw();
					return true;   // event is processed
				case KEY_Right: speed *= 1.2; return true;
				case KEY_Left:  speed /= 1.2; return true;
				case KEY_Up: ++rec_depth; return true;
				case KEY_Down: rec_depth = rec_depth>1?rec_depth-1:0; return true;
				}
			}
		}
		return false; // event not handled
	}
	/// declare timer_event method to connect the shoot signal of the trigger
	void timer_event(double, double dt)
	{
		angle += speed*dt;
		post_redraw();
	}
	/// draw a cube tree of given depth in the current coordinate system
	void draw_cube_tree(context& ctx, unsigned int depth, int nr_children = 3)
	{
		ctx.tesselate_unit_cube();
		if (depth < rec_depth)
			// iterate children
			for (int i=0; i<nr_children; ++i) {
				// remember current coordinate system
				glPushMatrix();
					// rotate around z -axis by -90, 0, 90 or 180 degrees
					glRotated(i*90-90, 0, 0, 1);
					// move along x axis by 2 units
					glTranslated(2,0,0);
					// shrink child cube by a factor of 1/2
					glScaled(0.5,0.5,0.5);
					// recursively draw child cube
					draw_cube_tree(ctx, depth+1);
				// restore coordinate system before moving on to next child cube
				glPopMatrix();
			}
	}
	/// setting the view transform yourself
	void draw(context& ctx)
	{
		glRotated(angle,0,1,0);
		glScaled(0.5,0.5,0.5);
		glColor3f(0,1,0.2f);
		draw_cube_tree(ctx, 0, 4);
	}
};

// register a newly created cube with a dummy constructor 
// argument to avoid elimination of this code by the compiler
extern factory_registration<simple_cube> simp_cube_factory("new/simple cube", 'C', false);
