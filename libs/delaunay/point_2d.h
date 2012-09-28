#pragma once

/// type of a point defining the location of a vertex
template <typename coord_type>
class point_2d
{
protected:
	coord_type coords[2];
public:
	point_2d(coord_type x = 0, coord_type y = 0) { coords[0] = x; coords[1] = y; }
	const coord_type& x() const { return coords[0]; }
	const coord_type& y() const { return coords[1]; }
	coord_type& x() { return coords[0]; }
	coord_type& y() { return coords[1]; }
	coord_type& operator[] (int i) { return coords[i]; }
	const coord_type& operator[] (int i) const { return coords[i]; }
};
