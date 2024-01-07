#include <cgv/gui/key_event.h>
#include <cgv/gui/key_control.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/render/drawable.h>
#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::math;
using namespace cgv::render;
using namespace cgv::utils;

class snell_demo : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider
{
protected:
	double n1,n2;
	cgv::dvec3   v;
	cgv::dvec3   n;
	cgv::dvec3   t;
	cgv::dvec3   t1,t2;
	cgv::dvec3   r;
	double aspect;
	cgv::media::illum::surface_material axes_mat, surface_mat;
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
	snell_demo() : 
		axes_mat(
			cgv::media::illum::BrdfType(cgv::media::illum::BT_OREN_NAYAR + cgv::media::illum::BT_COOK_TORRANCE),
		cgv::rgb(0.1f, 0.1f, 0.1f),
			0.1f, 0.5f, 0.2f
		),
		surface_mat(
			cgv::media::illum::BT_OREN_NAYAR,
		cgv::rgb(0.3f, 0.6f, 0.3f),
			0.1f, 0.0f, 0.2f
		)
	{
		set_name("snell_demo");

		n1 = 1.0;
		n2 = 1.3333;
		v = cgv::dvec3(1,0,-1);
		v.normalize();
		n = cgv::dvec3(0,0,1);
		compute_rt();
		aspect = 0.02;
	}
	void create_gui()
	{
		add_decorator("snell's law demo", "heading", "level=2");
		add_member_control(this, "n1", n1, "value_slider", "min=1;max=10;log=true;ticks=true");
		add_member_control(this, "n2", n2, "value_slider", "min=1;max=10;log=true;ticks=true");
		//add_gui("n", n, "direction", "main_label='n';gui_type='value_slider';options='min=-1;max=1;ticks=true'");
		add_gui("v", v, "direction", "main_label='';long_label=true;gui_type='value_slider';options='min=-1;max=1;ticks=true'");

		if (begin_tree_node("visualization", surface_mat, true)) {
			align("\a");
			add_member_control(this, "aspect", aspect, "value_slider", "min=0.001;max=0.1;log=true;ticks=true");
			add_gui("vector material", axes_mat);
			add_gui("surface material", surface_mat);
			align("\b");
			end_tree_node(surface_mat);

		}
	}
	void draw(context& c)
	{
		cgv::render::shader_program& prog = c.ref_surface_shader_program();
		prog.enable(c);
			prog.set_uniform(c, "map_color_to_material", 3);
			prog.set_uniform(c, "illumination_mode", 1);
			c.set_material(axes_mat);
			c.set_color(cgv::rgb(0.6f, 0.6f, 0.6f));
			c.tesselate_arrow(cgv::dvec3(0, 0, 0), n, aspect);
			c.set_color(cgv::rgb(0.9f, 0.8f, 0.3f));
			c.tesselate_arrow(-v, cgv::dvec3(0, 0, 0), aspect);
			c.set_color(cgv::rgb(1.0f, 0.5f, 0.1f));
			c.tesselate_arrow(cgv::dvec3(0, 0, 0), t, aspect);
			c.set_color(cgv::rgb(1.0f, 0.1f, 0.1f));
			c.tesselate_arrow(t2, t, aspect);
			c.set_color(cgv::rgb(0.1f, 1.0f, 0.1f));
			if (dot(t2,n) < 0)
				c.tesselate_arrow(cgv::dvec3(0, 0, 0), t2, aspect);
			else
				c.tesselate_arrow(t1, t, aspect);
			c.set_color(cgv::rgb(0.1f, 0.5f, 1.0f));
			c.tesselate_arrow(cgv::dvec3(0, 0, 0), r, aspect);

			prog.set_uniform(c, "map_color_to_material", 0);
			prog.set_uniform(c, "illumination_mode", 2);
			c.set_material(surface_mat);
			glDisable(GL_CULL_FACE);
				c.tesselate_unit_disk(50);
			glDisable(GL_CULL_FACE);

		c.ref_surface_shader_program().disable(c);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<snell_demo> snell_demo_fac("snell", "shortcut='Shift-Ctrl-S';menu_text='New/Demo/Snell'", true); 
