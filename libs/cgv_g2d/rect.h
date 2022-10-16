#pragma once

#include <cgv/render/render_types.h>

namespace cgv {
namespace g2d {
	/** A wrapper class for a 2d axis aligned bounding box with integer coordinates.
		Abstracts the box min and max points to provide position and size member
		functionality.
	*/
struct rect : public cgv::render::render_types {
	typedef cgv::media::axis_aligned_box<int, 2> ibox2;

	ibox2 box;

	rect() {
		box = ibox2(ivec2(0), ivec2(0));
	}

	// minimum point (bottom left)
	ivec2& a() { return box.ref_min_pnt(); }
	// maximum point (top right)
	ivec2& b() { return box.ref_max_pnt(); }

	// minimum x position (left)
	int x() const { return box.get_min_pnt().x(); }
	// minimum y position (bottom)
	int y() const { return box.get_min_pnt().y(); }

	// maximum x position (right)
	int x1() const { return box.get_max_pnt().x(); }
	// maximum y position (top)
	int y1() const { return box.get_max_pnt().y(); }

	// width
	int w() const { return size().x(); }
	// height
	int h() const { return size().y(); }

	// pivot position(equivalent to bottom left)
	ivec2 pos() const {
		return box.get_min_pnt();
	}

	// size (width and height)
	ivec2 size() const {
		return box.get_extent();
	}

	// set x position (pivot horizontal)
	void set_x(int x) {
		int w = b().x() - a().x();
		a().x() = x;
		b().x() = a().x() + w;
	}

	// set y position (pivot vertical)
	void set_y(int y) {
		int h = b().y() - a().y();
		a().y() = y;
		b().y() = a().y() + h;
	}

	// set width
	void set_w(int w) {
		b().x() = a().x() + w;
	}

	// set height
	void set_h(int h) {
		b().y() = a().y() + h;
	}

	// set pivot position
	void set_pos(int x, int y) {
		set_pos(ivec2(x, y));
	}

	// set pivot position
	void set_pos(ivec2 p) {
		ivec2 s = size();
		box.ref_min_pnt() = p;
		set_size(s);
	}

	// set size (width and height)
	void set_size(int x, int y) {
		set_size(ivec2(x, y));
	}

	// set size (width and height)
	void set_size(ivec2 s) {
		box.ref_max_pnt() = box.ref_min_pnt() + s;
	}

	// translate (move) whole rectangle by offset
	void translate(int dx, int dy) {
		translate(ivec2(dx, dy));
	}

	// translate (move) whole rectangle by offset
	void translate(ivec2 o) {
		box.ref_min_pnt() += o;
		box.ref_max_pnt() += o;
	}

	// resize by given size difference (delta); pivot unchanged
	void resize(int dx, int dy) {
		resize(ivec2(dx, dy));
	}

	// resize by given size difference (delta); pivot unchanged
	void resize(ivec2 d) {
		box.ref_max_pnt() += d;
	}

	// returns true if the query point is inside the rectangle, false otherwise
	bool is_inside(ivec2 p) {
		const ivec2& a = pos();
		const ivec2& b = a + size();
		return
			p.x() >= a.x() && p.x() <= b.x() &&
			p.y() >= a.y() && p.y() <= b.y();
	}
};

}
}
