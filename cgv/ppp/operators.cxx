#include "operators.h"

namespace cgv {
	namespace ppp {

		const char* operator_characters = "=|^&!<>+-*/%.:~?";

		struct operator_info
		{
			OperatorType ot;
			const char* word;
			int arity;
			int priority;
			OperatorPrecedence precedence;
			OperatorLocation location;
		};

		const operator_info& get_operator_info(OperatorType ot)
		{
			static const operator_info infos[] = {

			{ OT_ASSIGN,          "=",  2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_REF,      "=&", 2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_ADD,      "+=", 2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_SUB,      "-=", 2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_MUL,      "*=", 2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_DIV,      "/=", 2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_AND,      "&=", 2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_OR,       "|=", 2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_XOR,      "^=", 2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_LSH,      "<<=",2, 1, OP_RIGHT, OL_PREFIX },
			{ OT_ASSIGN_RSH,      ">>=",2, 1, OP_RIGHT, OL_PREFIX },

			{ OT_LOG_OR,          "||", 2, 2, OP_LEFT,  OL_PREFIX },

			{ OT_LOG_AND,         "&&", 2, 3, OP_LEFT,  OL_PREFIX },

			{ OT_OR,              "|",  2, 4, OP_LEFT,  OL_PREFIX },

			{ OT_XOR,             "^",  2, 5, OP_LEFT,  OL_PREFIX },

			{ OT_AND,             "&",  2, 6, OP_LEFT,  OL_PREFIX },

			{ OT_EQUAL,           "==", 2, 7, OP_LEFT,  OL_PREFIX },
			{ OT_UNEQUAL,         "!=", 2, 7, OP_LEFT,  OL_PREFIX },
			{ OT_EQUAL_TYPE,      "~~", 2, 7, OP_LEFT,  OL_PREFIX },
			{ OT_UNEQUAL_TYPE,    "!~", 2, 7, OP_LEFT,  OL_PREFIX },

			{ OT_LESS,            "<",  2, 8, OP_LEFT,  OL_PREFIX },
			{ OT_GREATER,         ">",  2, 8, OP_LEFT,  OL_PREFIX },
			{ OT_LESS_OR_EQUAL,   "<=", 2, 8, OP_LEFT,  OL_PREFIX },
			{ OT_GREATER_OR_EQUAL,">=", 2, 8, OP_LEFT,  OL_PREFIX },

			{ OT_LSH,             "<<", 2, 9, OP_LEFT,  OL_PREFIX },
			{ OT_RSH,             ">>", 2, 9, OP_LEFT,  OL_PREFIX },

			{ OT_ADD,             "+",  2,10, OP_LEFT,  OL_PREFIX },
			{ OT_SUB,             "-",  2,10, OP_LEFT,  OL_PREFIX },

			{ OT_MUL,             "*",  2,11, OP_LEFT,  OL_PREFIX },
			{ OT_DIV,             "/",  2,11, OP_LEFT,  OL_PREFIX },
			{ OT_MOD,             "%",  2,11, OP_LEFT,  OL_PREFIX },

			{ OT_NOT,             "!",  1,12, OP_RIGHT,  OL_PREFIX },
			{ OT_EXISTS,          "?",  1,12, OP_RIGHT,  OL_PREFIX },
			{ OT_INC,             "++", 1,12, OP_RIGHT,  OL_PREFIX },
			{ OT_DEC,             "--", 1,12, OP_RIGHT,  OL_PREFIX },
			{ OT_NEGATE,          "-",  1,12, OP_RIGHT,  OL_PREFIX },
			{ OT_COMPL,           "~",  1,12, OP_RIGHT,  OL_PREFIX },

			{ OT_DOT,             ".",  2,13, OP_LEFT,  OL_PREFIX },

			{ OT_MAP_UP,          ":>", 1,14, OP_LEFT,  OL_PREFIX },
			{ OT_MAP_DOWN,        "<:", 1,14, OP_LEFT,  OL_PREFIX },
			{ OT_BINARY_MAP,      "::", 2,14, OP_LEFT,  OL_PREFIX },
			{ OT_UNARY_MAP,       "::", 1,14, OP_LEFT,  OL_PREFIX },

			};

			return infos[ot];
		}

		OperatorPrecedence get_operator_precedence(OperatorType ot)
		{
			return get_operator_info(ot).precedence;
		}

		const char* get_operator_word(OperatorType ot)
		{
			return get_operator_info(ot).word;
		}

		int get_operator_priority(OperatorType ot)
		{
			return get_operator_info(ot).priority;
		}

		int get_operator_arity(OperatorType ot)
		{
			return get_operator_info(ot).arity;
		}
		OperatorLocation get_operator_location(OperatorType ot)
		{
			return get_operator_info(ot).location;
		}
		bool get_operator_at_begin(OperatorType ot)
		{
			return get_operator_info(ot).arity == 1 && get_operator_location(ot) == OL_PREFIX;
		}



		OperatorType get_operator_type(const std::string& s)
		{
			for (OperatorType ot = (OperatorType)0; ot < OT_LAST; ot = OperatorType(ot + 1))
				if (s == get_operator_word(ot))
					return ot;
			return OT_LAST;
		}

	}
}
