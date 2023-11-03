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
	fvec<int, 2> _origin = fvec<int, 2>(0);
	fvec<int, 2> _destination = fvec<int, 2>(0);
	fvec<int, 2> _position = fvec<int, 2>(0);
	bool _done = false;

	int dx = 0;
	int sx = 0;
	int dy = 0;
	int sy = 0;
	int error = 0;

public:

	bresenham_traverser(fvec<int, 2> origin, fvec<int, 2> destination) : _origin(origin), _destination(destination) {
		
		_position = _origin;

		if(_origin == _destination)
			_done = true;

		dx = std::abs(_destination.x() - _origin.x());
		sx = _origin.x() < _destination.x() ? 1 : -1;
		dy = -std::abs(_destination.y() - _origin.y());
		sy = _origin.y() < _destination.y() ? 1 : -1;
		error = dx + dy;
	}

	bool done() const {

		return _done;
	}

	fvec<int, 2> position() const {

		return _position;
	}

	fvec<int, 2> origin() const {

		return _origin;
	}

	fvec<int, 2> destination() const {

		return _destination;
	}

	void step() {
		
		if(_position == _destination)
			_done = true;

		int e2 = 2 * error;
		
		if(e2 >= dy) {
			error += dy;
			_position.x() += sx;
		}

		if(e2 <= dx) {
			error += dx;
			_position.y() += sy;
		}
	}

	void operator++() {

		step();
	}
};

} // namespace math
} // namespace cgv
