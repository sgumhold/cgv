#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace os {

/// copies the given text to the clipboard
extern CGV_API bool copy_text_to_clipboard(const char* text);

/// copies an image of width w and height h given as packed rgb pixels of type unsigned char to the clipboard
extern CGV_API bool copy_rgb_image_to_clipboard(int w, int h, const unsigned char* image_buffer);

	}
}

#include <cgv/config/lib_end.h>
