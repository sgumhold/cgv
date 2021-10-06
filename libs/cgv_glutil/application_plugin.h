#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>

#include "overlay.h"

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API application_plugin :
	public cgv::base::node,			// derive from node to integrate into global tree structure and to store a name
	public cgv::render::drawable,	// derive from drawable for being able to render
	public cgv::gui::provider,		// derive from provider to obtain a GUI tab
	public cgv::gui::event_handler	// derive from event handler to be able to directly react to user interaction
{
protected:
	std::vector<overlay*> overlays;
	overlay* last_blocking_overlay_ptr;
	overlay* blocking_overlay_ptr;

public:
	// TODO: unregister and destruct overlays when application plugin gets destroyed
	application_plugin(const std::string& name);

	template<class T>
	T* register_overlay(const std::string& name) {
		static_assert(std::is_base_of<overlay, T>::value, "T must inherit from overlay");
		T* ptr = new T();
		ptr->set_name(name);
		ptr->set_parent_handler(this);
		cgv::base::register_object(base_ptr(ptr));
		overlays.push_back(ptr);
		return ptr;
	}

	virtual bool handle(cgv::gui::event& e) final;

	virtual bool handle_event(cgv::gui::event& e) = 0;
};

}
}

#include <cgv/config/lib_end.h>
