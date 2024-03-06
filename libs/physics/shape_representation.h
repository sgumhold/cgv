#pragma once

#include <cgv/math/fvec.h>
#include <cgv/media/color.h>
#include <cgv_gl/gl/mesh_render_info.h>

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
#include <Jolt/Jolt.h>

namespace cgv {
namespace physics {

enum class ShapeRepresentationType {
	kSphere,
	kBox,
	kCapsule,
	kTaperedCapsule,
	kCylinder,
	kMesh
};

class abstract_shape_representation {
public:
	cgv::rgb color = { 0.5f };

	virtual ShapeRepresentationType type() const = 0;
};

class box_representation : public abstract_shape_representation {
public:
	cgv::vec3 extent = { 1.0f };

	ShapeRepresentationType type() const override {
		return ShapeRepresentationType::kBox;
	}
};

class capsule_representation : public abstract_shape_representation {
public:
	float height = 1.0f;
	float base_radius = 1.0f;

	ShapeRepresentationType type() const override {
		return ShapeRepresentationType::kCapsule;
	}
};

class tapered_capsule_representation : public capsule_representation {
public:
	float top_radius = 1.0f;

	ShapeRepresentationType type() const override {
		return ShapeRepresentationType::kTaperedCapsule;
	}
};

class cylinder_representation : public capsule_representation {
public:
	ShapeRepresentationType type() const override {
		return ShapeRepresentationType::kCylinder;
	}
};

class sphere_representation : public abstract_shape_representation {
public:
	float radius = 1.0f;

	ShapeRepresentationType type() const override {
		return ShapeRepresentationType::kSphere;
	}
};

class mesh_representation : public abstract_shape_representation {
public:
	cgv::render::mesh_render_info* mesh_info = nullptr;
	cgv::vec3 scale = { 1.0f };

	ShapeRepresentationType type() const override {
		return ShapeRepresentationType::kMesh;
	}
};

} // namespace physics
} // namespace cgv
