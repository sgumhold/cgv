#pragma once

#include <cgv_post/post_process_effect.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API outline : public post_process_effect {
protected:
	/// strength scale of the outline
	float strength = 1.0f;
	/// depth difference threshold
	float threshold = 0.025f;

	void create_gui_impl(cgv::base::base* b, cgv::gui::provider* p);

public:
	outline() : post_process_effect("Outline") {}

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	bool ensure(cgv::render::context& ctx);

	void begin(cgv::render::context& ctx);

	void end(cgv::render::context& ctx);

	/// accessors
	float get_strength() const { return strength; }

	void set_strength(float s) { set_and_update_member(strength, s); }

	float get_threshold() const { return threshold; }

	void set_threshold(float t) { set_and_update_member(threshold, t); }
};

}
}

#include <cgv/config/lib_end.h>
