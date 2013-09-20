#pragma once

#include "find_reflection_handler.h"
#include <cgv/config/cpp_version.h>

#include "lib_begin.h"

namespace cgv {
	namespace reflect {

/** provides access to a member variable of an instance. Use the cgv::reflect::get_member() function for simplest usage of this class. */
class CGV_API get_reflection_handler : public find_reflection_handler
{
protected:
	void* value_ptr;
	std::string value_type;
	abst_reflection_traits* value_rt;
public:
	/// construct from target, value type and pointer to value
	get_reflection_handler(const std::string& _target,
								const std::string& _value_type,
								void* _value_ptr,
								abst_reflection_traits* _value_rt = 0);
	/// construct from target, pointer to value and reflection_traits
	get_reflection_handler(const std::string& _target, void* _value_ptr, abst_reflection_traits* _value_rt);
	/// copy value of member to external value pointer
	void process_member_void(const std::string& member_name, void* member_ptr, 
						     abst_reflection_traits* rt, GroupKind group_kind, unsigned grp_size);
};

#ifdef REFLECT_TRAITS_WITH_DECLTYPE
//! uses cgv::reflect::get_reflection_handler to copy the value of a member from an instance
/*! \param[in]  variable Instance whoes member should be copied
    \param[in]  target   Specification of member (see cgv::reflect::find_reflection_handler for details)
	\param[out] value    Copy member into to this location.
*/
template <typename T, typename Q>
	bool get_member(T& variable, const std::string& target, Q& value) {
			reflection_traits_info<Q>::traits_type rt_value;
#else
namespace compatibility {
	template <typename T, typename Q, typename RQ>
	bool get_member_impl(T& variable, const std::string& target, Q& value, RQ&) {
		RQ rt_value;
#endif
		get_reflection_handler grh(target, &value, &rt_value);
		grh.reflect_member("", variable);
		return grh.found_valid_target();
	}
#ifndef REFLECT_TRAITS_WITH_DECLTYPE
	template <bool use_get, typename T, typename Q>
	struct get_member_dispatch {           static bool get_member(T& variable, const std::string& target, Q& value) { 
		return get_member_impl(variable, target, value, reflection_traits<Q>()); } };
	template <typename T, typename Q>
	struct get_member_dispatch<true,T,Q> { static bool get_member(T& variable, const std::string& target, Q& value) { 
		return get_member_impl(variable, target, value, get_reflection_traits(value)); } };
}

//! uses cgv::reflect::get_reflection_handler to copy the value of a member from an instance
/*! \param[in]  variable Instance whoes member should be copied
    \param[in]  target   Specification of member (see cgv::reflect::find_reflection_handler for details)
	\param[out] value    Copy member into to this location.
*/
template <typename T, typename Q>
bool get_member(T& variable, const std::string& target, Q& value)
{
	return compatibility::get_member_dispatch<reflection_traits_info<Q>::use_get,T,Q>::get_member(variable, target, value);
}
#endif
	}
}

#include <cgv/config/lib_end.h>