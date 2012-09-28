#pragma once

#include <string>

#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace text {
			namespace ppp {

enum OperatorType {
	OT_ASSIGN,
	OT_ASSIGN_REF,
	OT_ASSIGN_ADD,
	OT_ASSIGN_SUB,
	OT_ASSIGN_MUL,
	OT_ASSIGN_DIV,
	OT_ASSIGN_AND,
	OT_ASSIGN_OR,
	OT_ASSIGN_XOR,
	OT_ASSIGN_LSH,
	OT_ASSIGN_RSH,

	OT_LOG_OR,

	OT_LOG_AND,

	OT_OR, 
	
	OT_XOR, 
	
	OT_AND, 
	
	OT_EQUAL,
	OT_UNEQUAL,
	OT_EQUAL_TYPE,
	OT_UNEQUAL_TYPE,
	
	OT_LESS, 
	OT_GREATER, 
	OT_LESS_OR_EQUAL, 
	OT_GREATER_OR_EQUAL, 

	OT_LSH,
	OT_RSH,
	
	OT_ADD, 
	OT_SUB,
	
	OT_MUL, 
	OT_DIV, 
	OT_MOD,
	
	OT_NOT,
	OT_EXISTS,
	OT_INC, 
	OT_DEC,
	OT_NEGATE,
	OT_COMPL,
	
	OT_DOT,
	
	OT_MAP_UP,
	OT_MAP_DOWN,
	OT_BINARY_MAP,
	OT_UNARY_MAP,
	OT_LAST
};

enum OperatorPrecedence {
	OP_LEFT, OP_RIGHT
};

enum OperatorLocation {
	OL_PREFIX, OL_POSTFIX
};

extern CGV_API const char* operator_characters;

extern CGV_API OperatorPrecedence get_operator_precedence(OperatorType ot);
extern CGV_API const char*        get_operator_word(OperatorType ot);
extern CGV_API int                get_operator_priority(OperatorType ot);
extern CGV_API int                get_operator_arity(OperatorType ot);
extern CGV_API OperatorLocation   get_operator_location(OperatorType ot);

extern CGV_API OperatorType get_operator_type(const std::string& s);

			}
		}
	}
}
#include <cgv/config/lib_end.h>