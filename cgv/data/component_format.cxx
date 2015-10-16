#include "component_format.h"
#include <iostream>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>
#include <cgv/type/info/type_name.h>
#include <cgv/type/info/type_id.h>

using namespace cgv::utils;
using namespace cgv::type::info;

namespace cgv {
	namespace data {

static const char* component_formats[] = {
	"",
	"R",
	"G",
	"B",
	"A",
	"L",
	"I",
	"L,A",
	"I,A",
	"R,G,B",
	"R,G,B,A",
	"B,G,R",
	"B,G,R,A",
	"D",
	"S",
	"px,py",
	"px,py,pz",
	"px,py,pz,pw",
	"nx,ny",
	"nx,ny,nz",
	"ts,tt",
	"ts,tt,tr",
	"ts,tt,tr,tq",
};

/// extract component_positions from component string
void component_format::extract_components()
{
//	component_string += ',';
	component_positions.clear();
	if (component_string.empty())
		return;
	component_positions.push_back(0);
	for (unsigned int i=0; i<component_string.size(); ++i) {
		if (component_string[i] == ',' || component_string[i] == ';') {
//			component_string[i] = 0;
			if (i+1 < component_string.size())
				component_positions.push_back(i+1);
		}
	}
}

/// comma separated list of component descriptors, for example "R,G,B"
void component_format::set_components(const std::string& _components)
{
	component_string = _components;
	extract_components();
}

/// construct packed component format from component type, component sequence, and common component bit depth
component_format::component_format(TypeId _type_id, 
		const std::string& _components, unsigned int align, 
		unsigned int d0, unsigned int d1, unsigned int d2, unsigned int d3)
	: packing_info(align, d0,d1,d2,d3), component_type(_type_id), component_string(_components) 
{
	extract_components();
}

/// construct component format from component type, standard component format, component alignment and bit depths for packed formats
component_format::component_format(TypeId _type_id, 
		ComponentFormat cf, unsigned int align, 
		unsigned int d0, unsigned int d1, unsigned int d2, unsigned int d3)
	: packing_info(align, d0,d1,d2,d3), component_type(_type_id), 
	  component_string(component_formats[cf])
{
	extract_components();
}

/// construct from description string
component_format::component_format(const std::string& description)
{
	set_component_format(description);
}

/// set the component format from a description string
bool component_format::set_component_format(const std::string& description)
{
	if (description.empty()) {
	}
	last_error = "";
	set_packing(false);
	std::vector<token> toks;
	tokenizer(description).set_sep("[]:|,").bite_all(toks);
	if (toks.size() == 0) {
		last_error = "no tokens found that would define a component format";
		return false;
	}
	unsigned int i=1;
	if (to_string(toks[0]) == "[" || to_string(toks[0]) == ":" || 
		 to_string(toks[0]) == "|") { 
		component_type = TI_UNDEF;
		i = 0;
	}
	else if (to_string(toks[0]) == "undef")
		component_type = TI_UNDEF;
	else {
		component_type = get_type_id(to_string(toks[0]));
		if (component_type == TI_UNDEF) {
			last_error = to_string(toks[0])+" ... unknown type name";
			return false;
		}
	}
	if (toks.size() == i) {
		last_error = "description incomplete after <type_name>";
		return false;
	}
	if (toks[i] == ":") {
		if (toks.size() == ++i) {
			last_error = "description incomplete after <type_name:>";
			return false;
		}
		int bit_depth;
		if (is_integer(to_string(toks[i]), bit_depth)) {
			for (int ci=0; ci<4; ++ci)
				set_bit_depth(ci,bit_depth);
			set_packing(true);
			++i;
		}
		else {
			last_error = "expected integer token after <type_name:>";
			return false;
		}
	}
	if (toks[i] == "|") {
		if (toks.size() == ++i) {
			last_error = "description incomplete after <type_name|>";
			return false;
		}
		int align;
		if (is_integer(to_string(toks[i]), align)) {
			set_component_alignment(align);
			++i;
		}
		else {
			last_error = "expected integer token after <type_name|>";
			return false;
		}
	}
	if (toks.size() == i) {
		last_error = "description incomplete after <type_name>";
		return false;
	}
	int ci = 0;
	if (toks[i] != "[") {
		last_error = "expected <[> after type_name";
		return false;
	}
	if (toks.size() == ++i) {
		last_error = "description incomplete after <[>";
		return false;
	}
	component_string = "";
	while (toks[i] != "]") {
		if (component_string.size() > 0)
			component_string += ',';
		component_string += to_string(toks[i]);
		if (toks.size() == ++i) {
			last_error = "description incomplete after <[ or ,>";
			return false;
		}
		if (toks[i] == ":") {
			if (toks.size() == ++i) {
				last_error = "description incomplete after <component_name:>";
				return false;
			}
			int bit_depth;
			if (is_integer(to_string(toks[i]), bit_depth)) {
				if (ci < 4)
					set_bit_depth(ci,bit_depth);
				set_packing(true);
				if (toks.size() == ++i) {
					last_error = "description incomplete after <component_name:bit_depth>";
					return false;
				}
			}
			else {
				last_error = "expected integer token after <component_name:>";
				return false;
			}
		}
		if (toks[i] == ",") {
			if (toks.size() == ++i) {
				last_error = "description incomplete after <,>";
				return false;
			}
		}
		++ci;
	}
	extract_components();
	return true;
}

std::string component_format::last_error;

/// returns an error string after parsing of description string has failed
const std::string& component_format::get_last_error()
{
	return last_error;
}

/// clear the component format
void component_format::clear()
{
	component_type = TI_UNDEF;
	component_string.clear();
	component_positions.clear();
	packing_info::clear();
}

/// return whether the component format is defined
bool component_format::empty() const
{
	return component_type == TI_UNDEF;
}

/// return the number of component_positions
unsigned int component_format::get_nr_components() const
{
	return (unsigned int) component_positions.size();
}

/// return the index of a component given by name or -1 if not found
unsigned int component_format::get_component_index(const std::string& name) const
{
	for (unsigned int i=0; i<get_nr_components(); ++i) {
		if (name == get_component_name(i))
			return i;
	}
	return -1;
}


/// return the name of the component with index i
std::string component_format::get_component_name(unsigned int i) const
{
	if (i >= get_nr_components())
		return "";
	unsigned n = (unsigned)component_string.size();
	if (i+1 < get_nr_components())
		n = component_positions[i+1]-1;
	n -= component_positions[i];
	return component_string.substr(component_positions[i],n);
}

/// return whether the component format is one of the standard formats
ComponentFormat component_format::get_standard_component_format() const
{
	std::string component_name_list;
	for (unsigned int i=0; i<get_nr_components(); ++i) {
		if (!component_name_list.empty())
			component_name_list += ',';
		component_name_list += get_component_name(i);
	}
	for (ComponentFormat cf = ComponentFormat(CF_UNDEF+1); cf < CF_LAST; cf = ComponentFormat(cf+1)) {
		if (component_name_list == component_formats[cf])
			return cf;
	}
	return CF_UNDEF;
}

/// set component names from a comma or colon separated list of names
void component_format::set_component_names(const std::string& _component_name_list)
{
	component_string = _component_name_list;
	extract_components();
}

/// set the component names from a given component format
void component_format::set_component_format(ComponentFormat _cf)
{
	component_string = component_formats[_cf];
}

/// return the component type
TypeId component_format::get_component_type() const
{
	return component_type;
}

/// set the component type
void component_format::set_component_type(TypeId _type_id)
{
	component_type = _type_id;
}


/// return the size of one entry of component_positions in bytes
unsigned int component_format::get_entry_size() const
{
	if (is_packing()) {
		unsigned int nr_bits = 0;
		for (unsigned int ci = 0; ci < get_nr_components(); ++ci)
			nr_bits += align(get_bit_depth(ci), get_component_alignment());
		return align(nr_bits, 8)/8;
	}
	return get_nr_components()*align(get_type_size(get_component_type()),get_component_alignment());
}

/// return the packing info by simple conversion of the this pointer
const packing_info& component_format::get_packing_info() const
{
	return *this;
}

/// set packing info by simply assigning to a converted this pointer
void component_format::set_packing_info(const packing_info& pi)
{
	*static_cast<packing_info*>(this) = pi;
}


/// comparison between component formats
bool component_format::operator == (const component_format& cf) const
{
	if (get_nr_components() != cf.get_nr_components())
		return false;
	if (get_component_type() != cf.get_component_type())
		return false;
	if (get_packing_info() != cf.get_packing_info())
		return false;
	for (unsigned int i=0; i<get_nr_components(); ++i) {
		if (std::string(get_component_name(i)) != cf.get_component_name(i))
			return false;
	}
	return true;
}
/// comparison between component formats
bool component_format::operator != (const component_format& cf) const
{
	return !(*this == cf);
}

/// define stream out operator
std::ostream& operator << (std::ostream& os, const component_format& cf)
{
	os << get_type_name(cf.get_component_type());
	bool individual_bit_depth = false;
	if (cf.is_packing()) {
		if (cf.get_bit_depth(0) == cf.get_bit_depth(1) &&
		    cf.get_bit_depth(0) == cf.get_bit_depth(2) && 
			 cf.get_bit_depth(0) == cf.get_bit_depth(3))
			os << ':' << cf.get_bit_depth(0);
		else
			individual_bit_depth = true;
	}
	if (cf.get_component_alignment() != 1)
		os << '|' << cf.get_component_alignment();
	os << "[";
	for (unsigned int i=0; i<cf.get_nr_components(); ++i) {
		if (i > 0)
			os << ',';
		os << cf.get_component_name(i);
		if (individual_bit_depth)
			os << ':' << cf.get_bit_depth(i);
	}
	os << ']';
	return os;
}

bool fmt1_compares_better(const component_format& fmt,
					           const component_format& fmt1,
					           const component_format& fmt2)
{
	// check format
	ComponentFormat cf  = fmt.get_standard_component_format();
	if (cf != CF_UNDEF) {
		ComponentFormat cf1 = fmt1.get_standard_component_format();
		ComponentFormat cf2 = fmt2.get_standard_component_format();
		if ((cf1 == cf) != (cf2 == cf))
			return cf1 == cf;
	}
	unsigned int nc = fmt.get_nr_components();
	unsigned int nc1 = fmt1.get_nr_components();
	unsigned int nc2 = fmt2.get_nr_components();
	if ((nc1 == nc) != (nc2 == nc))
		return nc1 == nc;
	if ((nc1 >= nc) != (nc2 >= nc))
		return nc1 >= nc;

	TypeId ti = fmt.get_component_type();
	TypeId ti1 = fmt1.get_component_type();
	TypeId ti2 = fmt2.get_component_type();
	if ((ti1 == ti) != (ti2 == ti))
		return ti1 == ti;

	bool ip = fmt.is_packing();
	bool ip1 = fmt1.is_packing();
	bool ip2 = fmt2.is_packing();
	if ((ip1 == ip) != (ip2 == ip))
		return ip1 == ip;

	unsigned int diff1 = 0;
	unsigned int diff2 = 0;
	for (unsigned int i=0; i<nc; ++i) {
		unsigned int bd = get_type_size(fmt.get_component_type())*8;
		if (ip)
			bd = fmt.get_bit_depth(i);
		if (i >= nc1)
			diff1 += bd;
		else {
			unsigned int bd1 = get_type_size(fmt1.get_component_type())*8;
			if (ip1)
				bd1 = fmt1.get_bit_depth(i);
			if (bd1 > bd)
				diff1 += bd1-bd;
			else
				diff1 += 2*(bd-bd1);
		}
		if (i >= nc2)
			diff2 += bd;
		else {
			unsigned int bd2 = get_type_size(fmt2.get_component_type())*8;
			if (ip2)
				bd2 = fmt2.get_bit_depth(i);
			if (bd2 > bd)
				diff2 += bd2-bd;
			else
				diff2 += 2*(bd-bd2);
		}
	}
	if (diff1 != diff2)
		return diff1 < diff2;

	return true;
}

unsigned int find_best_match(
				const component_format& fmt,
				const char** format_descriptions,
				const component_format* fmt0,
				bool (*fmt1_better_match)(
				   const component_format& fmt,
					const component_format& fmt1,
					const component_format& fmt2))
{
	component_format best_match;
	unsigned int best_i = -1;
	unsigned int i = 0;
	if (fmt0)
		best_match = *fmt0;
	else {
		best_match = component_format(*format_descriptions);
		best_i = 0;
		++format_descriptions;
		++i;
	}
	while (*format_descriptions) {
		component_format fmt1(*format_descriptions);
		if (!fmt1_better_match(fmt, best_match, fmt1)) {
			best_match = fmt1;
			best_i     = i;
		}
		++format_descriptions;
		++i;
	}
	return best_i;
}


	}
}