#include <iostream>
#include <string>
#include "screen_grabber.h"

// https://stackoverflow.com/questions/26233848/c-read-pixels-with-getdibits

namespace trajectory {
namespace util {
	
	BOOL CALLBACK EnumWindowsProc(_In_ HWND hWnd, _In_ LPARAM lParam)
	{
		char title_buffer[1024];
		int length = GetWindowTextA(hWnd, title_buffer, 1024);
		if (length > 0) {
			std::string title(title_buffer, length);

			WINDOWINFO wi;
			if (GetWindowInfo(hWnd, &wi)) {
				const RECT& r = wi.rcWindow;
				if (r.bottom - r.top > 0 && r.right - r.left > 0) {
					std::cout << "<" << title << ">";
					if (wi.dwWindowStatus != 0)
						std::cout << "*";
					std::cout << " window:" << r.left << "," << r.top << ":" << r.right - r.left << "x" << r.bottom - r.top;
					RECT& R = wi.rcClient;
					std::cout << " client:" << R.left << "," << R.top << ":" << R.right - R.left << "x" << R.bottom - R.top;
					HDC dc = GetDC(hWnd);
					if (dc) {
						std::cout << " got dc";
					}
					std::cout << std::endl;

				}
			}
		}
		return TRUE;
	}
	void enum_windows()
	{
		EnumWindows(&EnumWindowsProc, 0);
	}

	screen_grabber::screen_grabber()
	{
		enum_windows();
		pixel_mutex = std::make_shared<std::mutex>();

		screen_width = GetSystemMetrics(SM_CXSCREEN);
		screen_height = GetSystemMetrics(SM_CYSCREEN);

		device_context_global = GetDC(NULL);
		device_context_capture = CreateCompatibleDC(device_context_global);
		bitmap =
		    CreateCompatibleBitmap(device_context_global, screen_width, screen_height);
		bitmap_info = {0};
		bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader); // necessary

		// get bitmap_info
		if (!GetDIBits(device_context_global, bitmap, 0, 0, NULL, &bitmap_info,
		               DIB_RGB_COLORS)) {
			std::cerr << "error getting screen bitmap info" << std::endl;
		}
		else {
			pixels.resize(bitmap_info.bmiHeader.biSizeImage /
			              sizeof(screen_grabber::BGRA8));

			// set info here as previous GetDIBits() overwrites them otherwise
			bitmap_info.bmiHeader.biBitCount = 32;        // BGRA => no padding/stride
			bitmap_info.bmiHeader.biCompression = BI_RGB; // request no compression
			// can potentially be flipped -> negative height
			bitmap_info.bmiHeader.biHeight = std::abs(bitmap_info.bmiHeader.biHeight);
			bitmap_info.bmiHeader.biPlanes = 1;
		}

		grab(); // grab once to test and initialize
	}

	screen_grabber::~screen_grabber()
	{
		DeleteObject(bitmap);
		DeleteDC(device_context_capture);
		ReleaseDC(NULL, device_context_global);

		stop_worker();
	}

	bool screen_grabber::grab() { return grab_impl(static_cast<LPVOID>(pixels.data())); }

	bool screen_grabber::grab(std::vector<BGRA8> &data)
	{
		data.clear();
		data.resize(bitmap_info.bmiHeader.biSizeImage / sizeof(screen_grabber::BGRA8));
		data.shrink_to_fit();
		return grab_impl(static_cast<LPVOID>(data.data()));
	}

	bool screen_grabber::grab(void *ptr) { return grab_impl(ptr); }

	bool screen_grabber::grab_impl(LPVOID ptr)
	{
		auto start = std::chrono::high_resolution_clock::now();
		bool success = true;

		{
			std::lock_guard<std::mutex> lock_internal(internal_mutex);

			// temporarily enable bitmap in capture context
			HGDIOBJ h_old = SelectObject(device_context_capture, bitmap);
				// copy data from screen to bitmap
				success = BitBlt(device_context_capture, 0, 0, screen_width, screen_height,
								 device_context_global, 0, 0, SRCCOPY /* | CAPTUREBLT*/);
				static int cnt = 0;
				std::cout << ++cnt << std::endl;
			SelectObject(device_context_capture, h_old);

			if (success) {
				std::lock_guard<std::mutex> lock_pixel(*pixel_mutex);
				if (!GetDIBits(device_context_global, bitmap, 0,
				               bitmap_info.bmiHeader.biHeight, ptr, &bitmap_info,
				               DIB_RGB_COLORS)) {
					std::cout << "error reading screen pixels" << std::endl;
					success = false;
				}
			}
			else {
				std::cerr << "couldn't blit the screen into bitmap" << std::endl;
			}
		}

		last_grab_timepoint = std::chrono::high_resolution_clock::now();
		last_grab_time = std::chrono::duration_cast<std::chrono::duration<float>>(
		                     (last_grab_timepoint - start))
		                     .count() *
		                 1000.0f; // to milliseconds

		return success;
	}

	std::vector<screen_grabber::BGRA8> screen_grabber::get_pixels() { return pixels; }

	std::vector<screen_grabber::BGRA8> &screen_grabber::get_pixels_ref()
	{
		return pixels;
	}

	void screen_grabber::start_worker()
	{
		if (worker_running) {
			std::cout << "tried to start already running screen grab worker" << std::endl;
			return;
		}

		last_grab_timepoint = std::chrono::high_resolution_clock::now();

		worker_running = true;
		worker_stop_flag = false;
		worker = std::thread(&screen_grabber::work, this);
	}

	void screen_grabber::stop_worker()
	{
		if (!worker_running) {
			std::cout << "tried to stop non-running screen grab worker" << std::endl;
			return;
		}

		worker_running = false;
		worker_stop_flag = true;

		std::lock_guard<std::mutex> lock_internal(internal_mutex);
		std::lock_guard<std::mutex> lock_pixel(*pixel_mutex);

		if (worker.joinable()) {
			worker.join();
		}
		else {
			std::cerr << "screen grab worker not joinable!" << std::endl;
		}
	}

	void screen_grabber::work()
	{
		while (!worker_stop_flag) {
			auto since =
			    std::chrono::duration_cast<std::chrono::duration<float>>(
			        (std::chrono::high_resolution_clock::now() - last_grab_timepoint))
			        .count() *
			    1000.0f;

			if (since < 1.0f / fps_limit) {
				std::this_thread::sleep_for(
				    std::chrono::duration<float>(1.0f / fps_limit - since));

				// could be set while sleeping
				if (worker_stop_flag) return;
			}

			grab();
			new_frame_available = true;
		}
	}

	std::shared_ptr<std::mutex> screen_grabber::get_pixel_mutex() const
	{
		return pixel_mutex;
	}

	int screen_grabber::get_screen_width() const { return screen_width; }

	int screen_grabber::get_screen_height() const { return screen_height; }

	float screen_grabber::get_last_grab_time() const { return last_grab_time; }

	bool screen_grabber::is_running() const { return worker_running; }

	bool screen_grabber::is_new_frame_available() const { return new_frame_available; }

	void screen_grabber::reset_new_frame_available() { new_frame_available = false; }

	bool screen_grabber::get_and_reset_new_frame_available()
	{
		if (new_frame_available) {
			new_frame_available = false;
			return true;
		}
		return false;
	}

	void screen_grabber::set_fps_limit(float f) { fps_limit = f; }

} // namespace util
} // namespace trajectory