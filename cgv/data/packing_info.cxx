#include <cgv/utils/bit_operations.h>
#include "packing_info.h"

using namespace cgv::utils;

namespace cgv {
	namespace data {
size_t packing_info::align(size_t v, unsigned a)
{
	return a*(v/a) == v ? v : a*((v/a)+1);
}
packing_info::packing_info(unsigned align, unsigned d0, unsigned d1, unsigned d2, unsigned d3) 
	: is_packed(true), bd0(d0), bd1(d1), bd2(d2), bd3(d3), ca(align)
{
	if (d0 == 0)
		is_packed = false;
	else if (d1 == 0)
		bd1 = bd2 = bd3 = d0;
}
void packing_info::clear()
{
	is_packed = false;
	ca = 1;
	bd0 = bd1 = bd2 = bd3 = 0;
}
unsigned packing_info::get_bit_depth(unsigned ci) const 
{ 
	return ci<2 ? (ci==0?bd0:bd1) : (ci==2?bd2:bd3); 
}
void packing_info::set_bit_depth(unsigned ci, unsigned depth) 
{
	if (ci<2) if (ci == 0) bd0 = depth; else bd1 = depth;
	else      if (ci == 2) bd2 = depth; else bd3 = depth;
}
bool packing_info::is_packing() const 
{
	return is_packed; 
}
void packing_info::set_packing(bool enable) 
{
	is_packed = enable; 
}
unsigned int packing_info::get_component_alignment() const
{
	return ca;
}
void packing_info::set_component_alignment(unsigned int a)
{
	ca = a;
}
bool packing_info::prepare_bit_operation(unsigned ci, void* ptr, unsigned &off, unsigned &bd, unsigned* &iptr) const
{
	off = get_bit_offset(ci);
	bd  = get_bit_depth(ci);
	ptr = static_cast<unsigned char*>(ptr)+off/8;
	off -= 8*(off/8);
	iptr = static_cast<unsigned*>(ptr);
	return true;
};
bool packing_info::prepare_bit_operation(unsigned ci, const void* ptr, unsigned &off, unsigned &bd, const unsigned* &iptr) const
{
	off = get_bit_offset(ci);
	bd  = get_bit_depth(ci);
	ptr = static_cast<const unsigned char*>(ptr)+off/8;
	off -= 8*(off/8);
	iptr = static_cast<const unsigned*>(ptr);
	return true;
};
unsigned packing_info::get_bit_offset(unsigned ci) const
{
	unsigned off = 0;
	for (unsigned i = 0; i < ci; ++i)
		off += unsigned(align(get_bit_depth(i),get_component_alignment()));
	return off;
}
int packing_info::get_signed(unsigned ci, const void* ptr) const
{
	unsigned off, bd;
	const unsigned *iptr;
	prepare_bit_operation(ci, ptr, off, bd, iptr);
	unsigned int i = *iptr >> off;
	if (is_bit_set(bd-1,i))
		enable_upper_bits(i, bd);
	else
		disable_upper_bits(i, bd);
	return (int) i;
}
unsigned packing_info::get_unsigned(unsigned ci, const void* ptr) const
{
	unsigned off, bd;
	const unsigned *iptr;
	prepare_bit_operation(ci, ptr, off, bd, iptr);
	unsigned i = *iptr >> off;
	disable_upper_bits(i, bd);
	return i;
}
bool packing_info::set_signed(unsigned ci, void* ptr, int v) const
{
	unsigned off, bd, *iptr;
	prepare_bit_operation(ci, ptr, off, bd, iptr);
	set_bits(*iptr, off, bd, (const unsigned&) v);
	return true;
}
bool packing_info::set_unsigned(unsigned ci, void* ptr, unsigned v) const
{
	unsigned off, bd, *iptr;
	prepare_bit_operation(ci, ptr, off, bd, iptr);
	set_bits(*iptr, off, bd, v);
	return true;
}
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
bool packing_info::operator != (const packing_info& pi) const
{
	return !(*this == pi);
}
	}
}