#pragma once

#include "reflection_handler.h"
#include "lib_begin.h"

namespace cgv {
	namespace reflect {

/// type independent functionality for all enum fallback implementations
struct CGV_API abst_enum_reflection_traits : public abst_reflection_traits
{
	/// compile information about enum interface
	static const bool has_enum = true;
	static const ReflectionTraitsKind kind = RTK_STRING;

	virtual std::vector<cgv::utils::token>& ref_names() = 0;
	virtual std::vector<int>& ref_values() = 0;
	virtual std::string& declarations() const = 0;

	void parse_declarations();
	bool has_string_conversions() const;
	bool set_from_string(void* member_ptr, const std::string& str_val);
	void get_to_string(const void* member_ptr, std::string& str_val);

	bool has_enum_interface() const;
	unsigned get_nr_enum_items() const;
	std::string get_enum_name(unsigned i) const;
	int get_enum_value(unsigned i) const;
};

/// this type specific reflection traits class is used by the reflect_enum function to reflect enum types
template <typename T> 
struct enum_reflection_traits : public reflection_traits_impl<T, abst_enum_reflection_traits>
{
	std::vector<cgv::utils::token>& ref_names() { static std::vector<cgv::utils::token> names; return names; }
	std::vector<int>& ref_values()              { static std::vector<int> values; return values; }
	std::string& declarations() const           { static std::string decs; return decs; }
	//! construct from declaration string which is of the same syntax as the C++ enum item declaration.
	/*! A typical example for a declaration string is "ITEM1 = 1, ITEM3 = 3, ITEM4".*/
	enum_reflection_traits(const std::string& _declarations = "")
	{ 
		if (!_declarations.empty()) {
			declarations() = _declarations; 
			this->parse_declarations();
		}
#ifdef REFLECT_TRAITS_WITH_DECLTYPE
		if (declarations().empty())
			get_reflection_traits(T());
#endif
	}
	/// clone function
	abst_reflection_traits* clone() { return new enum_reflection_traits<T>(); }
};

	}
}

#include <cgv/config/lib_end.h>
