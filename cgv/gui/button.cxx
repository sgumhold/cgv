#include "button.h"

namespace cgv {
	namespace gui {

// check whether the value represented by this element is pointing to the passed pointer
button::button(const std::string& name) : cgv::base::node(name)
{
}

// overload to return the type name of this object
std::string button::get_type_name() const
{
	return "button";
}


	}
}
