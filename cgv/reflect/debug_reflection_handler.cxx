#include "debug_reflection_handler.h"
#include <cgv/utils/scan.h>

using namespace cgv::type;

namespace cgv {
		namespace reflect {

std::string debug_reflection_handler::extend_name(const std::string& name) const
{
	std::string res;
	for (unsigned i=0; i<nesting_info_stack.size(); ++i) {
		res += *nesting_info_stack[i].name;
		switch (nesting_info_stack[i].group_kind) {
		case GK_BASE_CLASS :
		case GK_STRUCTURE  : 
			res += "."; 
			break;
		case GK_VECTOR :
		case GK_ARRAY :
			res += "[";
			res += cgv::utils::to_string(nesting_info_stack[i].idx) + "]";
			break;
		case GK_POINTER :
			res += "->";
			break;
		}
	}
	res += name;
	return res;
}

int debug_reflection_handler::reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* rt, unsigned grp_size) 
{ 
	output += tab;
	if (group_kind != GK_BASE_CLASS) {
		output += extend_name(group_name);
		output += ":";
	}
	output += group_kind_name(group_kind);
	switch (group_kind) {
	case GK_VECTOR :
	case GK_ARRAY :
		output += "[";
		output += cgv::utils::to_string(grp_size);
		output += "]";
		break;
	case GK_STRUCTURE : 
	case GK_BASE_CLASS : 
		output += "(";
		output += rt->get_type_name();
		output += ")";
		if (rt->has_string_conversions()) {
			output += "~'";
			std::string delta;
			rt->get_to_string(group_ptr, delta);
			output += delta;
			output += "'";
		}
	}
	output += "\n";
	tab += "  ";
	return GT_COMPLETE;
}

/// abstract interface to start reflection of a group of members
void debug_reflection_handler::reflect_group_end(GroupKind group_kind) 
{
	if (tab.size() > 1)
		tab = tab.substr(0, tab.size()-2);
}

bool debug_reflection_handler::reflect_member_void(const std::string& member_name, void* member_ptr, abst_reflection_traits* rt) 
{
	output += tab;
	output += extend_name(member_name);
	output += ":";
	output += rt->get_type_name();
	if (rt->get_type_id() == info::TI_STRING) {
		output += '=';
		output += '"';
		output += cgv::utils::escape_special(*((const std::string*)member_ptr));
		output += '"';
	}
	else if (rt->has_string_conversions()) {
		std::string delta;
		rt->get_to_string(member_ptr, delta);
		output += "=";
		output += delta;
	}
	output += "\n";
	return true;
}

bool debug_reflection_handler::reflect_method_void(const std::string& method_name, method_interface* mi_ptr,
							 abst_reflection_traits* return_traits, const std::vector<abst_reflection_traits*>& param_value_traits)
{
	output += method_name;
	output += ":";
	output += return_traits->get_type_name();
	output += "(";
	for (unsigned i=0; i<param_value_traits.size(); ++i) {
		output += param_value_traits[i]->get_type_name();
		if (i+1 < param_value_traits.size())
			output += ",";
	}
	output += ")\n";
	return true;
}
	}
}