#include "variables.h"
#include "expression_processor.h"
#include "ph_processor.h"

#include <iostream>

#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/convert.h>

using namespace cgv::utils;

namespace cgv {
	namespace ppp {

		bool check_cyclic_reference(const variant* v)
		{
			std::vector<const variant*> s;
			s.push_back(v);
			do {
				const variant* v1;
				if (v->get_type() == REFERENCE_VALUE)
					v1 = v->get_reference();
				else if (v->get_type() == NAME_VALUE)
					v1 = find_variable(v->get_name());
				else
					return false;

				for (unsigned i = 0; i < s.size(); ++i) {
					if (s[i] == v1)
						return true;
				}
				s.push_back(v1);
				v = v1;
			} while (true);
			return false;
		}

		expression_processor::expression_processor()
		{
			/*	debug_parse = true;
				debug_evaluate = true;
				found_error = true;*/

			debug_parse = false;
			debug_evaluate = false;
			found_error = false;
			issued_error = false;
		}

		unsigned int expression_processor::get_nr_comma_separated_expressions(
			std::vector<expr_stack_entry>& expression_stack,
			std::vector<ExpressionPart>& parenthesis_stack,
			std::stack<variant>& value_stack) const
		{
			unsigned int i = (unsigned int)expression_stack.size();
			if (i == 0)
				return 0;
			--i;
			unsigned int n = 1;
			if (value_stack.size() <= expression_stack.back().first)
				n = 0;
			while (parenthesis_stack[i] == EP_COMMA) {
				++n;
				if (i == 0)
					return n;
				--i;
			}
			return n;
		}

		ExpressionPart expression_processor::find_last_parenthesis(std::vector<expr_stack_entry>& expression_stack,
			std::vector<ExpressionPart>& parenthesis_stack) const
		{
			unsigned int i = (unsigned int)expression_stack.size();
			if (i == 0)
				return EP_VALUE;
			--i;
			while (parenthesis_stack[i] == EP_COMMA) {
				if (i == 0)
					return EP_VALUE;
				--i;
			}
			return parenthesis_stack[i];
		}

		bool expression_processor::parse(const token& input_token)
		{
			expression_tokens.clear();

			std::vector<token> input_tokens;
			bite_all(tokenizer(input_token).set_sep(std::string(operator_characters) + "()[],", false).
				set_skip("\"'", "\"'"), input_tokens);

			unsigned int i;
			for (i = 0; i < input_tokens.size(); ++i) {
				const token& tok = input_tokens[i];
				if (*tok.begin == '"' || *tok.begin == '\'') {
					variant v(interpret_special(to_string(tok).substr(1, tok.get_length() - 2)));
					expression_tokens.push_back(expression_token(tok, v));
					if (debug_parse)
						std::cout << i << " : string token '" << v.get_str() << "'" << std::endl;
				}
				else if (to_string(tok) == "(") {
					expression_tokens.push_back(expression_token(tok, EP_OPEN));
					if (debug_parse)
						std::cout << i << " : open token" << std::endl;
				}
				else if (to_string(tok) == ")") {
					expression_tokens.push_back(expression_token(tok, EP_CLOSE));
					if (debug_parse)
						std::cout << i << " : close token" << std::endl;
				}
				else if (to_string(tok) == "[") {
					expression_tokens.push_back(expression_token(tok, EP_LIST_OPEN));
					if (debug_parse)
						std::cout << i << " : open list token" << std::endl;
				}
				else if (to_string(tok) == "]") {
					expression_tokens.push_back(expression_token(tok, EP_LIST_CLOSE));
					if (debug_parse)
						std::cout << i << " : close list token" << std::endl;
				}
				else if (to_string(tok) == ",") {
					expression_tokens.push_back(expression_token(tok, EP_COMMA));
					if (debug_parse)
						std::cout << i << " : comma token" << std::endl;
				}
				else {
					OperatorType ot = OT_LAST;
					// check for operator token
					if (tok.size() == 1 && is_element(*tok.begin, operator_characters)) {
						// try to extend to two character operator
						token op_tok = tok;
						if (i + 1 < input_tokens.size() && input_tokens[i + 1].size() == 1 &&
							is_element(*input_tokens[i + 1].begin, operator_characters)) {
							ot = get_operator_type(to_string(tok) + to_string(input_tokens[i + 1]));
							if (ot != OT_LAST) {
								++i;
								op_tok.end = input_tokens[i].end;
							}
						}
						if (ot == OT_LAST)
							ot = get_operator_type(to_string(tok));
						if (ot != OT_LAST) {
							expression_tokens.push_back(expression_token(op_tok, ot));
							if (debug_parse)
								std::cout << i << " : operator " << get_operator_word(ot) << std::endl;
						}
					}
					if (ot == OT_LAST) {
						int vi;
						double d;
						token dbl_tok = tok;
						unsigned j = i + 1;
						bool considered_dot = false;
						bool considered_exp = false;
						bool considered_sign = false;
						while (j < input_tokens.size()) {
							bool can_extend = true;
							for (const char* p = dbl_tok.end; p < input_tokens[j].end; ++p) {
								if (!considered_dot && *p == '.')
									considered_dot = true;
								else if (!considered_exp && (*p == 'E' || *p == 'e'))
									considered_exp = true;
								else if (!considered_sign && (*p == '+' || *p == '-'))
									considered_sign = true;
								else if (*p < '0' || *p > '9') {
									can_extend = false;
									break;
								}
							}
							if (can_extend) {
								dbl_tok.end = input_tokens[j].end;
								++j;
							}
							else
								break;
						}
						bool rather_int = !considered_dot && !considered_exp && !considered_sign;
						if (!rather_int && dbl_tok[0] >= '0' && dbl_tok[0] <= '9' && is_double(to_string(dbl_tok), d)) {
							std::string t = to_string(dbl_tok);
							expression_tokens.push_back(expression_token(dbl_tok, variant(d)));
							if (debug_parse)
								std::cout << i << "-" << j - 1 << " : double tokens = " << d << std::endl;
							i = j - 1;
						}
						else if (is_integer(to_string(tok), vi)) {
							expression_tokens.push_back(expression_token(tok, variant(vi)));
							if (debug_parse)
								std::cout << i << " : int token = " << vi << std::endl;
						}
						else {
							variant v(NAME_VALUE, to_string(tok));
							expression_tokens.push_back(expression_token(tok, v));
							if (debug_parse)
								std::cout << i << " : variable >" << to_string(tok) << "< = " <<
								v.get_str() << std::endl;
						}
					}
				}
			}
			return true;
		}

		void expression_processor::prepare()
		{
			found_error = false;
			issued_error = false;
			//	expression_stack.clear();
			//	parenthesis_stack.clear();
			//	while (!value_stack.empty()) value_stack.pop();
			//	while (!operator_stack.empty()) operator_stack.pop();
		}

		void expression_processor::extract_begins(std::vector<unsigned>& begins, unsigned i0, unsigned ie) const
		{
			if (ie == -1)
				ie = (unsigned)expression_tokens.size();
			int parenthesis = 0;
			if (expression_tokens.size() > i0)
				begins.push_back(i0);
			for (unsigned i = i0; i < ie; ++i) {
				const expression_token& et = expression_tokens[i];
				switch (et.ep) {
				case EP_VALUE:
				case EP_OPERATOR:
					break;
				case EP_OPEN:
				case EP_FUNC:
				case EP_LIST_OPEN:
				case EP_LIST_ACCESS:
					++parenthesis;
					break;
				case EP_CLOSE:
				case EP_LIST_CLOSE:
					--parenthesis;
					break;
				case EP_COMMA:
					if (parenthesis == 0 && i + 1 < ie)
						begins.push_back(i + 1);
					break;
				}
			}
		}

		int expression_processor::classify_call(unsigned closing_i) const
		{
			unsigned opening_i = closing_i - 1;
			unsigned parenthesis = 1;
			do {
				const expression_token& et = expression_tokens[opening_i];
				switch (et.ep) {
				case EP_OPEN:
				case EP_FUNC:
				case EP_LIST_OPEN:
				case EP_LIST_ACCESS:
					--parenthesis;
					break;
				case EP_CLOSE:
				case EP_LIST_CLOSE:
					++parenthesis;
					break;
				default:
					break;
				}
				if (parenthesis == 0)
					break;
				--opening_i;
			} while (true);
			std::vector<unsigned> begins;
			extract_begins(begins, opening_i + 1, closing_i);
			unsigned nr_special = 0;
			for (unsigned i = 0; i < begins.size(); ++i) {
				unsigned j = begins[i];
				if (expression_tokens[j].ep != EP_OPERATOR || expression_tokens[j].ot != OT_MAP_DOWN)
					continue;
				++j;
				if (j >= expression_tokens.size() || expression_tokens[j].ep != EP_VALUE || expression_tokens[j].value.get_type() != NAME_VALUE)
					continue;
				++j;
				if (j >= expression_tokens.size() || expression_tokens[j].ep != EP_OPERATOR || (expression_tokens[j].ot != OT_ASSIGN && expression_tokens[j].ot != OT_ASSIGN_REF))
					continue;
				++nr_special;
			}
			if (nr_special == begins.size())
				return 1;
			if (nr_special == 0)
				return 0;
			return -1;
		}

		bool expression_processor::assign_func_decl(const std::vector<variant>& values) const
		{
			std::vector<unsigned> begins;
			extract_begins(begins);
			if (values.size() > begins.size()) {
				last_error = "function called with more parameters than declared by the function";
				last_error_token = expression_tokens.back();
				return false;
			}
			for (unsigned i = 0; i < values.size(); ++i) {
				variant& v = ref_variable(expression_tokens[begins[i] + 1].value.get_name(), true);
				v = values[i];
				if (check_cyclic_reference(&v)) {
					last_error = "a call to this function generated cyclic reference to variable ";
					if (v.is_name())
						last_error += v.get_name();
					last_error += ". Make sure to use map down operator <:";
					if (v.is_name())
						last_error += v.get_name();
					last_error += " to mark special function call syntax in the function call referenced in the next error message.";
					last_error_token = expression_tokens.back();
					return false;
				}
			}
			return true;
		}


		bool expression_processor::is_func_decl() const
		{
			std::vector<unsigned> begins;
			extract_begins(begins);
			for (unsigned i = 0; i < begins.size(); ++i) {
				unsigned j = begins[i];
				if (expression_tokens[j].ep != EP_OPERATOR || expression_tokens[j].ot != OT_MAP_UP) {
					last_error = "function declaration expression must be a comma separated list of expressions of the form ':>var_name=...' or ':>var_name=&...' (:> missing)";
					last_error_token = expression_tokens[j];
					return false;
				}
				++j;
				if (j >= expression_tokens.size() || expression_tokens[j].ep != EP_VALUE || expression_tokens[j].value.get_type() != NAME_VALUE) {
					last_error = "function declaration expression must be a comma separated list of expressions of the form ':>var_name=...' or ':>var_name=&...' (var_name missing)";
					if (j < expression_tokens.size())
						last_error_token = expression_tokens[j];
					else
						last_error_token = expression_tokens.back();
					return false;
				}
				++j;
				if (j >= expression_tokens.size() || expression_tokens[j].ep != EP_OPERATOR || (expression_tokens[j].ot != OT_ASSIGN && expression_tokens[j].ot != OT_ASSIGN_REF)) {
					last_error = "function declaration expression must be a comma separated list of expressions of the form ':>var_name=...' or ':>var_name=&...' (= or =& missing)";
					if (j < expression_tokens.size())
						last_error_token = expression_tokens[j];
					else
						last_error_token = expression_tokens.back();
					return false;
				}
			}
			return true;
		}

		bool expression_processor::validate(bool allow_several_values)
		{
			if (expression_tokens.size() == 0)
				return true;
			prepare();

			/// store for each started expression the sizes of the value and operator stacks before this expression
			std::vector<expr_stack_entry> expression_stack;
			/// store for each started expression the expression part that started the expression, i.e. open parentheses or commas 
			std::vector<ExpressionPart> parenthesis_stack;
			/// stack of values that will still be processed 
			std::stack<variant>        value_stack;
			/// stack of operators that separate the not yet processed values
			std::stack<OperatorType>   operator_stack;

			bool left_of_operand = true;
			unsigned int i;
			for (i = 0; i < expression_tokens.size(); ++i) {
				expression_token& et = expression_tokens[i];
				switch (et.ep) {
				case EP_VALUE:
					if (left_of_operand) {
						value_stack.push(et.value);
						left_of_operand = false;
					}
					else {
						last_error = "two successive values in expression without operator";
						last_error_token = expression_tokens[i];
						return false;
					}
					break;
				case EP_OPERATOR:
					// correct binary sub to unary if necessary
					if (get_operator_arity(et.ot) == 2 && left_of_operand && et.ot == OT_SUB)
						et.ot = OT_NEGATE;
					// correct binary map to unary if necessary
					if (get_operator_arity(et.ot) == 2 && left_of_operand && et.ot == OT_BINARY_MAP)
						et.ot = OT_UNARY_MAP;

					if (get_operator_arity(et.ot) == 1) {
						if (get_operator_location(et.ot) == OL_PREFIX) {
							if (left_of_operand) {
								operator_stack.push(et.ot);
							}
							else {
								last_error = std::string("unary prefix operator ") +
									get_operator_word(et.ot) +
									" cannot follow operand";
								last_error_token = expression_tokens[i];
								return false;
							}
						}
						else {
							if (left_of_operand) {
								last_error = std::string("unary postfix operator ") +
									get_operator_word(et.ot) +
									" cannot preceed operand";
								last_error_token = expression_tokens[i];
								return false;
							}
							else {
								operator_stack.push(et.ot);
							}
						}
					}
					else {
						if (left_of_operand) {
							last_error = std::string("binary operator ") +
								get_operator_word(et.ot) +
								" is missing left argument";
							last_error_token = expression_tokens[i];
							return false;
						}
						else {
							if (!compress_stack_validate(get_operator_priority(et.ot), expression_stack, parenthesis_stack, value_stack, operator_stack))
								return false;
							operator_stack.push(et.ot);
							left_of_operand = true;
						}
					}
					break;
				case EP_OPEN:
				case EP_FUNC:
					expression_stack.push_back(expr_stack_entry((unsigned int)value_stack.size(), (unsigned int)operator_stack.size()));
					if (left_of_operand) {
						parenthesis_stack.push_back(EP_OPEN);
					}
					else {
						if (!compress_stack_validate(get_operator_priority(OT_MAP_UP), expression_stack, parenthesis_stack, value_stack, operator_stack))
							return false;
						expression_tokens[i].ep = EP_FUNC;
						parenthesis_stack.push_back(EP_FUNC);
						left_of_operand = true;
					}
					break;
				case EP_CLOSE:
				{
					ExpressionPart last_parenthesis = find_last_parenthesis(expression_stack, parenthesis_stack);
					if (last_parenthesis == EP_OPEN && left_of_operand) {
						last_error = "closing paranthesis cannot init term";
						last_error_token = expression_tokens[i];
						return false;
					}
					if (last_parenthesis != EP_OPEN && last_parenthesis != EP_FUNC) {
						last_error = "wrongly or not matched parenthesis )";
						last_error_token = expression_tokens[i];
						return false;
					}
					if (!left_of_operand) {
						if (!compress_stack_validate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack)) {
							last_error_token = expression_tokens[i];
							return false;
						}
						if ((last_parenthesis != EP_FUNC) && (value_stack.size() <= expression_stack.back().first)) {
							last_error = "no resulting value in expression";
							last_error_token = expression_tokens[i];
							return false;
						}
						// if more than one comma separated expressions are defined, only leave last value on the value stack
						while (get_nr_comma_separated_expressions(expression_stack, parenthesis_stack, value_stack) > 1) {
							variant tmp = value_stack.top();
							value_stack.pop();
							value_stack.pop();
							value_stack.push(tmp);
							expression_stack.pop_back();
							parenthesis_stack.pop_back();
						}
						if (last_parenthesis == EP_FUNC)
							value_stack.pop();
					}
					else
						left_of_operand = false;
					expression_stack.pop_back();
					parenthesis_stack.pop_back();
				}
				break;
				case EP_LIST_ACCESS:
				case EP_LIST_OPEN:
					expression_stack.push_back(expr_stack_entry((unsigned int)value_stack.size(), (unsigned int)operator_stack.size()));
					if (left_of_operand)
						parenthesis_stack.push_back(EP_LIST_OPEN);
					else {
						if (!compress_stack_validate(get_operator_priority(OT_MAP_UP), expression_stack, parenthesis_stack, value_stack, operator_stack))
							return false;
						expression_tokens[i].ep = EP_LIST_ACCESS;
						parenthesis_stack.push_back(EP_LIST_ACCESS);
						left_of_operand = true;
					}
					break;
				case EP_LIST_CLOSE:
				{
					if (left_of_operand) {
						if (i == 0 || expression_tokens[i - 1].ep != EP_LIST_OPEN) {
							last_error = "closing paranthesis ] cannot init a term";
							last_error_token = expression_tokens[i];
							return false;
						}
						left_of_operand = false;
					}
					ExpressionPart last_parenthesis = find_last_parenthesis(expression_stack, parenthesis_stack);
					if (last_parenthesis != EP_LIST_OPEN &&
						last_parenthesis != EP_LIST_ACCESS) {
						last_error = "wrongly or not matched parenthesis ]";
						last_error_token = expression_tokens[i];
						return false;
					}
					if (!compress_stack_validate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack)) {
						last_error_token = expression_tokens[i];
						return false;
					}
					if (last_parenthesis == EP_LIST_OPEN) {
						unsigned int n = get_nr_comma_separated_expressions(expression_stack, parenthesis_stack, value_stack);
						variant v = variant(variant::list_type());
						if (n > 0) {
							std::vector<variant> tmp;
							tmp.resize(n);
							unsigned int i;
							for (i = 0; i < n; ++i) {
								tmp[n - i - 1] = value_stack.top();
								value_stack.pop();
								expression_stack.pop_back();
								parenthesis_stack.pop_back();
							}
							for (i = 0; i < n; ++i)
								v.append_to_list(tmp[i]);
						}
						else {
							expression_stack.pop_back();
							parenthesis_stack.pop_back();
						}
						value_stack.push(v);
					}
					else {
						unsigned int n = get_nr_comma_separated_expressions(expression_stack, parenthesis_stack, value_stack);
						if (n != 1) {
							last_error = "list access only with exactly one value allowed";
							last_error_token = expression_tokens[i];
							return false;
						}
						value_stack.pop();
						expression_stack.pop_back();
						parenthesis_stack.pop_back();
						if (value_stack.empty()) {
							last_error = "applied list access operator to empty value stack";
							last_error_token = expression_tokens[i];
							return false;
						}
						value_stack.pop();
						value_stack.push(variant());
					}
				}
				break;
				case EP_COMMA:
					if (left_of_operand) {
						last_error = "comma cannot init a term";
						last_error_token = expression_tokens[i];
						return false;
					}
					if (!compress_stack_validate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack)) {
						last_error_token = expression_tokens[i];
						return false;
					}
					expression_stack.push_back(expr_stack_entry((unsigned int)value_stack.size(), (unsigned int)operator_stack.size()));
					parenthesis_stack.push_back(EP_COMMA);
					left_of_operand = true;
					break;
				}
			}
			if (!compress_stack_validate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack))
				return false;
			if (value_stack.empty()) {
				last_error = "expression did not yield a value";
				last_error_token = token(expression_tokens.front().begin, expression_tokens.back().end);
				return false;
			}
			if (!allow_several_values && value_stack.size() > 1) {
				last_error = "expression yielded more than one value";
				last_error_token = token(expression_tokens.front().begin, expression_tokens.back().end);
				return false;
			}
			return true;
		}


		bool expression_processor::evaluate(variant& result, ph_processor* ph_proc)
		{
			if (expression_tokens.size() == 0) {
				//		value_stack.push(variant(0));
				return true;
			}
			prepare();

			/// store for each started expression the sizes of the value and operator stacks before this expression
			std::vector<expr_stack_entry> expression_stack;
			/// store for each started expression the expression part that started the expression, i.e. open parentheses or commas 
			std::vector<ExpressionPart> parenthesis_stack;
			/// stack of values that will still be processed 
			std::stack<variant>        value_stack;
			/// stack of operators that separate the not yet processed values
			std::stack<OperatorType>   operator_stack;

			unsigned int i;
			bool empty_func_arg = false;
			for (i = 0; i < expression_tokens.size(); ++i) {
				expression_token& et = expression_tokens[i];
				switch (et.ep) {
				case EP_VALUE:
					value_stack.push(et.value);
					empty_func_arg = false;
					break;
				case EP_OPERATOR:
					empty_func_arg = false;
					if (get_operator_arity(et.ot) == 1) {
						operator_stack.push(et.ot);
					}
					else {
						compress_stack_evaluate(get_operator_priority(et.ot), expression_stack, parenthesis_stack, value_stack, operator_stack);
						operator_stack.push(et.ot);
					}
					break;
				case EP_OPEN:
					expression_stack.push_back(expr_stack_entry((unsigned int)value_stack.size(), (unsigned int)operator_stack.size()));
					parenthesis_stack.push_back(EP_OPEN);
					break;
				case EP_FUNC:
					compress_stack_evaluate(get_operator_priority(OT_MAP_UP), expression_stack, parenthesis_stack, value_stack, operator_stack);
					empty_func_arg = true;
					expression_stack.push_back(expr_stack_entry((unsigned int)value_stack.size(), (unsigned int)operator_stack.size()));
					parenthesis_stack.push_back(EP_FUNC);
					if (value_stack.top().is_name() || value_stack.top().is_str() || value_stack.top().is_func()) {
						variant* func_var;
						if (value_stack.top().is_func())
							func_var = &value_stack.top().ref_value();
						else {
							std::string func_name = (value_stack.top().is_str() ? value_stack.top().get_str() : value_stack.top().get_name());
							func_var = &ref_variable(func_name);
						}
						if (func_var->is_func()) {
							func_type& func = func_var->ref_func();
							push_namespace(0, func.ns);
							ref_variable("return", true);
							variant tmp;
							if (func.ph_proc->commands[func.block_begin - 1].expressions.size() == 2) {
								ph_proc->swap_output(*func.ph_proc);
								expression_processor& ep = func.ph_proc->commands[func.block_begin - 1].expressions[1];
								if (!ep.evaluate(tmp, ph_proc)) {
									if (!ep.issued_error) {
										ph_proc->error(ep.last_error, ep.last_error_token);
										ep.issued_error = true;
									}
									ph_proc->swap_output(*func.ph_proc);
									last_error = "previous error was generated while evaluating this expression";
									last_error_token = expression_tokens[i];
									found_error = true;
									return false;
								}
								ph_proc->swap_output(*func.ph_proc);
								if (func.ph_proc->exit_code != 0) {
									ph_proc->exit_code = func.ph_proc->exit_code;
									return true;
								}
							}
							goto_parent_namespace();
						}
						else {
							last_error = "function call on variable not of type func";
							last_error_token = expression_tokens[i];
							found_error = true;
							return false;
						}
					}
					else {
						last_error = "function call on non name or string expression";
						last_error_token = expression_tokens[i];
						found_error = true;
						return false;
					}
					break;
				case EP_CLOSE:
				{
					ExpressionPart last_parenthesis = find_last_parenthesis(expression_stack, parenthesis_stack);
					if (last_parenthesis != EP_FUNC) {
						if (!compress_stack_evaluate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack)) {
							last_error_token = expression_tokens[i];
							found_error = true;
							return false;
						}
						// if more than one comma separated expressions are defined, only leave last value on the value stack
						while (get_nr_comma_separated_expressions(expression_stack, parenthesis_stack, value_stack) > 1) {
							variant tmp = value_stack.top();
							value_stack.pop();
							value_stack.pop();
							value_stack.push(tmp);
							expression_stack.pop_back();
							parenthesis_stack.pop_back();
						}
						expression_stack.pop_back();
						parenthesis_stack.pop_back();
					}
					else { // function call
						std::vector<variant> arguments;
						if (!empty_func_arg) {
							if (!compress_stack_evaluate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack)) {
								last_error_token = expression_tokens[i];
								found_error = true;
								return false;
							}
							unsigned argc = get_nr_comma_separated_expressions(expression_stack, parenthesis_stack, value_stack);
							int cc = classify_call(i);
							if (cc == -1) {
								last_error = "error calling a function mixing special and standard syntax";
								last_error_token = et;
								found_error = true;
								return false;
							}
							if (cc == 0) {
								arguments.resize(argc);
								for (unsigned i = 0; i < argc; ++i) {
									arguments[argc - 1 - i] = value_stack.top();
									if (arguments[argc - 1 - i].get_type() == NAME_VALUE) {
										arguments[argc - 1 - i] = variant(&ref_variable(arguments[argc - 1 - i].get_name()));
									}
									value_stack.pop();
									expression_stack.pop_back();
									parenthesis_stack.pop_back();
								}
							}
							else {
								for (unsigned i = 0; i < argc; ++i) {
									value_stack.pop();
									expression_stack.pop_back();
									parenthesis_stack.pop_back();
								}
							}
						}
						else {
							expression_stack.pop_back();
							parenthesis_stack.pop_back();
						}
						if (value_stack.top().is_name() || value_stack.top().is_str() || value_stack.top().is_func()) {
							variant* func_var;
							if (value_stack.top().is_func())
								func_var = &value_stack.top().ref_value();
							else {
								std::string func_name = (value_stack.top().is_str() ? value_stack.top().get_str() : value_stack.top().get_name());
								variant* func_var = &ref_variable(func_name);
							}
							if (func_var->is_func()) {
								goto_child_namespace();
								func_type& func = func_var->ref_func();
								ph_proc->swap_output(*func.ph_proc);
								if (arguments.size() > 0 && func.ph_proc->commands[func.block_begin - 1].expressions.size() == 2) {
									const expression_processor& ep = func.ph_proc->commands[func.block_begin - 1].expressions[1];
									if (!ep.assign_func_decl(arguments)) {
										func.ph_proc->error(ep.get_last_error(), ep.get_last_error_token());
										ph_proc->swap_output(*func.ph_proc);
										last_error = "error appeared in the following function call:";
										last_error_token = et;
										found_error = true;
										return false;
									}
								}
								else if (arguments.size() > 0) {
									ph_proc->swap_output(*func.ph_proc);
									last_error = "error calling a function without arguments passing values";
									last_error_token = et;
									found_error = true;
									return false;
								}
								if (!func.ph_proc->process(func.block_begin, func.block_end)) {
									ph_proc->swap_output(*func.ph_proc);
									last_error = "error in function call";
									last_error_token = expression_tokens[i];
									found_error = true;
									return false;
								}
								ph_proc->swap_output(*func.ph_proc);
								value_stack.pop();
								value_stack.push(ref_variable("return", true));
								pop_namespace();
							}
							else {
								last_error = "function call on variable not of type proc";
								last_error_token = expression_tokens[i];
								found_error = true;
								return false;
							}
						}
						else {
							last_error = "function call on non name or string expression";
							last_error_token = expression_tokens[i];
							found_error = true;
							return false;
						}
					}
					empty_func_arg = false;
				}
				break;
				case EP_LIST_OPEN:
					expression_stack.push_back(expr_stack_entry((unsigned int)value_stack.size(), (unsigned int)operator_stack.size()));
					parenthesis_stack.push_back(EP_LIST_OPEN);
					break;
				case EP_LIST_ACCESS:
					compress_stack_evaluate(get_operator_priority(OT_MAP_UP), expression_stack, parenthesis_stack, value_stack, operator_stack);
					expression_stack.push_back(expr_stack_entry((unsigned int)value_stack.size(), (unsigned int)operator_stack.size()));
					parenthesis_stack.push_back(EP_LIST_ACCESS);
					break;
				case EP_LIST_CLOSE:
				{
					ExpressionPart last_parenthesis = find_last_parenthesis(expression_stack, parenthesis_stack);
					if (!compress_stack_evaluate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack)) {
						last_error_token = expression_tokens[i];
						found_error = true;
						return false;
					}
					if (last_parenthesis == EP_LIST_OPEN) {
						unsigned int n = get_nr_comma_separated_expressions(expression_stack, parenthesis_stack, value_stack);
						variant v = variant(variant::list_type());
						if (n > 0) {
							std::vector<variant> tmp;
							tmp.resize(n);
							unsigned int i;
							for (i = 0; i < n; ++i) {
								tmp[n - i - 1] = value_stack.top();
								value_stack.pop();
								expression_stack.pop_back();
								parenthesis_stack.pop_back();
							}
							for (i = 0; i < n; ++i)
								v.append_to_list(tmp[i].get_value());
						}
						else {
							expression_stack.pop_back();
							parenthesis_stack.pop_back();
						}
						value_stack.push(v);
					}
					else {
						variant var_idx = value_stack.top();
						value_stack.pop();
						expression_stack.pop_back();
						parenthesis_stack.pop_back();
						if (value_stack.top().is_map()) {
							std::string var_name;
							if (var_idx.is_int()) {
								var_name = value_stack.top().get_element_name(var_idx.get_int());
								value_stack.pop();
								value_stack.push(var_name);
							}
							else {
								if (var_idx.is_str())
									var_name = var_idx.get_str();
								else {
									last_error = "map access key evaluate to name or string";
									last_error_token = expression_tokens[i];
									found_error = true;
									return false;
								}
								variant tmp;
								if (value_stack.top().is_reference()) {
									tmp = variant(&value_stack.top().ref_element(var_name));
								}
								else
									tmp = value_stack.top().get_element(var_name);
								value_stack.pop();
								value_stack.push(tmp);
							}
						}
						else {
							int idx = var_idx.get_int();
							if (value_stack.top().is_list()) {
								if (idx < 0)
									idx += (int)value_stack.top().get_size();
								if (idx < 0 || idx >= (int)value_stack.top().get_size()) {
									last_error = "list access with index ";
									last_error += to_string(idx) + " out of valid range [0," +
										to_string(value_stack.top().get_size()) + "]";
									last_error_token = expression_tokens[i];
									found_error = true;
									return false;
								}
								variant tmp;
								if (value_stack.top().is_reference()) {
									tmp = variant(&value_stack.top().ref_element(idx));
								}
								else
									tmp = value_stack.top().get_element(idx);
								value_stack.pop();
								value_stack.push(tmp);
							}
							else if (value_stack.top().is_str()) {
								int n = (int)value_stack.top().get_str().size();
								if (idx < 0)
									idx += n;
								if (idx < 0 || idx >= n) {
									last_error = "string access with index ";
									last_error += to_string(idx) + " out of valid range [" +
										to_string(-n) + "," + to_string(n - 1) + "]";
									last_error_token = expression_tokens[i];
									found_error = true;
									return false;
								}
								variant tmp(std::string(1, value_stack.top().get_str()[idx]));
								value_stack.pop();
								value_stack.push(tmp);
							}
							else {
								last_error = "applied list access operator to non list type";
								last_error_token = expression_tokens[i];
								found_error = true;
								return false;
							}
						}
					}
				}
				break;
				case EP_COMMA:
					if (!compress_stack_evaluate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack)) {
						last_error_token = expression_tokens[i];
						found_error = true;
						return false;
					}
					expression_stack.push_back(expr_stack_entry((unsigned int)value_stack.size(), (unsigned int)operator_stack.size()));
					parenthesis_stack.push_back(EP_COMMA);
					break;
				}
			}
			if (!compress_stack_evaluate(-1, expression_stack, parenthesis_stack, value_stack, operator_stack)) {
				last_error_token = token(expression_tokens.front().begin, expression_tokens.back().end);
				found_error = true;
				return false;
			}
			result = value_stack.top();
			if (debug_evaluate)
				std::cout << "expression evaluated to " << result.get_str() << std::endl;
			return true;
		}

		bool expression_processor::compress_stack_evaluate(int priority, std::vector<expr_stack_entry>& expression_stack,
			std::vector<ExpressionPart>& parenthesis_stack,
			std::stack<variant>& value_stack, std::stack<OperatorType>& operator_stack)
		{
			while (!found_error && !operator_stack.empty()) {
				OperatorType ot = operator_stack.top();
				if (priority > get_operator_priority(ot))
					return true;
				if (priority == get_operator_priority(ot) &&
					get_operator_precedence(ot) == OP_RIGHT)
					return true;
				switch (get_operator_arity(ot)) {
				case 1:
					if (!expression_stack.empty() && operator_stack.size() <= expression_stack.back().second)
						return true;
					if (!value_stack.top().unary_check_defined(operator_stack.top())) {
						last_error = std::string("applied unary operator ") +
							get_operator_word(operator_stack.top()) + " to undefined value";
						found_error = true;
					}
					else if (!value_stack.top().is_unary_applicable(operator_stack.top())) {
						last_error = std::string("could not apply unary operator ") +
							get_operator_word(ot);
						found_error = true;
					}
					else {
						value_stack.top().apply_unary(operator_stack.top());
						if (check_cyclic_reference(&value_stack.top())) {
							last_error = std::string("generated cyclic reference");
							found_error = true;
						}
						if (debug_evaluate)
							std::cout << "applied unary operator " << get_operator_word(operator_stack.top()) << std::endl;
					}
					operator_stack.pop();
					break;
				case 2:
					if (!expression_stack.empty() && operator_stack.size() <= expression_stack.back().second)
						return true;
					else {
						variant v2 = value_stack.top();
						value_stack.pop();
						if (!value_stack.top().is_binary_applicable(operator_stack.top(), v2)) {
							last_error = std::string("could not apply binary operator ") +
								get_operator_word(ot);
							found_error = true;
						}
						if (!value_stack.top().binary_check_defined(operator_stack.top(), v2)) {
							last_error = std::string("applied binary operator ") +
								get_operator_word(operator_stack.top()) + " to undefined value";
							found_error = true;
						}
						value_stack.top().apply_binary(operator_stack.top(), v2);
						if (check_cyclic_reference(&value_stack.top())) {
							last_error = std::string("generated cyclic reference");
							found_error = true;
						}
						if (debug_evaluate)
							std::cout << "applied binary operator " << get_operator_word(operator_stack.top()) << std::endl;
						operator_stack.pop();
						break;
					}
				}
			}
			return !found_error;
		}

		bool expression_processor::compress_stack_validate(int priority, std::vector<expr_stack_entry>& expression_stack,
			std::vector<ExpressionPart>& parenthesis_stack,
			std::stack<variant>& value_stack, std::stack<OperatorType>& operator_stack)
		{
			while (!operator_stack.empty()) {
				OperatorType ot = operator_stack.top();
				if (priority > get_operator_priority(ot))
					return true;
				if (priority == get_operator_priority(ot) &&
					get_operator_precedence(ot) == OP_RIGHT)
					return true;
				switch (get_operator_arity(ot)) {
				case 1:
					if (!expression_stack.empty() && operator_stack.size() <= expression_stack.back().second)
						return true;
					if (value_stack.empty()) {
						last_error = std::string("no value to apply unary operator ") +
							get_operator_word(ot);
						return false;
					}
					operator_stack.pop();
					break;
				case 2:
					if (!expression_stack.empty() && operator_stack.size() <= expression_stack.back().second)
						return true;
					if (value_stack.size() < 2) {
						last_error = std::string("no 2 values to apply binary operator ") +
							get_operator_word(ot);
						return false;
					}
					else {
						variant v2 = value_stack.top();
						value_stack.pop();
						operator_stack.pop();
						break;
					}
				}
			}
			return true;
		}

		const std::string& expression_processor::get_last_error() const
		{
			return last_error;
		}

		const token& expression_processor::get_last_error_token() const
		{
			return last_error_token;
		}

	}
}
