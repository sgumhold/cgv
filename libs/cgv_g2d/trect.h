#pragma once

#include <cgv/math/fvec.h>

namespace cgv {
namespace g2d {

/// @brief A templated rectangle class that stores 2D position and size members and provides convenient accessors and member methods.
/// @tparam coord_type the coordinate type
template<typename coord_type>
struct trect {
	typedef cgv::math::fvec<coord_type, 2> point_type;
	
	point_type position;
	point_type size;

	trect() {
		position = point_type(0);
		size = point_type(0);
	}

	trect(const point_type& position, const point_type& size) : position(position), size(size) {}

	bool operator==(const trect& other) const {
		return position == other.position && size == other.size;
	}

	// minimum point (bottom left)
	point_type a() const { return position; }
	// maximum point (top right)
	point_type b() const { return position + size; }

	// minimum x position (left)
	coord_type x() const { return position.x(); }
	// reference to minimum x position (left)
	coord_type& x() { return position.x(); }
	// minimum y position (bottom)
	coord_type y() const { return position.y(); }
	// reference to minimum y position (bottom)
	coord_type& y() { return position.y(); }

	// maximum x position (right)
	coord_type x1() const { return position.x() + size.x(); }
	// maximum y position (top)
	coord_type y1() const { return position.y() + size.y(); }

	// width
	coord_type w() const { return size.x(); }
	// reference to width
	coord_type& w() { return size.x(); }
	// height
	coord_type h() const { return size.y(); }
	// reference to height
	coord_type& h() { return size.y(); }

	// center position
	template<typename coord_type_ = coord_type, typename std::enable_if_t<std::is_integral<coord_type_>::value, bool> = true>
	point_type center() const {
		return position + size / coord_type_(2);
	}

	// center position
	template<typename coord_type_ = coord_type, typename std::enable_if_t<std::is_floating_point<coord_type_>::value, bool> = true>
	point_type center() const {
		return position + coord_type_(0.5) * size;
	}

	// translate (move) whole rectangle by offset
	void translate(coord_type dx, coord_type dy) {
		translate(point_type(dx, dy));
	}

	// translate (move) whole rectangle by offset
	void translate(point_type o) {
		position += o;
	}

	// scale from center by given size difference (delta);
	void scale(coord_type dx, coord_type dy) {
		scale(point_type(dx, dy));
	}

	// scale from center by given size difference (delta);
	void scale(point_type d) {
		position -= d;
		size += coord_type(2) * d;
	}

	// return true if w() or h() are less than or equal to zero
	bool empty() const {
		return size.x() <= coord_type(0) || size.y() <= coord_type(0);
	}

	// return true if the query point is inside the rectangle, false otherwise
	bool contains(point_type p) const {
		return
			p.x() >= x() && p.x() <= x1() &&
			p.y() >= y() && p.y() <= y1();
	}

	// allow cast to rectangle of other coordinate type
	template<typename coord_type2>
	operator trect<coord_type2>() const {
		return trect<coord_type2>(
			static_cast<typename trect<coord_type2>::point_type>(position),
			static_cast<typename trect<coord_type2>::point_type>(size)
		);
	}
};

// declare some concrete rectangle types

/// declare rectangle using unsigned integer coordinates
typedef trect<unsigned> urect;
/// declare rectangle using signed integer coordinates
typedef trect<int> irect;
/// declare rectangle using floating point coordinates
typedef trect<float> rect;
/// declare rectangle using double precision floating point coordinates
typedef trect<double> drect;

}
}
