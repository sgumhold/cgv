#include "bit_operations.h"

namespace cgv {
	namespace utils {

/// check if a bit of a bit field is set
bool is_bit_set(unsigned int bit_idx, unsigned int bit_field)
{
	static unsigned int bit_mask[32] = { 
		1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 
		32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 
		16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648u };

	return (bit_field & bit_mask[bit_idx]) != 0;
}
/// set all the bits of bit_field with index equal or larger than fst_bit_idx
void enable_upper_bits(unsigned int& bit_field, unsigned int fst_bit_idx)
{
	static unsigned int bit_mask[32] = { 
		4294967295u, 4294967294u, 4294967292u, 4294967288u, 4294967280u, 4294967264u, 4294967232u, 4294967168u, 4294967040u, 
		4294966784u, 4294966272u, 4294965248u, 4294963200u, 4294959104u, 4294950912u, 4294934528u, 4294901760u, 4294836224u, 
		4294705152u, 4294443008u, 4293918720u, 4292870144u, 4290772992u, 4286578688u, 4278190080u, 4261412864u, 4227858432u, 
		4160749568u, 4026531840u, 3758096384u, 3221225472u, 2147483648u };

	bit_field |= bit_mask[fst_bit_idx];
}

/// clear all the bits of bit_field with index equal or larger than fst_bit_idx
void disable_upper_bits(unsigned int& bit_field, unsigned int fst_bit_idx)
{
	static unsigned int bit_mask[32] = { 
		0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071, 262143, 
		524287, 1048575, 2097151, 4194303, 8388607, 16777215, 33554431, 67108863, 134217727, 268435455, 536870911, 
		1073741823, 2147483647u };

	bit_field &= bit_mask[fst_bit_idx];
}

/// set all the bits of bit_field with index equal or less than fst_bit_idx
void enable_lower_bits(unsigned int& bit_field, unsigned int fst_bit_idx)
{
	static unsigned int bit_mask[32] = { 
		1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071, 262143, 
		524287, 1048575, 2097151, 4194303, 8388607, 16777215, 33554431, 67108863, 134217727, 268435455, 536870911, 
		1073741823, 2147483647u, 4294967295u };

	bit_field |= bit_mask[fst_bit_idx];
}

/// clear all the bits of bit_field with index equal or less than fst_bit_idx
void disable_lower_bits(unsigned int& bit_field, unsigned int fst_bit_idx)
{
	static unsigned int bit_mask[32] = { 
		4294967294u, 4294967292u, 4294967288u, 4294967280u, 4294967264u, 4294967232u, 4294967168u, 4294967040u, 
		4294966784u, 4294966272u, 4294965248u, 4294963200u, 4294959104u, 4294950912u, 4294934528u, 4294901760u, 4294836224u, 
		4294705152u, 4294443008u, 4293918720u, 4292870144u, 4290772992u, 4286578688u, 4278190080u, 4261412864u, 4227858432u, 
		4160749568u, 4026531840u, 3758096384u, 3221225472u, 2147483648u, 0 };

	bit_field &= bit_mask[fst_bit_idx];
}


/// set n bits starting with index off of the given bit field from the first bits of the given integer value
void set_bits(unsigned int& bit_field, unsigned int off, unsigned int n, unsigned int value)
{
	unsigned int tmp = bit_field >> off;
	disable_lower_bits(tmp, n-1);
	disable_upper_bits(value, n);
	tmp += value;
	tmp <<= off;
	disable_upper_bits(bit_field, off);
	bit_field += tmp;
}

	}
}
