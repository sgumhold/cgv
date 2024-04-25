#include <cgv_reflect_types/media/color.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>

using namespace cgv::utils;
using namespace cgv::reflect;
using namespace cgv::signal;
using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;

class background : public node, public drawable, public provider, public event_handler
{
protected:
	shader_program prog;
	bool enable = true;
	cgv::rgba color_1 = cgv::rgba(0.7f, 0.7f, 0.7f, 1.0f);
	cgv::rgba color_2 = cgv::rgba(1.0f, 1.0f, 1.0f, 1.0f);
	int mode = 0;
	int checker_step = 16;
public:
	background() : node("Background")
	{
	}
	std::string get_type_name() const
	{
		return "background";
	}
	bool self_reflect(reflection_handler& rh)
	{
		return
			rh.reflect_member("enable", enable) &&
			rh.reflect_member("mode", mode) &&
			rh.reflect_member("color_1", color_1) &&
			rh.reflect_member("color_2", color_2) &&
			rh.reflect_member("checker_step", checker_step);
	}
	void synch_render_flags(context& ctx)
	{
		if (enable)
			ctx.set_default_render_pass_flags(RenderPassFlags(ctx.get_default_render_pass_flags() & ~cgv::render::RPF_CLEAR_COLOR));
		else
			ctx.set_default_render_pass_flags(RenderPassFlags(ctx.get_default_render_pass_flags() | cgv::render::RPF_CLEAR_COLOR));
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &enable) {
			auto* ctx_ptr = get_context();
			if (ctx_ptr)
				synch_render_flags(*ctx_ptr);
		}
		update_member(member_ptr);
		post_redraw();
	}
	void stream_help(std::ostream& os)
	{
		os << "background: <S-F12> to toggle mode";
	}
	bool handle(event& e)
	{
		if (e.get_kind() != EID_KEY)
			return false;
		auto& ke = reinterpret_cast<key_event&>(e);		
		if (ke.get_action() == KA_RELEASE)
			return false;
		switch (ke.get_key()) {
		case KEY_F12:
			if (ke.get_modifiers() == EM_SHIFT) {
				mode = (mode + 1) % 5;
				on_set(&mode);
				return true;
			}
			break;
		}
		return false;
	}
	bool init(context& ctx)
	{
		if (!prog.build_program(ctx, "background.glpr", true))
			return false;
		synch_render_flags(ctx);
		return true;
	}
	void clear(context& ctx)
	{
		prog.destruct(ctx);
	}
	void init_frame(context& ctx)
	{
		if (!enable)
			return;
		glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
				prog.set_uniform(ctx, "w", int(ctx.get_width()));
				prog.set_uniform(ctx, "h", int(ctx.get_height()));
				prog.set_uniform(ctx, "mode", mode);
				prog.set_uniform(ctx, "color_1", color_1);
				prog.set_uniform(ctx, "color_2", color_2);
				prog.set_uniform(ctx, "checker_step", checker_step);
				prog.enable(ctx);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				prog.disable(ctx);
			glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}
	void swap_colors()
	{
		std::swap(color_1, color_2);
		update_member(&color_1);
		update_member(&color_2);
		post_redraw();
	}
	void create_gui()
	{
		add_decorator("Background", "heading");
		add_member_control(this, "Enable", enable, "toggle");
		add_member_control(this, "Mode", (cgv::type::DummyEnum&)mode, "dropdown", "enums='Constant,Horizontal,Vertical,Checkerboard,Gamma Test'");
		add_member_control(this, "Color 1", color_1);
		add_member_control(this, "Color 2", color_2);
		connect_copy(add_button("Swap Colors")->click, rebind(this, &background::swap_colors));
		add_member_control(this, "Checkerboard Step", checker_step, "value_slider", "min=1;max=128;log=true;ticks=true");
	}
};

#include <cgv/base/register.h>

#include "lib_begin.h"

/// register a light interactor factory
CGV_API cgv::base::object_registration<background> bg_reg("");
