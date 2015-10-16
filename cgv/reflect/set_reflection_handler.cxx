#include "set_reflection_handler.h"
#include <cgv/type/variant.h>
#include <cgv/type/info/type_id.h>
#include <string.h>

using namespace cgv::type;

namespace cgv {
	namespace reflect {

/// this should return true
bool set_reflection_handler::is_creative() const
{
	return true;
}

/// construct from target, value type, pointer to value and optionally reflection_traits
set_reflection_handler::set_reflection_handler(const std::string& _target, const std::string& _value_type,
						const void* _value_ptr, abst_reflection_traits* _value_rt)
						: find_reflection_handler(_target), value_type(_value_type), value_ptr(_value_ptr), value_rt(_value_rt) {}

set_reflection_handler::set_reflection_handler(const std::string& _target, const void* _value_ptr, abst_reflection_traits* _value_rt) 
	: find_reflection_handler(_target), value_ptr(_value_ptr), value_rt(_value_rt) {}

///
void set_reflection_handler::process_member_void(const std::string& member_name, void* member_ptr, 
	                     abst_reflection_traits* rt, GroupKind group_kind, unsigned grp_size)
{
	find_reflection_handler::process_member_void(member_name, member_ptr, rt, group_kind, grp_size);
	valid = false;
	if (value_rt) {
		if (info::is_fundamental(rt->get_type_id())) {
			if (info::is_fundamental(value_rt->get_type_id())) {
				cgv::type::assign_variant(rt->get_type_name(), member_ptr, value_rt->get_type_name(), value_ptr);
				valid = true;
			}
			else if (value_rt->has_string_conversions()) {
				std::string tmp_str;
				value_rt->get_to_string(value_ptr, tmp_str);
				set_variant(tmp_str, rt->get_type_name(), member_ptr);
				valid = true;
			}

		}
		if (!valid && rt->has_string_conversions() && value_rt->has_string_conversions()) {
			std::string tmp;
			value_rt->get_to_string(value_ptr, tmp);
			valid = rt->set_from_string(member_ptr, tmp);
		}
		if (!valid && rt->has_string_conversions() && value_rt->get_type_id() == info::TI_STRING) {
			valid = rt->set_from_string(member_ptr, *static_cast<const std::string*>(value_ptr));
		}
	}
	else {
		if (info::is_fundamental(rt->get_type_id())) {
			cgv::type::assign_variant(rt->get_type_name(), member_ptr, value_type, value_ptr);
			valid = true;
		}
		else if (value_type == rt->get_type_name()) {
			memcpy(member_ptr, value_ptr, rt->size());
			valid = true;
		}
		else if (value_type == "string" && rt->has_string_conversions()) {
			valid = rt->set_from_string(member_ptr, *static_cast<const std::string*>(value_ptr));
		}
	}
}

	}
}