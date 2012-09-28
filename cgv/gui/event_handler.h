#pragma once

#include <cgv/base/group.h>
#include <cgv/base/traverser.h>
#include "event.h"
#include "control.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// interface for all classes that want to receive events
class CGV_API event_handler : public base::traverse_policy
{
protected:
	//std::vector<abst_control>
public:
	/// default construction
	event_handler();
	/// grab the focus in all parent nodes
	bool grab_focus();
	/// overload and implement this method to handle events
	virtual bool handle(event& e) = 0;
	/// overload to stream help information to the given output stream
	virtual void stream_help(std::ostream& os) = 0;
	//! add a key control for the given property with the given options. 
	/*! This should be called in classes derived from cgv::base::group and event_handler. The group argument should 
	    be the this pointer cast to cgv::base_group. If group is not given, a dynamic_cast is performed on the this
		pointer. */
	bool add_key_control(const std::string& property, const std::string& options, cgv::base::group_ptr group = cgv::base::group_ptr());
};

	}
}

#include <cgv/config/lib_end.h>