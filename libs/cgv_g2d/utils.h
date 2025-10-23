#pragma once

namespace cgv {
namespace g2d {

enum class Alignment {
	kCenter = 0,					// align to center
	kLeft = 1,						// align to center of left edge
	kRight = 2,						// align to center of right edge
	kTop = 4,						// align to center of top edge
	kBottom = 8,					// align to center of bottom edge
	kTopLeft = kLeft + kTop,		// align to top left corner
	kTopRight = kRight + kTop,		// align to top right corner
	kBottomLeft = kLeft + kBottom,	// align to bottom left corner
	kBottomRight = kRight + kBottom	// align to bottom right corner
};

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
