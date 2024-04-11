#include "cgv/math/fvec.h"
#include "cgv/render/context.h"
#include <cgv/base/base.h>
#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/math/ftransform.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_renderer.h>

#undef CGV_OAL_EXPORTS
#include <libs/cgv_oal/al_context.h>

class SpatialAudio : public cgv::base::node,
					 public cgv::render::drawable,
					 public cgv::gui::provider,
					 public cgv::gui::event_handler
{
  private:
	cgv::audio::OALContext oal_ctx;
	cgv::audio::OALSource source;
	bool play_sound{false};

	bool gui_node_toggle{true};

	cgv::math::fvec<float, 3> axis{0., 1., 0.};
	cgv::math::fvec<float, 3> position{
		  10.,
		  0.,
		  0.,
	};
	cgv::math::fvec<float, 3> old_pos {0., 0., 0.};
	float speed{1.};
	float velocity_multiplier{1.};
	bool animate{false};
	bool enable_hrtf{false};

	cgv::render::sphere_renderer sphere_renderer;
	cgv::render::sphere_render_style sphere_style;

  public:
	SpatialAudio() {
		set_name("SpatialAudio");
		cgv::signal::connect(cgv::gui::get_animation_trigger().shoot, this, &SpatialAudio::timer_event);
	}
	~SpatialAudio() = default;

	bool init(cgv::render::context& ctx) override {
		const auto& res_files = cgv::base::ref_resource_file_map();
		const auto& it = res_files.find("CGV_Jam.wav");
		assert(std::end(res_files) != it);
		const auto& file_info = it->second;

		oal_ctx.load_sample("CGV_Jam", file_info.file_data, file_info.file_length);
		source.init(oal_ctx, "CGV_Jam");
		source.set_looping(true);
		source.set_position(position);

		sphere_renderer = cgv::render::ref_sphere_renderer(ctx, 1);
		sphere_style.radius = 1.f;
		sphere_style.surface_color = cgv::rgb{1.f, 1.f, 1.f};

		sphere_renderer.set_render_style(sphere_style);
		return true;
	}

	void clear(cgv::render::context& ctx) override {
		cgv::render::ref_sphere_renderer(ctx, -1);
	}

	void on_set(void* member_ptr) override {
		if (&play_sound == member_ptr) {
			source.play_pause(play_sound);
		}
		if (&enable_hrtf == member_ptr) {
			oal_ctx.set_HRTF(enable_hrtf);
		}
		post_redraw();
	}

	std::string get_type_name() const override {
		return "Spatial Audio";
	}

	void stream_help(std::ostream& os) override {
		os << "Spatial Audio:\a\n"
		   << "";
	}

	bool handle(cgv::gui::event& e) override {
		return false;
	}

	void timer_event(double, double dt) {
		if (animate) {
			const auto& rot = cgv::math::rotate3(speed, axis);
			position = rot * position;
			post_redraw();
		}
	}

	void draw(cgv::render::context& ctx) override {
		sphere_renderer.set_y_view_angle(float(find_view_as_node()->get_y_view_angle()));
		sphere_renderer.set_position(ctx, position);

		source.set_position(position);
		source.set_velocity(velocity_multiplier * (position - old_pos));
		old_pos = position;

		auto view_node = find_view_as_node();
		cgv::audio::OALListener::set_position(view_node->get_eye());
		cgv::audio::OALListener::set_orientation(view_node->get_view_dir(), view_node->get_view_up_dir());

		sphere_renderer.render(ctx, 0, 1);
		ctx.ref_surface_shader_program().enable(ctx);
		ctx.tesselate_arrow({0.,0.,0.}, axis);
		ctx.ref_surface_shader_program().disable(ctx);
	}

	void create_gui() override {
		add_decorator("Spatial Audio GUI", "heading");
		add_member_control(this, "Animate", animate, "check");
		add_member_control(this, "Play Sound", play_sound, "check");
		add_member_control(this, "Enable HRTF", enable_hrtf, "check");
		add_member_control(this, "Velocity Multiplier", velocity_multiplier, "value_slider", "min=0;max=100;log=true;ticks=true");
		add_gui("axis", axis, "direction", "options='min=-1;max=1;ticks=true'");
		add_member_control(this, "Speed", speed, "value_slider", "min=0;max=100;log=true;ticks=true");
	}
};

cgv::base::factory_registration<SpatialAudio> spatial_audio_factory("New/Audio/Spatial Audio", "", true);