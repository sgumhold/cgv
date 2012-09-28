#pragma once

#include <string>
#include <cgv/type/info/type_id.h>
#include <cgv/type/cond/is_standard_type.h>
#include <typeinfo>

#include <cgv/type/lib_begin.h>

namespace cgv {
	namespace type {
		/// namespace for templates that provide type information
		namespace info {

/// extract a type name from a type_info structure that does not contain the class, struct nor enum keywords
extern CGV_API std::string extract_type_name(const std::type_info& ti);

namespace detail {
	// implementation of type_name traits for standard types
	template <typename T, bool is_std = true>
	struct dispatch_type_name
	{
		// use type id to determine name of standard type
		static const char* get_name() { return get_type_name(type_id<T>::get_id()); }
	};

	// implementation of type_name traits for all other types
	template <typename T>
	struct dispatch_type_name<T,false>
	{
		// use RTTI and clean up the resulting name from unwanted keywords
		static const char* get_name() { 
			static std::string type_name = extract_type_name(typeid(T));
			return type_name.c_str();
		}
	};
}

/** traits class with a static function get_name() of type const char* that returns the 
    type name of the template argument \c T. The standard types have the following textual names:
	 - "bool"
	 - "int[8|16|32|64]"
	 - "uint[8|16|32|64]"
	 - "flt[32|64]"
	 - "wchar"
	 - "string"
	 - "wstring"
   
   all other types are derived from the RTTI with the extract_type_name function applied to the 
   type_info structure returned by typeid.
*/
template <typename T>
struct type_name
{
	/// return special name for standard types or type name from RTTI cleaned from keywords for all other types
	static const char* get_name() { return detail::dispatch_type_name<T, cond::is_standard_type<T>::value>::get_name(); }
};

		}
	}
}

#include <cgv/config/lib_end.h>
