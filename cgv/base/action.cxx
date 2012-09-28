#include <cgv/base/action.h>

namespace cgv {
	namespace base {

action::action(bool _default_result_begin, bool _default_result_end)
: default_result_begin(_default_result_begin), default_result_end(_default_result_end)
{
}

/// sets the value that is returned, whenever no result is obtained from the traversed methods
void action::set_default_results(bool _default_result) 
{
	default_result_begin = default_result_end = _default_result; 
}

/// sets the value that is returned for on_begin events
void action::set_default_result_begin(bool _default_result_begin) 
{
	default_result_begin = _default_result_begin; 
}

/// sets the value that is returned for on_end events
void action::set_default_result_end(bool _default_result_end) 
{
	default_result_end = _default_result_end; 
}

void action::select(base_ptr)
{
}

bool action::implements_action() const
{
	return false;
}

traverse_policy* action::get_policy() const
{
	return 0;
}

bool action::begin()
{
	return default_result_begin;
}

bool action::end()
{
	return default_result_end;
}

/// check whether the action has registered a single begin method or both begin and end methods
bool action::has_begin_only() const 
{ 
	return false; 
}


	}
}
