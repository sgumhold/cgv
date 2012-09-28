#pragma once

#include <string>
#include <cgv/type/standard_types.h>
#include <cgv/type/info/type_id.h>

namespace cgv {
	namespace type {
		namespace info {

/// template to cast a pointer into a type known at compile time and specified as TypeId
template <TypeId type_id>
struct type_ptr
{
	static void*       cast(void* ptr)       { return ptr; }
	static const void* cast(const void* ptr) { return ptr; }
};

template <>
struct type_ptr<TI_BOOL>
{
	static bool*       cast(void* ptr)       { return static_cast<bool*>(ptr); }
	static const bool* cast(const void* ptr) { return static_cast<const bool*>(ptr); }
};

template <>
struct type_ptr<TI_INT8>
{
	static int8_type*       cast(void* ptr)       { return static_cast<int8_type*>(ptr); }
	static const int8_type* cast(const void* ptr) { return static_cast<const int8_type*>(ptr); }
};
template <>
struct type_ptr<TI_INT16>
{
	static int16_type*       cast(void* ptr)       { return static_cast<int16_type*>(ptr); }
	static const int16_type* cast(const void* ptr) { return static_cast<const int16_type*>(ptr); }
};
template <>
struct type_ptr<TI_INT32>
{
	static int32_type*       cast(void* ptr)       { return static_cast<int32_type*>(ptr); }
	static const int32_type* cast(const void* ptr) { return static_cast<const int32_type*>(ptr); }
};
template <>
struct type_ptr<TI_INT64>
{
	static int64_type*       cast(void* ptr)       { return static_cast<int64_type*>(ptr); }
	static const int64_type* cast(const void* ptr) { return static_cast<const int64_type*>(ptr); }
};
template <>
struct type_ptr<TI_UINT8>
{
	static uint8_type*       cast(void* ptr)       { return static_cast<uint8_type*>(ptr); }
	static const uint8_type* cast(const void* ptr) { return static_cast<const uint8_type*>(ptr); }
};
template <>
struct type_ptr<TI_UINT16>
{
	static uint16_type*       cast(void* ptr)       { return static_cast<uint16_type*>(ptr); }
	static const uint16_type* cast(const void* ptr) { return static_cast<const uint16_type*>(ptr); }
};
template <>
struct type_ptr<TI_UINT32>
{
	static uint32_type*       cast(void* ptr)       { return static_cast<uint32_type*>(ptr); }
	static const uint32_type* cast(const void* ptr) { return static_cast<const uint32_type*>(ptr); }
};
template <>
struct type_ptr<TI_UINT64>
{
	static uint64_type*       cast(void* ptr)       { return static_cast<uint64_type*>(ptr); }
	static const uint64_type* cast(const void* ptr) { return static_cast<const uint64_type*>(ptr); }
};
template <>
struct type_ptr<TI_FLT32>
{
	static flt32_type*       cast(void* ptr)       { return static_cast<flt32_type*>(ptr); }
	static const flt32_type* cast(const void* ptr) { return static_cast<const flt32_type*>(ptr); }
};
template <>
struct type_ptr<TI_FLT64>
{
	static flt64_type*       cast(void* ptr)       { return static_cast<flt64_type*>(ptr); }
	static const flt64_type* cast(const void* ptr) { return static_cast<const flt64_type*>(ptr); }
};
template <>
struct type_ptr<TI_STRING>
{
	static std::string*       cast(void* ptr)       { return static_cast<std::string*>(ptr); }
	static const std::string* cast(const void* ptr) { return static_cast<const std::string*>(ptr); }
};

template <>
struct type_ptr<TI_WCHAR>
{
	static wchar_type*       cast(void* ptr)       { return static_cast<wchar_type*>(ptr); }
	static const wchar_type* cast(const void* ptr) { return static_cast<const wchar_type*>(ptr); }
};

template <>
struct type_ptr<TI_WSTRING>
{
	static std::wstring*       cast(void* ptr)       { return static_cast<std::wstring*>(ptr); }
	static const std::wstring* cast(const void* ptr) { return static_cast<const std::wstring*>(ptr); }
};


		}
	}
}