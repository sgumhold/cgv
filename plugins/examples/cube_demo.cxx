#include <cgv/gui/key_event.h>
#include <cgv/gui/key_control.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/render/drawable.h>
#include <cgv/render/context.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::math;
using namespace cgv::render;
using namespace cgv::utils;

class cube_demo : public node, public drawable, public provider
{
	double angle;
	bool animate;
	double speed;
	double aspect;
	double s;
	double ax, ay, az;
	cgv::media::illum::phong_material axes_mat, cube_mat;
public:
	cube_demo()
	{
		set_name("cube_demo");

		angle = 0;
		s = 0.4;
		aspect = 0.02;
		animate = false;
		speed = 3;
		ax=1;
		ay=1;
		az = 0;

		axes_mat.set_ambient(cgv::media::illum::phong_material::color_type(0.1f, 0.1f, 0.1f, 1));
		axes_mat.set_diffuse(cgv::media::illum::phong_material::color_type(0, 0, 0, 1));
		axes_mat.set_specular(cgv::media::illum::phong_material::color_type(0.5f, 0.5f, 0.5f, 1));
		axes_mat.set_shininess(80);

		cube_mat.set_ambient(cgv::media::illum::phong_material::color_type(0.2f, 0.2f, 0.2f, 1));
		cube_mat.set_diffuse(cgv::media::illum::phong_material::color_type(0.6f, 0.5f, 0.4f, 1));
		cube_mat.set_specular(cgv::media::illum::phong_material::color_type(0.5f, 0.5f, 0.5f, 1));
		cube_mat.set_shininess(40);

		connect(get_animation_trigger().shoot, this, &cube_demo::timer_event);
	}
	void create_gui()
	{
		connect_copy(add_control("animate", animate, "check", "shortcut='a'")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("angle", angle, "value_slider", "min=0;max=360;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("ax", ax, "value_slider", "min=-1;max=1;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("ay", ay, "value_slider", "min=-1;max=1;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("az", az, "value_slider", "min=-1;max=1;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("speed", speed, "value_slider", "min=0.1;max=100;ticks=true;log=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("scale", s, "value_slider", "min=0.01;max=1;log=true;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("aspect", aspect, "value_slider", "min=0.01;max=1;ticks=true;log=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	}
	void timer_event(double, double dt)
	{
		if (animate) {
			angle += speed;
			if (angle > 360) {
				angle = 0;
				animate = false;
				update_member(&animate);
			}
			update_member(&angle);
			post_redraw();
		}
	}
	void draw_axes(context& ctx, bool transformed)
	{
		double c = transformed ? 0.7 : 1;
		double d = 1-c;
		double l = sqrt(ax*ax+ay*ay+az*az);
		glColor3d(c,d,d);
		ctx.tesselate_arrow(fvec<double,3>(0,0,0), fvec<double,3>(l,0,0), aspect);
		glColor3d(d,c,d);
		ctx.tesselate_arrow(fvec<double,3>(0,0,0), fvec<double,3>(0,l,0), aspect);
		glColor3d(d,d,c);
		ctx.tesselate_arrow(fvec<double,3>(0,0,0), fvec<double,3>(0,0,l), aspect);
	}
	void draw(context& c)
	{
		glPushMatrix();
		
		// enable material and lighting with standard shader program
		glEnable(GL_COLOR_MATERIAL); // tell framework to use color material in shader program
		c.enable_material(axes_mat);
			draw_axes(c, false);
			glColor3d(0.6,0.6,0.6);
			c.tesselate_arrow(fvec<double,3>(0,0,0), fvec<double,3>(ax,ay,az), aspect);
			glRotated(angle,ax,ay,az);
			draw_axes(c, true);
		// disable standard shader program
		c.disable_material(axes_mat);

		glScaled(s,s,s);
		glTranslated(1,1,-1);

		// without material / shader program active, draw without illumination
		glColor3d(0,0,0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(3);
		c.tesselate_unit_cube();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1,0);		
			glDisable(GL_COLOR_MATERIAL); // tell framework not to use color material in shader program
			c.enable_material(cube_mat);
				c.tesselate_unit_cube();
			c.disable_material(cube_mat);	
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPopMatrix();
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
extern cgv::base::factory_registration<cube_demo> cube_demo_fac("new/cube_demo", 'D');
