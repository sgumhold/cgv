#pragma once

#if defined(WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <vector>
#include <chrono>
#include <memory>

#include <thread>
#include <mutex>
#include <atomic>

#include <cgv/data/data_format.h>

namespace trajectory {
namespace util {
	class screen_grabber {
	  public:
		struct BGRA8 {
			unsigned char b;
			unsigned char g;
			unsigned char r;
			unsigned char a;
		};

	  private:
		// bitmap members
		int screen_width = 0;
		int screen_height = 0;
		HDC device_context_global;
		HDC device_context_capture;
		HBITMAP bitmap;
		BITMAPINFO bitmap_info;
		std::vector<BGRA8> pixels;

		// thread members
		std::thread worker;
		std::atomic<bool> worker_running = false;
		std::atomic<bool> worker_stop_flag;
		std::atomic<bool> new_frame_available = false;
		std::mutex internal_mutex;
		std::shared_ptr<std::mutex> pixel_mutex;

		// other members
		float last_grab_time = 0.0f; // float milliseconds
		std::chrono::high_resolution_clock::time_point last_grab_timepoint;
		float fps_limit = 60.0f;

	  public:
		screen_grabber();
		~screen_grabber();

		// async grab screen bitmap and store in interal pixel member
		// call get_pixels() after this to get raw image
		bool grab();

		// grab screen and store bitmap in provided vector
		bool grab(std::vector<BGRA8> &data);

		// grab screen and store bitmap in pointer
		// make sure pointer has width * height * sizeof(BGRA8) allocated memory
		bool grab(void *ptr);

		// get BGRA pixels
		// make sure to use mutex, otherwise undefined behaviour
		std::vector<BGRA8> get_pixels();
		std::vector<BGRA8> &get_pixels_ref();

		void start_worker();
		void stop_worker();

		// before reading pixels, aquire lock on this mutex
		std::shared_ptr<std::mutex> get_pixel_mutex() const;

		int get_screen_width() const;
		int get_screen_height() const;

		float get_last_grab_time() const;

		bool is_running() const;

		bool is_new_frame_available() const;
		// external sync so multiple units can grab frame before resetting flag
		void reset_new_frame_available();
		// convenience functions that combines is- and reset_new_frame_available
		bool get_and_reset_new_frame_available();

		void set_fps_limit(float f);

	  private:
		bool grab_impl(LPVOID ptr);

		void work();
	};
} // namespace util
} // namespace trajectory

#endif