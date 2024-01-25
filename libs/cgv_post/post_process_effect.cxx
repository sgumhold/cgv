#include "post_process_effect.h"

namespace cgv {
namespace post {

void post_process_effect::destruct(cgv::render::context& ctx) {

	fbc_draw.destruct(ctx);
	shaders.clear(ctx);
	is_initialized = false;
}

bool post_process_effect::init(cgv::render::context& ctx) {

	is_initialized = shaders.load_all(ctx);
	return is_initialized;
}

bool post_process_effect::ensure(cgv::render::context& ctx) {

	assert_init();

	bool update = false;
	if(fbc_draw.ensure(ctx)) {
		viewport_size.x() = static_cast<float>(fbc_draw.ref_frame_buffer().get_width());
		viewport_size.y() = static_cast<float>(fbc_draw.ref_frame_buffer().get_height());
		reset();
		return true;
	}
	
	return false;
}

void post_process_effect::create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) {
	
	p->add_member_control(b, "Enable", enable, "toggle");
}

void post_process_effect::assert_init() {

	const std::string msg = "Error: " + name + " has not been initialized.";
	assertm(is_initialized, msg);
}

}
}
