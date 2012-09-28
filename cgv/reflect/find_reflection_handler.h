#pragma once

#include "reflection_handler.h"
#include <cgv/utils/token.h>

#include "lib_begin.h"

namespace cgv {
	namespace reflect {

/** The cgv::reflect::find_reflection_hander steers traversal to a specific member and calls the virtual function
    process_member() that can be overloaded by derived classes such as cgv::reflect::get_reflection_handler.
	The target member is specified in the constructor by a string. This can be either directly the name of the member
	or recursively use the dot and array operators to access members of members or elements of array members. A typical 
	example could be \c complex_pnt_arr[4].x.re. Here the real part of the x coordinate of the 5th (indices start with 0)
	point is addressed.
*/
class CGV_API find_reflection_handler : public reflection_handler
{
private:
	std::vector<cgv::utils::token> tokens;
	unsigned token_idx;
	bool at_end() const;
	bool step_if_matches(const std::string& name);

protected:
	const std::string& target;
	bool found;
	bool valid;
	std::string member_name;
	abst_reflection_traits* rt;
	void* member_ptr;
	GroupKind group_kind;
	unsigned grp_size;

public:
	/// construct from target
	find_reflection_handler(const std::string& _target);
	/// 
	~find_reflection_handler();
	/// return whether target has been found
	bool found_target() const;
	/// return whether a valid target has been found. The semantic of being valid is defined in derived classes.
	bool found_valid_target() const;
	/// in case a valid target has been found, return a pointer to the member
	void* get_member_ptr() const;
	/// in case a valid target has been found, return the name of the member
	const std::string& get_member_name() const;
	/// in case a valid target has been found, return a point to the reflection traits describing the type of the member
	abst_reflection_traits* get_reflection_traits() const;
	/// virtual method that is overloaded by derived classes to handle the target member
	virtual void process_member_void(const std::string& member_name, void* member_ptr, 
								     abst_reflection_traits* rt, GroupKind group_kind = GK_NO_GROUP, unsigned grp_size = -1);
	/// overload to navigate grouped reflection information
	int reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, 
						    abst_reflection_traits* rt, unsigned grp_size);
	/// check for target during member reflection
	bool reflect_member_void(const std::string& member_name, void* member_ptr, abst_reflection_traits* rt);
	/// ignore methods
	bool reflect_method_void(const std::string& method_name, method_interface* mi_ptr,
							 abst_reflection_traits* return_traits, const std::vector<abst_reflection_traits*>& param_value_traits);
};
	}
}

#include <cgv/config/lib_end.h>