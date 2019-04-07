#pragma once

#include <utility>
#include <vector>
#include <stack>
#include "variant.h"
#include "operators.h"
#include <cgv/utils/token.h>

using cgv::utils::token;

#include "lib_begin.h"

namespace cgv {
	namespace ppp {

		class ph_processor;

		enum ExpressionPart { EP_VALUE, EP_OPERATOR, EP_OPEN, EP_FUNC, EP_CLOSE, EP_LIST_OPEN, EP_LIST_CLOSE, EP_LIST_ACCESS, EP_COMMA };

		struct expression_token : public token
		{
			variant value;
			OperatorType ot;
			ExpressionPart ep;
			expression_token(const token& tok, const variant& v) : token(tok), value(v), ep(EP_VALUE) {}
			expression_token(const token& tok, OperatorType _ot) : token(tok), ot(_ot), ep(EP_OPERATOR) {}
			expression_token(const token& tok, ExpressionPart _ep) : token(tok), ep(_ep) {}
		};

		class CGV_API expression_processor
		{
		protected:
			bool debug_parse;
			bool debug_evaluate;
			typedef std::pair<unsigned int, unsigned int> expr_stack_entry;

			void prepare();

			bool compress_stack_validate(int priority, std::vector<expr_stack_entry>& expression_stack,
				std::vector<ExpressionPart>& parenthesis_stack,
				std::stack<variant>& value_stack, std::stack<OperatorType>& operator_stack);

			bool compress_stack_evaluate(int priority, std::vector<expr_stack_entry>& expression_stack,
				std::vector<ExpressionPart>& parenthesis_stack,
				std::stack<variant>& value_stack, std::stack<OperatorType>& operator_stack);

			std::vector<expression_token> expression_tokens;

			unsigned int get_nr_comma_separated_expressions(std::vector<expr_stack_entry>& expression_stack,
				std::vector<ExpressionPart>& parenthesis_stack,
				std::stack<variant>& value_stack) const;

			ExpressionPart find_last_parenthesis(std::vector<expr_stack_entry>& expression_stack,
				std::vector<ExpressionPart>& parenthesis_stack) const;

			mutable std::string last_error;
			mutable token last_error_token;
		public:
			bool found_error;
			bool issued_error;

			expression_processor();

			bool parse(const token& input_token);

			void extract_begins(std::vector<unsigned>& begins, unsigned i0 = 0, unsigned ie = -1) const;
			bool assign_func_decl(const std::vector<variant>& values) const;
			bool is_func_decl() const;
			int classify_call(unsigned i) const;

			bool validate(bool allow_several_values = false);
			bool evaluate(variant& result, ph_processor* ph_proc);

			const std::string& get_last_error() const;
			const token& get_last_error_token() const;
		};
	}
}
#include <cgv/config/lib_end.h>