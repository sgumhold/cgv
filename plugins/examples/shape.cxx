#include "shape.h"
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

shape::shape(const char* name) : group(name), node_flag(true), ax(0), ay(0) 
{
	show_edges = true;
	show_faces = true;

	static double x0 = -2;
	x = x0;
	x0 += 4;
	shp = CUBE;
	no_flat = false;
	resolution = 25;

	key_control<double>* kc = new key_control<double>("ax", ax, "speed=100;more='X';less='Shift-X'");
	connect_copy(kc->value_change, rebind(static_cast<drawable*>(this), &shape::post_redraw)); 
	append_child(base_ptr(kc));
	kc = new key_control<double>("ay", ay, "speed=100;more='Y';less='Shift-Y'");
	connect_copy(kc->value_change, rebind(static_cast<drawable*>(this), &shape::post_redraw)); 
	append_child(base_ptr(kc));
	key_control<bool>* kcb = new key_control<bool>("no_flat", no_flat, "toggle='F'");
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
		"shape", "prism", "tetra", "octahedron", "dodecahedron", "icosahedron", "cylinder", "cone", "disk", "arrow", "sphere", "strip"
	};

	cgv::utils::oprintf(os, "%s: ax=%.1f<down arrow/up arrow>, ay=%.1f<left arrow/right arrow>, shape=%s<S>[%d<R>]\n", 
		get_name().c_str(), ax, ay, shape_name[shp], resolution);
}

void draw_strip(bool draw_clr = true)
{
	double c[] = {
		
		1,0,0,
		1,0.5,0,
		1,1,0,

		0,1,0,
		0,1,0.5,
		0,1,1,
		
		0,0,1,
		0.5,0,1,
		1,0,1
	};

	for (int i = 0; i<9; ++i) {
		if (draw_clr) {
			glColor3d(c[3*i],c[3*i+1],c[3*i+2]);
		}
		glVertex3d(i/2,1-i%2,0);
	}
}

void shape::draw_shape(context& c)
{
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
		glPushMatrix();
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
		glPopMatrix();
		break;
	case STRIP :
		if (no_flat)
			glShadeModel(GL_SMOOTH);
		else
			glShadeModel(GL_FLAT);
		glDisable(GL_LIGHTING);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(15);
		glBegin(GL_POINTS);
			draw_strip();
		glEnd();

		glColor3d(0,0,0);
		glPointSize(20);
		glBegin(GL_POINTS);
			draw_strip(false);
		glEnd();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(3);
		glBegin(GL_TRIANGLE_STRIP);
			draw_strip(false);
		glEnd();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_TRIANGLE_STRIP);
			draw_strip();
		glEnd();
		break;
	default:
		std::cerr << "unknown shape" << std::endl;
	}
}


void shape::draw(context& c)
{
	glPushMatrix();
	glTranslated(x,0,0);
	glRotated(ax,1,0,0);
	glRotated(ay,0,1,0);

	glPushAttrib(GL_LIGHTING_BIT|GL_POLYGON_BIT);
	if (show_edges) {
		glColor3f(1,1,0);
		//glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		//glLineWidth(3);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		draw_shape(c);
	}
	if (show_faces) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		static cgv::media::illum::phong_material def_mat;
		def_mat.set_diffuse(cgv::media::illum::phong_material::color_type(0, 0.1f, 1));
		c.enable_material(def_mat);
		glEnable(GL_CULL_FACE);
		if (shp < CYL || shp == STRIP)
			glShadeModel(GL_FLAT);
		draw_shape(c);
		c.disable_material(def_mat);
	}

	glPopAttrib();
	glPopMatrix();
}

/// return a path in the main menu to select the gui
std::string shape::get_menu_path() const
{
	return "example/shape";
}

void shape::select_prism(button&)
{
	shp = PRI;
	update_member(&shp);
	post_redraw();
}

void shape::select_tet(button&)
{
	shp = TET;
	update_member(&shp);
	post_redraw();
}

void shape::select_oct(button&)
{
	shp = OCT;
	update_member(&shp);
	post_redraw();
}

void shape::select_sphere(button&)
{
	shp = SPHERE;
	update_member(&shp);
	post_redraw();
}

void shape::select_cube(button&)
{
	shp = CUBE;
	update_member(&shp);
	post_redraw();
}

void shape::select_strip(button&)
{
	shp = STRIP;
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
	add_member_control(this, "edges", show_edges, "check");
	add_member_control(this, "faces", show_faces, "check");
	/// add buttons that directly select a shape type in a collapsable node
	if (add_tree_node("buttons", node_flag, 0)) {
		connect(add_button("cube", "w=40","")->click,this,&shape::select_cube);
		connect(add_button("prism","w=40","")->click,this,&shape::select_prism);
		connect(add_button("tet",  "w=40","")->click,this,&shape::select_tet);
		connect(add_button("oct",  "w=40", "")->click,this,&shape::select_oct);
		connect(add_button("sphere",  "w=40", "")->click,this,&shape::select_sphere);
		connect(add_button("strip",  "w=40")->click,this,&shape::select_strip);
	}
	connect_copy(add_control("no_flat", no_flat, "check")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));

	/// use a selection gui element to directly manipulate the shape enum
	connect_copy(add_control("shape", shp, 
		"dropdown", "enums='CUBE,PRI,TET,OCT,DOD,ICO,CYL,CONE,DISK,ARROW,SPHERE,STRIP'")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	/// store rotation angle
	cgv::data::ref_ptr<control<double> > e =
		add_control("rot x", ax, "dial", 
		            "w=50;h=50;min=-180;max=180", 
					"");
	connect_copy(e->value_change,rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	e->set("color", 0x00ffff);
	connect_copy(add_control("rot y", ay, "dial", "w=50;h=50;min=-180;max=180")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	/// store location along x-axis
	connect_copy(add_control("x", x, "adjuster", "step=0")->value_change,
		rebind(static_cast<drawable*>(this), &drawable::post_redraw));
}

#include <cgv/base/register.h>

/// register a factory to create new cubes
extern factory_registration_1<shape,const char*> shape_fac("new/shape", 'S', "shape");
