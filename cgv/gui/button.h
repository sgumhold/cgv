#pragma once

#include <cgv/base/node.h>
#include <cgv/signal/signal.h>
#include <cgv/data/ref_ptr.h>
#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// %gui independent %button class that provides a click signal
class CGV_API button : public base::node
{
public:
	/// construct from name
	button(const std::string& name = "");
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// this signal is sent when the user presses the %button
	cgv::signal::signal<button&> click;
};

/// ref counted pointer to button
typedef data::ref_ptr<button> button_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<button>;
#endif


	}
}

#include <cgv/config/lib_end.h>
