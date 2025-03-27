#pragma once
#include <string>
#include <vector>
#include "lib_begin.h"

namespace cgv {
	namespace os {

/// copies the given text to the clipboard
extern CGV_API bool copy_text_to_clipboard(const char* text);
/// copies an image of width w and height h given as packed rgb pixels of type unsigned char to the clipboard
extern CGV_API bool copy_rgb_image_to_clipboard(int w, int h, const unsigned char* image_buffer);
/// if clipboard contains text (return true), copy text to \c text and optionally clear clipboard
extern CGV_API bool get_text_from_clipboard(std::string& text, bool clear_clipboard = false);
/// if clipboard contains image (return true), copy image dims to \c w, \c h and pixel data in rgb24 format to \c data; optionally clear clipboard
extern CGV_API bool get_rgb_image_from_clipboard(int& w, int& h, std::vector<uint8_t>& data, bool clear_clipboard = false);

	}
}

#include <cgv/config/lib_end.h>
