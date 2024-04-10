#include "data_format.h"
#include <iostream>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>

using namespace cgv::utils;

namespace cgv {
	namespace data {
data_format::data_format()
{
}
data_format::data_format(const std::string& description)
{
	set_data_format(description);
}
bool data_format::set_data_format(const std::string& description)
{
	if (description.empty()) {
		dimensions.clear();
		component_format::clear();
		return true;
	}
	last_error = "";
	size_t p = description.find_first_of(']');
	if (p == std::string::npos) {
		last_error = "no <]> found in description";
		return false;
	}
	if (!component_format::set_component_format(description.substr(0,p+1)))
		return false;
	std::string sub_descr = description.substr(p+1);
	std::vector<token> toks;
	tokenizer(sub_descr).set_sep("():|,").bite_all(toks);
	if (toks.size() < 3) {
		last_error = "incomplete format description";
		return false;
	}
	if (to_string(toks.back()) != ")") {
		last_error = "format description not terminated by <)>";
		return false;
	}
	unsigned i=0;
	unsigned di = 0;
	int last_a = 1;
	if (to_string(toks[i]) == "|") {
		if (!is_integer(to_string(toks[1]), last_a)) {
			last_error = "expected integer after <|>";
			return false;
		}
		i += 2;
	}
	if (to_string(toks[i]) != "(") {
		last_error = "expected <(> to define dimensions";
		return false;
	}
	if (++i == toks.size()) {
		last_error = "incomplete format description";
		return false;
	}
	dimensions.clear();
	do {
		int n, ld = di, a = 1;
		if (!is_integer(to_string(toks[i]),n)) {
			last_error = "expected integer after <(> or <,>";
			return false;
		}
		++i;
		if (i+1 < toks.size()) {
			if (to_string(toks[i]) == ":") {
				if (!is_integer(to_string(toks[i+1]),ld)) {
					last_error = "expected integer after <:>";
					return false;
				}
				i += 2;
			}
		}
		if (i+1 < toks.size()) {
			if (to_string(toks[i]) == "|") {
				if (!is_integer(to_string(toks[i+1]),a)) {
					last_error = "expected integer after <:>";
					return false;
				}
				i += 2;
			}
		}
		dimensions.push_back(dimension_info(n,last_a,ld));
		last_a = a;
		if (to_string(toks[i]) == ")")
			return true;
		if (to_string(toks[i]) != ",") {
			last_error = "expected <,> after definition of dimension";
			return false;
		}
		++i;
		++di;
	} while (true);
	return true;
}
data_format::data_format(size_t _width, TypeId _ct, const std::string& _cnl, 
		                 unsigned a, unsigned d0, unsigned d1, unsigned d2, unsigned d3) 
	: component_format(_ct,_cnl,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
}
data_format::data_format(size_t _width, TypeId _ct, ComponentFormat _cf, 
						 unsigned a, unsigned d0, unsigned d1, unsigned d2, unsigned d3) 
	: component_format(_ct,_cf,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
}
data_format::data_format(size_t _width, size_t _height, TypeId _ct, const std::string& _cnl, 
	                     unsigned a, unsigned d0, unsigned d1, unsigned d2, unsigned d3) 
	: component_format(_ct,_cnl,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
}
data_format::data_format(size_t _width, size_t _height, TypeId _ct, ComponentFormat _cf, 
	                     unsigned a, unsigned d0, unsigned d1, unsigned d2, unsigned d3) 
	: component_format(_ct,_cf,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
}

data_format::data_format(size_t _width, size_t _height, size_t _depth, TypeId _ct, const std::string& _cnl,
	                     unsigned a, unsigned d0, unsigned d1, unsigned d2, unsigned d3) 
	: component_format(_ct,_cnl,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
	dimensions.push_back(dimension_info(_depth,1,2));
}
data_format::data_format(size_t _width, size_t _height, size_t _depth, TypeId _ct, ComponentFormat _cf, 
		                 unsigned a, unsigned d0, unsigned d1, unsigned d2, unsigned d3) 
	: component_format(_ct,_cf,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
	dimensions.push_back(dimension_info(_depth,1,2));
}
data_format::data_format(size_t _width, size_t _height, size_t _depth, size_t _count, TypeId _ct, const std::string& _cnl, 
                         unsigned a, unsigned d0, unsigned d1, unsigned d2, unsigned d3)
	: component_format(_ct, _cnl, a, d0, d1, d2, d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
	dimensions.push_back(dimension_info(_depth,1,2));
	dimensions.push_back(dimension_info(_count,1,3));
}
data_format::data_format(size_t _width, size_t _height, size_t _depth, size_t _count, TypeId _ct, ComponentFormat _cf, 
		                 unsigned a, unsigned d0, unsigned d1, unsigned d2, unsigned d3) 
	: component_format(_ct,_cf,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
	dimensions.push_back(dimension_info(_depth,1,2));
	dimensions.push_back(dimension_info(_count,1,3));
}
void data_format::set_dimensions(size_t _d0, size_t _d1, size_t _d2, size_t _d3)
{
	set_width(_d0);
	if (_d1 != -1)
		set_height(_d1);
	if (_d2 != -1)
		set_depth(_d2);
	if (_d3 != -1)
		set_nr_time_steps(_d3);
}
unsigned data_format::get_nr_dimensions() const
{
	return (unsigned) dimensions.size();
}
void data_format::set_nr_dimensions(unsigned _d)
{
	dimensions.resize(_d);
}
size_t data_format::get_resolution(unsigned i) const
{
	if (i >= get_nr_dimensions())
		return 0;
	return dimensions[i].resolution;
}
void data_format::set_resolution(unsigned i, size_t resolution)
{
	if (i >= get_nr_dimensions())
		dimensions.resize(i+1);
	dimensions[i].resolution = resolution;
}
unsigned data_format::get_layout_dimension(unsigned i) const
{
	if (i >= get_nr_dimensions())
		return i;
	return dimensions[i].layout_dimension;
}
void data_format::set_layout_dimension(unsigned i, unsigned layout_dim)
{
	if (i >= get_nr_dimensions())
		dimensions.resize(i+1);
	dimensions[i].layout_dimension = layout_dim;
}
size_t data_format::get_nr_entries() const
{
	size_t size = 1;
	for (unsigned int i=0; i<get_nr_dimensions(); ++i)
		size *= get_resolution(i);
	return size;
}
size_t data_format::get_nr_bytes() const
{
	return get_nr_entries()*get_entry_size();
}
size_t data_format::get_width() const
{
	return get_resolution(0);
}
size_t data_format::get_height() const
{
	return get_resolution(1);
}
size_t data_format::get_depth() const
{
	return get_resolution(2);
}
size_t data_format::get_nr_time_steps() const
{
	if (get_nr_dimensions() == 0)
		return 1;
	return get_resolution(get_nr_dimensions()-1);
}
unsigned data_format::get_entry_alignment() const
{
	if (get_nr_dimensions() == 0)
		return 1;
	return dimensions[0].alignment;
}
unsigned data_format::get_alignment(unsigned i) const
{
	if (i+1 >= get_nr_dimensions())
		return 1;
	return dimensions[i+1].alignment;
}
void data_format::set_width (size_t _width) 
{
	set_resolution(0, _width);
}
void data_format::set_height(size_t _height) 
{
	set_resolution(1, _height);
}
void data_format::set_depth (size_t _depth) 
{
	set_resolution(2, _depth);
}
void data_format::set_nr_time_steps(size_t _nr_time_steps) 
{
	if (get_nr_dimensions() == 0)
		return;
	set_resolution(get_nr_dimensions()-1, _nr_time_steps);
}
void data_format::set_entry_alignment(unsigned _a)
{
	if (get_nr_dimensions() == 0)
		dimensions.resize(1);
	dimensions[0].alignment = _a;
}
void data_format::set_alignment(unsigned i, unsigned _a)
{
	if (i >= get_nr_dimensions())
		dimensions.resize(i+1);
	if (i+1 >= get_nr_dimensions())
		return;
	dimensions[i+1].alignment = _a;
}
const component_format& data_format::get_component_format() const
{
	return *static_cast<const component_format*>(this);
}
void data_format::set_component_format(const component_format& cf)
{
	*static_cast<component_format*>(this) = cf;
}
bool data_format::operator == (const data_format& df) const
{
	if (get_nr_dimensions() != df.get_nr_dimensions())
		return false;
	for (unsigned int i=0; i<get_nr_dimensions(); ++i) {
		if (get_resolution(i) != df.get_resolution(i))
			return false;
		if (get_alignment(i) != df.get_alignment(i))
			return false;
	}
	return get_component_format() == df.get_component_format();
}
bool data_format::operator != (const data_format& df) const
{
	return !(*this == df);
}
std::ostream& operator << (std::ostream& os, const data_format& df)
{
	os << df.get_component_format();
	if (df.get_nr_dimensions() > 0 && df.get_entry_alignment() != 1)
		os << '|' << df.get_entry_alignment();
	os << "(";
	for (unsigned int i=0; i<df.get_nr_dimensions(); ++i) {
		if (i > 0)
			os << ',';
		os << df.get_resolution(i);
		if (df.get_layout_dimension(i) != i)
			os << ':' << df.get_layout_dimension(i);
		if (df.get_alignment(i) != 1)
			os << '|' << df.get_alignment(i);
	}
	return os << ')';
}
	}
}