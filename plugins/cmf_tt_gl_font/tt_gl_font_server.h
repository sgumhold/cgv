#pragma once

#include <cgv/media/font/font_server.h>
#include <tt_gl_font/tt_gl_font.h>

#include "lib_begin.h"

namespace cgv {
	/// class to provide font server for true type fonts with OpenGL
	class CGV_API tt_gl_font_server : public cgv::media::font::font_server
	{
	public:
		/// find an installed font by name
		cgv::media::font::font_ptr find_font(const std::string& font_name);
		/// extend font_server implementation by fall back strategy
		cgv::media::font::font_ptr default_font(bool mono_space);
		///
		void on_register();
		/// enumerate the names of all installed fonts
		void enumerate_font_names(std::vector<const char*>& font_names);
	};
}

#include <cgv/config/lib_end.h>