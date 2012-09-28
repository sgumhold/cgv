#include <cgv/base/named.h>

namespace cgv {
	namespace base {


/// construct from name
named::named(const std::string& _name) : name(_name)
{
}

/// return the parent node
const std::string& named::get_name() const
{
	return name;
}

/// set a new parent node
void named::set_name(const std::string& _name)
{
	name = _name;
}

/// cast upward to named
data::ref_ptr<named,true> named::get_named()
{
	return named_ptr(this);
}

/// overload to return the type name of this object
std::string named::get_type_name() const
{
	return "named";
}

	}
}
