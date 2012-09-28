#include <cgv/base/register.h>
#include <cgv/render/drawable.h>

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
	/// drawable a cube by using the tesselation method of the context
	void draw(context& ctx)
	{
		ctx.tesselate_unit_cube();
	}
};

// register a newly created cube with a dummy constructor 
// argument to avoid elimination of this code by the compiler
extern object_registration<simple_cube> simp_cube_instance("dummy");
