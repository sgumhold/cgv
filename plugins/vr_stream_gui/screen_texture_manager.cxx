#include <iostream>

#include "screen_texture_manager.h"

namespace trajectory {
namespace util {

	screen_texture_manager::screen_texture_manager()
	{
		tex = std::make_shared<texture>("uint8[R,G,B,A]", TF_LINEAR,
		                                TF_LINEAR_MIPMAP_LINEAR, TW_CLAMP_TO_EDGE,
		                                TW_CLAMP_TO_EDGE);
		tex->set_min_filter(TF_ANISOTROP, 16.0f);
		tex->set_border_color(0.f, 0.f, 0.f);

		grabber = std::make_unique<screen_grabber>(); // grabs once at initialization

		df = data_format(grabber->get_screen_width(), grabber->get_screen_height(),
		                 cgv::type::info::TI_UINT8, CF_BGRA);
		dv = data_view(&df, grabber->get_pixels_ref().data());
	}

	void screen_texture_manager::clear(context &ctx) { tex->destruct(ctx); }

	void screen_texture_manager::recreate_texture(context &ctx)
	{
		if (tex->is_created()) 
			tex->destruct(ctx);
		grabber->grab();
		tex->create(ctx, dv);
	}

	//#define PRINT_TIME

	void screen_texture_manager::update(context &ctx)
	{
		if (!tex->is_created()) {
			std::cerr << "screen blit texture not created" << std::endl;
			return;
		};

		if (grabber->is_running() && grabber->get_and_reset_new_frame_available()) {
			{
				std::lock_guard<std::mutex> lock(*grabber->get_pixel_mutex());

#ifdef PRINT_TIME
				auto start = std::chrono::high_resolution_clock::now();
#endif

				tex->replace(ctx, 0, 0, dv);

#ifdef PRINT_TIME
				auto end = std::chrono::high_resolution_clock::now();
				auto t = std::chrono::duration_cast<std::chrono::duration<float>>(
				             (end - start))
				             .count() *
				         1000.0f; // to milliseconds

				std::cout << "last grab: " << grabber->get_last_grab_time() << "ms "
				          << "tex upload: " << t << "ms" << std::endl;
#endif
			}
		}
	}

	void screen_texture_manager::start_grabbing_async() { grabber->start_worker(); }

	void screen_texture_manager::stop_grabbing_async() { grabber->stop_worker(); }

	void screen_texture_manager::set_fps_limit(float f) { grabber->set_fps_limit(f); }

	std::shared_ptr<texture> screen_texture_manager::get_texture() const { return tex; }

} // namespace util
} // namespace trajectory