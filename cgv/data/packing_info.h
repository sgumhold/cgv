#pragma once

#include <stddef.h>

#include <cgv/data/lib_begin.h>

namespace cgv {
	namespace data {

/** the packing_info class stores information about packed integers 
	 structures. It is typically used to define packed component formats.
	 The stored information includes, whether packing is used, 
	 the alignment of the components and bit depths of up to 4 components.
	 The aligment is an integer defining a multiple to which the components
	 are aligned. If the alignment is 1, the components are packed densely.
	 If it is for example 8, the components are stored at starting at 
	 locations that are multiples of 8. For example if the components have
	 bit depth 6 and the alignment is 8, then the first component is stored
	 starting with the first bit, the second component starting with the 8th
	 bit and so on. In case no packing is used the alignment is measured in
	 bytes. The bit depths and alignment are stored as unsigned integers
	 with 6 bits, i.e. the maximum value is 63.
*/
class CGV_API packing_info 
{
protected:
	bool is_packed   : 1;
	unsigned bd0 : 6;
	unsigned bd1 : 6;
	unsigned bd2 : 6;
	unsigned bd3 : 6;
	unsigned ca  : 6;
	///
	bool prepare_bit_operation(unsigned ci, void* ptr, unsigned &off, unsigned &bd, unsigned* &iptr) const;
	bool prepare_bit_operation(unsigned ci, const void* ptr, unsigned &off, unsigned &bd, const unsigned* &iptr) const;
	unsigned int get_bit_offset(unsigned ci) const;
public:
	/** construct packing information from alignment and bit depths. If no bit depths are given, the components are not
		packed and the alignment is in bytes. If one or more depths are specified, the alignment is in bits. If exactly
		one depth is given, all component bit depths are set to this bit depth */
	packing_info(unsigned align = 1, unsigned d0 = 0, unsigned d1 = 0, unsigned d2 = 0, unsigned d3 = 0);
	/// clear packing info information
	void clear();
	/// return the bit depth of the ci-th component
	unsigned get_bit_depth(unsigned ci) const;
	/// set the bit depth of the ci-th component
	void set_bit_depth(unsigned ci, unsigned depth);
	/// return whether packing is enabled
	bool is_packing() const;
	/// set the packing flag
	void set_packing(bool enable = true);
	/// return the component alignment in bits in the packed case and in bytes in the unpacked case
	unsigned int get_component_alignment() const;
	/// set the component alignment in bits in the packed case and in bytes in the unpacked case
	void set_component_alignment(unsigned a);
	/// return the next integer larger or equal to v which is dividable by a
	static size_t align(size_t v, unsigned a);
	/// return the ci-th component of the data entry pointed to by the given pointer of a signed packed component
	int get_signed(unsigned ci, const void* ptr) const;
	/// return the ci-th component of the data entry pointed to by the given pointer of an unsigned packed component
	unsigned int get_unsigned(unsigned ci, const void* ptr) const;
	/// set the ci-th component of the data entry pointed to by the given pointer of a signed packed component
	bool set_signed(unsigned ci, void* ptr, int v) const;
	/// set the ci-th component of the data entry pointed to by the given pointer of an unsigned packed component
	bool set_unsigned(unsigned ci, void* ptr, unsigned v) const;
	/// equal comparison
	bool operator == (const packing_info& pi) const;
	/// unequal comparison 
	bool operator != (const packing_info& pi) const;
};

	}
}

#include <cgv/config/lib_end.h>