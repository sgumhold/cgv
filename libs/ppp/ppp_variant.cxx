#include "variant.h"
#include "variables.h"
#include <cgv/utils/convert.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv/utils/scan.h>
#include <stdlib.h>
#include <iostream>

using namespace cgv::utils;

namespace cgv {
	namespace media {
		namespace text {
			namespace ppp {


func_type::func_type(unsigned _i, unsigned _j, ph_processor* _ph_proc, namespace_info* _ns)
	: block_begin(_i), block_end(_j), ph_proc(_ph_proc), ns(_ns)
{
}


variant::variant() : vt(UNDEF_VALUE), reference_value(0) {}

/// use assign operator for copy constructor
variant::variant(const variant& v)
{
	vt = v.vt;
	switch (vt) {
	case BOOL_VALUE : bool_value = v.bool_value; break;
	case INT_VALUE : int_value = v.int_value; break;
	case DOUBLE_VALUE : dbl_value = v.dbl_value; break;
	case STRING_VALUE : string_value = new std::string(*v.string_value); break;
	case REFERENCE_VALUE : reference_value = v.reference_value; break;
	case NAME_VALUE : name_value = new std::string(*v.name_value); break;
	case LIST_VALUE : list_value = new list_type(*v.list_value); break;
	case MAP_VALUE : map_value = new map_type(*v.map_value); break;
	case FUNC_VALUE : func_value = new func_type(*v.func_value); break;
	default: break;
	}
}

variant::variant(bool v) : vt(BOOL_VALUE), bool_value(v) {}
variant::variant(int v) : vt(INT_VALUE), int_value(v) {}
variant::variant(double v) : vt(DOUBLE_VALUE), dbl_value(v) {}
variant::variant(const std::string& v) : vt(STRING_VALUE), string_value(new std::string(v)) {}
variant::variant(variant* v) : vt(REFERENCE_VALUE), reference_value(v) {}
/// construct name value
variant::variant(ValueType, const std::string& v)
{
	vt = NAME_VALUE;
	name_value = new std::string(v);
}
/// construct list value
variant::variant(const list_type& v) : vt(LIST_VALUE), list_value(new list_type(v)) {}
/// construct map value
variant::variant(const map_type& v) : vt(MAP_VALUE), map_value(new map_type(v)) {}
/// construct func value
variant::variant(const func_type& v) : vt(FUNC_VALUE), func_value(new func_type(v)) {}

variant::~variant()
{
	clear();
}

void variant::clear()
{
	switch (vt) {
	case STRING_VALUE : delete string_value; string_value = 0; break;
	case REFERENCE_VALUE : reference_value = 0; break;
	case NAME_VALUE : delete name_value; name_value = 0; break;
	case LIST_VALUE : delete list_value; list_value = 0; break;
	case MAP_VALUE : delete map_value; map_value = 0; break;
	case FUNC_VALUE : delete func_value; func_value = 0; break;
	default:break;
	}
	vt = UNDEF_VALUE;
}


variant& variant::operator = (const variant& v)
{
	if (&v == this)
		return *this;
	clear();
	vt = v.vt;
	switch (vt) {
	case BOOL_VALUE : bool_value = v.bool_value; break;
	case INT_VALUE : int_value = v.int_value; break;
	case DOUBLE_VALUE : dbl_value = v.dbl_value; break;
	case STRING_VALUE : string_value = new std::string(*v.string_value); break;
	case REFERENCE_VALUE : reference_value = v.reference_value; break;
	case NAME_VALUE : name_value = new std::string(*v.name_value); break;
	case LIST_VALUE : list_value = new list_type(*v.list_value); break;
	case MAP_VALUE : map_value = new map_type(*v.map_value); break;
	case FUNC_VALUE : func_value = new func_type(*v.func_value); break;
	default : break;
	}
	return *this;
}

bool variant::is_unary_applicable(OperatorType ot)
{
	switch (ot) {
	case OT_INC: 
	case OT_DEC: 
		return is_reference() || is_str();
	case OT_NEGATE:
	case OT_COMPL:
		return is_int() || is_double() || is_bool();
	case OT_MAP_UP :
	case OT_UNARY_MAP :
		return is_str() || is_name();
	case OT_MAP_DOWN :
		return has_child_namespace() && ( is_str() || is_name() );
	case OT_EXISTS:
		return !is_list() && !is_map();
	default:break;
	}
	return true;
}

bool variant::unary_check_defined(OperatorType ot)
{
	switch (ot) {
	case OT_UNARY_MAP :
	case OT_MAP_UP: 
	case OT_MAP_DOWN: 
	case OT_INC: 
	case OT_DEC: 
		return true;
	default:
		return !is_undefined();
	}
}

void variant::apply_unary(OperatorType ot)
{
	switch (ot) {
	case OT_NOT :
		if (is_list() || is_map())
			set_int(get_size());
		else if (is_str())
			set_int((int)get_str().size());
		else
			set_bool(!get_bool());
		break;
	case OT_COMPL :
		set_int(~get_int());
		break;
	case OT_EXISTS:
		set_bool(cgv::utils::file::exists(get_str())||cgv::utils::dir::exists(get_str()));
		break;
	case OT_INC: 
		if (is_int())
			++ref_value().int_value;
		else if (is_double())
			ref_value().dbl_value += 1;
		else if (is_str())
			set_str(to_upper(*ref_value().string_value));
		break;
	case OT_DEC: 
		if (is_int())
			--ref_value().int_value;
		else if (is_double())
			ref_value().dbl_value -= 1;
		else if (is_str())
			set_str(to_lower(*ref_value().string_value));
		break;
	case OT_NEGATE :
		if (is_double())
			set_double(-get_double());
		else
			set_int(-get_int());
		break;
	case OT_UNARY_MAP :
		{
			namespace_info* cns = get_current_namespace();
			namespace_info* ns = cns;
			while (ns->environment_ns)
				ns = ns->environment_ns;
			set_current_namespace(ns);
			if (is_name())
				*this = variant(&ref_variable(get_name()));
			else
				*this = variant(&ref_variable(get_str()));
			set_current_namespace(cns);
		}
		break;
	case OT_MAP_UP :
		if (is_name())
			*this = variant(&ref_variable(get_name(),true));
		else
			*this = variant(&ref_variable(get_str(),true));
		break;
	case OT_MAP_DOWN :
		if (has_child_namespace()) {
			goto_child_namespace();
			if (is_name())
				*this = variant(&ref_variable(get_name()));
			else
				*this = variant(&ref_variable(get_str()));
			goto_parent_namespace();
		}
		break;
	default : break;
	}
}

bool variant::is_binary_applicable(OperatorType ot, const variant& v2)
{
	switch (ot) {
	case OT_ASSIGN: 
	case OT_ASSIGN_REF: 
	case OT_ASSIGN_ADD: 
	case OT_ASSIGN_SUB:
	case OT_ASSIGN_MUL:
	case OT_ASSIGN_DIV:
	case OT_ASSIGN_AND:
	case OT_ASSIGN_OR:
	case OT_ASSIGN_XOR:
	case OT_ASSIGN_LSH:
	case OT_ASSIGN_RSH:
		return is_reference();
	case OT_LOG_OR:
	case OT_LOG_AND:
	case OT_AND:
	case OT_OR:
	case OT_XOR:
	case OT_DIV:
	case OT_MOD:
	case OT_LSH:
	case OT_RSH:
		return !is_str() && !is_list() && !is_map() && !v2.is_str() && !v2.is_list() && !v2.is_map();
	case OT_MUL :
		return !is_list() && !is_map() && !v2.is_list() && !v2.is_map();
	case OT_LESS :
		return !is_list() && !is_map() && !v2.is_map();
	case OT_EQUAL_TYPE :
	case OT_UNEQUAL_TYPE :
		return true;
	case OT_GREATER :
	case OT_LESS_OR_EQUAL :
	case OT_GREATER_OR_EQUAL :
	case OT_EQUAL :
	case OT_UNEQUAL :
		return !is_list() && !is_map() && !v2.is_list() && !v2.is_map();
	case OT_ADD :
	case OT_SUB :
		return !is_map() && !v2.is_list() && !v2.is_map() && (!v2.is_str() || is_str());
	case OT_DOT :
		return 
			(is_str() && (v2.is_bool() || v2.is_int() || v2.is_double() || v2.is_str())) ||
			((is_bool() || is_int() || is_double()) && v2.is_str()) ||
			is_list() || 
			v2.is_list() ||
			(is_map() && v2.is_list());
	case OT_BINARY_MAP :
		return is_map() && (v2.is_name() || v2.is_str());
	default : break;
	}
	return true;
}

bool variant::binary_check_defined(OperatorType ot, const variant& v2)
{
/*
	if (ot == OT_BINARY_MAP) {
		static int x = ot;
	}
	*/
	if (ot == OT_EQUAL_TYPE || ot == OT_UNEQUAL_TYPE)
		return true;
	if (ot == OT_BINARY_MAP)
		return !is_undefined();
	if (v2.is_undefined())
		return false;
	if (ot >= OT_ASSIGN && ot <= OT_ASSIGN_RSH)
		return true;
	return !is_undefined();
}

void variant::apply_binary(OperatorType ot, const variant& v2)
{
	switch (ot) {
	case OT_ASSIGN :
		ref_value() = v2.get_value();
		break;
	case OT_ASSIGN_REF :
		if (v2.is_name())
			ref_value() = variant(&ref_variable(v2.get_name()));
		else
			ref_value() = v2;
		break;
	case OT_ASSIGN_ADD :
		if (match_number_type(v2))
			ref_value().ref_int() += v2.get_int();
		else
			ref_value().ref_double() += v2.get_double();
		break;
	case OT_ASSIGN_SUB :
		if (match_number_type(v2))
			ref_value().ref_int() -= v2.get_int();
		else
			ref_value().ref_double() -= v2.get_double();
		break;
	case OT_ASSIGN_MUL :
		if (match_number_type(v2))
			ref_value().ref_int() *= v2.get_int();
		else
			ref_value().ref_double() *= v2.get_double();
		break;
	case OT_ASSIGN_DIV :
		if (match_number_type(v2))
			ref_value().ref_int() /= v2.get_int();
		else
			ref_value().ref_double() /= v2.get_double();
		break;
	case OT_ASSIGN_AND :
		ensure_int_type();
		ref_value().ref_int() &= v2.get_int();
		break;
	case OT_ASSIGN_OR :
		ensure_int_type();
		ref_value().ref_int() |= v2.get_int();
		break;
	case OT_ASSIGN_XOR :
		ensure_int_type();
		ref_value().ref_int() ^= v2.get_int();
		break;
	case OT_ASSIGN_LSH :
		ensure_int_type();
		ref_value().ref_int() <<= v2.get_int();
		break;
	case OT_ASSIGN_RSH :
		ensure_int_type();
		ref_value().ref_int() >>= v2.get_int();
		break;
	case OT_LOG_OR :
		set_bool(get_bool() || v2.get_bool());
		break;
	case OT_LOG_AND :
		set_bool(get_bool() && v2.get_bool());
		break;
	case OT_OR :
		if (is_int() && v2.is_int())
			set_int(get_int() | v2.get_int());
		else
			set_bool(get_bool() || v2.get_bool());
		break;
	case OT_AND :
		if (is_int() && v2.is_int())
			set_int(get_int() & v2.get_int());
		else
			set_bool(get_bool() && v2.get_bool());
		break;
	case OT_XOR:
		if (is_int() && v2.is_int())
			set_int(get_int() ^ v2.get_int());
		else
			set_bool(get_bool() ^ v2.get_bool());
		break;
	case OT_LSH :
		set_int(get_int() << v2.get_int());
		break;
	case OT_RSH :
		set_int(get_int() >> v2.get_int());
		break;
	case OT_LESS :
		if (v2.is_list()) {
			bool found = false;
			for (unsigned int i=0; i<v2.get_size(); ++i) {
				variant tmp(*this);
				if (tmp.is_binary_applicable(OT_EQUAL, v2.get_element(i))) {
					tmp.apply_binary(OT_EQUAL,v2.get_element(i));
					if (tmp.get_bool()) {
						found = true;
						break;
					}
				}
			}
			set_bool(found);
		}
		else {
			switch (get_value_type()) {
			case BOOL_VALUE :
			case INT_VALUE : 
			case DOUBLE_VALUE : set_bool(get_double() < v2.get_double()); break;
			case STRING_VALUE : set_bool(get_str() < v2.get_str()); break;
			default: break;
			}
		}
		break;
	case OT_GREATER :
		switch (get_value_type()) {
		case BOOL_VALUE :
		case INT_VALUE :
		case DOUBLE_VALUE: set_bool(get_double() > v2.get_double()); break;
		case STRING_VALUE : set_bool(get_str() > v2.get_str()); break;
		default: break;
		}
		break;
	case OT_LESS_OR_EQUAL :
		switch (get_value_type()) {
		case BOOL_VALUE :
		case INT_VALUE :
		case DOUBLE_VALUE: set_bool(get_double() <= v2.get_double()); break;
		case STRING_VALUE : set_bool(get_str() <= v2.get_str()); break;
		default: break;
		}
		break;
	case OT_GREATER_OR_EQUAL :
		switch (get_value_type()) {
		case BOOL_VALUE :
		case INT_VALUE :
		case DOUBLE_VALUE: set_bool(get_double() >= v2.get_double()); break;
		case STRING_VALUE : set_bool(get_str() >= v2.get_str()); break;
		default: break;
		}
		break;
	case OT_EQUAL :
		switch (get_value_type()) {
		case BOOL_VALUE :
		case INT_VALUE :
		case DOUBLE_VALUE: set_bool(get_double() == v2.get_double()); break;
		case STRING_VALUE : set_bool(get_str() == v2.get_str()); break;
		default: break;
		}
		break;
	case OT_UNEQUAL :
		switch (get_value_type()) {
		case BOOL_VALUE :
		case INT_VALUE :
		case DOUBLE_VALUE: set_bool(get_double() != v2.get_double()); break;
		case STRING_VALUE : set_bool(get_str() != v2.get_str()); break;
		default: break;
		}
		break;
	case OT_EQUAL_TYPE :
		set_bool(get_value_type() == v2.get_value_type());
		break;
	case OT_UNEQUAL_TYPE :
		set_bool(get_value_type() != v2.get_value_type());
		break;
	case OT_ADD :
		if (is_str()) {
			if (v2.is_str()) {
				std::size_t p = get_str().find_first_of(v2.get_str());
				if (p != std::string::npos)
					set_str(get_str().substr(p));
			}
			else {
				int idx = v2.get_int();
				if (idx >= (int)get_str().size())
					set_str("");
				else
					set_str(get_str().substr(v2.get_int()));
			}
		}
		else if (is_list()) {
			int idx = v2.get_int();
			if (idx >= (int)get_size())
				set_list();
			else {
				std::vector<variant> l = get_list();
				l.erase(l.begin(), l.begin()+idx);
				set_list(l);
			}
		}
		else {
			if (is_double() || v2.is_double())
				set_double(get_double() + v2.get_double());
			else
				set_int(get_int() + v2.get_int());
		}
		break;
	case OT_SUB :
		if (is_str()) {
			if (v2.is_str()) {
				std::size_t p = get_str().find_last_of(v2.get_str());
				if (p != std::string::npos)
					set_str(get_str().substr(0,p));
			}
			else {
				int idx = v2.get_int();
				if (idx >= (int)get_str().size())
					set_str("");
				else
					set_str(get_str().substr(0,get_str().size()-idx));
			}
		}
		else if (is_list()) {
			int idx = v2.get_int();
			if (idx >= (int)get_size())
				set_list();
			else {
				std::vector<variant> l = get_list();
				for (int i = 0; i < idx; ++i)
					l.pop_back();
				set_list(l);
			}
		}
		else {
			if (is_double() || v2.is_double())
				set_double(get_double() - v2.get_double());
			else
				set_int(get_int() - v2.get_int());
		}
		break;
	case OT_MUL :
		if (is_str()) {
			std::string arg = v2.get_str();
			if (arg.size() > 3) {
				std::vector<token> toks;
				tokenizer(arg).set_ws(arg.substr(0,1)).bite_all(toks);
				if (toks.size() > 0) {
					std::string s1 = to_string(toks[0]);
					std::string s2;
					if (toks.size() > 1)
						s2 = to_string(toks[1]);
					std::string s = get_str();
					replace(s,s1,s2);
					/*
					std::cout << "replace('" << get_str().c_str() 
								 << "', '" << s1.c_str() << "', " 
								 << s2.c_str() << "') ==> " << s.c_str() << std::endl;*/
					set_str(s);
				}
			}
		}
		else {
			if (is_double() || v2.is_double())
				set_double(get_double() * v2.get_double());
			else
				set_int(get_int() * v2.get_int());
		}
		break;
	case OT_DIV :
		if (is_double() || v2.is_double())
			set_double(get_double() / v2.get_double());
		else
			set_int(get_int() / v2.get_int());
		break;
	case OT_MOD :
		set_int(get_int() % v2.get_int());
		break;
	case OT_DOT :
		if (is_map()) {
			if (v2.is_list()) {
				map_type m = get_map();
				list_type l = v2.get_list();
				if (l.size() > 0) {
					if (!l[0].is_list()) {
						if (l.size() == 2) {
							m[l[0].get_str()] = l[1].get_value();
						}
					}
					else {
						for (unsigned int i=0; i<l.size(); ++i) {
							if (l[i].is_list()) {
								if (l[i].get_size() == 2) {
									m[l[i].ref_element((unsigned int)0).get_str()] = l[i].get_element((unsigned int)1).get_value();
								}
							}
						}
					}
				}
				set_map(m);
			}
		}
		else if (!is_list())
			if (!v2.is_list())
				set_str(get_str() + v2.get_str());
			else {
				// prepend
				list_type l = v2.get_list();
				l.insert(l.begin(), get_value());
				set_list(l);
			}
		else
			if (v2.is_list()) {
				// concat lists
				std::vector<variant> l = get_list();
				unsigned int n = v2.get_size();
				for (unsigned int i = 0; i < n; ++i)
					l.push_back(v2.get_element(i).get_value());
				set_list(l);
			}
			else {
				// append to list
				std::vector<variant> l = get_list();
				l.push_back(v2.get_value());
				set_list(l);
			}
		break;
	case OT_BINARY_MAP:
		if (is_reference()) {
			if (v2.is_name())
				*this = variant(&ref_element(v2.get_name()));
			else
				*this = variant(&ref_element(v2.get_str()));
		}
		else {
			variant tmp;
			if (v2.is_name())
				tmp = get_element(v2.get_name());
			else
				tmp = get_element(v2.get_str());
			*this = tmp;
		}
		break;
	default : break;
	}
}


ValueType variant::get_type() const
{
	return vt;
}

ValueType variant::get_value_type() const
{
	if (get_type() == REFERENCE_VALUE)
		return reference_value->get_value_type();
	if (get_type() == NAME_VALUE) {
		variant* v = find_variable(get_name());
		if (v)
			return v->get_value_type();
		else
			return UNDEF_VALUE;
	}
	return get_type();
}

bool variant::is_undefined() const
{
	return get_value_type() == UNDEF_VALUE;
}

bool variant::is_bool() const
{
	return get_value_type() == BOOL_VALUE;
}

bool variant::is_int() const
{
	return get_value_type() == INT_VALUE;
}

bool variant::is_double() const
{
	return get_value_type() == DOUBLE_VALUE;
}

bool variant::is_str() const
{
	return get_value_type() == STRING_VALUE;
}

bool variant::is_list() const
{
	return get_value_type() == LIST_VALUE;
}

bool variant::is_map() const
{
	return get_value_type() == MAP_VALUE;
}

/// lookup names and follow references and return whether variant is func
bool variant::is_func() const
{
	return get_value_type() == FUNC_VALUE;
}


bool variant::is_reference() const
{
	return get_type() == REFERENCE_VALUE || get_type() == NAME_VALUE;
}

bool variant::is_name() const
{
	return get_type() == NAME_VALUE;
}


bool variant::get_bool() const 
{
	switch (get_value_type()) {
	case UNDEF_VALUE :
		return false;
	case BOOL_VALUE :
		return get_value().bool_value;
	case INT_VALUE :
		return get_value().int_value != 0;
	case DOUBLE_VALUE :
		return get_value().dbl_value != 0;
	case STRING_VALUE :
		return get_str().size() != 0;
	case LIST_VALUE :
	case MAP_VALUE :
		return get_value().get_size() > 0;
	default : break;
	}
	return false;
}

int variant::get_int() const 
{
	switch (get_value_type()) {
	case UNDEF_VALUE :
		return -1;
	case BOOL_VALUE :
		return get_value().bool_value?1:0;
	case INT_VALUE :
		return get_value().int_value;
	case DOUBLE_VALUE :
		return (int)get_value().dbl_value;
	case STRING_VALUE :
		return atoi(get_str().c_str());
	case LIST_VALUE :
	case MAP_VALUE :
		return get_value().get_size();
	default : break;
	}
	return 0;
}

double variant::get_double() const 
{
	switch (get_value_type()) {
	case UNDEF_VALUE :
		return -1;
	case BOOL_VALUE :
		return get_value().bool_value?1.0:0.0;
	case INT_VALUE :
		return get_value().int_value;
	case DOUBLE_VALUE :
		return get_value().dbl_value;
	case STRING_VALUE :
		return atof(get_str().c_str());
	case LIST_VALUE :
	case MAP_VALUE :
		return get_value().get_size();
	default : break;
	}
	return 0;
}

std::string variant::get_str() const 
{
	switch (get_value_type()) {
	case UNDEF_VALUE :
		return "UNDEFINED";
	case BOOL_VALUE :
		return to_string(get_value().bool_value);
	case INT_VALUE :
		return to_string(get_value().int_value);
	case DOUBLE_VALUE :
		return to_string(get_value().dbl_value);
	case STRING_VALUE:
		return *get_value().string_value;
	case LIST_VALUE :
	case MAP_VALUE :
		{
			const variant& v = get_value();
			std::string s("[");
			for (unsigned int i=0; i<v.get_size(); ++i) {
				if (i>0)
					s = s+",";
				if (v.get_type() == MAP_VALUE)
					s=s+v.get_element_name(i)+"::";
				s = s+v.get_element(i).get_str();
			}
			s = s + "]";
			return s;
		}
	}
	return "";
}

const std::string& variant::get_name() const 
{
	if (get_type() == NAME_VALUE)
		return *name_value;
	std::cerr << "attemt to access name of non name variant value" << std::endl;
	static std::string dummy;
	return dummy;
}

void variant::set_bool(bool v)
{
	*this = variant(v);
}

void variant::set_int(int v)
{
	*this = variant(v);
}

void variant::set_double(double v)
{
	*this = variant(v);
}

void variant::set_str(const std::string& v)
{
	*this = variant(v);
}

void variant::set_name(const std::string& v)
{
	*this = variant(NAME_VALUE, v);
}

void variant::set_list()
{
	*this = variant(list_type());
}

void variant::set_list(const list_type& l)
{
	*this = variant(l);
}

void variant::set_map()
{
	*this = variant(map_type());
}

void variant::set_map(const map_type& m)
{
	*this = variant(m);
}


variant* variant::get_reference() const
{
	return reference_value;
}


const variant::list_type& variant::get_list() const
{
	return *get_value().list_value;
}


bool&   variant::ref_bool()
{
	return ref_value().bool_value;
}

int&    variant::ref_int()
{
	return ref_value().int_value;
}

double& variant::ref_double()
{
	return ref_value().dbl_value;
}

std::string& variant::ref_str()
{
	return *ref_value().string_value;
}

variant::list_type& variant::ref_list() 
{
	return *ref_value().list_value;
}

const variant::map_type& variant::get_map() const
{
	return *get_value().map_value;
}

/// convert to int type
void variant::ensure_int_type()
{
	ref_value().set_int(get_int());
}

/// convert to int or double such that result of binary operators can be stored in this variant without loss of data, return whether conversion was to int
bool variant::match_number_type(const variant& v2)
{
	ValueType vt = get_type();
	if (vt == DOUBLE_VALUE || v2.get_type() == DOUBLE_VALUE) {
		if (vt != DOUBLE_VALUE)
			ref_value().set_double(get_double());
		return false;
	}
	if (vt != INT_VALUE)
		ensure_int_type();
	return true;
}

variant::map_type& variant::ref_map()
{
	return *ref_value().map_value;
}

/// access to func value
func_type& variant::ref_func()
{
	return *ref_value().func_value;
}


const variant& variant::get_value() const
{
	if (get_type() == REFERENCE_VALUE)
		return reference_value->get_value();
	if (get_type() == NAME_VALUE)
		return ref_variable(get_name()).get_value();
	return *this;
}

variant& variant::ref_value()
{
	if (get_type() == REFERENCE_VALUE)
		return reference_value->ref_value();
	if (get_type() == NAME_VALUE)
		return ref_variable(get_name()).ref_value();
	return *this;
}

unsigned int variant::get_size() const
{
	switch (get_value_type()) {
	case STRING_VALUE : return (unsigned int) get_value().string_value->size();
	case LIST_VALUE :  return (unsigned int)get_list().size();
	case MAP_VALUE: return (unsigned int)get_map().size();
	default : break;
	}
	return 1;
}

/// return total number of elements in a list or map summing over all elements recursively
unsigned variant::get_total_nr_elements() const
{
	switch (get_value_type()) {
	case LIST_VALUE :
		{
			const list_type& L = get_list();
			unsigned N=0,n = L.size();
			for (unsigned i=0; i<n; ++i)
				N += L[i].get_total_nr_elements();
			return N;
		}
	case MAP_VALUE: 
		{
			const map_type& M = get_map();
			unsigned N=0;
			for (map_type::const_iterator i=M.begin(); i != M.end(); ++i)
				N += i->second.get_total_nr_elements();
			return N;
		}
	default : break;
	}
	return 1;
}


variant& variant::ref_element(unsigned int i)
{
	if (get_value_type() == MAP_VALUE) {
		map_type::iterator mi = ref_map().begin();
		for (unsigned int j=0; j<i; ++j)
			++mi;
		return mi->second;
	}
	return ref_list()[i];
}

const variant& variant::get_element(unsigned int i) const
{
	if (get_value_type() == MAP_VALUE) {
		map_type::const_iterator mi = get_map().begin();
		for (unsigned int j=0; j<i; ++j)
			++mi;
		return mi->second;
	}
	return get_list()[i];
}

void variant::append_to_list(const variant& v)
{
	ref_list().push_back(v);
}

void variant::prepend_to_list(const variant& v)
{
	ref_list().insert(ref_list().begin(), v);
}

void variant::pop_back_from_list()
{
	ref_list().pop_back();
}

void variant::pop_front_from_list()
{
	ref_list().erase(ref_list().begin());
}

const variant& variant::get_element(const std::string& name)
{
	if (get_value_type() == MAP_VALUE)
		return ref_map()[name];
	std::cerr << "attemt to get_element of non map variant by name" << std::endl;
	static variant dummy;
	return dummy;
}

variant& variant::ref_element(const std::string& name)
{
	if (get_value_type() == MAP_VALUE) {
		map_type& M = ref_map();
		return M[name];
	}
	std::cerr << "attemt to ref_element of non map variant by name" << std::endl;
	static variant dummy;
	return dummy;
}


const std::string& variant::get_element_name(unsigned int i) const
{
	map_type::const_iterator mi = get_map().begin();
	for (unsigned int j=0; j<i; ++j)
		++mi;
	return mi->first;
}

void variant::insert(const std::string& name, const variant& v)
{
	ref_map()[name] = v;
}


std::ostream& operator << (std::ostream& os, const variant& v)
{
	static unsigned tab = 0;
	switch (v.get_type()) {
	case UNDEF_VALUE :
		return os << ">UNDEF<";
	case REFERENCE_VALUE :
		return os << "*" << *v.get_reference();
	case NAME_VALUE :
		return os << "<" << v.get_name() << ">";
	case LIST_VALUE :
	case MAP_VALUE :
		{
			unsigned int n = v.get_size();
			unsigned N = v.get_total_nr_elements();
			os << '[';
			if (N > 5)
				tab += 2;
			for (unsigned int i=0; i<n; ++i) {
				if (i>0)
					os << ", ";
				if (N > 5)
					os << "\n" << std::string(tab,' ');
				if (v.get_type() == MAP_VALUE)
					os << v.get_element_name(i) << "=";
				os << v.get_element(i);
			}
			if (N > 5) {
				tab -= 2;
				os << "\n" << std::string(tab,' ');
			}
			return os << ']';
		}
	default:
		return os << v.get_str().c_str();                
	}
}

			}
		}
	}
}
