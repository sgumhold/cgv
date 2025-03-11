#pragma once

#include <memory>

#include <cgv_gl/gl/gl.h>
#include <cgv/render/texture.h>
#include <cgv/render/context.h>

#include "screen_grabber.h"

namespace trajectory {
namespace util {
	class screen_texture_manager {
	  private:
		std::unique_ptr<screen_grabber> grabber;
		std::shared_ptr<cgv::render::texture> tex;
		cgv::data::data_format df;
		cgv::data::data_view dv;

		// TODO: https://bcmpinc.wordpress.com/2015/10/07/copy-a-texture-to-screen/

	  public:
		screen_texture_manager();
		void clear(cgv::render::context &ctx);

		void recreate_texture(cgv::render::context &ctx);
		void update(cgv::render::context &ctx);

		void start_grabbing_async();
		void stop_grabbing_async();

		void set_fps_limit(float f);

		std::shared_ptr<cgv::render::texture> get_texture() const;
	};
} // namespace util
} // namespace trajectory
