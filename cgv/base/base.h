#pragma once

#include <cgv/type/info/type_name.h>
#include <cgv/reflect/reflection_handler.h>
#include <cgv/data/ref_ptr.h>
#include <iostream>

#include <cgv/type/lib_begin.h>

namespace cgv {
	namespace type {
		namespace info {

struct CGV_API type_interface;

/// pointer to const type interface
typedef cgv::data::ref_ptr<const type_interface,true> const_type_ptr;

		}
	}
}
#include <cgv/config/lib_end.h>

#include "lib_begin.h"

/// the cgv namespace
namespace cgv {
	/// the base namespace holds the base hierarchy, support for plugin registration and signals
	namespace base {

class CGV_API base;
class CGV_API named;
class CGV_API node;
class CGV_API group;

/// ref counted pointer to base
typedef data::ref_ptr<base,true> base_ptr;

struct cast_helper_base
{
	template <class T>
	static data::ref_ptr<T,true> cast_of_base(base* b);
};

template <class T> 
struct cast_helper : public cast_helper_base
{ 
	inline static data::ref_ptr<T,true> cast(base* b) 
	{ 
		return cast_of_base<T>(b); 
	}
};

/** base class for all classes that can be registered with support 
    for dynamic properties (see also section \ref baseSEC of page \ref baseNS). */
class CGV_API base : public data::ref_counted, public cgv::reflect::self_reflection_tag
{
protected:
	/// give ref_ptr access to destructor
	friend class data::ref_ptr_impl<base,true>;
	/// make destructor virtual and not accessible from outside
	virtual ~base();
	/// give cast_helper_base access to cast_dynamic
	friend struct cast_helper_base;
	/// use dynamic cast for upcast to given class
	template <class T>
	inline static data::ref_ptr<T,true> cast_dynamic(base* b) 
	{
		return data::ref_ptr<T,true>(dynamic_cast<T*>(b)); 
	}
public:
	/// overload to return the type name of this object. By default the type interface is queried over get_type.
	virtual std::string get_type_name() const;
	/// overload to handle register events that is sent after the instance has been registered
	virtual void on_register();
	/// overload to handle unregistration of instances
	virtual void unregister();
	/// overload to show the content of this object
	virtual void stream_stats(std::ostream&);
	/// cast upward to named
	virtual data::ref_ptr<named,true> get_named();
	/// cast upward to node
	virtual data::ref_ptr<node,true> get_node();
	/// cast upward to group
	virtual data::ref_ptr<group,true> get_group();
	/// cast to arbitrary class, but use the casts to named, node and group from the interface
	template <class T>
	data::ref_ptr<T,true> cast() {
		return cast_helper<T>::cast(this);
	}
	/// use dynamic type cast to check for the given interface
	template <class T>
	T* get_interface() {
		return dynamic_cast<T*>(this);
	}
	/// use dynamic type cast to check for the given interface
	template <class T>
	const T* get_const_interface() const {
		return dynamic_cast<const T*>(this);
	}
	/// this virtual update allows for example to ask a view to update the viewed value. The default implementation is empty.
	virtual void update();
	/// this virtual method allows to pass application specific data for internal purposes
	virtual void* get_user_data() const;

	/**@name property interface*/
	//@{
	//! used for simple self reflection
	/*! The overloaded implementation is used by the default implementations of 
	    set_void, get_void and get_property_declarations
	    with corresponding reflection handlers. 
		The default implementation of self_reflect is empty. */
	virtual bool self_reflect(cgv::reflect::reflection_handler&);
	//! return a semicolon separated list of property declarations
	/*! of the form "name1:type1;name2:type2;...", by default an empty 
		list is returned. The types should by consistent with the names 
		returned by cgv::type::info::type_name::get_name. The default 
		implementation extracts names and types from the self_reflect()
		method and the meta type information provided by the get_type()
		method. */
	virtual std::string get_property_declarations();
	//! abstract interface for the setter of a dynamic property. 
	/*! The default implementation 
	    uses the self_reflect() method to find a member with the given property as name. If
		not found, the set_void method returns false. */
	virtual bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// this callback is called when the set_void method has changed a member and can be overloaded in derived class
	virtual void on_set(void* member_ptr);
	//! abstract interface for the getter of a dynamic property. 
	/*! The default implementation 
	    uses the self_reflect() method to find a member with the given property as name. If
		not found, the get_void method returns false. */
	virtual bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	//! abstract interface to call an action
	/*! , i.e. a class method based on the action name and 
	    the given parameters. The default implementation uses the self_reflect() method to
		dispatch this call. If not found, the get_void method returns false.*/
	virtual bool call_void(const std::string& method, 
						   const std::vector<std::string>& param_value_types,
						   const std::vector<const void*>& param_value_ptrs,
						   const std::string& result_type = "",
						   void* result_value_ptr = 0);

	//! specialization of set method to support const char* as strings
	void set(const std::string& property, const char* value) { const std::string& s(value); set_void(property, cgv::type::info::type_name<std::string>::get_name(), &s); }
	//! set a property of the element to the given value and perform standard conversions if necessary.
	/*! This templated version simply extracts the type of the value from the 
		reference and calls the set_void() method. Note that this only works if the template
		cgv::type::info::type_name<T> is overloaded for the value type. */
	template <typename T>
	void set(const std::string& property, const T& value) { set_void(property, cgv::type::info::type_name<T>::get_name(), &value); }
	//! query a property of the element and perform standard conversions if necessary.
	/*! This templated version simply extracts the type of the value from the 
		reference and calls the set_void() method. Note that this only works if the template
		cgv::type::info::type_name<T> is overloaded for the value type. */
	template <typename T>
	T get(const std::string& property) { T value; get_void(property, cgv::type::info::type_name<T>::get_name(), &value); return value; }
	//! set several properties
	/*! , which are defined as colon separated assignments, 
	    where the types are derived automatically to bool, int, 
		double or std::string. */
	void multi_set(const std::string& property_assignments, bool report_error = true);
	//! check if the given name specifies a property.
	/*! If the type name string pointer is provided, the type of the
	    property is copied to the referenced string. */
	bool is_property(const std::string& property_name, std::string* type_name = 0);
	//! find a member pointer by name.
	/*! If not found the null pointer is returned. If the type name 
		string pointer is provided, the type of the
	    property is copied to the referenced string.*/
	void* find_member_ptr(const std::string& property_name, std::string* type_name = 0);
	//@}
};


template <typename T>
inline data::ref_ptr<T,true> cast_helper_base::cast_of_base(base* b)
{
	return base::cast_dynamic<T>(b);
}

#if _MSC_VER > 1400
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<cgv::base::base>;
CGV_TEMPLATE template class CGV_API std::vector<cgv::data::ref_ptr<cgv::base::base> >;
#endif

	}
}


#include <cgv/config/lib_end.h>
