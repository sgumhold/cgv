#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>

#include "on_set_evaluator.h"
#include "overlay.h"

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API application_plugin :
	public cgv::base::group,		// derive from group to integrate into global tree structure and allow registration of child nodes
	public cgv::render::drawable,	// derive from drawable for being able to render
	public cgv::gui::provider,		// derive from provider to obtain a GUI tab
	public cgv::gui::event_handler	// derive from event handler to be able to directly react to user interaction
{
protected:
	cgv::render::view* view_ptr;

	std::vector<overlay_ptr> overlays;
	overlay_ptr last_blocking_overlay_ptr;
	overlay_ptr blocking_overlay_ptr;

public:
	// TODO: unregister and destruct overlays when application plugin gets destroyed
	application_plugin(const std::string& name);

	template<class T>
	cgv::data::ref_ptr<T> register_overlay(const std::string& name) {
		static_assert(std::is_base_of<overlay, T>::value, "T must inherit from overlay");
		cgv::data::ref_ptr<T> ptr(new T());
		ptr->set_name(name);
		cgv::base::group::append_child(ptr);
		overlays.push_back(ptr);
		return ptr;
	}

	virtual bool handle(cgv::gui::event& e) final;

	virtual bool handle_event(cgv::gui::event& e) = 0;

	virtual void on_set(void* member_ptr);

	virtual void on_set(const on_set_evaluator& m) {}

	virtual bool initialize_view_ptr() final;
};

}
}

#include <cgv/config/lib_end.h>
