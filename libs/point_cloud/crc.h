#pragma once
#include <array>
#include <cstdint>

namespace cgv {
namespace pointcloud {
namespace file_parser {

std::array<uint32_t, 256> generate_crc32_table(uint32_t trunc_polynomial);

template <uint32_t trunc_polynom, uint32_t initial_remainder = 0xFFFFFFFFu, uint32_t final_xor = 0xFFFFFFFFu>
uint32_t crc32(const void* data, size_t data_length)
{
	static const std::array<uint32_t, 256> crc32_table = generate_crc32_table(trunc_polynom);
	// static constexpr uint32_t final_xor = 0xFFFFFFFFu;

	uint32_t crc32 = initial_remainder;

	for (size_t i = 0; i < data_length; i++) {
		const uint32_t ix = (crc32 ^ reinterpret_cast<const uint8_t*>(data)[i]) & 0xff;
		crc32 = (crc32 >> 8) ^ crc32_table[ix];
	}

	crc32 ^= final_xor;
	return crc32;
}



}}}