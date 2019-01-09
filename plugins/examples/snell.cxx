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

class snell_demo : public node, public drawable, public provider
{
public:
	typedef cgv::math::fvec<double, 3> vec3;
protected:
	double n1,n2;
	vec3   v;
	vec3   n;
	vec3   t;
	vec3   t1,t2;
	vec3   r;
	double aspect;
	cgv::media::illum::phong_material axes_mat, surface_mat;
public:
	void compute_rt()
	{
		double q = n1 / n2;
		double dp = dot(n, v);
		if (dp > 0) {
			q = 1.0 / q;
		}
		r = v - 2.0*dp*n;
		t1 = q*v - q*dp*n;
		t2 = -sqrt(1.0 - q*q*(1 - dp*dp))*n;
		if (dp > 0)
			t2 = -t2;
		t = t1 + t2;
//		t = q*v - (q*dp + sqrt(1.0 - q*q*(1 - dp*dp)))*n;
	}
	void on_set(void* member_ptr)
	{
		compute_rt();
		update_member(&member_ptr);
		post_redraw();
		return;
	}
	snell_demo()
	{
		set_name("snell_demo");

		n1 = 1.0;
		n2 = 1.3333;
		v = vec3(1,0,-1);
		v.normalize();
		n = vec3(0,0,1);
		compute_rt();
		aspect = 0.02;

		axes_mat.set_ambient(cgv::media::illum::phong_material::color_type(0.1f, 0.1f, 0.1f, 1));
		axes_mat.set_diffuse(cgv::media::illum::phong_material::color_type(0, 0, 0, 1));
		axes_mat.set_specular(cgv::media::illum::phong_material::color_type(0.5f, 0.5f, 0.5f, 1));
		axes_mat.set_shininess(80);

		surface_mat.set_ambient(cgv::media::illum::phong_material::color_type(0.2f, 0.2f, 0.2f, 1));
		surface_mat.set_diffuse(cgv::media::illum::phong_material::color_type(0.6f, 0.5f, 0.4f, 1));
		surface_mat.set_specular(cgv::media::illum::phong_material::color_type(0.5f, 0.5f, 0.5f, 1));
		surface_mat.set_shininess(40);
	}
	void create_gui()
	{
		add_decorator("snell's law demo", "heading", "level=2");
		add_member_control(this, "n1", n1, "value_slider", "min=1;max=10;log=true;ticks=true");
		add_member_control(this, "n2", n2, "value_slider", "min=1;max=10;log=true;ticks=true");
		//add_gui("n", n, "direction", "main_label='n';gui_type='value_slider';options='min=-1;max=1;ticks=true'");
		add_gui("v", v, "direction", "main_label='';long_label=true;gui_type='value_slider';options='min=-1;max=1;ticks=true'");

		add_decorator("visualization", "heading", "level=3");
		add_member_control(this, "aspect", aspect, "value_slider", "min=0.001;max=0.1;log=true;ticks=true");
		add_gui("vector material", axes_mat);
		add_gui("surface material", surface_mat);
	}
	void draw(context& c)
	{
		glPushMatrix();
		
		// enable material and lighting with standard shader program
		glEnable(GL_COLOR_MATERIAL); // tell framework to use color material in shader program
		c.enable_material(axes_mat);
			glColor3d(0.6, 0.6, 0.6);
			c.tesselate_arrow(vec3(0, 0, 0), n, aspect);
			glColor3d(0.9, 0.8, 0.3);
			c.tesselate_arrow(-v, vec3(0, 0, 0), aspect);
			glColor3d(1.0, 0.5, 0.1);
			c.tesselate_arrow(vec3(0, 0, 0), t, aspect);
			glColor3d(1.0, 0.1, 0.1);
			//c.tesselate_arrow(vec3(0, 0, 0), t1, aspect);
			c.tesselate_arrow(t2, t, aspect);
			glColor3d(0.1, 1.0, 0.1);
			if (dot(t2,n) < 0)
				c.tesselate_arrow(vec3(0, 0, 0), t2, aspect);
			else
				c.tesselate_arrow(t1, t, aspect);
			glColor3d(0.1, 0.5, 1.0);
			c.tesselate_arrow(vec3(0, 0, 0), r, aspect);
		// disable standard shader program
		c.disable_material(axes_mat);
		glDisable(GL_COLOR_MATERIAL); // tell framework to use color material in shader program
		glDisable(GL_CULL_FACE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
		c.enable_material(surface_mat);
			c.tesselate_unit_disk(50);
		c.disable_material(surface_mat);
		glDisable(GL_CULL_FACE);
		glPopMatrix();
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
extern factory_registration<snell_demo> gp_fac("snell", "shortcut='Shift-Ctrl-S';menu_text='new/snell demo'", true); 
