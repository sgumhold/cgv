#include "base_generator.h"

namespace cgv {
	namespace base {


abst_property_access::abst_property_access() : has_changed(false)
{
}

/// overload to return the type name of this object. By default the type interface is queried over get_type.
std::string base_generator::get_type_name() const
{
	return "base_generator"; 
}

/// add a new property
void base_generator::add_void(const std::string& name, abst_property_access* apa)
{
	iter_type i = property_map.find(name);
	if (i != property_map.end()) {
		if (apa != i->second) {
			delete i->second;
			i->second = apa;
		}
	}
	else 
		property_map[name] = apa;
}

/// remove a property
void base_generator::del(const std::string& name)
{
	iter_type i = property_map.find(name);
	if (i != property_map.end()) {
		delete i->second;
		property_map.erase(i);
	}
}

/// return whether property has changed
bool base_generator::changed(const std::string& property) const
{
	const_iter_type i = property_map.find(property);
	if (i == property_map.end())
		return false;
	return i->second->has_changed;
}

//! returns a semicolon separated list of property declarations
std::string base_generator::get_property_declarations()
{
	std::string res;
	for (iter_type i = property_map.begin(); i != property_map.end(); ++i) {
		if (!res.empty())
			res += ';';
		res += i->first+':'+i->second->get_type_name();
	}
	return res;
}

//! abstract interface for the setter of a dynamic property. 
bool base_generator::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	iter_type i = property_map.find(property);
	if (i != property_map.end())
		return i->second->set(value_type, value_ptr);
	return false;
}

//! abstract interface for the getter of a dynamic property. 
bool base_generator::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	iter_type i = property_map.find(property);
	if (i != property_map.end())
		return i->second->get(value_type, value_ptr);
	return false;
}

	}
}
