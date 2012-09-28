#include "get_reflection_handler.h"
#include <cgv/type/variant.h>
#include <string.h>

using namespace cgv::type;

namespace cgv {
	namespace reflect {

/// construct from target, value type and pointer to value
get_reflection_handler::get_reflection_handler(const std::string& _target,
							const std::string& _value_type,
							void* _value_ptr,
							abst_reflection_traits* _value_rt) : find_reflection_handler(_target), value_ptr(_value_ptr), value_type(_value_type), value_rt(_value_rt) {}

get_reflection_handler::get_reflection_handler(const std::string& _target, void* _value_ptr, abst_reflection_traits* _value_rt) 
	: find_reflection_handler(_target), value_ptr(_value_ptr), value_rt(_value_rt) {}

///
void get_reflection_handler::process_member_void(const std::string& member_name, void* member_ptr, 
	                     abst_reflection_traits* rt, GroupKind group_kind, unsigned grp_size)
{
	find_reflection_handler::process_member_void(member_name, member_ptr, rt, group_kind, grp_size);
	valid = false;
	if (value_rt) {
		if (info::is_fundamental(value_rt->get_type_id())) {
			if (info::is_fundamental(rt->get_type_id())) {
				cgv::type::assign_variant(value_rt->get_type_name(), value_ptr, rt->get_type_name(), member_ptr);
				valid = true;
			}
			else if (info::is_fundamental(rt->get_type_id())) {
				std::string tmp_str;
				get_variant(tmp_str, rt->get_type_name(), member_ptr);
				value_rt->set_from_string(value_ptr, tmp_str);
				valid = true;
			}
		}
		if (!valid && rt->has_string_conversions() && value_rt->has_string_conversions()) {
			std::string tmp;
			rt->get_to_string(member_ptr, tmp);
			valid = value_rt->set_from_string(value_ptr, tmp);
		}
		if (!valid && rt->has_string_conversions() && value_rt->get_type_id() == info::TI_STRING) {
			rt->get_to_string(member_ptr, *static_cast<std::string*>(value_ptr));
			valid = true;
		}
	}
	else {
		if (info::is_fundamental(rt->get_type_id())) {
			cgv::type::assign_variant(value_type, value_ptr, rt->get_type_name(), member_ptr);
			valid = true;
		}
		else if (value_type == rt->get_type_name()) {
			memcpy(value_ptr, member_ptr, rt->size());
			valid = true;
		}
		else if (value_type == cgv::type::info::type_name<std::string>::get_name() && rt->has_string_conversions()) {
			rt->get_to_string(member_ptr, *static_cast<std::string*>(value_ptr));
			valid = true;
		}
	}
}

	}
}