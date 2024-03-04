#include "renderer.h"

#include <cgv/math/ftransform.h>

#include "convert_types.h"

namespace physics {

renderer::renderer() {
	capsules.style.rounded_caps = true;
	cylinders.style.rounded_caps = false;
}

bool renderer::init(cgv::render::context& ctx, const simulation_world& physics_world) {
	this->physics_world = &physics_world;

	bool success = this->physics_world != nullptr;

	success &= box_renderer.init(ctx);
	success &= boxes.init(ctx);

	success &= capsule_renderer.init(ctx);
	success &= cylinder_renderer.init(ctx);
	success &= capsules.init(ctx);
	success &= cylinders.init(ctx);

	success &= sphere_renderer.init(ctx);
	success &= spheres.init(ctx);

	return success;
}

void renderer::destruct(cgv::render::context& ctx) {
	physics_world = nullptr;

	boxes.destruct(ctx);
	capsules.destruct(ctx);
	cylinders.destruct(ctx);
	spheres.destruct(ctx);

	box_renderer.clear(ctx);
	capsule_renderer.clear(ctx);
	cylinder_renderer.clear(ctx);
	sphere_renderer.clear(ctx);
}

void renderer::update() {
	if(!physics_world)
		return;

	boxes.clear();
	capsules.clear();
	cylinders.clear();
	spheres.clear();
	mesh_infos.clear();

	const auto get_capsule_ends = [](std::shared_ptr<const capsule_representation> representation, const JPH::Body* physics_body) {
		cgv::vec3 position = to_vec3(physics_body->GetCenterOfMassPosition());
		cgv::quat rotation = to_quat(physics_body->GetRotation());
		
		cgv::vec3 axis_offset(0.0f, 0.5f * representation->height, 0.0f);
		rotation.rotate(axis_offset);

		return std::pair<cgv::vec3, cgv::vec3>(position - axis_offset, position + axis_offset);
	};

	for(const auto& body : physics_world->ref_rigid_bodies()) {
		const auto representation = body.get_shape_representation();

		const JPH::Body* physics_body = physics_world->get_body_by_id(body.get_body_id());

		switch(representation->type()) {
		case ShapeRepresentationType::kBox:
		{
			auto box = std::dynamic_pointer_cast<const box_representation>(representation);
			boxes.add(to_vec3(physics_body->GetCenterOfMassPosition()), box->extent, to_quat(physics_body->GetRotation()));
			boxes.add_color(box->color);
			break;
		}
		case ShapeRepresentationType::kCapsule:
		{
			auto capsule = std::dynamic_pointer_cast<const capsule_representation>(representation);
			auto ends = get_capsule_ends(capsule, physics_body);
			capsules.add(ends.first, ends.second, capsule->color, capsule->base_radius);
			break;
		}
		case ShapeRepresentationType::kTaperedCapsule:
		{
			auto capsule = std::dynamic_pointer_cast<const tapered_capsule_representation>(representation);
			auto ends = get_capsule_ends(capsule, physics_body);
			capsules.add(ends.first, ends.second, capsule->color);
			capsules.add(capsule->base_radius, capsule->top_radius);
			break;
		}
		case ShapeRepresentationType::kCylinder:
		{
			auto cylinder = std::dynamic_pointer_cast<const cylinder_representation>(representation);
			auto ends = get_capsule_ends(cylinder, physics_body);
			cylinders.add(ends.first, ends.second, cylinder->color, cylinder->base_radius);
			break;
		}
		case ShapeRepresentationType::kSphere:
		{
			auto sphere = std::dynamic_pointer_cast<const sphere_representation>(representation);
			spheres.add(to_vec3(physics_body->GetCenterOfMassPosition()), sphere->color, sphere->radius);
			break;
		}
		case ShapeRepresentationType::kMesh:
		{
			auto mesh = std::dynamic_pointer_cast<const mesh_representation>(representation);
			cgv::mat4 transformation = cgv::math::pose4(to_quat(physics_body->GetRotation()), to_vec3(physics_body->GetCenterOfMassPosition())) * cgv::math::scale4(mesh->scale);
			mesh_infos.push_back({ mesh->mesh_info, transformation });
			break;
		}
		default:
			break;
		}
	}
}

void renderer::draw(cgv::render::context& ctx) {
	boxes.render(ctx, box_renderer);
	capsules.render(ctx, capsule_renderer);
	cylinders.render(ctx, cylinder_renderer);
	spheres.render(ctx, sphere_renderer);

	for(const auto [mesh_info, transformation] : mesh_infos) {
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(transformation);
		mesh_info->draw_all(ctx);
		ctx.pop_modelview_matrix();
	}
}

} // namespace physics
