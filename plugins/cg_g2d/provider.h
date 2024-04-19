#pragma once

#include <memory>

#include <cgv/gui/button.h>
#include <cgv/gui/control.h>
#include <cgv/gui/event.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv/signal/rebind.h>
#include <cgv/signal/signal.h>

#include "styles.h"

#include "widget.h"
#include "input.h"
#include "slider.h"
#include "toggle.h"
#include "valuator.h"
#include "value_input.h"

#include "g2d_bool_control.h"
#include "g2d_button.h"
#include "g2d_string_control.h"
#include "g2d_value_control.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

class CGV_API provider : cgv::gui::theme_observer {
protected:
	styles style;
	cgv::ivec2 default_control_size = { 200, 20 };

	std::vector<cg::g2d::g2d_button_ptr> buttons;

	std::vector<cgv::gui::control_ptr> controls;
	std::vector<std::shared_ptr<widget>> control_widgets;

	void init_styles();

	void handle_theme_change(const cgv::gui::theme_info& theme) override;

	template<typename T>
	void connect_to_on_set(cgv::base::base* base_ptr, cgv::gui::control_ptr control, T& value) {
		auto ptr = control.down_cast<cgv::gui::control<T>>();
		if(ptr)
			connect_copy(ptr->value_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
	}

	template<typename T, typename W>
	std::shared_ptr<W> add_value_control_void(cgv::base::base* base_ptr, const std::string& label, T& value, cgv::ivec2 position) {
		std::shared_ptr<W> control_widget;
		controls.push_back(create_value_control<W>(label, &value, cgv::type::info::type_name<T>::get_name(), cgv::g2d::irect(position, default_control_size), control_widget));
		control_widgets.push_back(control_widget);

		connect_to_on_set(base_ptr, controls.back(), value);

		return control_widget;
	}

public:
	bool init(cgv::render::context& ctx);

	void destruct(cgv::render::context& ctx);

	void clear();

	bool handle(cgv::gui::event& e, const cgv::ivec2& viewport_size, const cgv::g2d::irect& container = cgv::g2d::irect());

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs);

	cg::g2d::g2d_button_ptr add_button(const std::string& label, cgv::ivec2 position) {
		auto ptr = cg::g2d::g2d_button_ptr(new g2d_button(label, { position, default_control_size }));
		buttons.push_back(ptr);
		return ptr;
	}
	
	template<class X>
	cg::g2d::g2d_button_ptr add_button(const std::string& label, cgv::ivec2 position, const X& click_handler) {
		auto ptr = add_button(label, position);
		if(ptr)
			cgv::signal::connect_copy(ptr->click, click_handler);
		return ptr;
	}

	std::shared_ptr<toggle> add_bool_control(cgv::base::base* base_ptr, const std::string& label, bool& value, cgv::ivec2 position) {
		std::shared_ptr<toggle> control_widget;
		controls.push_back(create_bool_control(label, &value, cgv::type::info::type_name<bool>::get_name(), cgv::g2d::irect(position, default_control_size), control_widget));
		control_widgets.push_back(control_widget);

		connect_to_on_set(base_ptr, controls.back(), value);

		return control_widget;
	}

	template<typename T>
	std::shared_ptr<slider> add_slider_control(cgv::base::base* base_ptr, const std::string& label, T& value, cgv::ivec2 position) {
		return add_value_control_void<T, slider>(base_ptr, label, value, position);
	}

	template<typename T>
	std::shared_ptr<value_input> add_value_control(cgv::base::base* base_ptr, const std::string& label, T& value, cgv::ivec2 position) {
		return add_value_control_void<T, value_input>(base_ptr, label, value, position);
	}

	std::shared_ptr<input> add_string_control(cgv::base::base* base_ptr, const std::string& label, std::string& value, cgv::ivec2 position) {
		std::shared_ptr<input> control_widget;
		controls.push_back(create_string_control(label, &value, cgv::type::info::type_name<std::string>::get_name(), cgv::g2d::irect(position, default_control_size), control_widget));
		control_widgets.push_back(control_widget);

		connect_to_on_set(base_ptr, controls.back(), value);

		return control_widget;
	}
};

}
}

#include <cgv/config/lib_end.h>
