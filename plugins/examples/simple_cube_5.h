#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;

class simple_cube : 
	public base,    // base class of all to be registered classes
	public tacker,  // necessary to allow attachment of signals
	public drawable // registers for drawing with opengl
{
protected:
	/// rotation angle around y-axis in degrees
	double angle;
	/// rotation speed in degrees per second
	double speed;
public:
	/// initialize rotation angle
	simple_cube() : angle(0), speed(90)
	{
		connect(get_animation_trigger().shoot, this, &simple_cube::timer_event);
	}
	/// declare timer_event method to connect the shoot signal of the trigger
	void timer_event(double t, double dt)
	{
		angle += speed*dt;
		post_redraw();
	}
	/// return the type name of the class derived from base
	std::string get_type_name() const 
	{
		return "simple_cube"; 
	}
	/// set the viewing transformation
	void init_frame(context& ctx)
	{
		/// transformation from eye to image space
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		double aspect = (double)ctx.get_width()/ctx.get_height();
		gluPerspective(45,aspect,0.01,100.0);
		/// transformation from world to eye space
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(3,3,6, 0,0,0, 0,1,0);
	}
	/// setting the view transform yourself
	void draw(context& ctx)
	{
		glRotated(angle,0,1,0);
		glColor3f(0,1,0.2f);
		ctx.tesselate_unit_cube();
	}
};

// register a newly created cube with a dummy constructor 
// argument to avoid elimination of this code by the compiler
extern object_registration<simple_cube> simp_cube_instance("dummy");
