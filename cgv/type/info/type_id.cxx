#include "type_id.h"
#include <cgv/type/standard_types.h>

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

/// function that returns the type id of a type name
TypeId get_type_id(const std::string& _type_name)
{
	for (TypeId tid = TI_FIRST; tid < TI_LAST; tid = TypeId(tid+1)) {
		if (_type_name == get_type_name(tid))
			return tid;
	}
	return TI_UNDEF;
}


		}
	}
}