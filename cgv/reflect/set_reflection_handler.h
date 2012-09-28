#pragma once

#include "find_reflection_handler.h"
#include <cgv/config/cpp_version.h>

#include "lib_begin.h"

namespace cgv {
	namespace reflect {

class CGV_API set_reflection_handler : public find_reflection_handler
{
protected:
	const void* value_ptr;
	std::string value_type;
	abst_reflection_traits* value_rt;
public:
	/// construct from target, value type, pointer to value and optionally reflection_traits
	set_reflection_handler(const std::string& _target,
						   const std::string& _value_type,
						   const void* _value_ptr,
						   abst_reflection_traits* _value_rt = 0);
	/// construct from target, pointer to value and reflection traits
	set_reflection_handler(const std::string& _target, const void* _value_ptr, abst_reflection_traits* _value_rt);
	///
	void process_member_void(const std::string& member_name, void* member_ptr, 
						     abst_reflection_traits* rt, GroupKind group_kind, unsigned grp_size);
};

#ifdef CPP11
/// call this function to set a variable inside a reflected instance from the given value
template <typename T, typename Q>
	bool set_member(T& variable, const std::string& target, const Q& value) {
			reflection_traits_info<Q>::traits_type rt_value;
#else
namespace compatibility {
	template <typename T, typename Q, typename RQ>
	bool set_member_impl(T& variable, const std::string& target, const Q& value, const RQ&)
	{
		RQ rt_value;
#endif
		set_reflection_handler srh(target, &value, &rt_value);
		srh.reflect_member("", variable);
		return srh.found_target();
	}
#ifndef CPP11
	template <bool use_get, typename T, typename Q>
	struct set_member_dispatch {           static bool set_member(T& variable, const std::string& target, const Q& value) { 
		return set_member_impl(variable, target, value, reflection_traits<Q>()); } };
	template <typename T, typename Q>
	struct set_member_dispatch<true,T,Q> { static bool set_member(T& variable, const std::string& target, const Q& value) { 
		return set_member_impl(variable, target, value, get_reflection_traits(value)); } };
}

template <typename T, typename Q>
bool set_member(T& variable, const std::string& target, const Q& value)
{
	return compatibility::set_member_dispatch<reflection_traits_info<Q>::use_get,T,Q>::set_member(variable, target, value);
}
#endif

	}
}

#include <cgv/config/lib_end.h>