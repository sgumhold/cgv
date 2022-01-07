#include <cgv/base/named.h>

namespace cgv {
	namespace base {

named::named(const std::string& _name) : name(_name)
{
}
const std::string& named::get_name() const
{
	return name;
}
void named::set_name(const std::string& _name)
{
	name = _name;
}
named_ptr named::get_named()
{
	return named_ptr(this);
}
const_named_ptr named::get_named_const()
{
	return named_ptr(this);
}
std::string named::get_type_name() const
{
	return "named";
}

	}
}
