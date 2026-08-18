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

// Specifies the origin of the coordinate system relative to the window.
enum class CoordinateOrigin {
	kUpperLeft, // Origin used in FLTK and typical for drawing GUIs.
	kLowerLeft // Default origin of OpenGL
};

/// @brief Transforms the position given in the source origin setting according to the destination origin setting.
/// 
/// @tparam T The coordinate data type.
/// @param pos The input position.
/// @param reference_size The size of the reference container. Typically the window.
/// @param src_origin The source origin setting.
/// @param dst_origin The destination origin setting.
/// @return The position realtive to the destination origin setting.
template<typename T>
inline math::fvec<T, 2u> change_origin(const math::fvec<T, 2u>& pos, const math::fvec<T, 2u>& reference_size, CoordinateOrigin src_origin, CoordinateOrigin dst_origin) {
	if(src_origin != dst_origin)
		return math::fvec<T, 2u>(pos.x(), reference_size.y() - pos.y() - T(1));
	return pos;
}

}
}
