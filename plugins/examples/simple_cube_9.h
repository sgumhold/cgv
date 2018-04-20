#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/gui/provider.h>
#include <cgv/media/illum/phong_material.hh>

using namespace cgv::base;
using namespace cgv::reflect;
using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::render;

class simple_cube : 
	public base,    // base class of all to be registered classes
	public provider, // is derived from tacker, which is not necessary as base anymore
	public event_handler, // necessary to receive events
	public drawable // registers for drawing with opengl
{
private:
	/// flag used to represent the state of the extensible gui node
	bool toggle;
protected:
	/// whether animation is turned on
	bool animate;
	/// rotation angle around y-axis in degrees
	double angle;
	/// rotation speed in degrees per second
	double speed;
	/// recursion depth
	unsigned int rec_depth;
	/// resolution of smooth shapes
	int resolution;
	///
	cgv::media::illum::phong_material material;
	/// different shape types
	enum Shape { CUBE, PRI, TET, OCT, DOD, ICO, CYL, CONE, DISK, ARROW, SPHERE } shp;
public:
	/// initialize rotation angle
	simple_cube() : toggle(false), angle(0), speed(90), rec_depth(4), animate(true), resolution(25), shp(CUBE)
	{
		material.set_diffuse(cgv::media::illum::phong_material::color_type(0.7f, 0.2f, 0.4f));
		connect(get_animation_trigger().shoot, this, &simple_cube::timer_event);
	}
	/// self reflection allows to change values in the config file
	bool self_reflect(reflection_handler& rh)
	{
		return
			rh.reflect_member("animate", animate) &&
			rh.reflect_member("angle", angle) &&
			rh.reflect_member("speed", speed) &&
			rh.reflect_member("rec_depth", rec_depth);
	}
	/// return the type name of the class derived from base
	std::string get_type_name() const 
	{
		return "simple_cube"; 
	}
	bool init(context& ctx)
	{
		return true;
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
					 update_member(&angle);
					return true;   // event is processed
				case 'A' : animate = !animate; update_member(&animate); post_redraw(); return true;
				case KEY_Right: speed *= 1.2; update_member(&speed); return true;
				case KEY_Left:  speed /= 1.2; update_member(&speed); return true;
				case KEY_Up: ++rec_depth; update_member(&rec_depth); post_redraw(); return true;
				case KEY_Down: rec_depth = rec_depth>1?rec_depth-1:0; update_member(&rec_depth); post_redraw(); return true;
				}
			}
		}
		return false; // event not handled
	}
	/// declare timer_event method to connect the shoot signal of the trigger
	void timer_event(double, double dt)
	{
		if (animate) {
			angle += speed*dt;
			if (angle > 360)
				angle -= 360;
			update_member(&angle);
			post_redraw();
		}
	}
	/// draw a cube tree of given depth in the current coordinate system
	void draw_cube_tree(context& c, unsigned int depth, int nr_children = 3)
	{
		if (shp < CYL)
			glShadeModel(GL_FLAT);
		switch (shp) {
		case CUBE: c.tesselate_unit_cube(); break;
		case PRI: c.tesselate_unit_prism(); break;
		case TET: c.tesselate_unit_tetrahedron(); break;
		case OCT: c.tesselate_unit_octahedron(); break;
		case DOD: c.tesselate_unit_dodecahedron(); break;
		case ICO: c.tesselate_unit_icosahedron(); break;
		case CYL: c.tesselate_unit_cylinder(resolution); break;
		case CONE: c.tesselate_unit_cone(resolution); break;
		case DISK: c.tesselate_unit_disk(resolution); break;
		case SPHERE: c.tesselate_unit_sphere(resolution); break;
		case ARROW: 
			glTranslated(0,0,2);
			glScaled(0.5,0.5,1);
			c.tesselate_unit_disk(resolution);
			glTranslated(0,0,-1);
			c.tesselate_unit_cylinder(resolution);
			glScaled(2,2,1);
			glTranslated(0,0,-1);
			c.tesselate_unit_disk(resolution);
			glRotated(180,1,0,0);
			glTranslated(0,0,1);
			c.tesselate_unit_cone(resolution);
			break;
		default:
			std::cerr << "unknown shape" << std::endl;
		}
		if (shp < CYL)
			glShadeModel(GL_SMOOTH);

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
					draw_cube_tree(c, depth+1);
				// restore coordinate system before moving on to next child cube
				glPopMatrix();
			}
	}
	/// setting the view transform yourself
	void draw(context& ctx)
	{
		ctx.enable_material(material);
		glRotated(angle,0,1,0);
		glScaled(0.5,0.5,0.5);
		glColor3f(0,1,0.2f);
		draw_cube_tree(ctx, 0, 4);
		ctx.disable_material(material);
	}
	/// overload the create gui method
	void create_gui()
	{
		add_decorator("Simple Cube GUI", "heading", "level=1"); // level=1 is default and can be skipped
		connect_copy(add_control("recursion depth", rec_depth, "value_slider", 
				                 "min=1;max=8;ticks=true")->value_change,
						 rebind(static_cast<drawable*>(this), 
						        &drawable::post_redraw));
		/// use a selection gui element to directly manipulate the shape enum
		connect_copy(add_control("shape", shp, 
			"CUBE,PRI,TET,OCT,DOD,ICO,CYL,CONE,DISK,ARROW,SPHERE")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		if (add_tree_node("Animation Settings", toggle, 2)) {
			add_control("animate", animate, "check");
			connect_copy(add_control("angle", angle, "value_slider", 
				                     "min=0;max=360;ticks=true;tooltip"\
									 "=\"rotation angle\"")->value_change,
						 rebind(static_cast<drawable*>(this),
						        &drawable::post_redraw));
			add_control("speed", speed, "value_slider", 
				        "min=0;max=720;log=true;ticks=true;"\
				        "tooltip=\"rotation speed in radians per second\"");
		}
	}
};

// register a newly created cube without options 
extern factory_registration<simple_cube> simp_cube_fac("new/simple cube", 'A', true);
