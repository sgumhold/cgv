#pragma once

#include <cgv/base/base.h>
#include "event.h"
#include "gui_group.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// %gui independent %window class
class CGV_API window : public gui_group
{
public:
	/// construct from name
	window(const std::string& name);
	/// overload to return the %type name of this object
	std::string get_type_name() const;
	/// return the group that is managing the content of the window
	virtual gui_group_ptr get_inner_group();
	/// show the %window. This needs to be called after creation to make the %window visible
	virtual void show(bool modal = false) = 0;
	/// hide the %window
	virtual void hide() = 0;
	/// dispatch a cgv event
	virtual bool dispatch_event(event& e);
};

/// ref counted pointer to &window
typedef data::ref_ptr<window> window_ptr;

/// ref counted pointer to const %window
typedef data::ref_ptr<const window> const_window_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<window>;
CGV_TEMPLATE template class CGV_API data::ref_ptr<const window>;
#endif


	}
}

#include <cgv/config/lib_end.h>
