#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
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
	bool simulate_reverse;
	int mode;
	int before_revert;
	int nr_steps;
	float speed;
	float threshold;
	int rule[18];
	std::vector<cgv::rgba> HPP_colors;

	texture T[3];
	frame_buffer fbo;
	shader_program prog;

public:
	ping_pong()
	{
		set_name("ping_pong");
		w = 512;
		h = 512;
		before_revert = -1;
		do_step = false;
		current_target = 1;
		mode = 8;
		restart = true;
		simulate_reverse = false;
		speed = 1;
		nr_steps = 1;
		time_step = 0;
		animate = false;
		threshold = 0.6f;
		interpolate = false;
		T[1].set_mag_filter(TF_NEAREST);
		T[2].set_mag_filter(TF_NEAREST);
		std::fill(rule, rule + 18, 0);
		rule[3] = rule[9+2] = rule[9+3] = 1;
		HPP_colors.resize(16);
		HPP_colors[ 0] = cgv::rgba(0, 0, 0, 1);
		HPP_colors[15] = cgv::rgba(1, 1, 1, 1);

		HPP_colors[ 1] = cgv::rgba(0.5f, 0.2f, 0.2f, 1);
		HPP_colors[ 2] = cgv::rgba(0.75f, 0.4f, 0.4f, 1);
		HPP_colors[ 4] = cgv::rgba(0.75f, 0, 0, 1);
		HPP_colors[ 8] = cgv::rgba(1, 0.2f, 0.2f, 1);

		HPP_colors[ 3] = cgv::rgba(0.75f, 0.75f, 0.25f, 1);
		HPP_colors[12] = cgv::rgba(1, 1, 0, 1);

		HPP_colors[ 9] = cgv::rgba(0.2f, 0.5f, 0.2f, 1);
		HPP_colors[ 6] = cgv::rgba(0.4f, 0.75f, 0.4f, 1);
		HPP_colors[ 5] = cgv::rgba(0, 0.75f, 0, 1);
		HPP_colors[10] = cgv::rgba(0.2f, 1, 0.2f, 1);

		HPP_colors[14] = cgv::rgba(0.5f, 0.2f, 0.5f, 1);
		HPP_colors[13] = cgv::rgba(0.75f, 0.4f, 0.75f, 1);
		HPP_colors[11] = cgv::rgba(0, 0, 0.75f, 1);
		HPP_colors[ 7] = cgv::rgba(0.2f, 0.2f, 1, 1);

		connect(get_animation_trigger().shoot, this, &ping_pong::timer_event);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &interpolate) {
			T[1].set_mag_filter(interpolate ? TF_LINEAR : TF_NEAREST);
			T[2].set_mag_filter(interpolate ? TF_LINEAR : TF_NEAREST);
		}
		if (member_ptr == &threshold) {
			restart = true;
			update_member(&restart);
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
		add_member_control(this, "mode", (DummyEnum&)mode, "dropdown", "enums='copy=0;smooth=1;GameOfLife=2;HPPcollid=3;HPPtransport=4;HPPrevert=5;roll_x=7;roll_y=8'");
		connect_copy(add_button("step", "shortcut='s'")->click, rebind(this, &ping_pong::step));
		connect_copy(add_button("revert", "shortcut='i'")->click, rebind(this, &ping_pong::revert));
		add_view("simulate_reverse", simulate_reverse);
		add_member_control(this, "restart", restart, "toggle", "shortcut='r'");
		add_member_control(this, "animate", animate, "check", "shortcut='a'");
		add_member_control(this, "interpolate", interpolate, "check", "shortcut='i'");
		add_member_control(this, "nr_steps", nr_steps, "value_slider", "min=1;max=100;ticks=true");
		add_member_control(this, "threshold", threshold, "value_slider", "min=0;max=1;ticks=true");
		add_decorator("game of life", "heading", "level=3");
		add_member_control(this, "birth 0", (bool&)rule[0], "check", "w=15;align='L'", " ");
		unsigned i;
		for (i = 1; i < 9; ++i)
			add_member_control(this, std::to_string(i), (bool&)rule[i], "check", "w=15;align='L'", (i == 8 ? "\n" : " "));
		add_member_control(this, "alive 0", (bool&)rule[9], "check", "w=15;align='L'", " ");
		for (i = 1; i < 9; ++i)
			add_member_control(this, std::to_string(i), (bool&)rule[9+i], "check", "w=15;align='L'", (i == 8 ? "\n" : " "));
		add_member_control(this, "HPP_color_0000", HPP_colors[0]);
		add_member_control(this, "HPP_color_lrdu", HPP_colors[15]);

		add_member_control(this, "HPP_color_l000", HPP_colors[1]);
		add_member_control(this, "HPP_color_0r00", HPP_colors[2]);
		add_member_control(this, "HPP_color_00d0", HPP_colors[4]);
		add_member_control(this, "HPP_color_000u", HPP_colors[8]);
		
		add_member_control(this, "HPP_color_lr00", HPP_colors[3]);
		add_member_control(this, "HPP_color_00du", HPP_colors[12]);

		add_member_control(this, "HPP_color_l00u", HPP_colors[9]);
		add_member_control(this, "HPP_color_0rd0", HPP_colors[6]);
		add_member_control(this, "HPP_color_l0d0", HPP_colors[5]);
		add_member_control(this, "HPP_color_0r0u", HPP_colors[10]);

		add_member_control(this, "HPP_color_0rdu", HPP_colors[14]);
		add_member_control(this, "HPP_color_l0du", HPP_colors[13]);
		add_member_control(this, "HPP_color_lr0u", HPP_colors[11]);
		add_member_control(this, "HPP_color_lrd0", HPP_colors[7]);
	}
	bool on_step()
	{
		if (simulate_reverse)
			--time_step;
		else
			++time_step;

		current_target = 3 - current_target;
		update_member(&time_step);
		update_member(&current_target);
		if (mode == 3 || mode == 4) {
			mode = 7 - mode;
			update_member(&mode);
		}
		if (time_step == 0) {
			animate = false;
			update_member(&animate);
			return true;
		}
		return mode == 5;
	}
	void step()
	{
		do_step = true;
		post_redraw();
	}
	void revert()
	{
		if (mode < 3 || mode > 6) {
			simulate_reverse = !simulate_reverse;
			update_member(&simulate_reverse);
		}
		else {
			if (simulate_reverse)
				--time_step;
			else
				++time_step;

			simulate_reverse = !simulate_reverse;
			update_member(&simulate_reverse);
			before_revert = 7 - mode;
			mode = 5;
			step();
		}
	}
	void timer_event(double, double dt)
	{
		if (animate) {
			step();
		}
	}

	bool init(context& ctx)
	{
		if (!prog.build_program(ctx, "ping_pong.glpr", true))
			return false;
		T[0].create_from_image(ctx, "res://alhambra.png", &w, &h);
		T[0].set_wrap_s(cgv::render::TW_REPEAT);
		T[0].set_wrap_t(cgv::render::TW_REPEAT);
		T[1].set_component_format(T[0].get_component_format());
		T[1].set_wrap_s(cgv::render::TW_REPEAT);
		T[1].set_wrap_t(cgv::render::TW_REPEAT);
		T[1].create(ctx, TT_2D, w, h);

		T[2].set_component_format("[R,G,B,A]");
		T[2].set_wrap_s(cgv::render::TW_REPEAT);
		T[2].set_wrap_t(cgv::render::TW_REPEAT);
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
		const cgv::vec2 P[4] = { cgv::vec2(-1,1), cgv::vec2(-1,-1), cgv::vec2(1,1), cgv::vec2(1,-1) };
		const cgv::vec2 T[4] = { cgv::vec2(0,1), cgv::vec2(0.0f,0.0f), cgv::vec2(1,1), cgv::vec2(1.0f,0.0f) };
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
		glViewport(0, 0, w, h);
		prog.enable(ctx);
		prog.set_uniform(ctx, "w", w);
		prog.set_uniform(ctx, "h", h);
		prog.set_uniform(ctx, "threshold", threshold);
		prog.set_uniform(ctx, "simulate_reverse", simulate_reverse);
		
		prog.set_uniform_array(ctx, "rule", rule, 16);
		if (restart) {
			fbo.enable(ctx, current_target);
				T[0].enable(ctx, 0);
					prog.set_uniform(ctx, "mode", 0);
					prog.set_uniform(ctx, "tex", 0);
					draw_quad(ctx);
				T[0].disable(ctx);
			fbo.disable(ctx);
			restart = false;
			current_target = 3 - current_target;
			time_step = 0;
			update_member(&restart);
			update_member(&current_target);
			update_member(&time_step);
		}
		else {
			for (int i = 0; i < nr_steps; ++i) {
				fbo.enable(ctx, current_target);
					T[3 - current_target].enable(ctx, 3 - current_target);
						prog.set_uniform(ctx, "mode", mode);
						prog.set_uniform(ctx, "tex", 3 - current_target);
						draw_quad(ctx);
					T[3 - current_target].disable(ctx);
				fbo.disable(ctx);
				if (on_step())
					break;
			}
			do_step = false;
			if (before_revert != -1) {
				mode = before_revert;
				update_member(&mode);
				before_revert = -1;
			}
		}
		prog.disable(ctx);
		glViewport(vp[0], vp[1], vp[2], vp[3]);
		ctx.pop_projection_matrix();
		ctx.pop_modelview_matrix();
	}
	void draw(context& ctx)
	{
		glDisable(GL_CULL_FACE);
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(cgv::math::scale4<double>(double(w) / h, 1.0, 1.0));
		prog.enable(ctx);
			T[3-current_target].enable(ctx, current_target);
				prog.set_uniform_array(ctx, "HPP_colors", HPP_colors);
				prog.set_uniform(ctx, "mode", (mode >= 3 && mode <= 5) ? 6 : 0);
				prog.set_uniform(ctx, "tex", current_target);
				draw_quad(ctx);
			T[3-current_target].disable(ctx);
		prog.disable(ctx);
		ctx.pop_modelview_matrix();
		glEnable(GL_CULL_FACE);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<ping_pong> ping_pong_fac("New/Render/Ping Pong", 'W');

