#pragma once

#include <cgv_g2d/shape2d_styles.h>

namespace cg {
namespace g2d {

struct styles {
	cgv::rgb background_color;
	cgv::rgb group_color;
	cgv::rgb control_color;
	cgv::rgb shadow_color;
	cgv::rgb text_color;
	cgv::rgb selection_color;
	cgv::rgb highlight_color;

	cgv::g2d::shape2d_style flat_box;
	cgv::g2d::shape2d_style rounded_box;

	cgv::g2d::text2d_style text;
};

}
}
