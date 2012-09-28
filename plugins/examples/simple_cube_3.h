#include <cgv/base/register.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl.h>

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
	/// drawable a colored cube yourself
	void draw(context& ctx)
	{
		glColor3f(0,1,0.2f);
		glBegin(GL_QUADS);
			glNormal3d(-1,0,0);
			glVertex3d(-1, -1, -1);
			glVertex3d(-1, -1, 1);
			glVertex3d(-1, 1, 1);
			glVertex3d(-1, 1, -1);
			glNormal3d(0,0,1);
			glVertex3d(-1, -1, 1);
			glVertex3d(1, -1, 1);
			glVertex3d(1, 1, 1);
			glVertex3d(-1, 1, 1);
			glNormal3d(1,0,0);
			glVertex3d(1, -1, 1);
			glVertex3d(1, -1, -1);
			glVertex3d(1, 1, -1);
			glVertex3d(1, 1, 1);
			glNormal3d(0,0,-1);
			glVertex3d(1, -1, -1);
			glVertex3d(-1, -1, -1);
			glVertex3d(-1, 1, -1);
			glVertex3d(1, 1, -1);
			glNormal3d(0,-1,0);
			glVertex3d(1, -1, -1);
			glVertex3d(1, -1, 1);
			glVertex3d(-1, -1, 1);
			glVertex3d(-1, -1, -1);
			glNormal3d(0,1,0);
			glVertex3d(-1, 1, -1);
			glVertex3d(-1, 1, 1);
			glVertex3d(1, 1, 1);
			glVertex3d(1, 1, -1);
		glEnd();
	}
};

// register a newly created cube with a dummy constructor 
// argument to avoid elimination of this code by the compiler
extern object_registration<simple_cube> simp_cube_instance("dummy");
