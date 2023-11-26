#pragma once

#include <cgv/render/render_types.h>
#include <cgv_g2d/shape2d_styles.h>

namespace cg {
namespace g2d {

struct styles {
	cgv::g2d::shape2d_style control_box;
	cgv::g2d::shape2d_style colored_box;
	cgv::g2d::text2d_style text;

	cgv::render::rgb control_color;
	cgv::render::rgb background_color;
	cgv::render::rgb shadow_color;
};

}
}
