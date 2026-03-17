#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/reflect/reflect_enum.h>
#include <cgv/render/shader_program.h>

#include "lib_begin.h"

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
extern CGV_API enum_reflection_traits<BackgroundMode> get_reflection_traits(const BackgroundMode&);

} // namespace reflect
} // namespace cgv

class CGV_API background : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider, public cgv::gui::event_handler
{
public:
	background() : node("Background") {}

	std::string get_type_name() const override
	{
		return "background";
	}

	bool self_reflect(cgv::reflect::reflection_handler& rh) override;

	void on_set(void* member_ptr) override;

	void stream_help(std::ostream& os) override;

	bool handle(cgv::gui::event& e) override;

	bool init(cgv::render::context& ctx) override;

	void clear(cgv::render::context& ctx) override;

	void init_frame(cgv::render::context& ctx) override;

	void create_gui() override;

	void swap_colors();

private:
	void sync_render_flags(cgv::render::context& ctx) const;

	cgv::render::shader_program prog;
	bool enable = true;
	cgv::rgba color_1 = cgv::rgba(0.7f, 0.7f, 0.7f, 1.0f);
	cgv::rgba color_2 = cgv::rgba(1.0f, 1.0f, 1.0f, 1.0f);
	BackgroundMode mode = BackgroundMode::kSolidColor;
	int checker_step = 16;
	bool use_gamma_correction = true;
};

#include <cgv/config/lib_end.h>
