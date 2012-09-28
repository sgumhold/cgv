#include "reflection_handler.h"

#include <vector>
#include <cgv/type/info/type_name.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/scan_enum.h>
#include <cgv/type/traits/method_pointer.h>
#include <cgv/type/info/type_id.h>

namespace cgv {
	namespace reflect {

/// declare virtual destructor
reflection_handler::~reflection_handler()
{
}


/// check whether a group kind is of array or vector kind
bool reflection_handler::is_array_kind(GroupKind gk) 
{
	return gk == GK_VECTOR || gk == GK_ARRAY; 
}

int reflection_handler::reflect_group_begin(GroupKind group_kind, 
								const std::string& group_name, 
								void* group_ptr,
								abst_reflection_traits* rt,
								unsigned grp_size) 
{
	return GT_COMPLETE; 
}

void reflection_handler::reflect_group_end(GroupKind group_kind)
{
}

/// return the group traversals as a string
std::string reflection_handler::group_traversal_name(GroupTraversal gt)
{
	switch (gt) {
		case GT_TERMINATE : return "terminate";
		case GT_SKIP  : return "skip";
		case GT_COMPLETE : return "complete";
		default: return cgv::utils::to_string((int&)gt)+"-th element";
	}
}

const char* reflection_handler::group_kind_name(GroupKind gk)
{
	static const char* names[] = { "no group", "base class", "struct", "vector", "array", "pointer" };
	return names[gk];
}

reflection_handler::GroupTraversal reflection_handler::process_structural_group_begin(GroupKind gk, const std::string& member_name, GroupTraversal gt)
{
	switch (gt) {
	case GT_COMPLETE :  
		nesting_info_stack.push_back(nesting_info(gk, &member_name));
	case GT_TERMINATE : 
	case GT_SKIP : 
		return gt;
	default:
		std::cerr << "group traversal " << group_traversal_name(gt) << " not allowed for " << member_name << " on group of kind '" << group_kind_name(gk) << "'!" << std::endl;
		return GT_TERMINATE;
	}
}

bool reflection_handler::group_end(GroupKind gk)
{
	nesting_info_stack.pop_back();
	reflect_group_end(gk);
	return true;
}

int reflection_handler::reflect_array_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* rt, unsigned grp_size)
{
	int grp_tra = reflect_group_begin(group_kind, group_name, group_ptr, rt, grp_size);
	if (grp_tra == GT_TERMINATE || grp_tra == GT_SKIP)
		return grp_tra;

	if (grp_tra == GT_COMPLETE)
		nesting_info_stack.push_back(nesting_info(group_kind, &group_name));
	else {
		if (grp_tra >= 0 && grp_tra < (int)grp_size)
			nesting_info_stack.push_back(nesting_info(group_kind, &group_name, grp_tra));
		else {
			std::cerr << "invalid access to " << group_kind_name(group_kind) << " " << group_name << ": index " 
				      << grp_tra << " out of range [0," << grp_size << "[" << std::endl;
			return GT_TERMINATE;
		}
	}
	return grp_tra;
}

	}
}
