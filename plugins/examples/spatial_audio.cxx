#include <cgv/base/base.h>
#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/render/drawable.h>

#include <libs/cgv_oal/al_context.h>

class SpatialAudio : public cgv::base::base,
					 public cgv::gui::provider,
					 public cgv::gui::event_handler,
					 public cgv::render::drawable
{
  private:
	cgv::audio::OALContext oal_ctx;
	bool gui_node_toggle{true};

	cgv::math::fvec<float, 3> angular_velocity{0., 1., 0.};
	cgv::math::fvec<float, 3> position{
		  10.,
		  0.,
		  0.,
	};
	float speed{1.};

  public:
	SpatialAudio() {
		cgv::signal::connect(cgv::gui::get_animation_trigger().shoot, this, &SpatialAudio::timer_event);
	}
	~SpatialAudio() = default;

	void on_set(void* member_ptr) override {
		//TODO
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

	}

	void draw(cgv::render::context& ctx) override {
		//TODO
	}

	void create_gui() override {
		add_decorator("Spatial Audio GUI", "heading");
	}
};

cgv::base::factory_registration<SpatialAudio> spatial_audio_factory("New/Audio/Spatial Audio", "", true);