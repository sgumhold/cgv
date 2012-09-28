#pragma once

#include <string>
#include <cgv/type/standard_types.h>
#include <cgv/type/lib_begin.h>

namespace cgv {
	namespace type {
		namespace info {

/// ids for the different types and type constructs
enum TypeId { 
	TI_UNDEF, /// used for undefined type 
	TI_BIT,   /// bit based types

	TI_VOID,  /// void

	TI_BOOL,  /// boolean
	TI_INT8,  /// signed integer stored in 8 bits
	TI_INT16, /// signed integer stored in 16 bits
	TI_INT32, /// signed integer stored in 32 bits
	TI_INT64, /// signed integer stored in 64 bits
	TI_UINT8, /// unsigned integer stored in 8 bits
	TI_UINT16, /// unsigned integer stored in 16 bits
	TI_UINT32, /// unsigned integer stored in 32 bits
	TI_UINT64, /// unsigned integer stored in 64 bits
	TI_FLT16,  /// floating point type stored in 16 bits
	TI_FLT32, /// floating point type stored in 32 bits
	TI_FLT64, /// floating point type stored in 64 bits
	TI_WCHAR,  /// wide character type
	TI_STRING, /// string type
	TI_WSTRING, /// string type

	TI_ENUM,            /// all enum types
	TI_REFERENCE,   /// reference type construct
	TI_POINTER,     /// pointer type construct
	TI_ARRAY,       /// array type construct
	TI_FUNCTION_POINTER, /// function pointer type construct
	TI_MEMBER_POINTER,   /// member pointer type construct
	TI_METHOD_POINTER,	/// method pointer type construct
	TI_STRUCT,           /// struct type compound
	TI_CLASS,				/// class type compound
	TI_UNION,				/// union type compound

	TI_CONST,            /// const modifier
	TI_EXPRESSION,	     /// expression used for default parameters
	TI_PARAMETER,        /// function or method parameter
	TI_SIGNATURE,        /// function or method signature
	TI_FUNCTION,         /// function not a function pointer
	TI_BASE,             /// base type of a compound type
	TI_MEMBER,           /// member of a compound
	TI_METHOD,           /// method of a compound, not a method pointer
	TI_CONSTRUCTOR,      /// constructor of a compound
	TI_INSTANCE,         /// an instance not a type
	TI_TYPEDEF,          /// a type definition
	TI_TYPENAME,         /// an undefined typename used as template parameter
	TI_CLASSNAME,        /// an undefined class name used as template parameter
	TI_TEMPLATE,			/// a template construct

	TI_LAST,    /// always the index after the last type construct

	TI_CHAR = TI_INT8, /// alias for character type
	TI_FIRST = TI_UNDEF, /// always the first type construct
	TI_FIRST_STD_TYPE = TI_BOOL, /// always the first standard type
	TI_LAST_STD_TYPE = TI_WSTRING, /// always the last standard type
	TI_FIRST_TYPE = TI_VOID, /// always the first standard type
	TI_LAST_TYPE = TI_UNION, /// always the last standard type
};

/// query if a kind is an unsigned integral type
inline bool is_unsigned_integral(TypeId tid) { return tid>=TI_UINT8 && tid<=TI_UINT64; }
inline bool is_signed_integral(TypeId tid)   { return tid>=TI_INT8 && tid<=TI_INT64; }
inline bool is_integral(TypeId tid)          { return tid>=TI_INT8 && tid<=TI_UINT64; }
inline bool is_floating(TypeId tid)          { return tid>=TI_FLT16 && tid<=TI_FLT64; }
inline bool is_number(TypeId tid)            { return tid>=TI_INT8 && tid<=TI_FLT64; }
inline bool is_fundamental(TypeId tid)       { return tid>=TI_FIRST_STD_TYPE && tid<=TI_LAST_STD_TYPE; }
inline bool is_compound(TypeId tid)          { return tid>=TI_STRUCT && tid<=TI_UNION; }
inline bool is_type(TypeId tid)				 { return tid>=TI_FIRST_TYPE && tid<=TI_LAST_TYPE || tid > TI_LAST; }

extern CGV_API TypeId get_new_type_id();

//! template with a static member function get_id() of type TypeId returning the TypeId of the template argument T.
/*! The id corresponds to the enum TypeId for the standard types as well as the void type and is a unique integer larger
    than TI_LAST for all other types. */
template <typename T>
struct type_id {
	static TypeId get_id() { static TypeId ti = get_new_type_id(); return ti; }
};

template <> struct type_id<void>         { static TypeId get_id() { return TI_VOID; } };
template <> struct type_id<bool>         { static TypeId get_id() { return TI_BOOL; } };
template <> struct type_id<int8_type>    { static TypeId get_id() { return TI_INT8; } };
template <> struct type_id<int16_type>   { static TypeId get_id() { return TI_INT16; } };
template <> struct type_id<int32_type>   { static TypeId get_id() { return TI_INT32; } };
template <> struct type_id<int64_type>   { static TypeId get_id() { return TI_INT64; } };
template <> struct type_id<uint8_type>   { static TypeId get_id() { return TI_UINT8;  } };
template <> struct type_id<uint16_type>  { static TypeId get_id() { return TI_UINT16; } };
template <> struct type_id<uint32_type>  { static TypeId get_id() { return TI_UINT32; } };
template <> struct type_id<uint64_type>  { static TypeId get_id() { return TI_UINT64; } };
template <> struct type_id<flt32_type>   { static TypeId get_id() { return TI_FLT32; } };
template <> struct type_id<flt64_type>   { static TypeId get_id() { return TI_FLT64; } };
template <> struct type_id<wchar_type>   { static TypeId get_id() { return TI_WCHAR; } };
template <> struct type_id<std::string>  { static TypeId get_id() { return TI_STRING; } };
template <> struct type_id<std::wstring> { static TypeId get_id() { return TI_WSTRING; } };

/// function that returns the size of a type specified through TypeId
extern CGV_API unsigned int get_type_size(TypeId tid);

/// function that returns the name of a type specified through TypeId
extern CGV_API const char* get_type_name(TypeId tid);

/// function that returns the type id of a type name
extern CGV_API TypeId get_type_id(const std::string& _type_name);

		}
	}
}

#include <cgv/config/lib_end.h>
