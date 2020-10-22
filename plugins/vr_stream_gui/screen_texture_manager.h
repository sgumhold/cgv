#pragma once

#include <memory>

#include <cgv_gl/gl/gl.h>
#include <cgv/render/texture.h>
#include <cgv/render/context.h>

#include "screen_grabber.h"

using namespace cgv::render;
using namespace cgv::data;

namespace trajectory {
namespace util {
	class screen_texture_manager {
	  private:
		std::unique_ptr<screen_grabber> grabber;
		std::shared_ptr<texture> tex;
		data_format df;
		data_view dv;

		// TODO: https://bcmpinc.wordpress.com/2015/10/07/copy-a-texture-to-screen/

	  public:
		screen_texture_manager();
		void clear(context &ctx);

		void recreate_texture(context &ctx);
		void update(context &ctx);

		void start_grabbing_async();
		void stop_grabbing_async();

		void set_fps_limit(float f);

		std::shared_ptr<texture> get_texture() const;
	};
} // namespace util
} // namespace trajectory
