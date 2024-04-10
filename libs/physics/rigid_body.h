#pragma once

#include "shape_representation.h"

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

namespace cgv {
namespace physics {

// TODO: comment: consisting of the BodyID supplied and used by Jolt physics and the visual shape used to represent the object.
class rigid_body {
private:
	JPH::BodyID body_id;
	std::shared_ptr<const abstract_shape_representation> shape_representation;

public:
	rigid_body(JPH::BodyID body_id, const std::shared_ptr<const abstract_shape_representation> shape) : body_id(body_id), shape_representation(shape) {}

	// TODO: rename to jolt_id or just id
	JPH::BodyID get_body_id() const {
		return body_id;
	}

	std::shared_ptr<const abstract_shape_representation> get_shape_representation() const {
		return shape_representation;
	}
};

} // namespace physics
} // namespace cgv
