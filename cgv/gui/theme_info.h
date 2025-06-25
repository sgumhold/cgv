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
	return cgv::media::to_hex(FIELD##_col); \
}

class CGV_API theme_info {
protected:
	int index_ = -1;

	// the spacing between user interface groups (usually a 1px wide border)
	int spacing_ = 1;
	// the scaling of user interface elements (used for DPI-adjustment); currently not supported by fltk controls
	float scaling_ = 1.0f;

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

public:
	static theme_info& instance();

	theme_info(const theme_info&) = delete;
	void operator=(const theme_info&) = delete;

	/// construct
	theme_info();
	/// destruct
	~theme_info() {}

	// general info
	void index(int index) { index_ = index; }
	int index() const { return index_; }
	bool is_dark() const;

	// layout variables
	int spacing() const { return spacing_; }
	void spacing(int i) { spacing_ = i; }

	float scaling() const { return scaling_; }
	void scaling(float f) { scaling_ = f; }

	// theme colors
	DEF_COLOR_MEMBER_METHODS(background);
	DEF_COLOR_MEMBER_METHODS(group);
	DEF_COLOR_MEMBER_METHODS(control);
	DEF_COLOR_MEMBER_METHODS(border);
	DEF_COLOR_MEMBER_METHODS(text);
	DEF_COLOR_MEMBER_METHODS(text_background);
	DEF_COLOR_MEMBER_METHODS(selection);
	DEF_COLOR_MEMBER_METHODS(highlight);
	DEF_COLOR_MEMBER_METHODS(warning);
	DEF_COLOR_MEMBER_METHODS(shadow);

	// calls the on_change signal to notify theme_observers upon theme changes
	void notify_observers() const;

	cgv::signal::signal<const theme_info&> on_change;
};

#undef DEF_COLOR_MEMBER_METHODS

// derive from this class to handle theme changes whenever theme_info::notify_changes is called
class CGV_API theme_observer : virtual public cgv::signal::tacker {
public:
	theme_observer() {
		cgv::gui::theme_info& theme = cgv::gui::theme_info::instance();
		cgv::signal::connect(theme.on_change, this, &theme_observer::handle_theme_change);
	}

	~theme_observer() {
		cgv::gui::theme_info& theme = cgv::gui::theme_info::instance();
		cgv::signal::disconnect(theme.on_change, this, &theme_observer::handle_theme_change);
	}

	/// override in your class to handle theme changes
	virtual void handle_theme_change(const cgv::gui::theme_info& theme) {}
};

}
}

#include <cgv/config/lib_end.h>
