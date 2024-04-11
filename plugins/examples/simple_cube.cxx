#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/illum/surface_material.h>

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
	cgv::media::illum::surface_material material;
	/// different shape types
	enum Shape { CUBE, PRI, TET, OCT, DOD, ICO, CYL, CONE, DISK, ARROW, SPHERE } shp;
public:
	/// initialize rotation angle
	simple_cube() : toggle(false), angle(0), speed(90), rec_depth(4), animate(true), resolution(25), shp(CUBE)
	{
		material.set_diffuse_reflectance(cgv::rgb(0.7f, 0.2f, 0.4f));
		connect(get_animation_trigger().shoot, this, &simple_cube::timer_event);
	}
	/// 
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
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
				case 'A': animate = !animate; update_member(&animate); post_redraw(); return true;
				case KEY_Right: speed *= 1.2; update_member(&speed); return true;
				case KEY_Left:  speed /= 1.2; update_member(&speed); return true;
				case KEY_Up: ++rec_depth; update_member(&rec_depth); post_redraw(); return true;
				case KEY_Down: rec_depth = rec_depth > 1 ? rec_depth - 1 : 0; update_member(&rec_depth); post_redraw(); return true;
				}
			}
		}
		return false; // event not handled
	}
	/// declare timer_event method to connect the shoot signal of the trigger
	void timer_event(double, double dt)
	{
		if (animate) {
			angle += speed * dt;
			if (angle > 360)
				angle -= 360;
			update_member(&angle);
			post_redraw();
		}
	}
	/// draw a cube tree of given depth in the current coordinate system
	void draw_cube_tree(context& ctx, unsigned int depth, int nr_children = 3)
	{
		switch (shp) {
		case CUBE:   ctx.tesselate_unit_cube(); break;
		case PRI:    ctx.tesselate_unit_prism(); break;
		case TET:    ctx.tesselate_unit_tetrahedron(); break;
		case OCT:    ctx.tesselate_unit_octahedron(); break;
		case DOD:    ctx.tesselate_unit_dodecahedron(); break;
		case ICO:    ctx.tesselate_unit_icosahedron(); break;
		case CYL:    ctx.tesselate_unit_cylinder(resolution); break;
		case CONE:   ctx.tesselate_unit_cone(resolution); break;
		case DISK:   ctx.tesselate_unit_disk(resolution); break;
		case SPHERE: ctx.tesselate_unit_sphere(resolution); break;
		case ARROW:  ctx.tesselate_arrow(0.75, 1, 1.5, 2, resolution); break;
		default:
			std::cerr << "unknown shape" << std::endl;
		}
		if (depth < rec_depth)
			// iterate children
			for (int i = 0; i < nr_children; ++i) {
				// remember current coordinate system
				ctx.push_modelview_matrix();
				ctx.mul_modelview_matrix(
					// rotate around z -axis by -90, 0, 90 or 180 degrees
					cgv::math::rotate4<double>(i * 90 - 90, 0, 0, 1)*
					// move along x axis by 2 units
					cgv::math::translate4<double>(2, 0, 0)*
					// shrink child cube by a factor of 1/2
					cgv::math::scale4<double>(0.5, 0.5, 0.5)
				);
				// recursively draw child cube
				draw_cube_tree(ctx, depth + 1);
				// restore coordinate system before moving on to next child cube
				ctx.pop_modelview_matrix();
			}
	}
	/// setting the view transform yourself
	void draw(context& ctx)
	{
		GLboolean was_culling = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);
		ctx.ref_surface_shader_program().enable(ctx);
		ctx.set_material(material);
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(cgv::math::rotate4<double>(angle, 0, 1, 0)*cgv::math::scale4<double>(0.5, 0.5, 0.5));
		ctx.set_color(cgv::rgb(0, 1, 0.2f));
		draw_cube_tree(ctx, 0, 4);
		ctx.pop_modelview_matrix();
		ctx.ref_surface_shader_program().disable(ctx);
		if (was_culling)
			glEnable(GL_CULL_FACE);
	}
	/// overload the create gui method
	void create_gui()
	{
		add_decorator("Simple Cube GUI", "heading", "level=1"); // level=1 is default and can be skipped
		add_member_control(this, "recursion depth", rec_depth, "value_slider", "min=1;max=8;ticks=true");
		/// use a selection gui element to directly manipulate the shape enum
		add_member_control(this, "shape", shp, "dropdown", "enums='CUBE,PRI,TET,OCT,DOD,ICO,CYL,CONE,DISK,ARROW,SPHERE'");
		// start a tree node, where shp is used as unique reference 
		if (begin_tree_node("Animation Settings", shp, false, "level=2")) {
			align("\a"); // increases identation level
			add_control("animate", animate, "check");
			add_member_control(this, "angle", angle, "value_slider",
				"min=0;max=360;ticks=true;tooltip"\
				"=\"rotation angle\"");
			add_control("speed", speed, "value_slider",
				"min=0;max=720;log=true;ticks=true;"\
				"tooltip=\"rotation speed in degree per second\"");
			align("\b"); // decreases identation level
			end_tree_node(shp); // ensure same unique reference passed as in corresponding begin_tree_node
		}
		// start a tree node, where shp is used as unique reference 
		if (begin_tree_node("Material Settings", material, false, "level=2")) {
			align("\a"); // increases identation level
			add_gui("material", material); // use gui registered for surface_material type
			align("\b"); // decreases identation level
			end_tree_node(material); // ensure same unique reference passed as in corresponding begin_tree_node
		}
	}
};

// register a newly created cube without options 
factory_registration<simple_cube> simp_cube_fac("New/Animate/Simple Cube", 'A', true);
