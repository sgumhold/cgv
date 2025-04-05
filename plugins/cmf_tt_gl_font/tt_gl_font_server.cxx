#include "tt_gl_font_server.h"

using namespace cgv::media::font;

namespace cgv {
	font_ptr tt_gl_font_server::find_font(const std::string& font_name)
	{
		return cgv::find_font(font_name);
	}
	font_ptr tt_gl_font_server::find_font_by_prefix(const std::string& font_name_prefix)
	{
		return cgv::find_font_by_prefix(font_name_prefix);
	}
	void tt_gl_font_server::on_register()
	{
		register_font_server(this);
	}
	/// enumerate the names of all installed fonts
	void tt_gl_font_server::enumerate_font_names(std::vector<const char*>& font_names)
	{
		for (const auto& name : get_font_names())
			font_names.push_back(name.c_str());
	}
}

#include <cgv/base/register.h>

cgv::base::object_registration<cgv::tt_gl_font_server> tt_gl_font_serv("register tt_gl_font server");
