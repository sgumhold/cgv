#pragma once

#include <cgv/render/render_types.h>
#include <cgv_g2d/shape2d_styles.h>

namespace cg {
namespace g2d {

struct styles {
	cgv::render::rgb background_color;
	cgv::render::rgb group_color;
	cgv::render::rgb control_color;
	cgv::render::rgb shadow_color;
	cgv::render::rgb text_color;
	cgv::render::rgb selection_color;
	cgv::render::rgb highlight_color;

	cgv::g2d::shape2d_style flat_box;
	cgv::g2d::shape2d_style rounded_box;

	cgv::g2d::text2d_style text;
};

}
}
