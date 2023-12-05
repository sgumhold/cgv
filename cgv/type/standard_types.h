#pragma once

namespace cgv {
	/// namespace for compile time type information
	namespace type {

/// this type provides an 8 bit signed integer type
typedef signed char             int8_type;
/// this type provides an 16 bit signed integer type
typedef short            int16_type;
/// this type provides an 32 bit signed integer type
typedef int              int32_type;
/// this type provides an 64 bit signed integer type
typedef long long          int64_type;
/// this type provides an 8 bit unsigned integer type
typedef unsigned char    uint8_type;
/// this type provides an 16 bit unsigned integer type
typedef unsigned short   uint16_type;
/// this type provides an 32 bit unsigned integer type
typedef unsigned int     uint32_type;
/// this type provides an 64 bit unsigned integer type
typedef unsigned long long uint64_type;
/// this type provides a 32 bit floating point type
typedef float            flt32_type;
/// this type provides a 64 bit floating point type
typedef double           flt64_type;
/// wide character type
typedef wchar_t          wchar_type;

/// some enum to mark an integral parameter to be of enum type
enum DummyEnum { AAAAA,BBBBB };

	}
}
