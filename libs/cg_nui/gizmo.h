#pragma once

#include <cg_nui/interactable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// Abstract base class for gizmos.
class CGV_API gizmo : public cgv::nui::interactable
{
protected:
	bool is_attached = false;
public:
	gizmo(const std::string& name = "") : interactable(name) {}

	void attach() { is_attached = true; }

	void detach() { is_attached = false; }


	//@name cgv::base::base interface
	//@{
	std::string get_type_name() const override
	{
		return "gizmo";
	}
	//@}

	//@name cgv::nui::focusable interface
	//@{
	bool focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info) override
	{
		if (!is_attached)
			return false;
		return interactable::focus_change(action, rfa, demand, e, dis_info);
	}
	bool handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request) override
	{
		if (!is_attached)
			return false;
		return interactable::handle(e, dis_info, request);
	}
	//@}
};

	}
}

#include <cgv/config/lib_end.h>