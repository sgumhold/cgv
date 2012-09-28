#pragma once

#include "base.h"
#include <cgv/type/variant.h>
#include <map>

#include "lib_begin.h"

using namespace cgv::type;

/// the cgv namespace
namespace cgv {
	/// the base namespace holds the base hierarchy, support for plugin registration and signals
	namespace base {

struct CGV_API abst_property_access
{
	bool has_changed;
	abst_property_access();
	virtual const char* get_type_name() const = 0;
	virtual bool set(const std::string& value_type, const void* value_ptr) = 0;
	virtual bool get(const std::string& value_type, void* value_ptr) = 0;
};

template <typename T>
struct standard_type_property_access : public abst_property_access
{
	T* ptr;
	standard_type_property_access(T* _ptr) : ptr(_ptr) {}
	const char* get_type_name() const { return cgv::type::info::type_name<T>::get_name(); }
	bool set(const std::string& value_type, const void* value_ptr) { get_variant(*ptr,value_type,value_ptr); has_changed = true; return true; }
	bool get(const std::string& value_type, void* value_ptr) { set_variant(*ptr,value_type,value_ptr); return true; }
};

template <typename T>
struct emulated_property_access : public abst_property_access
{
	T* ptr;
	emulated_property_access(T* _ptr) : ptr(_ptr) {}
	const char* get_type_name() const { return cgv::type::info::type_name<T>::get_name(); }
	bool set(const std::string& value_type, const void* value_ptr) {
		if (value_type == get_type_name()) {
			*ptr = *((const T*) value_ptr);
			has_changed = true;
			return true;
		}
		else if (value_type == "string") {
			return has_changed = cgv::utils::from_string(*ptr, *((std::string*)value_ptr));
		}
		return false;
	}
	bool get(const std::string& value_type, void* value_ptr) {
		if (value_type == get_type_name()) {
			*((T*) value_ptr) = *ptr;
			return true;
		}
		else if (value_type == "string") {
			*((std::string*)value_ptr) = cgv::utils::to_string(*ptr);
		}
		return false;
	}
};

template <typename T>
struct property_access : public emulated_property_access<T>
{
	property_access(T* _ptr) : emulated_property_access<T>(_ptr) {}
};

template <> struct property_access<int8_type>   : public standard_type_property_access<int8_type>   { property_access<int8_type>( int8_type* _ptr)  : standard_type_property_access<int8_type>(_ptr) {}; }; 
template <> struct property_access<int16_type>  : public standard_type_property_access<int16_type>  { property_access<int16_type>(int16_type* _ptr) : standard_type_property_access<int16_type>(_ptr) {}; }; 
template <> struct property_access<int32_type>  : public standard_type_property_access<int32_type>  { property_access<int32_type>(int32_type* _ptr) : standard_type_property_access<int32_type>(_ptr) {}; }; 
template <> struct property_access<int64_type>  : public standard_type_property_access<int64_type>  { property_access<int64_type>(int64_type* _ptr) : standard_type_property_access<int64_type>(_ptr) {}; }; 
template <> struct property_access<uint8_type>   : public standard_type_property_access<uint8_type>   { property_access<uint8_type>( uint8_type* _ptr)  : standard_type_property_access<uint8_type>(_ptr) {}; }; 
template <> struct property_access<uint16_type>  : public standard_type_property_access<uint16_type>  { property_access<uint16_type>(uint16_type* _ptr) : standard_type_property_access<uint16_type>(_ptr) {}; }; 
template <> struct property_access<uint32_type>  : public standard_type_property_access<uint32_type>  { property_access<uint32_type>(uint32_type* _ptr) : standard_type_property_access<uint32_type>(_ptr) {}; }; 
template <> struct property_access<uint64_type>  : public standard_type_property_access<uint64_type>  { property_access<uint64_type>(uint64_type* _ptr) : standard_type_property_access<uint64_type>(_ptr) {}; }; 
template <> struct property_access<float>       : public standard_type_property_access<float> { property_access<float>(float* _ptr) : standard_type_property_access<float>(_ptr) {}; }; 
template <> struct property_access<double>      : public standard_type_property_access<double> { property_access<double>(double* _ptr) : standard_type_property_access<double>(_ptr) {}; }; 
template <> struct property_access<bool>        : public standard_type_property_access<bool>        { property_access<bool>(bool* _ptr) : standard_type_property_access<bool>(_ptr) {}; }; 
template <> struct property_access<std::string> : public standard_type_property_access<std::string> { property_access<std::string>(std::string* _ptr) : standard_type_property_access<std::string>(_ptr) {}; }; 

/** implements a dynamic object, that can be composed of independent variables, which are 
    handled as properties of the base_generator and published through the property interface
	of the base class. */
class CGV_API base_generator : public cgv::base::base
{
protected:
	typedef std::map<std::string,abst_property_access*> map_type;
	typedef map_type::iterator iter_type;
	typedef map_type::const_iterator const_iter_type;
	/// store the properties as map from property name to type and pointer to instance
	map_type property_map;
public:
	/// overload to return the type name of this object. By default the type interface is queried over get_type.
	std::string get_type_name() const;
	/// add a new property
	void add_void(const std::string& name, abst_property_access* apa);
	/// add a property by deriving property access from type of reference value
	template <typename T>
	void add(const std::string& property, T& value) { add_void(property, new property_access<T>(&value)); }
	/// remove a property
	void del(const std::string& property);
	/// return whether property has changed
	bool changed(const std::string& property) const;
	//! returns a semicolon separated list of property declarations
	std::string get_property_declarations();
	//! abstract interface for the setter of a dynamic property. 
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	//! abstract interface for the getter of a dynamic property. 
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
};

template <typename T>
bool has_property(const std::string& options, const std::string& property, T& value) {
	base_generator bg;
	bg.add(property, value);
	bg.multi_set(options);
	return bg.changed(property);
}

	}
}


#include <cgv/config/lib_end.h>
