#pragma once

#include "trect.h"

namespace cgv {
namespace g2d {

enum class OriginSetting {
	kUpperLeft,
	kLowerLeft
};

/// @brief Transforms the position given in the source origin setting according to the destination origin setting.
/// 
/// @tparam T The coordinate data type.
/// @param pos The input position.
/// @param reference_size The 
/// @param src_origin_setting 
/// @param dst_origin_setting 
/// @return 
template<typename T>
inline math::fvec<T, 2u> apply_origin_setting(const math::fvec<T, 2u>& pos, const math::fvec<T, 2u>& reference_size, OriginSetting src_origin_setting, OriginSetting dst_origin_setting) {
	if(src_origin_setting != dst_origin_setting)
		return math::fvec<T, 2u>(pos.x(), reference_size.y() - pos.y() - T(1));
	return pos;
}

}
}
