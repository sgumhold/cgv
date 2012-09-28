#pragma once

#include <string>
#include <cgv/type/standard_types.h>

namespace cgv {
	namespace type {
		namespace cond {

/// template condition returning, whether the given type is a standard type (excluding void and enum types)
template <typename T>
struct is_standard_type
{ 
	static const bool value = false;
};

template <> struct is_standard_type<bool>	     { static const bool value = true; };
template <> struct is_standard_type<int8_type>   { static const bool value = true; };
template <> struct is_standard_type<int16_type>  { static const bool value = true; };
template <> struct is_standard_type<int32_type>  { static const bool value = true; };
template <> struct is_standard_type<int64_type>  { static const bool value = true; };
template <> struct is_standard_type<uint8_type>  { static const bool value = true; };
template <> struct is_standard_type<uint16_type> { static const bool value = true; };
template <> struct is_standard_type<uint32_type> { static const bool value = true; };
template <> struct is_standard_type<uint64_type> { static const bool value = true; };
template <> struct is_standard_type<flt32_type>  { static const bool value = true; };
template <> struct is_standard_type<flt64_type>  { static const bool value = true; };
template <> struct is_standard_type<wchar_type>  { static const bool value = true; };
template <> struct is_standard_type<std::wstring> { static const bool value = true; };
template <> struct is_standard_type<std::string> { static const bool value = true; };

		}
	}
}
