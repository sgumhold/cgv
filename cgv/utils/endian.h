#pragma once

#include <cgv/math/fvec.h>
#include <cgv/media/color.h>
#include <vector>

#include "lib_begin.h"

namespace cgv {
	namespace utils {

/// different endianess
enum Endian
{
	E_LITTLE = 0, // 0x8007060504030201L is layed out in memory as 1,2,3,4,5,6,7,8
	E_BIG_32 = 1, // 0x8007060504030201L is layed out in memory as 4,3,2,1,8,7,6,5
	E_BIG_64 = 2  // 0x8007060504030201L is layed out in memory as 8,7,6,5,4,3,2,1
};

/// return endianess of current machine
extern CGV_API Endian get_endian();

/// endianess conversion function templated over type, source and target endianess
template <Endian src, Endian tar, typename T>
inline void convert_endian(T& value)
{
	if (src == tar)
		return;
	if (src > E_LITTLE && tar > E_LITTLE) {
		if (sizeof(T) == 8) {
			uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(&value);
			std::swap(ui32_ptr[0], ui32_ptr[1]);
		}
	}
	else {
		if ((src == E_BIG_32 || tar == E_BIG_32) && (sizeof(T) == 8)) {
			uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(&value);
			convert_endian<src, tar>(ui32_ptr[0]);
			convert_endian<src, tar>(ui32_ptr[1]);
		}
		else {
			uint8_t* byte_ptr = reinterpret_cast<uint8_t*>(&value);
			for (size_t i = 0; 2 * i < sizeof(T); ++i)
				std::swap(byte_ptr[i], byte_ptr[sizeof(T) - i - 1]);
		}
	}
}

/// specialization for std::vector
template <Endian src, Endian tar, typename T>
inline void convert_endian(std::vector<T>& vector)
{
	if (src == tar)
		return;
	for (auto& value : vector)
		convert_endian<src, tar>(value);
}

/// endianess conversion function for arrays templated over type size, source and target endianess
template <Endian src, Endian tar, uint32_t type_size>
inline void convert_endian(void* values, size_t cnt)
{
	if (src == tar)
		return;
	if (type_size == 1)
		return;
	if (src > E_LITTLE && tar > E_LITTLE) {
		if (type_size == 8) {
			uint64_t* ui64_ptr = reinterpret_cast<uint64_t*>(&values);
			for (size_t i = 0; i < cnt; ++i) {
				uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(&ui64_ptr[i]);
				std::swap(ui32_ptr[0], ui32_ptr[1]);
			}
		}
	}
	else {
		if ((src == E_BIG_32 || tar == E_BIG_32) && (type_size == 8)) {
			uint64_t* ui64_ptr = reinterpret_cast<uint64_t*>(&values);
			for (size_t i = 0; i < cnt; ++i) {
				uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(&ui64_ptr[i]);
				convert_endian<src, tar>(ui32_ptr[0]);
				convert_endian<src, tar>(ui32_ptr[1]);
			}
		}
		else {
			for (size_t i = 0; i < cnt; ++i) {
				uint8_t* byte_ptr = reinterpret_cast<uint8_t*>(values) + i * type_size;
				for (size_t j = 0; 2 * j < type_size; ++j)
					std::swap(byte_ptr[j], byte_ptr[type_size - j - 1]);
			}
		}
	}
}

/// runtime mapping of source and target arguments for endianess conversion
template <typename T>
inline void map_convert_endian(T& value, Endian src, Endian tar)
{
	switch (src) {
	case E_LITTLE:
		switch (tar) {
		case E_LITTLE: break;
		case E_BIG_32: convert_endian<E_LITTLE, E_BIG_32>(value); break;
		case E_BIG_64: convert_endian<E_LITTLE, E_BIG_64>(value); break;
		}
		break;
	case E_BIG_32:
		switch (tar) {
		case E_LITTLE: convert_endian<E_BIG_32, E_LITTLE>(value); break;
		case E_BIG_32: break;
		case E_BIG_64: convert_endian<E_BIG_32, E_BIG_64>(value); break;
		}
		break;
	case E_BIG_64:
		switch (tar) {
		case E_LITTLE: convert_endian<E_BIG_64, E_LITTLE>(value); break;
		case E_BIG_32: convert_endian<E_BIG_64, E_BIG_32>(value); break;
		case E_BIG_64: break;
		}
		break;
	}
}

/// runtime mapping of source and target arguments for endianess conversion
extern CGV_API void map_convert_endian(void* values, size_t cnt, Endian src, Endian tar, uint32_t type_size);

/// convenience function to convert from specific endian <src> to endian of current machine
template <Endian src, typename T>
inline void convert_endian_from(T& value)
{
	switch (get_endian()) {
	case E_LITTLE : convert_endian<src, E_LITTLE>(value); break;
	case E_BIG_32 : convert_endian<src, E_BIG_32>(value); break;
	case E_BIG_64 : convert_endian<src, E_BIG_64>(value); break;
	}
}

/// specialization for vector
template <Endian src, typename T>
inline void convert_endian_from(std::vector<T>& vector)
{
	switch (get_endian()) {
	case E_LITTLE: convert_endian<src, E_LITTLE>(vector); break;
	case E_BIG_32: convert_endian<src, E_BIG_32>(vector); break;
	case E_BIG_64: convert_endian<src, E_BIG_64>(vector); break;
	}

}

/// endianess conversion function for arrays templated over type size and source endianess
template <Endian src, uint32_t type_size>
inline void convert_endian_from(void* values, size_t cnt)
{
	switch (get_endian()) {
	case E_LITTLE: convert_endian<src, E_LITTLE, type_size>(values, cnt); break;
	case E_BIG_32: convert_endian<src, E_BIG_32, type_size>(values, cnt); break;
	case E_BIG_64: convert_endian<src, E_BIG_64, type_size>(values, cnt); break;
	}
}

/// runtime mapping of source endianess to machine endianess conversion
template <typename T>
inline void map_convert_endian_from(T& value, Endian src)
{
	map_convert_endian(value, src, get_endian());
}

/// runtime mapping of source endianess to machine endianess conversion
extern CGV_API void map_convert_endian_from(void* values, size_t cnt, Endian src, uint32_t type_size);

/// convenience function to convert from endian of current machine to specific endian <tar>
template <Endian tar, typename T>
inline void convert_endian_to(T& value)
{
	switch (get_endian()) {
	case E_LITTLE: convert_endian<E_LITTLE, tar>(value); break;
	case E_BIG_32: convert_endian<E_BIG_32, tar>(value); break;
	case E_BIG_64: convert_endian<E_BIG_64, tar>(value); break;
	}
}

/// specialization for vector
template <Endian tar, typename T>
inline void convert_endian_to(std::vector<T>& vector)
{
	switch (get_endian()) {
	case E_LITTLE: convert_endian<E_LITTLE, tar>(vector); break;
	case E_BIG_32: convert_endian<E_BIG_32, tar>(vector); break;
	case E_BIG_64: convert_endian<E_BIG_64, tar>(vector); break;
	}
}

/// endianess conversion function for arrays templated over type size and source endianess
template <Endian tar, uint32_t type_size>
inline void convert_endian_to(void* values, size_t cnt)
{
	switch (get_endian()) {
	case E_LITTLE: convert_endian<E_LITTLE, tar, type_size>(values, cnt); break;
	case E_BIG_32: convert_endian<E_BIG_32, tar, type_size>(values, cnt); break;
	case E_BIG_64: convert_endian<E_BIG_64, tar, type_size>(values, cnt); break;
	}
}

/// runtime mapping of machine endianess to target endianess conversion
template <typename T>
inline void map_convert_endian_to(T& value, Endian tar)
{
	map_convert_endian(value, get_endian(), tar);
}

/// runtime mapping of machine endianess to target endianess conversion
extern CGV_API void map_convert_endian_to(void* values, size_t cnt, Endian tar, uint32_t type_size);

	}
}

#include <cgv/config/lib_end.h>