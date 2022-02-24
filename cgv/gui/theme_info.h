#pragma once
#include <cgv/media/color.h>

#include "lib_begin.h"

namespace cgv {
namespace gui {

/// declare rgb color type
typedef cgv::media::color<float, cgv::media::RGB> rgb;

#define col_functions(FIELD) \
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
std::string FIELD##_hex() { \
	return rgb_to_hex(FIELD##_col); \
}

class CGV_API theme_info {
protected:
	rgb background_col;
	rgb group_col;
	rgb control_col;
	rgb border_col;
	rgb selection_col;
	rgb highlight_col;
	rgb warning_col;

	std::string char_to_hex(unsigned char c) {
		static const char hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		std::string s = "00";
		s[0] = hex_chars[(c & 0xF0) >> 4];
		s[1] = hex_chars[(c & 0x0F) >> 0];
		return s;
	}

	std::string rgb_to_hex(const rgb& c) {
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

	col_functions(background);
	col_functions(group);
	col_functions(control);
	col_functions(border);
	col_functions(selection);
	col_functions(highlight);
	col_functions(warning);
};

#undef col_functions

}
}

#include <cgv/config/lib_end.h>
