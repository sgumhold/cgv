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
#include <cgv/media/illum/surface_material.h>
#include <cgv/math/ftransform.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::math;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::illum;

class cube_demo : public node, public drawable, public provider
{
	double angle;
	bool animate;
	double speed;
	double aspect;
	double s;
	dvec3  axis;
	surface_material axes_mat, cube_mat;
public:
	cube_demo() :
		axes_mat(BT_OREN_NAYAR, surface_material::color_type(0, 0, 0), 0.2f),
		cube_mat(BT_OREN_NAYAR, surface_material::color_type(0.6f, 0.5f, 0.4f), 0.5f),
		axis(1, 0, 0)
	{
		set_name("cube_demo");
		angle = 0;
		s = 0.4;
		aspect = 0.02;
		animate = false;
		speed = 3;
		connect(get_animation_trigger().shoot, this, &cube_demo::timer_event);
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	void create_gui()
	{
		add_member_control(this, "animate", animate, "check", "shortcut='a'");
		add_member_control(this, "angle", angle, "value_slider", "min=0;max=360;ticks=true");
		add_gui("axis", axis, "direction", "options='min=-1;max=1;ticks=true'");
		add_member_control(this, "speed", speed, "value_slider", "min=0.1;max=100;ticks=true;log=true");
		add_member_control(this, "scale", s, "value_slider", "min=0.01;max=1;log=true;ticks=true");
		add_member_control(this, "aspect", aspect, "value_slider", "min=0.01;max=1;ticks=true;log=true");
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
		float c = transformed ? 0.7f : 1;
		float d = 1-c;
		float l = float(axis.length()); 
		ctx.set_color(rgb(c, d, d));
		ctx.tesselate_arrow(fvec<double,3>(0,0,0), fvec<double,3>(l,0,0), aspect);
		ctx.set_color(rgb(d, c, d));
		ctx.tesselate_arrow(fvec<double,3>(0,0,0), fvec<double,3>(0,l,0), aspect);
		ctx.set_color(rgb(d, d, c));
		ctx.tesselate_arrow(fvec<double,3>(0,0,0), fvec<double,3>(0,0,l), aspect);
	}
	void draw(context& c)
	{
		c.push_modelview_matrix();
		
			c.ref_surface_shader_program().enable(c);
				c.ref_surface_shader_program().set_uniform(c, "map_color_to_material", 3);
				c.set_material(axes_mat);
				draw_axes(c, false);
				c.set_color(rgb(0.6f,0.6f,0.6f));
				c.tesselate_arrow(fvec<double,3>(0,0,0), axis, aspect);
				c.mul_modelview_matrix(rotate4<double>(angle,axis));
				draw_axes(c, true);
				c.mul_modelview_matrix(scale4<double>(s,s,s)*translate4<double>(1,1,-1));				
			c.ref_surface_shader_program().disable(c);

			c.ref_default_shader_program().enable(c);
				c.set_color(rgb(0, 0, 0));
				glLineWidth(3);
				c.tesselate_unit_cube(false, true);
			c.ref_default_shader_program().disable(c);

			c.ref_surface_shader_program().enable(c);
				c.ref_surface_shader_program().set_uniform(c, "map_color_to_material", 0);
				glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1,0);		
					c.set_material(cube_mat);
					c.tesselate_unit_cube();
				glDisable(GL_POLYGON_OFFSET_FILL);
			c.ref_surface_shader_program().disable(c);
		c.pop_modelview_matrix();
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<cube_demo> cube_demo_fac("new/animate/cube_demo", 'D');
