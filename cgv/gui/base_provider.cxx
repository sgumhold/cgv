#include "base_provider.h"
#include <cgv/utils/file.h>
#include <cgv/gui/application.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/tokenizer.h>
#include <stack>
#include <stdlib.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::utils;

namespace cgv {
	namespace gui {

/// construct from instance and gui definition
base_provider::base_provider(cgv::base::base_ptr _instance, const std::string& gui_def, bool _is_named_gui_assignment)
{
	instance = _instance;
	textual_gui_definition = gui_def;
	toggles = 0;
	nr_toggles = -1;
	is_named_gui_assignment_m = _is_named_gui_assignment;
	parent_type = "align_group";
}

/// construct from gui definition file and instance
base_provider::base_provider(const std::string& file_name, cgv::base::base_ptr _instance)
{
	toggles = 0;
	nr_toggles = -1;
	instance = _instance;
	parent_type = "align_group";
	is_named_gui_assignment_m = true;
	read_gui_definition(file_name);
}
///
base_provider::~base_provider()
{
	if (toggles)
		delete [] toggles;
	toggles = 0;
}

///
std::string base_provider::get_parent_type() const
{
	return parent_type;
}

///
bool base_provider::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 
		rh.reflect_member("parent_type", parent_type) &&
		rh.reflect_member("parent_options", parent_options);
}

std::string base_provider::error_start(const char* ptr) const
{
	std::string error("error (");
	const char* end = ptr;
	if (&textual_gui_definition[textual_gui_definition.size()-1] < ptr)
		end = &textual_gui_definition[textual_gui_definition.size()-1];

	unsigned line_nr = 1;
	for (const char* p = &textual_gui_definition[0]; p<end;++p)
		if (*p == '\n')
			++line_nr;

	error += to_string(line_nr);
	error += ") : ";
	return error;
}

/// overload to return the type name of this object. By default the type interface is queried over get_type.
std::string base_provider::get_type_name() const
{
	if (instance)
		return instance->get_type_name();
	return "base_provider";
}

///
data::ref_ptr<named,true> base_provider::get_named()
{
	if (instance)
		return instance->get_named();
	return named_ptr();
}

///
void base_provider::read_gui_definition(const std::string& file_name)
{
	std::string content;
	if (cgv::utils::file::read(file_name, content, true))
		set_gui_definition(content);
	else
		std::cerr << "could not read gui definition from " << file_name.c_str() << std::endl;
}

///
void base_provider::set_gui_definition(const std::string& new_def)
{
	textual_gui_definition = new_def;

	if (toggles)
		delete [] toggles;
	toggles = 0;
	nr_toggles = -1;

	post_recreate_gui();
}

///
const std::string& base_provider::get_gui_definition() const
{
	return textual_gui_definition;
}

///
void base_provider::set_instance(cgv::base::base_ptr _instance)
{
	instance = _instance;
	post_recreate_gui();
}

///
cgv::base::base_ptr base_provider::get_instance() const
{
	return instance;
}

enum CommandId { 
	CMD_WINDOW, CMD_GROUP, CMD_ALIGN, CMD_DECORATOR, CMD_BUTTON, CMD_VIEW, 
	CMD_CONTROL, CMD_TREE_NODE, CMD_LAST };

struct command_info
{
	/// id of command
	CommandId cmd_id;
	/// command name
	const char* command;
	//! lower case letters for necessary arguments and upper case for optional
	/*! r / R ... reference to reflected member
	    s / S ... string
		b / B ... bool
		i / I ... int
	*/
	/// string with arguments
	const char* arguments;
	/// length of arguments string
	unsigned nr_arguments;
	/// whether command is followed by a {}-block
	bool block_follows;
};

const command_info* get_command_infos()
{
	static command_info infos[] = {
		{ CMD_WINDOW, "window", "iisSS", 5, true },
		{ CMD_GROUP, "group", "ssSS", 4, true },
		{ CMD_ALIGN, "align", "s", 1, false },
		{ CMD_DECORATOR, "decorator", "ssSS", 4, false },
		{ CMD_BUTTON, "button", "sSS", 3, false },
		{ CMD_VIEW, "view", "srSSS", 5, false },
		{ CMD_CONTROL, "control", "srSSS", 5, false },
		{ CMD_TREE_NODE, "tree_node", "sbiS", 4, true },
		{ CMD_LAST, "", "", 0, false }
	};
	return infos;
}

bool base_provider::find_member(const std::string& name, void*& member_ptr, std::string& member_type)
{
	member_ptr = instance->find_member_ptr(name, &member_type);
	return member_ptr != 0;
}


void base_provider::parse_definition(ParsingTasks pt)
{
	std::vector<token> T;
	std::stack<bool> visible;
	std::stack<CommandId> block_cmd;
	std::stack<gui_group_ptr> gui_group_stack;
	visible.push(true);
	bool expect_block_begin = false;

	tokenizer(textual_gui_definition).set_sep(";,(){}").set_skip("'\"","'\"").bite_all(T);
	const command_info* cis  = get_command_infos();

	unsigned i = 0;
	do {
		if (T[i] == ";") {
			++i;
			continue;
		}
		if (expect_block_begin) {
			if (T[i] == "{") {
				expect_block_begin = false;
				++i;
				continue;
			} 
			else
				std::cerr << error_start(T[i].begin) << "expected { after " 
						  << get_command_infos()[block_cmd.top()].command 
						  << " command to enclose child elements" << std::endl;
		}
		if (T[i] == "{") {
			block_cmd.push(CMD_LAST);
			std::cerr << error_start(T[i].begin) << "{ only allowed after window / group or tree_node command" << std::endl;
			++i;
			continue;
		}
		expect_block_begin = false;
		if (T[i] == "}") {
			if (block_cmd.empty())
				std::cerr << error_start(T[i].begin) << "found unmatched }" << std::endl;
			else {
				if (pt == PT_CREATE_GUI) {
					switch (block_cmd.top()) {
					case CMD_TREE_NODE : visible.pop(); break;
					case CMD_GROUP :
					case CMD_WINDOW : gui_group_stack.pop(); break;
					default: break;
					}
				}
			}
			++i;
			continue;
		}

		// determine command
		unsigned j = 0;
		while (cis[j].cmd_id != CMD_LAST) {
			if (T[i] == cis[j].command)
				break;
			++j;
		}
		// continue if no command found
		if (cis[j].cmd_id == CMD_LAST) {
			std::cerr << error_start(T[i].begin) << "command " << to_string(T[i]).c_str() << " not known" << std::endl;
			++i;
			continue;
		}
		const command_info& ci = cis[j];

		// parse arguments
		if (++i >= T.size()) {
			std::cerr << error_start(T[i-1].begin) << "command " << ci.command << " incomplete" << std::endl;
			break;
		}
		if (T[i] != "(") {
			std::cerr << error_start(T[i].begin) << "expected ( after command " << ci.command << " but found " << to_string(T[i]).c_str() << " instead" << std::endl;
			continue;
		}
		bool terminate = false;
		std::vector<std::string> args;
		bool last_was_argument = false;
		while (!terminate) {
			if (++i >= T.size())
				break;
			std::string arg_str;
			if (T[i] == ",") {
				if (!last_was_argument && args.size() < ci.nr_arguments)
					args.push_back("");
				last_was_argument = false;
			}
			else if (T[i] == ")") {
				if (!last_was_argument && args.size() < ci.nr_arguments)
					args.push_back("");
				last_was_argument = false;
				terminate = true;
			}
			else {
				if (last_was_argument) {
					args.back() += " ";
					args.back() += to_string(T[i]);
				}
				else if (args.size() < ci.nr_arguments) {
					if (to_lower(ci.arguments[args.size()]) == 's') {
						if (*T[i].begin == '"' && T[i].size() > 1 && T[i].end[-1] == '"') {
							++T[i].begin;
							--T[i].end;
						}
						if (*T[i].begin == '\'' && T[i].size() > 1 && T[i].end[-1] == '\'') {
							++T[i].begin;
							--T[i].end;
						}
					}
					args.push_back(to_string(T[i]));
				}
				last_was_argument = true;
			}
		}
		if (!terminate) {
			std::cerr << error_start(T[i-1].begin) << "did not find enclosing )" << std::endl;
			break;
		}

		// check if necessary arguments are provided
		if (args.size() < ci.nr_arguments) {
			char c = ci.arguments[args.size()];
			if (to_lower(c) == c) {
				std::cerr << error_start(T[i-1].begin) << "not all necessary arguments provided for command " << ci.command << std::endl;
				++i;
				continue;
			}
		}
		expect_block_begin = get_command_infos()[ci.cmd_id].block_follows;
		if (expect_block_begin)
			block_cmd.push(ci.cmd_id);
		//
		switch (pt) {
		case PT_NR_TOGGLES: 
			if (ci.cmd_id == CMD_TREE_NODE)
				++nr_toggles;
			break;
		case PT_INIT_TOGGLES:
			if (ci.cmd_id == CMD_TREE_NODE) {
				if (args[1] == "true")
					toggles[nr_toggles] = true;
				else if (args[1] == "false")
					toggles[nr_toggles] = false;
				else {
					toggles[nr_toggles] = true;
					std::cerr << error_start(T[i-1].begin) << "error parsing initial toggle value which must be true or false" << std::endl;
				}
				++nr_toggles;
			}
			break;
		case PT_CREATE_GUI :
			if (!visible.top()) {
				if (ci.cmd_id == CMD_TREE_NODE)
					++nr_toggles;
			}
			else {
				void* member_ptr;
				std::string member_type;
				gui_group_ptr ggp = parent_group;
				if (!gui_group_stack.empty())
					ggp = gui_group_stack.top();
/*
				std::cout << ggp->get_name() << ":" << get_command_infos()[ci.cmd_id].command << "(";
				for (unsigned ai=0; ai<args.size(); ++ai) {
					std::cout << args[ai];
					if (ai < args.size()-1)
						std::cout << ", ";
				}
				std::cout << ")" << std::endl;
*/
				switch (ci.cmd_id) {
				case CMD_WINDOW :
					{
						window_ptr wp = application::create_window(atoi(args[0].c_str()), 
								  atoi(args[1].c_str()), args[2], 
								  args.size()>3 ? args[3] : std::string("viewer"));
						if (args.size()>4)
							wp->multi_set(args[4], true);
						gui_group_stack.push(wp);
						break;
					}
					break;
				case CMD_GROUP :
					gui_group_stack.push(ggp->add_group(args[0], args[1],
								  args.size()>2 ? args[2] : std::string(""),
								  args.size()>3 ? args[3] : std::string("\n")));
					break;
				case CMD_ALIGN : 
					ggp->align(args[0]);
					break;
				case CMD_DECORATOR :
					ggp->add_decorator(args[0], args[1],
								  args.size()>2 ? args[2] : std::string(""),
								  args.size()>3 ? args[3] : std::string("\n"));
					break;
				case CMD_BUTTON :
					ggp->add_button(args[0], 
							   args.size()>1 ? args[1] : std::string(""),
							   args.size()>2 ? args[2] : std::string("\n"));
					break;
				case CMD_VIEW :
					if (find_member(args[1],member_ptr, member_type))
						ggp->add_view_void(args[0],member_ptr,member_type,
								args.size()>2 ? args[2] : std::string(""),
								args.size()>3 ? args[3] : std::string(""),
								args.size()>4 ? args[4] : std::string("\n"));
					break;
				case CMD_CONTROL :
					if (find_member(args[1],member_ptr, member_type)) {

						object_functor<
							1,
							rebind1<
								mtd_functor<
									void, 
									1, 
									base, 
									void*
								>, 
								void, 
								const_expression<
									void*
								>
							>, 
							control_ptr
						> of(
							 rebind1<
								mtd_functor<
									void, 
									1, 
									base, 
									void*
								>, 
								void, 
								const_expression<
									void*
								>
							>(mtd_functor<
									void, 
									1, 
									base, 
									void*
							  >(instance.operator->(),
								&base::on_set),
							  member_ptr)
						);

						ggp->add_control_void(args[0],member_ptr,0,member_type,
							args.size()>2 ? args[2] : std::string(""),
							args.size()>3 ? args[3] : std::string(""),
							args.size()>4 ? args[4] : std::string("\n"), 0)
							->attach_to_value_change(of.clone());
					}
					break;
				case CMD_TREE_NODE :
					add_tree_node(args[0], toggles[nr_toggles], atoi(args[2].c_str()),
						args.size() > 3 ? args[3] : std::string("\n"), ggp);
					visible.push(toggles[nr_toggles]);
					++nr_toggles;
					break;
				}
/*
				std::cout << "[\n";
				for (unsigned ci=0; ci<ggp->get_nr_children(); ++ci)
					std::cout << "  " << ggp->get_child(ci)->get_named()->get_name() << ":" << ggp->get_child(ci)->get_type_name() << "\n";
				std::cout << "]" << std::endl;
*/
			}
		}
		++i;
	} while (i < T.size());
}

///
void base_provider::create_gui()
{
	if (!parent_options.empty())
		parent_group->multi_set(parent_options, true);
	if (nr_toggles == -1) {
		nr_toggles = 0;
		parse_definition(PT_NR_TOGGLES);
		if (nr_toggles > 0) {
			if (toggles)
				delete[] toggles;
			toggles = new bool[nr_toggles];
			nr_toggles = 0;
			parse_definition(PT_INIT_TOGGLES);
		}
	}
	nr_toggles = 0;
	parse_definition(PT_CREATE_GUI);
}

	}
}
