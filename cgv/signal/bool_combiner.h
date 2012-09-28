#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace signal {

/// base class for signals that combine the boolean result of the attached functors with boolean and/or operations
class CGV_API bool_combiner
{
protected:
	bool combine_result(bool new_value, bool& value) const;
	bool combine_with_and;
	bool short_circuit;
public:
	//! construct from option string.
	/*! The option string can be composed of the following characters:
	
	- '&' ... combine with logical and
	- '|' ... combine with logical or
	- '*' ... short-circuit evaluation of boolean expression
	- '+' ... full evaluation
	*/
	bool_combiner(const char* opt);
	/// return the neutral bool value with which one can initialize the result variable
	bool get_neutral_value() const;
	/// set a different option string
	void set_options(const char* opt);
	/// return whether combination is done with the boolean and operation
	bool is_combination_with_and() const;
	/// return whether to perform short-circuit evaluation
	bool is_evaluation_short_circuit() const;
};
	}
}
#include <cgv/config/lib_end.h>