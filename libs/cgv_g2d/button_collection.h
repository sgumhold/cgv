#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv/render/render_types.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>
#include <cgv_g2d/rect.h>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class CGV_API button_collection : cgv::gui::theme_observer {
protected:
	bool state_out_of_date = true;
	bool style_out_of_date = false;

	cgv::render::shader_library shaders;

	struct button {
		irect rect;
		std::string label;
		cgv::render::TextAlignment label_alignment;
		std::function<void(const std::string&)> callback;
	};

	std::vector<button> buttons;
	msdf_text_geometry labels;

	cgv::render::ivec2 default_button_size;
	std::function<void(const std::string&)> default_callback;

	text2d_style label_style;

	shape2d_style btn_style;
	cgv::render::rgba button_color;
	cgv::render::rgba button_press_color;

	size_t pressed_button_index = static_cast<size_t>(-1);
	bool hovers_pressed_button = false;

	void init_styles(cgv::render::context& ctx);

	void handle_theme_change(const cgv::gui::theme_info& theme) override;

	void create_labels();

	void move_label(int offset);

public:

	button_collection();

	void destruct(cgv::render::context& ctx);

	void clear();

	bool init(cgv::render::context& ctx);

	bool handle(cgv::gui::event& e, const cgv::render::ivec2& viewport_size, const irect& container = irect());

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs);

	size_t size() const { return buttons.size(); }

	void set_default_button_size(const cgv::render::ivec2& size) { default_button_size = size; }

	cgv::render::ivec2 get_default_button_size() const { return default_button_size; }

	void set_default_callback(std::function<void(const std::string&)> func) { default_callback = func; }

	void add(const std::string& label, const cgv::render::ivec2& pos, std::function<void(const std::string&)> callback = nullptr);

	void add(const std::string& label, const cgv::render::ivec2& pos, cgv::render::TextAlignment label_alignment, std::function<void(const std::string&)> callback = nullptr);

	void add(const std::string& label, const irect& rect, std::function<void(const std::string&)> callback = nullptr);

	void add(const std::string& label, const irect& rect, cgv::render::TextAlignment label_alignment, std::function<void(const std::string&)> callback = nullptr);
};

}
}

#include <cgv/config/lib_end.h>
