#pragma once

#include <string>
#include <cgv/type/info/type_name.h>
#include <cgv/type/info/type_id.h>
#include <cgv/type/cond/is_enum.h>
#include <cgv/type/cond/is_abstract.h>
#include <cgv/utils/convert_string.h>

#include "lib_begin.h"

/// the cgv namespace
namespace cgv {
	/// in this namespace reflection of types is implemented
	namespace reflect {

class CGV_API reflection_handler;

/// different types of reflection traits
enum ReflectionTraitsKind { RTK_STD_TYPE, RTK_SELF_REFLECT, RTK_EXTERNAL_SELF_REFLECT, RTK_STRING };

/// abstract interface for type reflection with basic type management and optional string conversion
struct CGV_API abst_reflection_traits
{
	/// provide virtual destructor to allow generation of copies
	virtual ~abst_reflection_traits();
	/// clone function
	virtual abst_reflection_traits* clone() = 0;

	/**@name basic type interface */
	//@{
	/// return the size of the type
	virtual unsigned size() const = 0;
	/// construct an instance on the heap with the new operator
	virtual void* new_instance() const = 0;
	/// delete an instance with the delete operator
	virtual void delete_instance(void*) const = 0;
	/// construct n instances on the heap with the new operator
	virtual void* new_instances(unsigned n) const = 0;
	/// delete instances with the delete [] operator
	virtual void delete_instances(void*) const = 0;
	/// return the type id
	virtual cgv::type::info::TypeId get_type_id() const = 0;
	/// return the type name
	virtual const char* get_type_name() const = 0;
	//@}

	/**@name self reflection through external implementation */
	//@{
	/// compile information about external implementation
	static const bool has_external = false;
	/// whether type can be converted to string, defaults to false
	virtual bool has_external_implementation() const;
	/// call the external implementation
	virtual bool external_implementation(reflection_handler& rh, void* member_ptr);
	//@}

	/**@name self reflection through string conversions */
	//@{
	/// compile information about string conversions
	static const bool has_string = false;
	/// whether type can be converted to string, defaults to false
	virtual bool has_string_conversions() const;
	/// convert a given string value to the reflected type and store in the instance pointer
	virtual bool set_from_string(void* instance_ptr, const std::string& str_val);
	/// convert given instance into a string value
	virtual void get_to_string(const void* instance_ptr, std::string& str_val);
	//@}


	/**@name additional enum interface */
	//@{
	/// compile information about enum interface
	static const bool has_enum = false;
	/// return whether type is an enum type - this is independent of whether enum interface is implemented
	virtual bool is_enum_type() const;
	/// return whether the traits class implements the enum interface
	virtual bool has_enum_interface() const;
	/// return the number of enum items
	virtual unsigned get_nr_enum_items() const;
	/// return the name of the i-th enum item
	virtual std::string get_enum_name(unsigned i) const;
	/// return the value of the i-th enum item
	virtual int get_enum_value(unsigned i) const;
	//@}
};

/** \addtogroup detail
 *  @{
 */

/// implementation of the reflection traits providing type specific interface for variable base class
template <typename T, typename B, bool base_is_abst = cgv::type::cond::is_abstract<B>::value>
struct reflection_traits_impl : public B
{
	/// return the size of the type
	unsigned size() const { return sizeof(T); }
	/// construct an instance on the heap with the new operator
	void* new_instance() const { return new T(); }
	/// delete an instance with the delete operator
	void delete_instance(void* instance_ptr) const { delete static_cast<T*>(instance_ptr); }
	/// construct n instances on the heap with the new operator
	void* new_instances(unsigned n) const { return new T[n]; }
	/// delete instances with the delete [] operator
	void delete_instances(void* instance_array) const { delete [] static_cast<T*>(instance_array); }
	/// return the type id
	cgv::type::info::TypeId get_type_id() const { return cgv::type::info::type_id<T>::get_id(); }
	/// return the type name
	const char* get_type_name() const { return cgv::type::info::type_name<T>::get_name(); }
	/// return whether type is an enum type - this is independent of whether enum interface is implemented
	bool is_enum_type() const { return cgv::type::cond::is_enum<T>::value; }
};

/// implementation variant for abstract base classes
template <typename T, typename B>
struct reflection_traits_impl<T,B,true> : public B
{
	/// return the size of the type
	unsigned size() const { return sizeof(T); }
	/// construct an instance on the heap with the new operator
	void* new_instance() const { return 0; }
	/// delete an instance with the delete operator
	void delete_instance(void* instance_ptr) const { delete static_cast<T*>(instance_ptr); }
	/// construct n instances on the heap with the new operator
	void* new_instances(unsigned n) const { return 0; }
	/// delete instances with the delete [] operator
	void delete_instances(void* instance_array) const { delete [] static_cast<T*>(instance_array); }
	/// return the type id
	cgv::type::info::TypeId get_type_id() const { return cgv::type::info::type_id<T>::get_id(); }
	/// return the type name
	const char* get_type_name() const { return cgv::type::info::type_name<T>::get_name(); }
	/// return whether type is an enum type - this is independent of whether enum interface is implemented
	bool is_enum_type() const { return cgv::type::cond::is_enum<T>::value; }
};

/// this template allows to distinguish between traits with and without string conversions
template <bool has_str, typename T, typename B>
struct reflection_traits_string_impl : public reflection_traits_impl<T,B>
{
};

/// this is the implementation with string conversions
template <typename T, typename B>
struct reflection_traits_string_impl<true,T,B> : public reflection_traits_impl<T,B>
{
	/// compile information about string conversions
	static const bool has_string = true;

	bool has_string_conversions() const { return true; }
	bool set_from_string(void* member_ptr, const std::string& str_val) {
		return cgv::utils::from_string(*static_cast<T*>(member_ptr), str_val);
	}
	void get_to_string(const void* member_ptr, std::string& str_val) {
		str_val = cgv::utils::to_string(*static_cast<const T*>(member_ptr));
	}
};

/// Default implementation of the reflection traits providing type specific interface
template <typename T, ReflectionTraitsKind KIND = RTK_STRING, bool has_str = true>
struct reflection_traits : public reflection_traits_string_impl<has_str, T, abst_reflection_traits>
{
	static const ReflectionTraitsKind kind = KIND;
	/// clone function
	abst_reflection_traits* clone() { return new reflection_traits<T,KIND,has_str>(); }
};

//! example implementation of the cgv::reflect::get_reflection_traits() function
/*! reimplement for non standard types in order to provide information on how type reflection
    is implemented for each type. */
extern CGV_API reflection_traits<bool,RTK_STD_TYPE> get_reflection_traits(const bool&);

/** @}*/

	}
}


#include <cgv/config/lib_end.h>
