#include <cgv/base/base.h>
#include "stereo_view.h"

namespace cgv {
	namespace render {

///
stereo_view::stereo_view()
{
	set_default_values();
}

/// set distance between eyes
void stereo_view::set_eye_distance(double e)
{
	eye_distance = e;
}

///
void stereo_view::set_default_values()
{
	set_default_view();
	set_y_view_angle(45);
	set_z_near(0.01);
	set_z_far(10000.0);
	set_eye_distance(0.03);
	set_parallax_zero_scale(0.5);
}

/// query distance between eyes
double stereo_view::get_eye_distance() const 
{ 
	return eye_distance; 
}

/// query scale of parallax zero depth with respect to eye focus distance
double stereo_view::get_parallax_zero_scale() const 
{
	return parallax_zero_scale; 
}
/// query parallax zero depth
double stereo_view::get_parallax_zero_depth() const
{
	return (1.0 / (1.0 - parallax_zero_scale) - 1.0) * dot(get_focus() - get_eye(), view_dir);
}

/// set parallax zero scale
void stereo_view::set_parallax_zero_scale(double pzs) 
{
	parallax_zero_scale = pzs; 
}

	}
}