#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace utils {
		/// check if a bit of a bit field is set
		CGV_API bool is_bit_set(unsigned int bit_idx, unsigned int bit_field);
		/// set all the bits of bit_field with index equal or larger than fst_bit_idx
		CGV_API void enable_upper_bits(unsigned int& bit_field, unsigned int fst_bit_idx);
		/// clear all the bits of bit_field with index equal or larger than fst_bit_idx
		CGV_API void disable_upper_bits(unsigned int& bit_field, unsigned int fst_bit_idx);
		/// set all the bits of bit_field with index equal or less than fst_bit_idx
		CGV_API void enable_lower_bits(unsigned int& bit_field, unsigned int fst_bit_idx);
		/// clear all the bits of bit_field with index equal or less than fst_bit_idx
		CGV_API void disable_lower_bits(unsigned int& bit_field, unsigned int fst_bit_idx);
		/// set n bits starting with index off of the given bit field from the first bits of the given integer value
		CGV_API void set_bits(unsigned int& bit_field, unsigned int off, unsigned int n, unsigned int value);
	}
}

#include <cgv/config/lib_end.h>