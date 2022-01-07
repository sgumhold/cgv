#include "base.h"
#include "named.h"
#include "node.h"
#include "group.h"
#include <cgv/reflect/get_reflection_handler.h>
#include <cgv/reflect/set_reflection_handler.h>
#include <cgv/type/variant.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>
#include <iostream>
#include <stdlib.h>

using namespace cgv::type;
using namespace cgv::reflect;
using namespace cgv::type::info;
using namespace cgv::utils;

namespace cgv {
	namespace base {

base::~base()
{
}
void base::stream_stats(std::ostream&)
{
}
void base::on_register()
{
}
void base::unregister()
{
}
bool base::on_exit_request()
{
	return true;
}
data::ref_ptr<named, true> base::get_named()
{
	return data::ref_ptr<named, true>();
}
data::ref_ptr<node, true> base::get_node()
{
	return data::ref_ptr<node, true>();
}
data::ref_ptr<group, true> base::get_group()
{
	return data::ref_ptr<group, true>();
}
data::ref_ptr<const named, true> base::get_named_const() const
{
	return data::ref_ptr<const named, true>();
}
data::ref_ptr<const node, true> base::get_node_const() const
{
	return data::ref_ptr<const node, true>();
}
data::ref_ptr<const group, true> base::get_group_const() const
{
	return data::ref_ptr<const group, true>();
}
void base::update()
{
}
void* base::get_user_data() const
{
	return 0;
}

class property_declaration_reflection_handler : public reflection_handler
{
public:
	std::string property_declarations;
	property_declaration_reflection_handler() {}
	///
	int reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* group_rt, unsigned grp_size)
	{
		if (group_kind == GK_BASE_CLASS)
			return GT_COMPLETE;

		if (!property_declarations.empty())
			property_declarations += ';'; 
		std::string group_type = group_rt->get_type_name();
		switch (group_kind) {
		case GK_STRUCTURE : property_declarations += group_name+':'+group_type; break;
		case GK_ARRAY : property_declarations += group_name+':'+group_type+'['+to_string(grp_size)+']'; break;
		case GK_VECTOR : property_declarations += group_name+":vector<"+group_type+">"; break;
		case GK_POINTER: property_declarations += group_name+":pointer("+group_type+")"; break;
		}
		return GT_SKIP;
	}
	///
	bool reflect_member_void(const std::string& member_name, 
							 void*, abst_reflection_traits* rt)
	{
		if (!property_declarations.empty())
			property_declarations += ';'; 
		property_declarations += member_name+':'+rt->get_type_name();
		return true;
	}
	///
	bool reflect_method_void(const std::string& method_name, method_interface* mi_ptr,
							 abst_reflection_traits* return_traits, const std::vector<abst_reflection_traits*>& param_value_traits)
	{
		if (!property_declarations.empty())
			property_declarations += ';'; 
		property_declarations += method_name+'(';
		for (unsigned int i=0; i<param_value_traits.size(); ++i) {
			if (i > 0)
				property_declarations += ',';
			property_declarations += param_value_traits[i]->get_type_name();
		}
		property_declarations += ')';
		if (return_traits) {
			property_declarations += ':';
			property_declarations += return_traits->get_type_name();
		}
		return true;
	}
};

class call_reflection_handler : public reflection_handler
{
public:
	void* instance;
	const std::string& method;
	const std::vector<std::string>& param_value_types;
	const std::vector<const void*>& param_value_ptrs;
	const std::string& result_type;
	void* result_value_ptr;
	bool found;
	call_reflection_handler(void* _instance,
		                   const std::string& _method,
						   const std::vector<std::string>& _param_value_types,
						   const std::vector<const void*>& _param_value_ptrs,
						   const std::string& _result_type,
						   void* _result_value_ptr) 
		: instance(_instance),
		  method(_method), 
		  param_value_types(_param_value_types),
		  param_value_ptrs(_param_value_ptrs),
		  result_type(_result_type),
		  result_value_ptr(_result_value_ptr),
		  found(false) {}
	/// empty implementation
	bool reflect_member_void(const std::string& member_name, void* member_ptr, abst_reflection_traits* rt)
	{
		return true;
	}
	///
	bool reflect_method_void(const std::string& method_name, method_interface* mi_ptr,
							 abst_reflection_traits* return_traits, const std::vector<abst_reflection_traits*>& param_value_traits)
	{
		if (method_name != method)
			return true;
		mi_ptr->call_void(instance, param_value_traits, param_value_ptrs, param_value_types, return_traits, result_value_ptr, result_type);
		return false;
	}
};

/// is used by default implementation of set_void, get_void and get_property_declarations
bool base::self_reflect(reflection_handler&) 
{
	return true;
}


/// overload to implement the execution of a method based on the method name and the given parameters
bool base::call_void(const std::string& method, 
					   const std::vector<std::string>& param_value_types,
					   const std::vector<const void*>& param_value_ptrs,
					   const std::string& result_type,
					   void* result_value_ptr) 
{ 
	call_reflection_handler csrh(this, method, param_value_types, param_value_ptrs, result_type, result_value_ptr);
	self_reflect(csrh);
	return csrh.found;
}


/// abstract interface for the setter, by default it simply returns false
bool base::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	set_reflection_handler ssrh(property, value_type, value_ptr);
	self_reflect(ssrh);
	if (ssrh.found_valid_target()) {
		on_set(ssrh.get_member_ptr());
		return true;
	}
/*
	const_type_ptr ctp = get_type();
	if (!ctp)
		return false;
	const compound_interface* ci = ctp->get_compound();
	if (!ci)
		return false;
	const std::vector<const compound_interface::member_interface*>& mbrs = ci->get_members();
	for (unsigned int i=0; i<mbrs.size(); ++i) {
		if (mbrs[i]->get_name() == property) {
			void* member_ptr = mbrs[i]->access(this);
			assign_variant(name_type(mbrs[i]->get_member_type()), member_ptr, value_type, value_ptr);
			on_set(mbrs[i]->access(this));
			return true;
		}
	}
	*/
	return false;
}

/// this callback is called when the set_void method has changed a member and can be overloaded in derived class
void base::on_set(void* member_ptr)
{
}

/// abstract interface for the getter, by default it simply returns false
bool base::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	get_reflection_handler gsrh(property, value_type, value_ptr);
	self_reflect(gsrh);
	if (gsrh.found_valid_target())
		return true;
/*
	const_type_ptr ctp = get_type();
	if (!ctp)
		return false;
	const compound_interface* ci = ctp->get_compound();
	if (!ci)
		return false;
	const std::vector<const compound_interface::member_interface*>& mbrs = ci->get_members();
	for (unsigned int i=0; i<mbrs.size(); ++i) {
		if (mbrs[i]->get_name() == property) {
			assign_variant(value_type, value_ptr, name_type(mbrs[i]->get_member_type()), mbrs[i]->access_const(this));
			return true;
		}
	}
	*/
	return false;
}


/// overload to return the type name of this object
std::string base::get_type_name() const
{
//	const_type_ptr ctp = get_type();
//	if (ctp)
//		return name_type(ctp);
	return cgv::type::info::type_name<base>::get_name();
}

/// overload to provide default options for registration
std::string base::get_default_options() const
{
	return std::string();
}
/// determine name of instance by checking cgv::base::named interface and in failure fallback to get_type_name()
std::string base::get_name_or_type_name() const
{
	const_named_ptr n = get_named_const();
	if (n)
		return n->get_name();
	return get_type_name();
}

/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
std::string base::get_property_declarations()
{
	property_declaration_reflection_handler pdsrh;
	self_reflect(pdsrh);
	std::string declarations = pdsrh.property_declarations;
/*
	const_type_ptr ctp = get_type();
	if (!ctp)
		return declarations;
	const compound_interface* ci = ctp->get_compound();
	if (!ci)
		return declarations;
	const std::vector<const compound_interface::member_interface*>& mbrs = ci->get_members();
	for (unsigned int i=0; i<mbrs.size(); ++i) {
		if (!declarations.empty())
			declarations += ";";
		declarations += mbrs[i]->get_name() + ":" + name_type(mbrs[i]->get_member_type());
	}
	*/
	return declarations;
}

/// set several properties, which are defined as colon separated assignments, where the types are derived automatically
void base::multi_set(const std::string& property_assignments, bool report_error)
{
	// split into single assignments
	std::vector<token> toks;
	bite_all(tokenizer(property_assignments).set_skip("'\"","'\"","\\\\").set_ws(";"),toks);
	// for each assignment
	for (unsigned int i=0; i<toks.size(); ++i) {
		std::vector<token> sides;
		bite_all(tokenizer(toks[i]).set_skip("'\"","'\"","\\\\").set_ws("="),sides);
		if (sides.size() != 2) {
			if (report_error)
				std::cerr << "property assignment >" << to_string(toks[i]).c_str() << "< does not match pattern lhs=rhs" << std::endl;
			continue;
		}
		std::string lhs(to_string(sides[0]));
		std::string rhs(to_string(sides[1]));
		if (rhs[0] == '"' || rhs[0] == '\'') {
			unsigned int n = (unsigned int) (rhs.size()-1);
			char open = rhs[0];
			if (rhs[n] == rhs[0])
				--n;
			rhs = rhs.substr(1, n);
			for (unsigned i=1; i<rhs.size(); ++i) {
				if (rhs[i-1] == '\\' && (rhs[i] == '\\' || rhs[i] == open))
					rhs.erase(i-1,1);
			}
			set_void(lhs,"string",&rhs);
		}
		else if (rhs == "true" || rhs == "false") {
			bool value = rhs == "true";
			set_void(lhs, "bool", &value);
		}
		else if (is_digit(rhs[0]) || rhs[0] == '.' || rhs[0] == '+' || rhs[0] == '-') {
			int int_value;
			if (is_integer(rhs,int_value))
				set_void(lhs, "int32", &int_value);
			else {
				double value = atof(rhs.c_str());
				set_void(lhs, "flt64", &value);
			}
		}
		else
			set_void(lhs,"string",&rhs);
	}
}

//! check if the given name specifies a property.
/*! If the type name string pointer is provided, the type of the
    property is copied to the referenced string. */
bool base::is_property(const std::string& property_name, std::string* type_name)
{
	std::string prop_decls(get_property_declarations());
	std::vector<token> toks;
	bite_all(tokenizer(prop_decls).set_ws(";"),toks);
	// for each assignment
	for (unsigned int i=0; i<toks.size(); ++i) {
		std::vector<token> ts;
		bite_all(tokenizer(toks[i]).set_ws(":"),ts);
		if (ts.size() != 2)
			continue;
		if (ts[0] == property_name.c_str()) {
			if (type_name)
				*type_name = to_string(ts[1]);
			return true;
		}
	}
	return false;
}

//! find a member pointer by name.
/*! If not found the null pointer is returned. If the type name 
	string pointer is provided, the type of the
    property is copied to the referenced string.*/
void* base::find_member_ptr(const std::string& property_name, std::string* type_name)
{
	find_reflection_handler fsrh(property_name);
	self_reflect(fsrh);
	if (!fsrh.found_target())
		return 0;
	if (type_name)
		*type_name = fsrh.get_reflection_traits()->get_type_name();
	return fsrh.get_member_ptr();
}

	}
}
