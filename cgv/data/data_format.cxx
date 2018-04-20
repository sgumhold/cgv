#include "data_format.h"
#include <iostream>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>

using namespace cgv::utils;

namespace cgv {
	namespace data {

/// construct an undefined data format
data_format::data_format()
{
}

/// construct from description string
data_format::data_format(const std::string& description)
{
	set_data_format(description);
}

/// set the data format from a description string
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
	unsigned int i=0;
	unsigned int di = 0;
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

/// construct a 1d data format from width and the information needed to construct a component info
data_format::data_format(unsigned int _width, 
		TypeId _ct, const std::string& _cnl, 
		unsigned int a, unsigned int d0, unsigned int d1, 
		unsigned int d2, unsigned int d3) : component_format(_ct,_cnl,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
}

/// construct a 1d data format from width and the information needed to construct a component info
data_format::data_format(unsigned int _width, 
		TypeId _ct, ComponentFormat _cf, 
		unsigned int a, unsigned int d0, unsigned int d1, 
		unsigned int d2, unsigned int d3) : component_format(_ct,_cf,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
}

/// construct a 2d data format from width and height
data_format::data_format(unsigned int _width, unsigned int _height, 
		TypeId _ct, const std::string& _cnl, 
		unsigned int a, unsigned int d0, unsigned int d1, 
		unsigned int d2, unsigned int d3) : component_format(_ct,_cnl,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
}
/// construct a 2d data format from width and height
data_format::data_format(unsigned int _width, unsigned int _height, 
		TypeId _ct, ComponentFormat _cf, 
		unsigned int a, unsigned int d0, unsigned int d1, 
		unsigned int d2, unsigned int d3) : component_format(_ct,_cf,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
}

/// construct a 3d data format from width, height and depth
data_format::data_format(unsigned int _width, unsigned int _height, unsigned int _depth,
		TypeId _ct, const std::string& _cnl, 
		unsigned int a, unsigned int d0, unsigned int d1, 
		unsigned int d2, unsigned int d3) : component_format(_ct,_cnl,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
	dimensions.push_back(dimension_info(_depth,1,2));
}
/// construct a 3d data format from width, height and depth
data_format::data_format(unsigned int _width, unsigned int _height, unsigned int _depth,
		TypeId _ct, ComponentFormat _cf, 
		unsigned int a, unsigned int d0, unsigned int d1, 
		unsigned int d2, unsigned int d3) : component_format(_ct,_cf,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
	dimensions.push_back(dimension_info(_depth,1,2));
}

/// construct a 4d data format from width, height, depth and count
data_format::data_format(unsigned int _width, unsigned int _height, 
				unsigned int _depth, unsigned int _count,
		TypeId _ct, const std::string& _cnl, 
		unsigned int a, unsigned int d0, unsigned int d1, 
		unsigned int d2, unsigned int d3) : component_format(_ct,_cnl,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
	dimensions.push_back(dimension_info(_depth,1,2));
	dimensions.push_back(dimension_info(_count,1,3));
}
/// construct a 4d data format from width, height, depth and count
data_format::data_format(unsigned int _width, unsigned int _height, 
				unsigned int _depth, unsigned int _count,
		TypeId _ct, ComponentFormat _cf, 
		unsigned int a, unsigned int d0, unsigned int d1, 
		unsigned int d2, unsigned int d3) : component_format(_ct,_cf,a,d0,d1,d2,d3)
{
	dimensions.push_back(dimension_info(_width));
	dimensions.push_back(dimension_info(_height,1,1));
	dimensions.push_back(dimension_info(_depth,1,2));
	dimensions.push_back(dimension_info(_count,1,3));
}

/// set the dimensions to the given values
void data_format::set_dimensions(unsigned _d0, unsigned _d1, unsigned _d2, unsigned _d3)
{
	set_width(_d0);
	if (_d1 != -1)
		set_height(_d1);
	if (_d2 != -1)
		set_depth(_d2);
	if (_d3 != -1)
		set_nr_time_steps(_d3);
}

/// return the dimension of the data set
unsigned int data_format::get_nr_dimensions() const
{
	return (unsigned int) dimensions.size();
}
/// set the dimension of the data set
void data_format::set_nr_dimensions(unsigned int _d)
{
	dimensions.resize(_d);
}
/// return the number of data entries in the given dimension
unsigned int data_format::get_resolution(unsigned int i) const
{
	if (i >= get_nr_dimensions())
		return 0;
	return dimensions[i].resolution;
}

/// set the number of data entries in the given dimension
void data_format::set_resolution(unsigned int i, unsigned int resolution)
{
	if (i >= get_nr_dimensions())
		dimensions.resize(i+1);
	dimensions[i].resolution = resolution;
}

/// return the layout dimension of a given dimension
unsigned int data_format::get_layout_dimension(unsigned int i) const
{
	if (i >= get_nr_dimensions())
		return i;
	return dimensions[i].layout_dimension;
}

/// set the layout dimension of a given dimension
void data_format::get_layout_dimension(unsigned int i, unsigned int layout_dim)
{
	if (i >= get_nr_dimensions())
		dimensions.resize(i+1);
	dimensions[i].layout_dimension = layout_dim;
}


/// return the total number of data entries 
unsigned int data_format::get_nr_entries() const
{
	unsigned int size = 1;
	for (unsigned int i=0; i<get_nr_dimensions(); ++i)
		size *= get_resolution(i);
	return size;
}

/// return the total number of bytes necessary to store the data
unsigned int data_format::get_nr_bytes() const
{
	return get_nr_entries()*get_entry_size();
}


/// return the number of entries in the first dimension
unsigned int data_format::get_width() const
{
	return get_resolution(0);
}
/// return the number of entries in the second dimension
unsigned int data_format::get_height() const
{
	return get_resolution(1);
}
/// return the number of entries in the third dimension
unsigned int data_format::get_depth() const
{
	return get_resolution(2);
}
/// return the number of entries in the last dimension
unsigned int data_format::get_nr_time_steps() const
{
	if (get_nr_dimensions() == 0)
		return 1;
	return get_resolution(get_nr_dimensions()-1);
}

/// return the alignment of entries
unsigned int data_format::get_entry_alignment() const
{
	if (get_nr_dimensions() == 0)
		return 1;
	return dimensions[0].alignment;
}

/// return the alignment of a given dimension
unsigned int data_format::get_alignment(unsigned int i) const
{
	if (i+1 >= get_nr_dimensions())
		return 1;
	return dimensions[i+1].alignment;
}
/// set the number of entries in the first dimension
void data_format::set_width(unsigned int _width) 
{
	set_resolution(0, _width);
}
/// set the number of entries in the second dimension
void data_format::set_height(unsigned int _height) 
{
	set_resolution(1, _height);
}
/// set the number of entries in the third dimension
void data_format::set_depth(unsigned int _depth) 
{
	set_resolution(2, _depth);
}
/// set the number of entries in the last dimension
void data_format::set_nr_time_steps(unsigned int _nr_time_steps) 
{
	if (get_nr_dimensions() == 0)
		return;
	set_resolution(get_nr_dimensions()-1, _nr_time_steps);
}
/// set the alignment of entries
void data_format::set_entry_alignment(unsigned int _a)
{
	if (get_nr_dimensions() == 0)
		dimensions.resize(1);
	dimensions[0].alignment = _a;
}

/// set the alignment of a given dimension
void data_format::set_alignment(unsigned int i, unsigned int _a)
{
	if (i >= get_nr_dimensions())
		dimensions.resize(i+1);
	if (i+1 >= get_nr_dimensions())
		return;
	dimensions[i+1].alignment = _a;
}

/// return the component_format info by simple conversion of the this pointer
const component_format& data_format::get_component_format() const
{
	return *static_cast<const component_format*>(this);
}

/// set component_format by simply assigning to a converted this pointer
void data_format::set_component_format(const component_format& cf)
{
	*static_cast<component_format*>(this) = cf;
}

/// comparison between component formats
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

/// comparison between component formats
bool data_format::operator != (const data_format& df) const
{
	return !(*this == df);
}

/// define stream out operator
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