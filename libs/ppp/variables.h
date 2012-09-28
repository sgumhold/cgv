#pragma once

#include <string>
#include <vector>
#include "variant.h"

#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace text {
			namespace ppp {

/** for each namespace the following information is stored
- whether the namespace is a real namespace or the namespace created by a function call 
- the environment namespace whose symbols are visible in the current namespace
- the parent namespace that triggered creation of this namespace and will be activated on destruction
- the child namespace which is the next namespace created in the current namespace
- the map from names to variants
*/
struct namespace_info
{
	bool is_function_call_ns;
	variant::map_type* ns;
	bool ns_allocated;
	namespace_info* environment_ns;
	namespace_info* parent_ns;
	namespace_info* child_ns;

	namespace_info* get_environment() {
		if (is_function_call_ns)
			return environment_ns;
		return this;
	}
	// construct from central information
	namespace_info(bool _is_function_call_ns, variant::map_type* _ns = 0, namespace_info* _environment_ns = 0, namespace_info* _parent_ns = 0);
	// destruct variable map in case of function call namespaces
	~namespace_info();
};

/// return the current namespace info
extern CGV_API namespace_info* get_current_namespace();

/// set the current namespace info
extern CGV_API void set_current_namespace(namespace_info* _cns);

//! find a variable without automatic declaration. 
/*! If it does not exist, return null pointer. 
    If the parameter only_current is true, only the current namespace is searched. */
extern CGV_API variant* find_variable(const std::string& name, bool only_current = false);

/// find a variable and return reference to it. If it does not exist, create it in the current namespace.
extern CGV_API variant& ref_variable(const std::string& name, bool only_current = false);

/// remove all variables and namespaces
extern CGV_API void clear_variables();

/// initialize the variables such that "env" is a map with all environment variables and "ARGS" as a list with all command line arguments
extern CGV_API void init_environment(int argc, char** argv);

/// create a new namespace as a function call namespace if ns_var is not given or as a standard namespace with name map in ns_var otherwise
extern CGV_API void push_namespace(variant* ns_var, namespace_info* environment_ns = 0);

/// pop a namespace from the i-th namespace stack
extern CGV_API void pop_namespace();

/// return whether the current namespace has a child namespace 
extern CGV_API bool has_child_namespace();

/// make the child namespace of the current namespace current
extern CGV_API void goto_child_namespace();

/// return whether the current namespace has a parent namespace 
extern CGV_API bool has_parent_namespace();

/// make the parent namespace of the current namespace current
extern CGV_API void goto_parent_namespace();

			}
		}
	}
}
#include <cgv/config/lib_end.h>