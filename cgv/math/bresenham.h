#pragma once

#include <vector>

#include "fvec.h"

/// This header provides implementations for Bresenham's line algorithm.
namespace cgv {
namespace math {

/// @brief Compute a list of points along a rasterized line according to Bresenham's line drawing algorithm.
/// @param start The line start point.
/// @param end The line end point.
/// @return The list of rasterized points.
static std::vector<fvec<int, 2>> bresenham(fvec<int, 2> origin, fvec<int, 2> destination) {
	
	bool flip = std::abs(destination.y() - origin.y()) >= std::abs(destination.x() - origin.x());

	if(flip) {
		std::swap(origin.x(), origin.y());
		std::swap(destination.x(), destination.y());
	}

	if(origin.x() > destination.x())
		std::swap(origin, destination);

	fvec<int, 2> d = destination - origin;

	int yi = 1;
	if(d.y() < 0) {
		yi = -1;
		d.y() = -d.y();
	}
	int D = (2 * d.y()) - d.x();
	int y = origin.y();

	std::vector<fvec<int, 2>> points;

	for(int x = origin.x(); x <= destination.x(); ++x) {
		flip ? points.emplace_back(y, x) : points.emplace_back(x, y);
		if(D > 0) {
			y = y + yi;
			D = D + (2 * (d.y() - d.x()));
		} else {
			D = D + 2 * d.y();
		}
	}

	return points;
};

class bresenham_traverser {
private:
	fvec<int, 2> origin_ = fvec<int, 2>(0);
	fvec<int, 2> destination_ = fvec<int, 2>(0);
	fvec<int, 2> position_ = fvec<int, 2>(0);
	bool done_ = false;

	int dx = 0;
	int sx = 0;
	int dy = 0;
	int sy = 0;
	int error = 0;

public:
	bresenham_traverser(fvec<int, 2> origin, fvec<int, 2> destination) : origin_(origin), destination_(destination) {
		
		position_ = origin_;

		if(origin_ == destination_)
			done_ = true;

		dx = std::abs(destination_.x() - origin_.x());
		sx = origin_.x() < destination_.x() ? 1 : -1;
		dy = -std::abs(destination_.y() - origin_.y());
		sy = origin_.y() < destination_.y() ? 1 : -1;
		error = dx + dy;
	}

	bool done() const {

		return done_;
	}

	fvec<int, 2> position() const {

		return position_;
	}

	fvec<int, 2> origin() const {

		return origin_;
	}

	fvec<int, 2> destination() const {

		return destination_;
	}

	void step() {
		
		int e2 = 2 * error;
		
		if(e2 >= dy) {
			error += dy;
			position_.x() += sx;
		}

		if(e2 <= dx) {
			error += dx;
			position_.y() += sy;
		}

		if(position_ == destination_)
			done_ = true;
	}

	void operator++() {

		step();
	}
};

} // namespace math
} // namespace cgv
