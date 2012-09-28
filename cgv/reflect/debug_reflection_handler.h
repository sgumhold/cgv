#pragma once

#include "find_reflection_handler.h"

#include "lib_begin.h"

namespace cgv {
	namespace reflect {

/** the debug reflection handler generates a string in the member \c output that
    contains a complete description of the reflected instance */
class CGV_API debug_reflection_handler : public reflection_handler
{
private:
	std::string tab;
protected:
	std::string extend_name(const std::string& name) const;
public:
	/// contains the description in form of a string
	std::string output;
	/// 
	int reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* rt, unsigned grp_size);
	/// 
	void reflect_group_end(GroupKind group_kind);
	///
	bool reflect_member_void(const std::string& member_name, void* member_ptr, abst_reflection_traits* rt);
	/// empty implementation
	bool reflect_method_void(const std::string& method_name, method_interface* mi_ptr,
							 abst_reflection_traits* return_traits, const std::vector<abst_reflection_traits*>& param_value_traits);
};
	}
}

#include <cgv/config/lib_end.h>