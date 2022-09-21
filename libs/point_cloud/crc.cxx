#include "crc.h"

namespace cgv {
namespace pointcloud {
namespace file_parser {

uint8_t reverse_bits(uint8_t b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

uint32_t reverse_bits(uint32_t b)
{
	b = (b & 0xFFFF0000) >> 16 | (b & 0x0000FFFF) << 16;
	b = (b & 0xFF00FF00) >> 8 | (b & 0x00FF00FF) << 8;
	b = (b & 0xF0F0F0F0) >> 4 | (b & 0x0F0F0F0F) << 4;
	b = (b & 0xCCCCCCCC) >> 2 | (b & 0x33333333) << 2;
	b = (b & 0xAAAAAAAA) >> 1 | (b & 0x55555555) << 1;
	return b;
}


std::array<uint32_t, 256> generate_crc32_table(uint32_t trunc_polynomial)
{
	std::array<uint32_t, 256> table;
	const uint32_t mask = (1 << (32 - 1));

	for (unsigned i = 0; i < table.size(); i++) {
		uint32_t remainder = 0;

		uint32_t dividend_bits = reverse_bits((uint8_t)i);

		 for (int bit = 8; bit; --bit) {
			 remainder ^= (dividend_bits & 1u) ? mask : 0u;

			const bool quotient = (remainder & mask) != 0;

			remainder <<= 1;
			remainder ^= quotient ? trunc_polynomial : 0u;
			
			dividend_bits >>= 1;
		 }

		table[reverse_bits((uint8_t)i)] = reverse_bits(remainder);
	}
	return table;
}

}}}