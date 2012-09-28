#include <cgv/base/register.h>
#include <cgv/render/drawable.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <gl/gl.h>
#include <gl/glu.h>

using namespace cgv::render;
using namespace cgv::base;

class simple_cube : 
	public base,    // base class of all to be registered classes
	public drawable // registers for drawing with opengl
{
public:
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
		glColor3f(0,1,0.2f);
		ctx.tesselate_unit_cube();
	}
};

// register a newly created cube with a dummy constructor 
// argument to avoid elimination of this code by the compiler
extern object_registration<simple_cube> simp_cube_instance("dummy");
