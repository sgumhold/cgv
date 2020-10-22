#pragma once

#include <array>

#include <cgv/math/fvec.h>
#include <cgv/render/render_types.h>

namespace cgv {
namespace data {
	// no class for now, just typedef
	// do not inherit from stl containers (no virtual destructors)
	using rectangle = std::array<cgv::render::render_types::vec3, 4>;
	static const rectangle null_rectangle = {
	    cgv::render::render_types::vec3{0.0f, 0.0f, 0.0f}, 
		cgv::render::render_types::vec3{0.0f, 0.0f, 0.0f}, 
		cgv::render::render_types::vec3{0.0f, 0.0f, 0.0f},
	    cgv::render::render_types::vec3{0.0f, 0.0f, 0.0f}};
} // namespace data
} // namespace cgv