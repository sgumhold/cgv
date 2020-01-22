#include "endian.h"

namespace cgv {
	namespace utils {

/// return endianess of current machine
Endian get_endian()
{
	uint64_t i = 0x0200000001000000L;
	return Endian(reinterpret_cast<uint8_t&>(i));
}

/// runtime mapping of source and target arguments for endianess conversion
void map_convert_endian(void* values, size_t cnt, Endian src, Endian tar, uint32_t type_size)
{
	if (src == tar)
		return;
	if (type_size == 1)
		return;
	switch (src) {
	case E_LITTLE:
		switch (tar) {
		case E_BIG_32: 
			switch (type_size) {
			case 2: convert_endian<E_LITTLE, E_BIG_32, 2>(values, cnt); break;
			case 4: convert_endian<E_LITTLE, E_BIG_32, 4>(values, cnt); break;
			case 8: convert_endian<E_LITTLE, E_BIG_32, 8>(values, cnt); break;
			}
			break;
		case E_BIG_64: 
			switch (type_size) {
			case 2: convert_endian<E_LITTLE, E_BIG_64, 2>(values, cnt); break;
			case 4: convert_endian<E_LITTLE, E_BIG_64, 4>(values, cnt); break;
			case 8: convert_endian<E_LITTLE, E_BIG_64, 8>(values, cnt); break;
			}
			break;
		}
		break;
	case E_BIG_32:
		switch (tar) {
		case E_LITTLE: 
			switch (type_size) {
			case 2: convert_endian<E_BIG_32, E_LITTLE, 2>(values, cnt); break;
			case 4: convert_endian<E_BIG_32, E_LITTLE, 4>(values, cnt); break;
			case 8: convert_endian<E_BIG_32, E_LITTLE, 8>(values, cnt); break;
			}
			break;
		case E_BIG_64: 
			switch (type_size) {
			case 2: convert_endian<E_BIG_32, E_BIG_64, 2>(values, cnt); break;
			case 4: convert_endian<E_BIG_32, E_BIG_64, 4>(values, cnt); break;
			case 8: convert_endian<E_BIG_32, E_BIG_64, 8>(values, cnt); break;
			}
			break;
		}
		break;
	case E_BIG_64:
		switch (tar) {
		case E_LITTLE: 
			switch (type_size) {
			case 2: convert_endian<E_BIG_64, E_LITTLE, 2>(values, cnt); break;
			case 4: convert_endian<E_BIG_64, E_LITTLE, 4>(values, cnt); break;
			case 8: convert_endian<E_BIG_64, E_LITTLE, 8>(values, cnt); break;
			}
			break;
		case E_BIG_32: 
			switch (type_size) {
			case 2: convert_endian<E_BIG_64, E_BIG_32, 2>(values, cnt); break;
			case 4: convert_endian<E_BIG_64, E_BIG_32, 4>(values, cnt); break;
			case 8: convert_endian<E_BIG_64, E_BIG_32, 8>(values, cnt); break;
			}
			break;
		}
		break;
	}
}

/// runtime mapping of source endianess to machine endianess conversion
void map_convert_endian_from(void* values, size_t cnt, Endian src, uint32_t type_size)
{
	map_convert_endian(values, cnt, src, get_endian(), type_size);
}

/// runtime mapping of machine endianess to target endianess conversion
void map_convert_endian_to(void* values, size_t cnt, Endian tar, uint32_t type_size)
{
	map_convert_endian(values, cnt, get_endian(), tar, type_size);
}

	}
}