#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/context.h>
#include <cgv/render/texture.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/math/ftransform.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::type;

class ping_pong : public node, public drawable, public provider
{
	int current_target;
	bool do_step;
protected:
	bool restart;
	bool animate;
	bool interpolate;
	int w, h;
	int time_step;
	int mode;
	float speed;
	float threshold;
	int rule[18];

	texture T[3];
	frame_buffer fbo;
	shader_program prog;

public:
	ping_pong()
	{
		set_name("ping_pong");
		w = 512;
		h = 512;
		do_step = false;
		current_target = 1;
		mode = 2;
		restart = true;
		speed = 1;
		time_step = 0;
		animate = false;
		threshold = 0.6f;
		interpolate = false;
		T[1].set_mag_filter(TF_NEAREST);
		T[2].set_mag_filter(TF_NEAREST);
		std::fill(rule, rule + 18, 0);
		rule[3] = rule[9+2] = rule[9+3] = 1;
		connect(get_animation_trigger().shoot, this, &ping_pong::timer_event);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &interpolate) {
			T[1].set_mag_filter(interpolate ? TF_LINEAR : TF_NEAREST);
			T[2].set_mag_filter(interpolate ? TF_LINEAR : TF_NEAREST);
		}
		update_member(member_ptr);
		post_redraw();
	}
	void create_gui()
	{
		add_view("w", w);
		add_view("h", h);
		add_view("current_target", current_target);
		add_view("time_step", time_step);
		add_member_control(this, "mode", (DummyEnum&)mode, "dropdown", "enums='copy=0;smooth=1;GameOfLife=2;advect=3'");
		connect_copy(add_button("step")->click, rebind(this, &ping_pong::step));
		add_member_control(this, "restart", restart, "toggle", "shortcut='r'");
		add_member_control(this, "animate", animate, "check", "shortcut='a'");
		add_member_control(this, "interpolate", interpolate, "check", "shortcut='i'");
		add_member_control(this, "threshold", threshold, "value_slider", "min=0;max=1;ticks=true");
		add_decorator("game of life", "heading", "level=3");
		add_member_control(this, "birth 0", (bool&)rule[0], "check", "w=15;align='L'", " ");
		for (unsigned i = 1; i < 9; ++i)
			add_member_control(this, std::to_string(i), (bool&)rule[i], "check", "w=15;align='L'", (i == 8 ? "\n" : " "));
		add_member_control(this, "alive 0", (bool&)rule[9], "check", "w=15;align='L'", " ");
		for (unsigned i = 1; i < 9; ++i)
			add_member_control(this, std::to_string(i), (bool&)rule[9+i], "check", "w=15;align='L'", (i == 8 ? "\n" : " "));
	}
	void step()
	{
		++time_step;
		current_target = 3 - current_target;
		do_step = true;
		update_member(&time_step);
		update_member(&current_target);
		post_redraw();
	}
	void timer_event(double, double dt)
	{
		if (animate)
			step();
	}

	bool init(context& ctx)
	{
		if (!prog.build_program(ctx, "ping_pong.glpr", true))
			return false;
		T[0].create_from_image(ctx, "res://alhambra.png", &w, &h);
		T[1].set_component_format(T[0].get_component_format());
		T[1].create(ctx, TT_2D, w, h);
		T[1].set_component_format(T[0].get_component_format());
		T[2].create(ctx, TT_2D, w, h);
		fbo.create(ctx, w, h);
		fbo.attach(ctx, T[0], 0, 0);
		fbo.attach(ctx, T[1], 0, 1);
		fbo.attach(ctx, T[2], 0, 2);
		if (!fbo.is_complete(ctx)) {
			ctx.error("ping_pong fbo not complete");
			return false;
		}
		return true;
	}
	void draw_quad(context& ctx)
	{
		const vec2 P[4] = { vec2(-1,1), vec2(-1,-1), vec2(1,1), vec2(1,-1) };
		const vec2 T[4] = { vec2(0,1), vec2(0.0f,0.0f), vec2(1,1), vec2(1.0f,0.0f) };
		attribute_array_binding::set_global_attribute_array(ctx, 0, P, 4);
		attribute_array_binding::enable_global_array(ctx, 0);
		attribute_array_binding::set_global_attribute_array(ctx, 1, T, 4);
		attribute_array_binding::enable_global_array(ctx, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		attribute_array_binding::disable_global_array(ctx, 1);
		attribute_array_binding::disable_global_array(ctx, 0);
	}
	void init_frame(context& ctx)
	{
		if (!restart && !do_step)
			return;
		GLint vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(cgv::math::identity4<double>());
		ctx.push_projection_matrix();
		ctx.set_projection_matrix(cgv::math::identity4<double>());
		fbo.enable(ctx, current_target);
		glViewport(0, 0, w, h);
		prog.enable(ctx);
		prog.set_uniform(ctx, "w", w);
		prog.set_uniform(ctx, "h", h);
		prog.set_uniform(ctx, "threshold", threshold);
		prog.set_uniform_array(ctx, "rule", rule, 16);
		if (restart) {
			T[0].enable(ctx, 0);
				prog.set_uniform(ctx, "mode", 0);
				prog.set_uniform(ctx, "tex", 0);
				draw_quad(ctx);
			T[0].disable(ctx);
			restart = false;
			update_member(&restart);
			time_step = 0;
			update_member(&time_step);
		}
		else {
			T[3 - current_target].enable(ctx, 3 - current_target);
				prog.set_uniform(ctx, "mode", mode);
				prog.set_uniform(ctx, "tex", 3 - current_target);
				draw_quad(ctx);
			T[3 - current_target].disable(ctx);
			do_step = false;
		}
		prog.disable(ctx);
		glViewport(vp[0], vp[1], vp[2], vp[3]);
		fbo.disable(ctx);
		ctx.pop_projection_matrix();
		ctx.pop_modelview_matrix();
	}
	void draw(context& ctx)
	{
		prog.enable(ctx);
			T[current_target].enable(ctx, current_target);
				prog.set_uniform(ctx, "mode", 0);
				prog.set_uniform(ctx, "tex", current_target);
				draw_quad(ctx);
			T[current_target].disable(ctx);
		prog.disable(ctx);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
extern cgv::base::factory_registration<ping_pong> ping_pong_fac("new/ping pong", 'W');
