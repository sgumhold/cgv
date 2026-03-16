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

// Define BackgroundMode outside of background class to be able to use it with type reflection.
enum class BackgroundMode {
	kSolidColor,
	kHorizontalGradient,
	kVerticalGradient,
	kCheckerboard,
	kGammaTestPattern
};

static const int k_background_mode_count = 5;

namespace cgv {
namespace reflect {

// Define custom reflection traits for the BackgroundMode
enum_reflection_traits<BackgroundMode> get_reflection_traits(const BackgroundMode&) {
	return enum_reflection_traits<BackgroundMode>("solid_color,horizontal_gradient,vertical_gradient,checkerboard,gamma_test_pattern");
}

} // namespace reflect
} // namespace cgv

class background : public node, public drawable, public provider, public event_handler
{
protected:
	shader_program prog;
	bool enable = true;
	cgv::rgba color_1 = cgv::rgba(0.7f, 0.7f, 0.7f, 1.0f);
	cgv::rgba color_2 = cgv::rgba(1.0f, 1.0f, 1.0f, 1.0f);
	BackgroundMode mode = BackgroundMode::kSolidColor;
	int checker_step = 16;
	bool use_gamma_correction = true;

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
			rh.reflect_member("checker_step", checker_step) &&
			rh.reflect_member("use_gamma_correction", use_gamma_correction);
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
		os << "background: <Shift + F12> to toggle mode";
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
				int mode_idx = static_cast<int>(mode);
				mode = static_cast<BackgroundMode>((mode_idx + 1) % k_background_mode_count);
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
		ctx.push_depth_test_state();
		ctx.disable_depth_test();
		
		prog.enable(ctx);
		prog.set_uniform(ctx, "w", int(ctx.get_width()));
		prog.set_uniform(ctx, "h", int(ctx.get_height()));
		prog.set_uniform(ctx, "mode", static_cast<int>(mode));
		prog.set_uniform(ctx, "color_1", color_1);
		prog.set_uniform(ctx, "color_2", color_2);
		prog.set_uniform(ctx, "checker_step", checker_step);
		if(!use_gamma_correction)
			prog.set_uniform(ctx, "gamma3", cgv::vec3(1.0f));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		prog.disable(ctx);
		
		ctx.pop_depth_test_state();
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
		add_member_control(this, "Mode", mode, "dropdown", "enums='Solid color,Horizontal gradient,Vertical gradient,Checkerboard,Gamma test pattern'");
		add_member_control(this, "Color 1", color_1);
		add_member_control(this, "Color 2", color_2);
		add_member_control(this, "Use gamma correction", use_gamma_correction, "check");
		connect_copy(add_button("Swap colors")->click, rebind(this, &background::swap_colors));
		add_member_control(this, "Checkerboard step size", checker_step, "value_slider", "min=1;max=128;log=true;ticks=true");
	}
};

#include <cgv/base/register.h>

#include "lib_begin.h"

/// register a background object
CGV_API cgv::base::object_registration<background> bg_reg("");
