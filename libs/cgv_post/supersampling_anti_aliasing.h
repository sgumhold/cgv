#pragma once

#include <cgv_post/post_process_effect.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API supersampling_anti_aliasing : public post_process_effect {
protected:
	/// multiplier of the screen resolution
	int multiplier = 2;
	
	cgv::ivec2 last_screen_size = { -1 };

	void ensure_fbc_size(cgv::render::context& ctx);

	void create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) override;

public:
	supersampling_anti_aliasing() : post_process_effect("SSAO") {}

	void destruct(cgv::render::context& ctx) override;

	bool init(cgv::render::context& ctx) override;

	bool ensure(cgv::render::context& ctx) override;

	void begin(cgv::render::context& ctx, bool push_viewport=true) override;

	void end(cgv::render::context& ctx, bool pop_viewport=true) override;

	unsigned get_resolution_multiplier() const { return multiplier; }

	void set_resolution_multiplier(int m) { set_and_update_member(multiplier, m); }
};

}
}

#include <cgv/config/lib_end.h>
