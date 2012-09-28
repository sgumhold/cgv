#include "event_handler.h"
#include "key_control.h"
#include <cgv/base/group.h>
#include <cgv/type/info/type_name.h>
#include <cgv/signal/rebind.h>

namespace cgv {
	namespace gui {

/// default construction
event_handler::event_handler() : traverse_policy(base::TP_AUTO_FOCUS+base::TP_STOP_ON_SUCCESS)
{
}

/// grab the focus in all parent nodes
bool event_handler::grab_focus()
{
	return base::grab_focus(this);
}

/// grab the focus in all parent nodes
bool event_handler::add_key_control(const std::string& property, const std::string& options, cgv::base::group_ptr gp)
{
	cgv::base::group* g;
	if (gp.empty())
		g = dynamic_cast<cgv::base::group*>(this);
	else
		g = &(*gp);
	if (!g)
		return false;

	std::string type_name;
	void* property_ptr = g->find_member_ptr(property, &type_name);
	if (!property_ptr)
		return false;
	if (type_name == cgv::type::info::type_name<float>::get_name()) {
		cgv::data::ref_ptr<control<float> > cp = new key_control<float>(property+" key_control", *((float*)property_ptr), options);
		g->append_child(cp);
		connect_copy(cp->value_change, cgv::signal::rebind(static_cast<cgv::base::base*>(g), &cgv::base::base::on_set, property_ptr));
	}
	else if (type_name == cgv::type::info::type_name<double>::get_name()) {
		cgv::data::ref_ptr<control<double> > cp = new key_control<double>(property+" key_control", *((double*)property_ptr), options);
		g->append_child(cp);
		connect_copy(cp->value_change, cgv::signal::rebind(static_cast<cgv::base::base*>(g), &cgv::base::base::on_set, property_ptr));
	}
	else if (type_name == cgv::type::info::type_name<bool>::get_name()) {
		cgv::data::ref_ptr<control<bool> > cp = new key_control<bool>(property+" key_control", *((bool*)property_ptr), options);
		g->append_child(cp);
		connect_copy(cp->value_change, cgv::signal::rebind(static_cast<cgv::base::base*>(g), &cgv::base::base::on_set, property_ptr));
	}
	else
		return false;
	return true;
}
	}
}