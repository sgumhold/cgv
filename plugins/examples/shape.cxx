#include "shape.h"
#include <cgv/render/shader_program.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/illum/surface_material.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/key_control.h>
#include <cgv/gui/trigger.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/tokenizer.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::illum;

shape::shape(const char* name) : group(name), node_flag(true), ax(0), ay(0),
		mat(BT_OREN_NAYAR, surface_material::color_type(0, 0.1f, 1)), col(1, 1, 0, 1)
{
	show_edges = true;
	show_faces = true;
	
	static double x0 = -2;
	x = x0;
	x0 += 4;
	shp = CUBE;
	flip_normals = false;
	resolution = 25;

	
	key_control<double>* kc = new key_control<double>("ax", ax, "speed=100;more='X';less='Shift-X'");
	connect_copy(kc->value_change, rebind(static_cast<drawable*>(this), &shape::post_redraw)); 
	append_child(base_ptr(kc));
	kc = new key_control<double>("ay", ay, "speed=100;more='Y';less='Shift-Y'");
	connect_copy(kc->value_change, rebind(static_cast<drawable*>(this), &shape::post_redraw)); 
	append_child(base_ptr(kc));
	key_control<bool>* kcb = new key_control<bool>("flip_normals", flip_normals, "toggle='F'");
	connect_copy(kcb->value_change, rebind(static_cast<drawable*>(this), &shape::post_redraw)); 
	append_child(base_ptr(kcb));

}


bool shape::handle(event& e)
{
	if (e.get_kind() == EID_KEY) {
		key_event& ke = static_cast<key_event&>(e);
		if (ke.get_action() == KA_PRESS && ke.get_modifiers() == 0) {
			switch (ke.get_key()) {
			case KEY_Right:
				{
					for (int i=0; i<300; ++i) {
						ay += 1;
						force_redraw();
					}
				}
				return true;
			case KEY_Left:
				ay -= 10;
				post_redraw();
				return true;
			case KEY_Up:
				ax += 10;
				post_redraw();
				return true;
			case KEY_Down:
				ax -= 10;
				post_redraw();
				return true;
			case 'S':
				if (ke.get_modifiers() == EM_SHIFT) {
					if (shp == CUBE)
						shp = SPHERE;					
					else
						--(int&)shp;
				}
				else
					if (++(int&)shp > SPHERE)
						shp = CUBE;
				post_redraw();
				return true;
			case 'R':
				if (ke.get_modifiers() == EM_SHIFT) {
					if (resolution > 4)
						--resolution;
				}
				else 
					++resolution;
				post_redraw();
				return true;
			}
		}
	}
	return false;
}

void shape::on_set(void* mp)
{
	post_redraw();
	update_member(mp);
}

void shape::stream_help(std::ostream&)
{
}

void shape::stream_stats(std::ostream& os)
{
	const char* shape_name[] = { 
		"shape", "prism", "tetra", "octahedron", "dodecahedron", "icosahedron", "cylinder", "cone", "disk", "arrow", "sphere"
	};

	cgv::utils::oprintf(os, "%s: ax=%.1f<down arrow/up arrow>, ay=%.1f<left arrow/right arrow>, shape=%s<S>[%d<R>]\n", 
		get_name().c_str(), ax, ay, shape_name[shp], resolution);
}

void shape::draw_shape(context& c, bool edges)
{
	switch (shp) {
	case CUBE: c.tesselate_unit_cube(flip_normals,edges); break;
	case PRI: c.tesselate_unit_prism(flip_normals, edges); break;
	case TET: c.tesselate_unit_tetrahedron(flip_normals, edges); break;
	case OCT: c.tesselate_unit_octahedron(flip_normals, edges); break;
	case DOD: c.tesselate_unit_dodecahedron(flip_normals, edges); break;
	case ICO: c.tesselate_unit_icosahedron(flip_normals, edges); break;
	case CYL: c.tesselate_unit_cylinder(resolution, flip_normals, edges); break;
	case CONE: c.tesselate_unit_cone(resolution, flip_normals, edges); break;
	case DISK: c.tesselate_unit_disk(resolution, flip_normals, edges); break;
	case SPHERE: c.tesselate_unit_sphere(resolution, flip_normals, edges); break;
	case ARROW: c.tesselate_arrow(1, 0.1,2.0,0.3,resolution,edges); break;
	default:
		std::cerr << "unknown shape" << std::endl;
	}
}

void shape::draw(context& c)
{
	c.push_modelview_matrix();
	c.mul_modelview_matrix(
		cgv::math::translate4<double>(x, 0, 0)*
		cgv::math::rotate4<double>(ax, 1, 0, 0)*
		cgv::math::rotate4<double>(ay, 0, 1, 0)
	);

	if (show_edges) {
		c.ref_default_shader_program().enable(c);
		c.set_color(col);
		glLineWidth(3);
		draw_shape(c, true);
		c.ref_default_shader_program().disable(c);
	}
	if (show_faces) {
		c.ref_surface_shader_program().enable(c);
		c.set_material(mat);
		draw_shape(c, false);
		c.ref_surface_shader_program().disable(c);
	}

	c.pop_modelview_matrix();
}

/// return a path in the main menu to select the gui
std::string shape::get_menu_path() const
{
	return "example/shape";
}

void shape::select_shape(Shape s)
{
	shp = s;
	update_member(&shp);
	post_redraw();
}

/// you must overload this for gui creation
void shape::create_gui()
{
	// connect a permanent copy of the functor returned by the
	// rebind function to the value_change signal of the newly
	// created integer control
	connect_copy(
		/// add a slider that controls the resolution
		add_control("resolution", resolution, "value_slider", 
		// configure the sliders min and max values, add tick marks
		// and use a logarithmic scaling along the slider
		"min=4;max=50;ticks=true;log=true")->value_change,
		// construct a functor that always calls the post_redraw
		// method of the drawable base class
		rebind(static_cast<drawable*>(this), &drawable::post_redraw)
	);
	// simpler is the use of the add_member_control with the this pointer
	// here the on_set method is called automatically on value changes
	add_member_control(this, "edges", show_edges, "check");
	add_member_control(this, "faces", show_faces, "check");
	/// add buttons that directly select a shape type in a collapsable node
	if (add_tree_node("buttons", node_flag, 0)) {
		connect_copy(add_button("cube", "w=40","")->click,    rebind(this,&shape::select_shape, _c<Shape>(CUBE)));
		connect_copy(add_button("prism", "w=40", "")->click,  rebind(this, &shape::select_shape, _c(PRI)));
		connect_copy(add_button("tet",  "w=40","")->click,    rebind(this, &shape::select_shape, _c(TET)));
		connect_copy(add_button("oct",  "w=40", "")->click,   rebind(this, &shape::select_shape, _c(OCT)));
		connect_copy(add_button("sphere",  "w=40", "")->click,rebind(this, &shape::select_shape, _c(SPHERE)));
		connect_copy(add_button("arrow",  "w=40")->click,     rebind(this, &shape::select_shape, _c(ARROW)));
	}
	add_member_control(this, "flip_normals", flip_normals, "check");
	/// use a selection gui element to directly manipulate the shape enum
	add_member_control(this, "shape", shp, "dropdown", "enums='CUBE,PRI,TET,OCT,DOD,ICO,CYL,CONE,DISK,ARROW,SPHERE'");
	/// store rotation angle
	cgv::data::ref_ptr<control<double> > e =
		add_control("rot x", ax, "dial", 
		            "w=50;h=50;min=-180;max=180", 
					"");
	connect_copy(e->value_change,rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	e->set("color", 0x00ffff);
	add_member_control(this, "rot y", ay, "dial", "w=50;h=50;min=-180;max=180");
	/// store location along x-axis
	add_member_control(this, "x", x, "adjuster", "step=0");

	add_member_control(this, "color", col);
	add_gui("material", mat);
}

#include <cgv/base/register.h>

/// register a factory to create new cubes
extern factory_registration_1<shape,const char*> shape_fac("new/shape", 'S', "shape");
