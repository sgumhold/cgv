#pragma once

#include <memory>

#include <cgv/gui/control.h>
#include <cgv/gui/event.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv/render/render_types.h>
#include <cgv/signal/rebind.h>
#include <cgv/signal/signal.h>

/*
#include <cgv/gui/control.h>
#include <cgv/type/variant.h>
#include <cgv/type/info/type_id.h>
*/

#include "styles.h"

#include "control_base.h"
#include "input_control.h"
#include "slider_control.h"
#include "value_control.h"
#include "value_input_control.h"

#include "g2d_string_control.h"
#include "g2d_value_control.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

class CGV_API control_manager : cgv::gui::theme_observer {
protected:
	//cgv::render::shader_library shaders;

	styles style;
	cgv::render::ivec2 default_control_size = cgv::render::ivec2(200, 20);

	std::vector<cgv::gui::control_ptr> controls;
	std::vector<std::shared_ptr<control_base>> g2d_controls;

	std::function<void(void*)> default_on_change_callback;

	void init_styles();

	void handle_theme_change(const cgv::gui::theme_info& theme) override;

public:
	bool init(cgv::render::context& ctx);

	void destruct(cgv::render::context& ctx);

	void clear();

	// TODO: remove reference of container argument and use copy instead?
	bool handle(cgv::gui::event& e, const cgv::render::ivec2& viewport_size, const cgv::g2d::irect& container = cgv::g2d::irect());

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs);



	
	/*
	std::shared_ptr<button_control> add_button(const std::string& label, cgv::render::ivec2 position) {
		return add_control<button_control>(label, position);
	}

	std::shared_ptr<string_input_control> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, std::string& value) {
		auto ptr = add_control<string_input_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}

	std::shared_ptr<float_input_control> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, float& value) {
		auto ptr = add_control<float_input_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}

	std::shared_ptr<int_input_control> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, int& value) {
		auto ptr = add_control<int_input_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}
	*/

	/*
	std::shared_ptr<slider_control> add_slider_control(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, float& value) {
		auto ptr = add_control<slider_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}
	*/




	std::shared_ptr<input_control> add_string_control(cgv::base::base* base_ptr, const std::string& label, std::string* value_ptr, cgv::render::ivec2 position) {
		auto gl_control_ptr = new g2d_string_control(
			label, *value_ptr, cgv::g2d::irect(position, default_control_size)
		);

		cgv::data::ref_ptr<cgv::gui::control<std::string>> control_ptr = gl_control_ptr;

		if(control_ptr)
			connect_copy(control_ptr->value_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, value_ptr));

		controls.push_back(cgv::gui::control_ptr(gl_control_ptr));

		auto control_view_ptr = gl_control_ptr->get_gl_control();

		g2d_controls.push_back(control_view_ptr);
		return control_view_ptr;
	}


	//template <class B>
	std::shared_ptr<slider_control> add_slider_control(cgv::base::base* base_ptr, const std::string& label, float* value_ptr, cgv::render::ivec2 position) {
		auto gl_control_ptr = new g2d_value_control<cgv::type::flt32_type, /* B */slider_control>(
			label, *static_cast<cgv::type::flt32_type*>(value_ptr), cgv::g2d::irect(position, default_control_size)
		);

		cgv::data::ref_ptr<cgv::gui::control<float>> control_ptr = gl_control_ptr;

		if(control_ptr)
			connect_copy(control_ptr->value_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, value_ptr));

		controls.push_back(cgv::gui::control_ptr(gl_control_ptr));

		auto control_view_ptr = gl_control_ptr->get_gl_control();

		g2d_controls.push_back(control_view_ptr);
		return control_view_ptr;
	}

	std::shared_ptr<value_input_control> add_value_control(cgv::base::base* base_ptr, const std::string& label, float* value_ptr, cgv::render::ivec2 position) {
		auto gl_control_ptr = new g2d_value_control<cgv::type::flt32_type, value_input_control>(
			label, *static_cast<cgv::type::flt32_type*>(value_ptr), cgv::g2d::irect(position, default_control_size)
		);

		cgv::data::ref_ptr<cgv::gui::control<float>> control_ptr = gl_control_ptr;

		if(control_ptr)
			connect_copy(control_ptr->value_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, value_ptr));

		controls.push_back(cgv::gui::control_ptr(gl_control_ptr));

		auto control_view_ptr = gl_control_ptr->get_gl_control();

		g2d_controls.push_back(control_view_ptr);
		return control_view_ptr;
	}
};

}
}

#include <cgv/config/lib_end.h>
