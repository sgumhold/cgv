#pragma once

#include <cgv/render/render_types.h>
#include <cgv/media/font/font.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

struct label
{
	std::string text;
	cgv::render::render_types::rgba text_color;
	cgv::render::render_types::rgba background_color;
	int border_x, border_y;
	int width, height;
	cgv::media::font::font_face_ptr font_face;
	float font_size;
};
	}
}

#include <cgv/config/lib_end.h>