#include "variables.h"
#include <map>
#include <iostream>
#include <stdlib.h>

#ifdef WIN32
#pragma warning (disable:4503)
#else
extern char **environ;
#endif


namespace cgv {
	namespace ppp {

		namespace_info::namespace_info(bool _is_function_call_ns, variant::map_type* _ns, namespace_info* _environment_ns, namespace_info* _parent_ns) :
			is_function_call_ns(_is_function_call_ns), ns(_ns), ns_allocated(_ns == 0), environment_ns(_environment_ns), parent_ns(_parent_ns)
		{
			child_ns = 0;
			if (ns_allocated)
				ns = new variant::map_type();
		}

		namespace_info::~namespace_info()
		{
			if (ns_allocated) {
				delete ns;
				ns = 0;
			}
		}


		namespace_info*& ref_current_namespace()
		{
			static namespace_info* current_namespace = 0;
			if (!current_namespace) {
				current_namespace = new namespace_info(false);
				ref_variable("UNDEF") = variant();
				ref_variable("BOOL") = variant(true);
				ref_variable("INT") = variant(7);
				ref_variable("STRING") = variant(std::string());
				ref_variable("LIST") = variant(std::vector<variant>());
				ref_variable("MAP") = variant(variant::map_type());
				ref_variable("FUNC") = variant(func_type(-1, -2));
#ifdef WIN32
				ref_variable("SYSTEM") = variant(std::string("windows"));
#else
				ref_variable("SYSTEM") = variant(std::string("linux"));
#endif
			}
			return current_namespace;
		}

		namespace_info* get_current_namespace()
		{
			return ref_current_namespace();
		}

		/// set the current namespace info
		void set_current_namespace(namespace_info* _cns)
		{
			ref_current_namespace() = _cns;
		}


		variant* find_variable(const std::string& var_name, bool only_current)
		{
			namespace_info* cns = ref_current_namespace();
			while (cns) {
				variant::map_type::iterator iter = cns->ns->find(var_name);
				if (iter != cns->ns->end())
					return &iter->second;
				if (only_current)
					return 0;
				cns = cns->environment_ns;
			}
			return 0;
		}

		variant& ref_variable(const std::string& var_name, bool only_current)
		{
			variant* var = find_variable(var_name, only_current);
			if (var)
				return *var;
			return (*ref_current_namespace()->ns)[var_name];
		}

		void remove_namespace(namespace_info* ns)
		{
			if (ns->child_ns) {
				ns->child_ns->parent_ns = 0;
				remove_namespace(ns->child_ns);
			}
			namespace_info* pns = ns->parent_ns;
			delete ns;
			if (pns) {
				pns->child_ns = 0;
				remove_namespace(pns);
			}
		}

		/// remove all variables and namespace stacks from the stack of namespace stacks
		void clear_variables()
		{
			remove_namespace(ref_current_namespace());
			ref_current_namespace() = 0;
		}


		void push_namespace(variant* ns_var, namespace_info* environment_ns)
		{
			namespace_info *ns, *cns = ref_current_namespace();
			if (environment_ns)
				ns = new namespace_info(true, 0, environment_ns, cns);
			else {
				if (cns->is_function_call_ns)
					ns = new namespace_info(false, &ns_var->ref_map(), cns, cns);
				else
					ns = new namespace_info(false, &ns_var->ref_map(), cns->get_environment(), cns);
			}
			ns->child_ns = cns->child_ns;
			if (ns->child_ns)
				ns->child_ns->parent_ns = ns;
			cns->child_ns = ns;
			ref_current_namespace() = ns;
		}

		void pop_namespace()
		{
			namespace_info* cns = ref_current_namespace();
			if (!cns->parent_ns) {
				std::cerr << "pop_namespace called without a pushed namespace available" << std::endl;
				return;
			}
			cns->parent_ns->child_ns = cns->child_ns;
			if (cns->child_ns)
				cns->child_ns->parent_ns = cns->parent_ns;
			ref_current_namespace() = cns->parent_ns;
			delete cns;
		}

		/// return whether the current namespace has a child namespace 
		bool has_child_namespace()
		{
			return ref_current_namespace()->child_ns != 0;
		}

		/// make the child namespace of the current namespace current
		void goto_child_namespace()
		{
			namespace_info* cns = ref_current_namespace();
			if (cns->child_ns)
				ref_current_namespace() = cns->child_ns;
			else
				std::cerr << "goto_child_namespace called without a child namespace available" << std::endl;
		}

		/// return whether the current namespace has a parent namespace 
		bool has_parent_namespace()
		{
			return ref_current_namespace()->parent_ns != 0;
		}

		/// make the parent namespace of the current namespace current
		void goto_parent_namespace()
		{
			namespace_info* cns = ref_current_namespace();
			if (cns->parent_ns)
				ref_current_namespace() = cns->parent_ns;
			else
				std::cerr << "goto_parent_namespace called without a parent namespace available" << std::endl;
		}


		void init_environment(int argc, char** argv)
		{
			variant& env = ref_variable("env");
			env = variant(variant::map_type());
			variant::map_type& env_map = env.ref_map();
			char** glob_vars = environ;
			while (*glob_vars != NULL) {
				std::string def(*glob_vars), var_name, value;
				size_t pos = def.find_first_of("=");
				if (pos == std::string::npos)
					var_name = def;
				else {
					var_name = def.substr(0, pos);
					value = def.substr(pos + 1);
				}
				env_map[var_name] = variant(value);
				++glob_vars;
			}
			variant::list_type& arg_list = (env_map["ARGS"] = variant(variant::list_type())).ref_list();
			for (int i = 0; i < argc; ++i)
				arg_list.push_back(variant(std::string(argv[i])));
		}

	}
}
