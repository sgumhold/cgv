#pragma once

#include <cgv/render/render_types.h>

namespace cgv {
namespace g2d {
/** A wrapper class for a templated 2d axis aligned bounding box.
	Abstracts the box min and max points to provide position and size member
	functionality.
*/
template<typename coord_type>
struct rect : public cgv::render::render_types {
	typedef cgv::math::fvec<coord_type, 2> point_type;
	typedef cgv::media::axis_aligned_box<coord_type, 2> box_type;

	box_type box;

	rect() {
		box = box_type(point_type(0), point_type(0));
	}

	rect(const point_type& pos, const point_type& size) : rect() {
		set_pos(pos);
		set_size(size);
	}

	// minimum point (bottom left)
	point_type& a() { return box.ref_min_pnt(); }
	// maximum point (top right)
	point_type& b() { return box.ref_max_pnt(); }

	// minimum x position (left)
	coord_type x() const { return box.get_min_pnt().x(); }
	// minimum y position (bottom)
	coord_type y() const { return box.get_min_pnt().y(); }

	// maximum x position (right)
	coord_type x1() const { return box.get_max_pnt().x(); }
	// maximum y position (top)
	coord_type y1() const { return box.get_max_pnt().y(); }

	// width
	coord_type w() const { return size().x(); }
	// height
	coord_type h() const { return size().y(); }

	// pivot position (equivalent to bottom left)
	point_type pos() const {
		return box.get_min_pnt();
	}

	// size (width and height)
	point_type size() const {
		return box.get_extent();
	}

	// center position
	point_type center() const {
		return pos() + 0.5f * size();
	}

	// set x position (pivot horizontal)
	void set_x(coord_type x) {
		coord_type w = b().x() - a().x();
		a().x() = x;
		b().x() = a().x() + w;
	}

	// set y position (pivot vertical)
	void set_y(coord_type y) {
		coord_type h = b().y() - a().y();
		a().y() = y;
		b().y() = a().y() + h;
	}

	// set width
	void set_w(coord_type w) {
		b().x() = a().x() + w;
	}

	// set height
	void set_h(coord_type h) {
		b().y() = a().y() + h;
	}

	// set pivot position
	void set_pos(coord_type x, coord_type y) {
		set_pos(point_type(x, y));
	}

	// set pivot position
	void set_pos(point_type p) {
		point_type s = size();
		box.ref_min_pnt() = p;
		set_size(s);
	}

	// set size (width and height)
	void set_size(coord_type x, coord_type y) {
		set_size(point_type(x, y));
	}

	// set size (width and height)
	void set_size(point_type s) {
		box.ref_max_pnt() = box.ref_min_pnt() + s;
	}

	// translate (move) whole rectangle by offset
	void translate(coord_type dx, coord_type dy) {
		translate(point_type(dx, dy));
	}

	// translate (move) whole rectangle by offset
	void translate(point_type o) {
		box.ref_min_pnt() += o;
		box.ref_max_pnt() += o;
	}

	// resize by given size difference (delta); pivot unchanged
	void resize(coord_type dx, coord_type dy) {
		resize(point_type(dx, dy));
	}

	// resize by given size difference (delta); pivot unchanged
	void resize(point_type d) {
		box.ref_max_pnt() += d;
	}

	// returns true if the query point is inside the rectangle, false otherwise
	bool is_inside(point_type p) const {
		const point_type& a = pos();
		const point_type& b = a + size();
		return
			p.x() >= a.x() && p.x() <= b.x() &&
			p.y() >= a.y() && p.y() <= b.y();
	}
};

typedef rect<unsigned> urect;
typedef rect<int> irect;
typedef rect<float> frect;
typedef rect<double> drect;

}
}
