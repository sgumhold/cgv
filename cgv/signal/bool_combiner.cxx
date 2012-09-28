#include <iostream>
#include <cgv/signal/bool_combiner.h>

namespace cgv {
	namespace signal {

/// base class for signals that combine the boolean result of the attached functors with boolean and/or operations
bool bool_combiner::combine_result(bool new_value, bool& value) const
{
	if (combine_with_and) {
		value &= new_value;
		return short_circuit && !value;
	}
	value |= new_value;
	return short_circuit && value;
}

bool_combiner::bool_combiner(const char* opt)
{
	short_circuit = true;
	combine_with_and = true;
	set_options(opt);
}

/// return the neutral bool value with which one can initialize the result variable
bool bool_combiner::get_neutral_value() const
{
	return combine_with_and;
}


/// set a different option string
void bool_combiner::set_options(const char* opt)
{
	std::string s(opt);
	for (int i=0; i<(int)s.size(); ++i) {
		switch (s[i]) {
		case '*' : short_circuit = true;  break;
		case '+' : short_circuit = false; break;
		case '&' : combine_with_and = true; break;
		case '|' : combine_with_and = false; break;
		default: std::cerr << "unkown option character '" << s[i] << "' in constructor of cgv::signal::bool_combiner" << std::endl;
		}
	}
}

/// return whether combination is done with the boolean and operation
bool bool_combiner::is_combination_with_and() const
{
	return combine_with_and;
}
/// return whether to perform short-circuit evaluation
bool bool_combiner::is_evaluation_short_circuit() const
{
	return short_circuit;
}

	}
}