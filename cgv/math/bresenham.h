#pragma once

#include <vector>

#include "fvec.h"

/// This header provides implementations for Bresenham's line algorithm.
namespace cgv {
namespace math {

/// @brief Computes a list of points along a rasterized line according to Bresenham_s line drawing algorithm.
/// @param start The line start point.
/// @param end The line end point.
/// @return The list of rasterized points.
static std::vector<fvec<int, 2>> bresenham(fvec<int, 2> start, fvec<int, 2> end) {
	
	bool flip = std::abs(end.y() - start.y()) >= std::abs(end.x() - start.x());

	if(flip) {
		std::swap(start.x(), start.y());
		std::swap(end.x(), end.y());
	}

	if(start.x() > end.x())
		std::swap(start, end);

	fvec<int, 2> d = end - start;

	int yi = 1;
	if(d.y() < 0) {
		yi = -1;
		d.y() = -d.y();
	}
	int D = (2 * d.y()) - d.x();
	int y = start.y();

	std::vector<fvec<int, 2>> points;

	for(int x = start.x(); x <= end.x(); ++x) {
		flip ? points.emplace_back(x, y) : points.emplace_back(y, x);
		if(D > 0) {
			y = y + yi;
			D = D + (2 * (d.y() - d.x()));
		} else {
			D = D + 2 * d.y();
		}
	}

	return points;
};

} // namespace math
} // namespace cgv
