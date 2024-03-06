#pragma once

#include <cgv/render/context.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/box_render_data.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/cone_render_data.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_gl/gl/mesh_render_info.h>

#include "simulation_world.h"

#include "lib_begin.h"

namespace cgv {
namespace physics {

class CGV_API renderer {
private:
	const simulation_world* physics_world = nullptr;

	cgv::render::box_renderer box_renderer;
	cgv::render::box_render_data<> boxes;

	cgv::render::cone_renderer capsule_renderer;
	cgv::render::cone_renderer cylinder_renderer;
	cgv::render::cone_render_data<> capsules;
	cgv::render::cone_render_data<> cylinders;

	cgv::render::sphere_renderer sphere_renderer;
	cgv::render::sphere_render_data<> spheres;

	std::vector<std::pair<cgv::render::mesh_render_info*, cgv::mat4>> mesh_render_data;

public:
	renderer();
	~renderer() {}

	bool init(cgv::render::context& ctx, const simulation_world& physics_world);
	void destruct(cgv::render::context& ctx);

	void update();

	void draw(cgv::render::context& ctx);
};

} // namespace physics
} // namespace cgv

#include <cgv/config/lib_end.h>
