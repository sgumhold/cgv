#include "reflect_enum.h"

#include <cgv/utils/convert.h>
#include <cgv/utils/scan_enum.h>

namespace cgv {
	namespace reflect {

void abst_enum_reflection_traits::parse_declarations() 
{
	cgv::utils::parse_enum_declarations(declarations(), ref_names(), ref_values());
}

bool abst_enum_reflection_traits::has_string_conversions() const 
{
	return true; 
}

bool abst_enum_reflection_traits::set_from_string(void* member_ptr, const std::string& str_val) 
{
	unsigned i = cgv::utils::find_enum_index(str_val, ref_names());
	if (i == -1)
		return false;
	*static_cast<int*>(member_ptr) = ref_values()[i];
	return true;
}

void abst_enum_reflection_traits::get_to_string(const void* member_ptr, std::string& str_val) 
{
	unsigned i = cgv::utils::find_enum_index(*static_cast<const int*>(member_ptr), ref_values());
	if (i != -1)
		str_val = to_string(ref_names()[i]);
	else
		str_val = "UNDEF";
}

bool abst_enum_reflection_traits::has_enum_interface() const 
{
	return true; 
}
unsigned abst_enum_reflection_traits::get_nr_enum_items() const 
{
	return const_cast<abst_enum_reflection_traits*>(this)->ref_values().size(); 
}
std::string abst_enum_reflection_traits::get_enum_name(unsigned i) const 
{
	return to_string(const_cast<abst_enum_reflection_traits*>(this)->ref_names()[i]); 
}
int abst_enum_reflection_traits::get_enum_value(unsigned i) const 
{
	return const_cast<abst_enum_reflection_traits*>(this)->ref_values()[i]; 
}

	}
}
