#include "find_reflection_handler.h"

#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>

namespace cgv {
	namespace reflect {

bool find_reflection_handler::at_end() const
{
	return token_idx == tokens.size();
}

bool find_reflection_handler::step_if_matches(const std::string& name)
{
	if (name == "")
		return true;
	if (!at_end()) {
		if (tokens[token_idx] == name) {
			++token_idx;
			return true;
		}
	}
	return false;
}

find_reflection_handler::find_reflection_handler(const std::string& _target) : target(_target), found(false), valid(true)
{
	cgv::utils::tokenizer(target).set_sep(".[]").set_skip("'\"", "'\"", "\\\\").set_ws("").bite_all(tokens);
	token_idx = 0;
	rt = 0;
}

///
find_reflection_handler::~find_reflection_handler()
{
	if (rt)
		delete rt;
}

///
bool find_reflection_handler::found_target() const
{
	return found;
}

///
bool find_reflection_handler::found_valid_target() const
{
	return found && valid;
}

///
void* find_reflection_handler::get_member_ptr() const
{
	return member_ptr;
}


///
const std::string& find_reflection_handler::get_member_name() const
{
	return member_name;
}

///
abst_reflection_traits* find_reflection_handler::get_reflection_traits() const
{
	return rt;
}

///
void find_reflection_handler::process_member_void(const std::string& member_name, void* member_ptr, 
				         abst_reflection_traits* rt, GroupKind group_kind, unsigned grp_size)
{
	this->member_name = member_name;
	this->rt = rt->clone();
	this->member_ptr  = member_ptr;
	this->group_kind  = group_kind;
	this->grp_size    = grp_size;
	found = true;
}

///
int find_reflection_handler::reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, 
				    abst_reflection_traits* rt, unsigned grp_size)
{
	switch (group_kind) {
	case GK_BASE_CLASS :
		if (at_end()) {
			std::cerr << "did not expect base " << rt->get_type_name() << std::endl;
			return GT_TERMINATE;
		}
		else if (tokens[token_idx] == rt->get_type_name()) {
			if (token_idx == tokens.size()-1) {
				process_member_void("base_class", group_ptr, rt, group_kind, grp_size);
				return GT_TERMINATE;
			}
			else {
				++token_idx;
				if (tokens[token_idx] == ".") {
					++token_idx;
					return GT_COMPLETE;
				}
				else {
					std::cerr << "only .-operator allowed on base class " << group_name << "!" << std::endl;
					return GT_TERMINATE;
				}
			}
		}
		else
			return GT_COMPLETE;

	case GK_STRUCTURE:
		if (step_if_matches(group_name)) {
			if (at_end()) {
				process_member_void(group_name, group_ptr, rt, group_kind);
				return GT_TERMINATE;
			}
			else {
				if (group_name == "" && tokens[token_idx] != ".")
					return GT_COMPLETE;
				if (tokens[token_idx] == ".") {
					++token_idx;
					if (token_idx == tokens.size()) {
						std::cerr << ".-operator applied on " << group_name << ":" << rt->get_type_name() << " must be followed by element name" << std::endl;
						return GT_TERMINATE;
					}
					return GT_COMPLETE;
				}
				else {
					std::cerr << "only .-operator allowed on structure " << group_name << ":" << rt->get_type_name() << "!" << std::endl;
					return GT_TERMINATE;
				}
			}
		}
		else
			return GT_SKIP;

	case GK_ARRAY : 
		if (step_if_matches(group_name)) {
			if (at_end()) {
				process_member_void(group_name, group_ptr, rt, group_kind);
				return GT_TERMINATE;
			}
			else {
				if (tokens[token_idx] == "[") {
					++token_idx;
					if (token_idx == tokens.size()) {
						std::cerr << "[-operator applied on array " << group_name << " must be followed by index" << std::endl;
						return GT_TERMINATE;
					}
					int idx;
					if (!cgv::utils::is_integer(tokens[token_idx].begin,tokens[token_idx].end,idx)) {
						std::cerr << "index to access array " << group_name << " can only be of integer type, but found token " << tokens[token_idx] << std::endl;
						return GT_TERMINATE;
					}
					if (idx < 0 || idx >= (int)grp_size) {
						std::cerr << "index " << idx << " to access array " << group_name << " is out of range [0," << grp_size << "[! " << std::endl;
						return GT_TERMINATE;
					}
					++token_idx;
					if (token_idx == tokens.size()) {
						std::cerr << "[" << idx << "-operator applied to array " << group_name << " must be followed by ]" << std::endl;
						return GT_TERMINATE;
					}
					++token_idx;
					return idx;
				}
				else {
					std::cerr << "only []-operator allowed on array " << group_name << "!" << std::endl;
					return GT_TERMINATE;
				}
			}
		}
		else
			return GT_SKIP;
	
	case GK_VECTOR:
		if (step_if_matches(group_name)) {
			if (at_end()) {
				process_member_void(group_name, group_ptr, rt, group_kind);
				return GT_TERMINATE;
			}
			else {
				if (tokens[token_idx] == "[") {
					++token_idx;
					if (token_idx == tokens.size()) {
						std::cerr << "[-operator applied on vector " << group_name << " must be followed by index" << std::endl;
						return GT_TERMINATE;
					}
					int idx;
					if (!cgv::utils::is_integer(tokens[token_idx].begin,tokens[token_idx].end,idx)) {
						std::cerr << "index to access vector " << group_name << " can only be of integer type, but found token " << tokens[token_idx] << std::endl;
						return GT_TERMINATE;
					}
					if (idx < 0 || idx >= (int)grp_size) {
						std::cerr << "index " << idx << " to access vector " << group_name << " is out of range [0," << grp_size << "[! " << std::endl;
						return GT_TERMINATE;
					}
					++token_idx;
					if (token_idx == tokens.size()) {
						std::cerr << "[" << idx << "-operator applied to vector " << group_name << " must be followed by ]" << std::endl;
						return GT_TERMINATE;
					}
					++token_idx;
					return idx;
				}
				else if (tokens[token_idx] == ".") {
					++token_idx;
					if (token_idx == tokens.size()) {
						std::cerr << ".-operator applied on vector " << group_name << ":" << rt->get_type_name() << " must be followed by element name" << std::endl;
						return GT_TERMINATE;
					}
					return GT_COMPLETE;
				}
				else {
					std::cerr << "only . or [-operator allowed on vector " << group_name << ":" << rt->get_type_name() << "!" << std::endl;
					return GT_TERMINATE;
				}
			}
		}
		else
			return GT_SKIP;
	default:
		return GT_SKIP;
	}
}

bool find_reflection_handler::reflect_member_void(const std::string& member_name, void* member_ptr, abst_reflection_traits* rt)
{
	if (step_if_matches(member_name)) {
		if (at_end()) {
			process_member_void(member_name, member_ptr, rt);
			return false;
		}
		else {
			std::cerr << "non group member " << member_name << ":" << rt->get_type_name() << " may not follow operators!" << std::endl;
			return false;
		}
	}
	return true;
}

bool find_reflection_handler::reflect_method_void(const std::string& method_name, method_interface* mi_ptr,
						 abst_reflection_traits* return_traits, const std::vector<abst_reflection_traits*>& param_value_traits)
{
	return true; 
}

	}
}
