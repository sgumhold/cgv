#pragma once

#include <cgv/media/color.h>
#include <cgv/signal/signal.h>

#include "lib_begin.h"

namespace cgv {
namespace gui {

#define DEF_COLOR_MEMBER_METHODS(FIELD) \
void FIELD(unsigned char r, unsigned char g, unsigned char b) { \
	FIELD##_col = rgb( \
		static_cast<float>(r) / 255.0f, \
		static_cast<float>(g) / 255.0f, \
		static_cast<float>(b) / 255.0f \
	); \
} \
rgb FIELD() const { \
	return FIELD##_col; \
}\
std::string FIELD##_hex() const { \
	return rgb_to_hex(FIELD##_col); \
}

class CGV_API theme_info {
protected:
	int theme_idx;

	int spacing_ = 1;

	rgb background_col;
	rgb group_col;
	rgb control_col;
	rgb border_col;
	rgb text_col;
	rgb text_background_col;
	rgb selection_col;
	rgb highlight_col;
	rgb warning_col;
	rgb shadow_col;

	static std::string char_to_hex(unsigned char c) {
		static const char hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		std::string s = "00";
		s[0] = hex_chars[(c & 0xF0) >> 4];
		s[1] = hex_chars[(c & 0x0F) >> 0];
		return s;
	}

	static std::string rgb_to_hex(const rgb& c) {
		unsigned char r = static_cast<unsigned char>(255.0f * c.R());
		unsigned char g = static_cast<unsigned char>(255.0f * c.G());
		unsigned char b = static_cast<unsigned char>(255.0f * c.B());
		std::string s = "0x" + char_to_hex(r) + char_to_hex(g) + char_to_hex(b);
		return s;
	}

public:
	static theme_info& instance();

	theme_info(const theme_info&) = delete;
	void operator=(const theme_info&) = delete;

	/// construct
	theme_info();
	/// destruct
	~theme_info() {}

	void set_index(int idx);
	int get_index() const;
	bool is_dark() const;

	int spacing() const { return spacing_; }
	void spacing(int i) { spacing_ = i; }

	DEF_COLOR_MEMBER_METHODS(background)
	DEF_COLOR_MEMBER_METHODS(group)
	DEF_COLOR_MEMBER_METHODS(control)
	DEF_COLOR_MEMBER_METHODS(border)
	DEF_COLOR_MEMBER_METHODS(text)
	DEF_COLOR_MEMBER_METHODS(text_background)
	DEF_COLOR_MEMBER_METHODS(selection)
	DEF_COLOR_MEMBER_METHODS(highlight)
	DEF_COLOR_MEMBER_METHODS(warning)
	DEF_COLOR_MEMBER_METHODS(shadow)

	cgv::signal::signal<const theme_info&> on_change;
};

#undef DEF_COLOR_MEMBER_METHODS

class CGV_API theme_observer : virtual public cgv::signal::tacker {
public:
	theme_observer() {
		cgv::gui::theme_info& theme = cgv::gui::theme_info::instance();
		cgv::signal::connect(theme.on_change, this, &theme_observer::handle_theme_change);
	}

	/// override in your class to handle theme changes
	virtual void handle_theme_change(const cgv::gui::theme_info& theme) {}
};

}
}

#include <cgv/config/lib_end.h>
