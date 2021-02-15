#pragma once

#include "component_format.h"

#include "lib_begin.h"

using namespace cgv::type::info;

namespace cgv {
	namespace data {

/** A data_format describes a multidimensional data block of data entries. 
    It inherits the information stored in component info which describes
	 which components each data entry has. The data_format adds information
	 about the dimensionality and the alignment and resolution in each 
	 dimension. */
class CGV_API data_format : public component_format
{
protected:
	struct dimension_info
	{
		unsigned int resolution;
		unsigned int alignment;
		unsigned int layout_dimension;
		dimension_info(unsigned int n = 0, unsigned int a = 1, unsigned int ld = 0) 
			: resolution(n), alignment(a), layout_dimension(ld) {}
	};
	/// store for each dimension resolution and alignment in a dimension_info struct
	std::vector<dimension_info> dimensions;
public:
	/// construct an undefined data format
	data_format();
	/// construct from description string, see set_data_format for docu
	explicit data_format(const std::string& description);
	/** 
Set data format from description string, which adds information to the description string
of the component format and has the following syntax. For the definition of the token
\c component_format in the syntax definition refer to the docu of 
component_format::set_component_format(). If a parse error arises, return false and set 
the static last_error member, which can be queried with get_last_error().

Syntax definition:

\verbatim
data_format <- component_format['|' alignment_in_bytes] '(' dimension_info (',' dimension_info)* ')'

dimension_info <- resolution [':' layout_dimension]['|' alignment_in_bytes]

- resolution : unsigned int ... number of entries in specified dimension

- layout_dimension : unsigned int ... index of dimension in memory layout, which defaults
                                    to the specified dimension
- alignment_in_bytes : unsigned int ... alignment in the memory layout for entries with index zero
                                        in the specified dimension
\endverbatim

In the standard memory layout of for example an image, the entries are arranged line by line,
i.e. first the entries of the first line are stored from left to right, followed by the entries
of the second line and so on. To change this order of the layout, one can specify for each
dimension in the data format a layout dimension which defaults to the dimension index, i.e.
if the layout dimensions of all dimensions correspond to the dimension index, the memory layout
is starting with the 0-th dimension. To store an image in a columns first memory layout, the
layout dimensions would be 1 and 0 for dimensions 0 and 1.

Some examples of valid data format description strings:
- \c "uint8[R,G,B](127|8,256)" ... 127x256 RGB-image, where each line is aligned to multiples of 8
- \c "uint8[R,G,B](127:1,256:0)" ... 127x256 RGB-image stored in column major memory layout
- \c "uint16[L:12,A:12]|4(32,32,32)" ... 32x32x32 12-Bit Luminance Alpha Volume, where each 
                                         entry is aligned to 4 bytes
*/
	bool set_data_format(const std::string& description);
	/// construct a 1d data format from width and the information needed to construct a component info
	data_format(unsigned int _width, 
					TypeId _component_type, 
				   const std::string& _component_name_list, 
					unsigned int align = 1, 
					unsigned int d0 = 0, unsigned int d1 = 0, 
					unsigned int d2 = 0, unsigned int d3 = 0);
	/// construct a 1d data format from width and the information needed to construct a component info
	data_format(unsigned int _width, 
					TypeId _component_type, 
				   ComponentFormat _cf, 
					unsigned int align = 1, 
					unsigned int d0 = 0, unsigned int d1 = 0, 
					unsigned int d2 = 0, unsigned int d3 = 0);
	/// construct a 2d data format from width and height
	data_format(unsigned int _width, unsigned int _height, 
					TypeId _component_type, 
				   const std::string& _component_name_list, 
					unsigned int align = 1, 
					unsigned int d0 = 0, unsigned int d1 = 0, 
					unsigned int d2 = 0, unsigned int d3 = 0);
	/// construct a 2d data format from width and height
	data_format(unsigned int _width, unsigned int _height, 
					TypeId _component_type, 
				   ComponentFormat _cf, 
					unsigned int align = 1, 
					unsigned int d0 = 0, unsigned int d1 = 0, 
					unsigned int d2 = 0, unsigned int d3 = 0);
	/// construct a 3d data format from width, height and depth
	data_format(unsigned int _width, unsigned int _height, unsigned int _depth,
					TypeId _component_type, 
				   const std::string& _component_name_list, 
					unsigned int align = 1, 
					unsigned int d0 = 0, unsigned int d1 = 0, 
					unsigned int d2 = 0, unsigned int d3 = 0);
	/// construct a 3d data format from width, height and depth
	data_format(unsigned int _width, unsigned int _height, unsigned int _depth,
					TypeId _component_type, 
				   ComponentFormat _cf, 
					unsigned int align = 1, 
					unsigned int d0 = 0, unsigned int d1 = 0, 
					unsigned int d2 = 0, unsigned int d3 = 0);
	/// construct a 4d data format from width, height, depth and count
	data_format(unsigned int _width, unsigned int _height, 
					unsigned int _depth, unsigned int _count,
					TypeId _component_type, 
				   const std::string& _component_name_list, 
					unsigned int align = 1, 
					unsigned int d0 = 0, unsigned int d1 = 0, 
					unsigned int d2 = 0, unsigned int d3 = 0);
	/// construct a 4d data format from width, height, depth and count
	data_format(unsigned int _width, unsigned int _height, 
					unsigned int _depth, unsigned int _count,
					TypeId _component_type, 
				   ComponentFormat _cf, 
					unsigned int align = 1, 
					unsigned int d0 = 0, unsigned int d1 = 0, 
					unsigned int d2 = 0, unsigned int d3 = 0);
	/// define stream out operator
	friend FRIEND_MEMBER_API std::ostream& operator << (std::ostream& os, const data_format& df);
	/// set the dimensions to the given values
	void set_dimensions(unsigned _d0, unsigned _d1 = -1, unsigned _d2 = -1, unsigned _d3 = -1);
	/// return the number of dimensions of the data set
	unsigned int get_nr_dimensions() const;
	/// set the number of dimensions of the data set
	void set_nr_dimensions(unsigned int _d);
	/// return the resolution in the i-th dimension, or 0 if not defined
	unsigned int get_resolution(unsigned int i) const;
	/// return the resolution in the first dimension, or 1 if not defined
	unsigned int get_width() const;
	/// return the resolution in the second dimension, or 1 if not defined
	unsigned int get_height() const;
	/// return the resolution in the third dimension, or 1 if not defined
	unsigned int get_depth() const;
	/// return the resolution in the highest dimension, or 1 if not defined
	unsigned int get_nr_time_steps() const;
	/// return the total number of data entries 
	size_t get_nr_entries() const;
	/// return the total number of bytes necessary to store the data
	size_t get_nr_bytes() const;
	/// set the resolution in the i-th dimension, add dimensions if necessary
	void set_resolution(unsigned int i, unsigned int resolution);
	/// set the resolution in the first dimension, add dimensions if necessary
	void set_width(unsigned int _width);
	/// set the resolution in the second dimension, add dimensions if necessary
	void set_height(unsigned int _height);
	/// set the resolution in the third dimension, add dimensions if necessary
	void set_depth(unsigned int _depth);
	/// set the resolution in the last dimension, add dimensions if necessary
	void set_nr_time_steps(unsigned int _nr_time_steps);
	/// return the alignment of entries
	unsigned int get_entry_alignment() const;
	/** return the alignment of a given dimension, where the alignment of the 
	    last dimension is always 1 and cannot be changed. This method also returns 1
		 if i is out of the range of valid dimensions. */
	unsigned int get_alignment(unsigned int i) const;
	/// set the alignment of entries
	void set_entry_alignment(unsigned int _a);
	/** set the alignment of a given dimension, add dimensions if necessary. The
	    alignment of the last dimension is always 1 and cannot be set.*/
	void set_alignment(unsigned int i, unsigned int _a);
	/// return the layout dimension of a given dimension
	unsigned int get_layout_dimension(unsigned int dim) const;
	/// set the layout dimension of a given dimension, add dimensions if necessary
	void get_layout_dimension(unsigned int dim, unsigned int layout_dim);
	/// return the component_format info by simple conversion of the this pointer
	const component_format& get_component_format() const;
	/// set component_format by simply assigning to a converted this pointer
	void set_component_format(const component_format& cf);
	/// comparison between component formats
	bool operator == (const data_format& df) const;
	/// comparison between component formats
	bool operator != (const data_format& df) const;
};

/** stream out operator writes the data format in the syntax of description strings
    as defined in the docu of set_data_format(). */
extern CGV_API std::ostream& operator << (std::ostream& os, const data_format& df);

	}
}

#include <cgv/config/lib_end.h>