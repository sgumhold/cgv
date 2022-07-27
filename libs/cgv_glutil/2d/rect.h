#pragma once

#include <cgv/render/render_types.h>

#include "../lib_begin.h"

namespace cgv {
namespace glutil {
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

	ivec2& a() { return box.ref_min_pnt(); }
	ivec2& b() { return box.ref_max_pnt(); }

	int x() const { return box.get_min_pnt().x(); }
	int y() const { return box.get_min_pnt().y(); }

	int x1() const { return box.get_max_pnt().x(); }
	int y1() const { return box.get_max_pnt().y(); }

	int w() const { return size().x(); }
	int h() const { return size().y(); }

	ivec2 pos() const {
		return box.get_min_pnt();
	}

	ivec2 size() const {
		return box.get_extent();
	}

	void set_x(int x) {
		int w = b().x() - a().x();
		a().x() = x;
		b().x() = a().x() + w;
	}

	void set_pos(int x, int y) {
		set_pos(ivec2(x, y));
	}

	void set_pos(ivec2 p) {
		ivec2 s = size();
		box.ref_min_pnt() = p;
		set_size(s);
	}

	void set_size(int x, int y) {
		set_size(ivec2(x, y));
	}

	void set_size(ivec2 s) {
		box.ref_max_pnt() = box.ref_min_pnt() + s;
	}

	void translate(int dx, int dy) {
		translate(ivec2(dx, dy));
	}

	void translate(ivec2 o) {
		box.ref_min_pnt() += o;
		box.ref_max_pnt() += o;
	}

	void resize(int dx, int dy) {
		resize(ivec2(dx, dy));
	}

	void resize(ivec2 d) {
		box.ref_max_pnt() += d;
	}

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

#include <cgv/config/lib_end.h>
