#pragma once

#include <cgv/defines/assert.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/subprovider.h>
#include <cgv/render/context.h>
#include <cgv/render/managed_frame_buffer.h>
#include <cgv/render/shader_library.h>
#include <cgv/render/view.h>
#include <cgv_gl/gl/gl_context.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API post_process_effect : public cgv::gui::subprovider {
protected:
	/// a framebuffer to draw geometry into and use as a source to apply effects
	cgv::render::managed_frame_buffer fbc_draw;
	/// shader library to manage used shaders
	cgv::render::shader_library shaders;
	/// the size of the current viewport
	vec2 viewport_size = vec2(0.0f);
	/// whether the class has been initialized
	bool is_initialized = false;
	/// whether to enable the effect
	bool enable = true;
	/// the name of the effect
	std::string name = "";

	void assert_init();

	virtual void create_gui_impl(cgv::base::base* b, cgv::gui::provider* p) override;

public:
	post_process_effect(const std::string& name) : name(name) {}

	virtual ~post_process_effect() {}

	virtual void destruct(cgv::render::context& ctx);

	virtual bool init(cgv::render::context& ctx);

	virtual bool ensure(cgv::render::context& ctx);

	virtual void reset() {}

	virtual void begin(cgv::render::context& ctx) = 0;

	virtual void end(cgv::render::context& ctx) = 0;

	vec2 get_viewport_size() const { return viewport_size; }

	bool is_enabled() const { return enable; }

	void set_enabled(bool enable) { set_and_update_member(this->enable, enable); }
};

}
}

#include <cgv/config/lib_end.h>
