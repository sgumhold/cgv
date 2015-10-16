#include "type_id.h"
#include <cgv/type/standard_types.h>
#include <map>
#include <typeinfo>

namespace cgv {
	namespace type {
		namespace info {

TypeId get_new_type_id()
{
	static int ti_counter = TI_LAST;
	return TypeId(++ti_counter);
}


/// function that returns the size of a type specified through TypeId
unsigned int get_type_size(TypeId tid)
{
	static unsigned int type_size_table[] = {
		-1, 
		-1,
		-1, 
		sizeof(bool), 
		sizeof(int8_type), 
		sizeof(int16_type), 
		sizeof(int32_type), 
		sizeof(int64_type), 
		sizeof(uint8_type), 
		sizeof(uint16_type), 
		sizeof(uint32_type), 
		sizeof(uint64_type), 
		2, // flt16 
		sizeof(flt32_type), 
		sizeof(flt64_type), 
		sizeof(wchar_t), 
		sizeof(std::string),
		sizeof(std::wstring),
		sizeof(DummyEnum)
	};
	if (tid > TI_ENUM)
		return -1;
	return type_size_table[tid];
}


struct name_type_id_pair
{
	const char* name;
	TypeId tip;
};

TypeId get_type_id(const std::string& _type_name)
{
	static const name_type_id_pair nti_pairs[] = {
		{ "bit", TI_BIT },
		{ "void", TI_VOID },
		{ "bool", TI_BOOL },
		{ "int8",  TI_INT8 },  { typeid(int8_type).name(),  TI_INT8 },
		{ "int16", TI_INT16 }, { typeid(int16_type).name(), TI_INT16 },
		{ "int32", TI_INT32 }, { typeid(int32_type).name(), TI_INT32 },
		{ "int64", TI_INT64 }, { typeid(int64_type).name(), TI_INT64 },
		{ "uint8",  TI_UINT8 },  { typeid(uint8_type).name(),  TI_UINT8 },
		{ "uint16", TI_UINT16 }, { typeid(uint16_type).name(), TI_UINT16 },
		{ "uint32", TI_UINT32 }, { typeid(uint32_type).name(), TI_UINT32 },
		{ "uint64", TI_UINT64 }, { typeid(uint64_type).name(), TI_UINT64 },
		{ "flt16", TI_FLT16 },
		{ "flt32", TI_FLT32 }, { typeid(flt32_type).name(), TI_FLT32 },
		{ "flt64", TI_FLT64 }, { typeid(flt64_type).name(), TI_FLT64 },
		{ "wchar", TI_WCHAR },
		{ "string", TI_STRING },
		{ "wstring", TI_WSTRING },

		{ "enum", TI_ENUM },
		{ "reference", TI_REFERENCE },
		{ "pointer", TI_POINTER },
		{ "array", TI_ARRAY },
		{ "function_pointer", TI_FUNCTION_POINTER },
		{ "member_pointer", TI_MEMBER_POINTER },
		{ "method_pointer", TI_METHOD_POINTER },
		{ "struct", TI_STRUCT },
		{ "class", TI_CLASS },
		{ "union", TI_UNION },

		{ "const", TI_CONST },
		{ "expression", TI_EXPRESSION },
		{ "parameter", TI_PARAMETER },
		{ "signature", TI_SIGNATURE },
		{ "function", TI_FUNCTION },
		{ "base", TI_BASE },
		{ "member", TI_MEMBER },
		{ "method", TI_METHOD },
		{ "constructor", TI_CONSTRUCTOR },
		{ "instance", TI_INSTANCE },
		{ "typedef", TI_TYPEDEF },
		{ "typename", TI_TYPENAME },
		{ "classname", TI_CLASSNAME },
		{ "template", TI_TEMPLATE },
		{ 0, TI_UNDEF }
	};
	static std::map<const std::string,TypeId> type_name_id_map;
	if (type_name_id_map.empty()) {
		unsigned i = 0;
		do {
			type_name_id_map[nti_pairs[i].name] = nti_pairs[i].tip;
			++i;
		} while (nti_pairs[i].name);
	}
	std::map<const std::string,TypeId>::iterator it = type_name_id_map.find(_type_name.c_str());
	if (it == type_name_id_map.end())
		return TI_UNDEF;
	else
		return it->second;
}

/// function that returns the name of a type specified through TypeId
const char* get_type_name(TypeId tid)
{
	static const char* type_name_table[] = {
		"undef",
		"bit",

		"void",

		"bool",
		"int8",
		"int16",
		"int32",
		"int64",
		"uint8",
		"uint16",
		"uint32",
		"uint64",
		"flt16",
		"flt32",
		"flt64",
		"wchar",
		"string",
		"wstring",

		"enum",
		"reference",
		"pointer",
		"array",
		"function_pointer",
		"member_pointer",
		"method_pointer",
		"struct",
		"class",
		"union",

		"const",
		"expression",
		"parameter",
		"signature",
		"function",
		"base",
		"member",
		"method",
		"constructor",
		"instance",
		"typedef",
		"typename",
		"classname",
		"template",
		"last"
	};
	return type_name_table[tid];
}

/*
/// function that returns the type id of a type name
TypeId get_type_id(const std::string& _type_name)
{
	for (TypeId tid = TI_FIRST; tid < TI_LAST; tid = TypeId(tid+1)) {
		if (_type_name == get_type_name(tid))
			return tid;
	}
	return TI_UNDEF;
}

*/
		}
	}
}