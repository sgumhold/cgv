#include "reflection_traits.h"
#include "reflection_handler.h"

namespace cgv {
	namespace reflect {

abst_reflection_traits::~abst_reflection_traits() 
{
}

bool abst_reflection_traits::has_external_implementation() const 
{
	return true; 
}

bool abst_reflection_traits::external_implementation(reflection_handler&, void* member_ptr)
{
	return true; 
}

bool abst_reflection_traits::has_string_conversions() const 
{
	return false; 
}

bool abst_reflection_traits::set_from_string(void* member_ptr, const std::string& str_val) 
{
	return false; 
}

void abst_reflection_traits::get_to_string(const void* member_ptr, std::string& str_val)
{
}

bool abst_reflection_traits::is_enum_type() const
{
	return false;
}


bool abst_reflection_traits::has_enum_interface() const
{
	return false;
}

unsigned abst_reflection_traits::get_nr_enum_items() const
{
	return 0;
}

std::string abst_reflection_traits::get_enum_name(unsigned i) const
{
	return "";
}

int abst_reflection_traits::get_enum_value(unsigned i) const
{
	return -1;
}

reflection_traits<bool,RTK_STD_TYPE> get_reflection_traits(const bool&)
{ 
	return reflection_traits<bool,RTK_STD_TYPE>(); 
}


	}
}