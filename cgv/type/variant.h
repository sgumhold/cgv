#pragma once

#include <cgv/utils/convert.h>
#include <stdlib.h>
#include <cgv/type/standard_types.h>
#include <cgv/type/info/type_id.h>

#include "lib_begin.h"

namespace cgv {
	namespace type {

/** convenience template to access a value pointed to by a void pointer where the type of the
    value is given by a string as it results from cgv::type::info::type_name<T>::get_name().
	 The template argument T is the type into which the stored type is to be converted. With the
	 get method the pointed to value can be read out and with the set method written. If type T
	 is not equal to the stored type, an automatic conversion is performed. A specialization for
	 std::string converts number types to strings with the help of atoi and atof. */
template <typename T>
struct variant
{
	/// convert the value pointed to by value_ptr of type value_type to type T and return it
	static T get(const std::string& value_type, const void* value_ptr)
	{
		if (value_type == info::get_type_name(info::TI_BOOL))
			return (T) *static_cast<const bool*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_INT8))
			return (T) *static_cast<const int8_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_INT16))
			return (T) *static_cast<const int16_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_INT32))
			return (T) *static_cast<const int32_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_INT64))
			return (T) *static_cast<const int64_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_UINT8))
			return (T) *static_cast<const uint8_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_UINT16))
			return (T) *static_cast<const uint16_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_UINT32))
			return (T) *static_cast<const uint32_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_UINT64))
			return (T) *static_cast<const uint64_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_FLT32))
			return (T) *static_cast<const flt32_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_FLT64))
			return (T) *static_cast<const flt64_type*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_STRING))
			return (T) atof(static_cast<const std::string*>(value_ptr)->c_str());
		if (value_type == info::get_type_name(info::TI_WCHAR))
			return (T) *static_cast<const short*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_WSTRING))
			return (T) atof(cgv::utils::wstr2str(*static_cast<const std::wstring*>(value_ptr)).c_str());
		return T();
	}
	/// convert the first parameter of type T into value_type and store the value at the location pointed to by value_ptr
	static void set(const T& value, const std::string& value_type, void* value_ptr)
	{
		if (value_type == info::get_type_name(info::TI_BOOL))
			*static_cast<bool*>(value_ptr) = value != T();
		else if (value_type == info::get_type_name(info::TI_INT8))
			*static_cast<int8_type*>(value_ptr) = (int8_type) value;
		else if (value_type == info::get_type_name(info::TI_INT16))
			*static_cast<int16_type*>(value_ptr) = (int16_type) value;
		else if (value_type == info::get_type_name(info::TI_INT32))
			*static_cast<int32_type*>(value_ptr) = (int32_type) value;
		else if (value_type == info::get_type_name(info::TI_INT64))
			*static_cast<int64_type*>(value_ptr) = (int64_type) value;
		else if (value_type == info::get_type_name(info::TI_UINT8))
			*static_cast<uint8_type*>(value_ptr) = (uint8_type) value;
		else if (value_type == info::get_type_name(info::TI_UINT16))
			*static_cast<uint16_type*>(value_ptr) = (uint16_type) value;
		else if (value_type == info::get_type_name(info::TI_UINT32))
			*static_cast<uint32_type*>(value_ptr) = (uint32_type) value;
		else if (value_type == info::get_type_name(info::TI_UINT64))
			*static_cast<uint64_type*>(value_ptr) = (uint64_type) value;
		else if (value_type == info::get_type_name(info::TI_FLT32))
			*static_cast<flt32_type*>(value_ptr) = (flt32_type) value;
		else if (value_type == info::get_type_name(info::TI_FLT64))
			*static_cast<flt64_type*>(value_ptr) = (flt64_type) value;
		else if (value_type == info::get_type_name(info::TI_WCHAR))
			*static_cast<int16_type*>(value_ptr) = (int16_type) value;
		else if (value_type == info::get_type_name(info::TI_STRING))
			*static_cast<std::string*>(value_ptr) = cgv::utils::to_string(value);
		else if (value_type == info::get_type_name(info::TI_WSTRING))
			*static_cast<std::wstring*>(value_ptr) = cgv::utils::str2wstr(cgv::utils::to_string(value));
	}
};

template <>
struct variant<bool>
{
	static bool get(const std::string& value_type, const void* value_ptr)
	{
		if (value_type == info::get_type_name(info::TI_BOOL))
			return *static_cast<const bool*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_INT8))
			return *static_cast<const int8_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_INT16))
			return *static_cast<const int16_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_INT32))
			return *static_cast<const int32_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_INT64))
			return *static_cast<const int64_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_UINT8))
			return *static_cast<const uint8_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_UINT16))
			return *static_cast<const uint16_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_UINT32))
			return *static_cast<const uint32_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_UINT64))
			return *static_cast<const uint64_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_FLT32))
			return *static_cast<const flt32_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_FLT64))
			return *static_cast<const flt64_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_WCHAR))
			return *static_cast<const wchar_type*>(value_ptr) != 0;
		if (value_type == info::get_type_name(info::TI_STRING))
			return *static_cast<const std::string*>(value_ptr) == "true";
		if (value_type == info::get_type_name(info::TI_WSTRING))
			return *static_cast<const std::wstring*>(value_ptr) == L"true";
		return false;
	}
	static void set(const bool& value, const std::string& value_type, void* value_ptr)
	{
		if (value_type == info::get_type_name(info::TI_BOOL))
			*static_cast<bool*>(value_ptr) = value;
		else if (value_type == info::get_type_name(info::TI_INT8))
			*static_cast<int8_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_INT16))
			*static_cast<int16_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_INT32))
			*static_cast<int32_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_INT64))
			*static_cast<int64_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_UINT8))
			*static_cast<uint8_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_UINT16))
			*static_cast<uint16_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_UINT32))
			*static_cast<uint32_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_UINT64))
			*static_cast<uint64_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_FLT32))
			*static_cast<flt32_type*>(value_ptr) = value?1.0f:0.0f;
		else if (value_type == info::get_type_name(info::TI_FLT64))
			*static_cast<flt64_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_WCHAR))
			*static_cast<wchar_type*>(value_ptr) = value?1:0;
		else if (value_type == info::get_type_name(info::TI_STRING))
			*static_cast<std::string*>(value_ptr) = value?"true":"false";
		else if (value_type == info::get_type_name(info::TI_WSTRING))
			*static_cast<std::wstring*>(value_ptr) = value?L"true":L"false";
	}
};


template <>
struct variant<std::string>
{
	static std::string get(const std::string& value_type, const void* value_ptr)
	{
		if (value_type == info::get_type_name(info::TI_BOOL))
			return *static_cast<const bool*>(value_ptr)?"true":"false";
		if (value_type == info::get_type_name(info::TI_INT8))
			return cgv::utils::to_string((int)*static_cast<const int8_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_INT16))
			return cgv::utils::to_string(*static_cast<const int16_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_INT32))
			return cgv::utils::to_string(*static_cast<const int32_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_INT64))
			return cgv::utils::to_string(*static_cast<const int64_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_UINT8))
			return cgv::utils::to_string((int)*static_cast<const uint8_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_UINT16))
			return cgv::utils::to_string(*static_cast<const uint16_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_UINT32))
			return cgv::utils::to_string(*static_cast<const uint32_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_UINT64))
			return cgv::utils::to_string(*static_cast<const uint64_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_FLT32))
			return cgv::utils::to_string(*static_cast<const flt32_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_FLT64))
			return cgv::utils::to_string(*static_cast<const flt64_type*>(value_ptr));
		if (value_type == info::get_type_name(info::TI_WCHAR))
			return cgv::utils::wstr2str(std::wstring(*static_cast<const wchar_type*>(value_ptr), 1));
		if (value_type == info::get_type_name(info::TI_STRING))
			return *static_cast<const std::string*>(value_ptr);
		if (value_type == info::get_type_name(info::TI_WSTRING))
			return cgv::utils::wstr2str(*static_cast<const std::wstring*>(value_ptr));
		return "";
	}
	static void set(const std::string& value, const std::string& value_type, void* value_ptr)
	{
		if (value_type == info::get_type_name(info::TI_BOOL))
			*static_cast<bool*>(value_ptr) = value=="true"?true:false;
		else if (value_type == info::get_type_name(info::TI_INT8))
			*static_cast<int8_type*>(value_ptr) = atoi(value.c_str());
		else if (value_type == info::get_type_name(info::TI_INT16))
			*static_cast<int16_type*>(value_ptr) = (int16_type) atoi(value.c_str());
		else if (value_type == info::get_type_name(info::TI_INT32))
			*static_cast<int32_type*>(value_ptr) = (int32_type) atoi(value.c_str());
		else if (value_type == info::get_type_name(info::TI_INT64))
			*static_cast<int64_type*>(value_ptr) = (int64_type) atoi(value.c_str());
		else if (value_type == info::get_type_name(info::TI_UINT8))
			*static_cast<uint8_type*>(value_ptr) = (uint8_type) atoi(value.c_str());
		else if (value_type == info::get_type_name(info::TI_UINT16))
			*static_cast<uint16_type*>(value_ptr) = (uint16_type) atoi(value.c_str());
		else if (value_type == info::get_type_name(info::TI_UINT32))
			*static_cast<uint32_type*>(value_ptr) = (uint32_type) atoi(value.c_str());
		else if (value_type == info::get_type_name(info::TI_UINT64))
			*static_cast<uint64_type*>(value_ptr) = (uint64_type) atoi(value.c_str());
		else if (value_type == info::get_type_name(info::TI_FLT32))
			*static_cast<flt32_type*>(value_ptr) = (flt32_type) atof(value.c_str());
		else if (value_type == info::get_type_name(info::TI_FLT64))
			*static_cast<flt64_type*>(value_ptr) = (flt64_type) atof(value.c_str());
		else if (value_type == info::get_type_name(info::TI_STRING))
			*static_cast<std::string*>(value_ptr) = value;
		else if (value_type == info::get_type_name(info::TI_WCHAR))
			*static_cast<wchar_type*>(value_ptr) = value.empty() ? 0 : value[0];
		else if (value_type == info::get_type_name(info::TI_WSTRING))
			*static_cast<std::wstring*>(value_ptr) = cgv::utils::str2wstr(value);
	}
};

template <>
struct variant<std::wstring>
{
	static std::wstring get(const std::string& value_type, const void* value_ptr)
	{
		if (value_type == info::get_type_name(info::TI_WSTRING))
			return *static_cast<const std::wstring*>(value_ptr);
		return cgv::utils::str2wstr(variant<std::string>::get(value_type, value_ptr));
	}
	static void set(const std::wstring& value, const std::string& value_type, void* value_ptr)
	{
		if (value_type == info::get_type_name(info::TI_WSTRING))
			*static_cast<std::wstring*>(value_ptr) = value;
		else {
			std::string v = cgv::utils::wstr2str(value);
			variant<std::string>::set(v, value_type, value_ptr);
		}
	}
};


template <>
struct variant<const char*>
{
	static void set(const char* value, const std::string& value_type, void* value_ptr)
	{
		variant<std::string>::set(value?value:"", value_type, value_ptr);
	}
};

template <typename T>
void set_variant(const T& value, const std::string& value_type, void* value_ptr)
{
	variant<T>::set(value,value_type,value_ptr);
}

template <typename T>
void get_variant(T& value, const std::string& value_type, const void* value_ptr)
{
	value = variant<T>::get(value_type,value_ptr);
}

extern CGV_API void assign_variant(const std::string& dst_value_type, void* dst_value_ptr, 
						           const std::string& src_value_type, const void* src_value_ptr);

	}
}

#include <cgv/config/lib_end.h>