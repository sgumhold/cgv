#include <cgv/utils/bit_operations.h>
#include "packing_info.h"

using namespace cgv::utils;

namespace cgv {
	namespace data {

unsigned int packing_info::align(unsigned int v, unsigned int a)
{
	return a*(v/a) == v ? v : a*((v/a)+1);
}

/// construct packed format with different bit depths
packing_info::packing_info(unsigned int align, unsigned int d0, unsigned int d1, unsigned int d2, unsigned int d3) 
	: is_packed(true), bd0(d0), bd1(d1), bd2(d2), bd3(d3), ca(align)
{
	if (d0 == 0)
		is_packed = false;
	else if (d1 == 0)
		bd1 = bd2 = bd3 = d0;
}

/// clear packing info information
void packing_info::clear()
{
	is_packed = false;
	ca = 1;
	bd0 = bd1 = bd2 = bd3 = 0;
}


/// return the bit depth of the ci-th component
unsigned int packing_info::get_bit_depth(unsigned int ci) const 
{ 
	return ci<2 ? (ci==0?bd0:bd1) : (ci==2?bd2:bd3); 
}
/// set the bit depth of the ci-th component
void packing_info::set_bit_depth(unsigned int ci, unsigned int depth) 
{
	if (ci<2) if (ci == 0) bd0 = depth; else bd1 = depth;
	else      if (ci == 2) bd2 = depth; else bd3 = depth;
}
/// return whether packing is enabled
bool packing_info::is_packing() const 
{
	return is_packed; 
}
/// set the packing flag
void packing_info::set_packing(bool enable) 
{
	is_packed = enable; 
}

/// return the component alignment in bits in the packed case and in bytes in the unpacked case
unsigned int packing_info::get_component_alignment() const
{
	return ca;
}
/// set the component alignment in bits in the packed case and in bytes in the unpacked case
void packing_info::set_component_alignment(unsigned int a)
{
	ca = a;
}

bool packing_info::prepare_bit_operation(unsigned int ci, void* ptr, unsigned int &off, unsigned int &bd, unsigned int* &iptr) const
{
	off = get_bit_offset(ci);
	bd  = get_bit_depth(ci);
	ptr = static_cast<unsigned char*>(ptr)+off/8;
	off -= 8*(off/8);
	iptr = static_cast<unsigned int*>(ptr);
	return true;
};

bool packing_info::prepare_bit_operation(unsigned int ci, const void* ptr, unsigned int &off, unsigned int &bd, const unsigned int* &iptr) const
{
	off = get_bit_offset(ci);
	bd  = get_bit_depth(ci);
	ptr = static_cast<const unsigned char*>(ptr)+off/8;
	off -= 8*(off/8);
	iptr = static_cast<const unsigned int*>(ptr);
	return true;
};

unsigned int packing_info::get_bit_offset(unsigned int ci) const
{
	unsigned int off = 0;
	for (unsigned int i = 0; i < ci; ++i)
		off += align(get_bit_depth(i),get_component_alignment());
	return off;
}

/// return the ci-th component of the data entry pointed to by the given pointer of a signed packed component
int packing_info::get_signed(unsigned int ci, const void* ptr) const
{
	unsigned int off, bd;
	const unsigned int *iptr;
	prepare_bit_operation(ci, ptr, off, bd, iptr);
	unsigned int i = *iptr >> off;
	if (is_bit_set(bd-1,i))
		enable_upper_bits(i, bd);
	else
		disable_upper_bits(i, bd);
	return (int) i;
}

/// return the ci-th component of the data entry pointed to by the given pointer of an unsigned packed component
unsigned int packing_info::get_unsigned(unsigned int ci, const void* ptr) const
{
	unsigned int off, bd;
	const unsigned int *iptr;
	prepare_bit_operation(ci, ptr, off, bd, iptr);
	unsigned int i = *iptr >> off;
	disable_upper_bits(i, bd);
	return i;
}

/// set the ci-th component of the data entry pointed to by the given pointer of a signed packed component
bool packing_info::set_signed(unsigned int ci, void* ptr, int v) const
{
	unsigned int off, bd, *iptr;
	prepare_bit_operation(ci, ptr, off, bd, iptr);
	set_bits(*iptr, off, bd, (const unsigned int&) v);
	return true;
}

/// set the ci-th component of the data entry pointed to by the given pointer of an unsigned packed component
bool packing_info::set_unsigned(unsigned int ci, void* ptr, unsigned int v) const
{
	unsigned int off, bd, *iptr;
	prepare_bit_operation(ci, ptr, off, bd, iptr);
	set_bits(*iptr, off, bd, v);
	return true;
}

/// equal comparison
bool packing_info::operator == (const packing_info& pi) const
{
	if (is_packing() != pi.is_packing())
		return false;
	if (get_component_alignment() != pi.get_component_alignment())
		return false;
	for (unsigned int i=0; i<4; ++i)
		if (get_bit_depth(i) != pi.get_bit_depth(i))
			return false;
	return true;
}

/// unequal comparison 
bool packing_info::operator != (const packing_info& pi) const
{
	return !(*this == pi);
}


	}
}