#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/quaternion.h>

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
#include <Jolt/Jolt.h>

namespace cgv {
namespace physics {
namespace convert {

/// Convert cgv::vec3 to JPH::Vec3
static JPH::Vec3 to_Jolt_Vec3(const cgv::vec3& v) {
	return JPH::Vec3(v.x(), v.y(), v.z());
}

/// Convert JPH::Vec3 to cgv::vec3
static cgv::vec3 to_vec3(const JPH::Vec3& v) {
	return cgv::vec3(v.GetX(), v.GetY(), v.GetZ());
}

/// Convert cgv::quat to JPH::Quat
static JPH::Quat to_Jolt_Quat(const cgv::quat& q) {
	return JPH::Quat(q.x(), q.y(), q.z(), q.w());
}

/// Convert JPH::Quat to cgv::quat
static cgv::quat to_quat(const JPH::Quat& q) {
	return cgv::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
}

/// Convert std::vector<cgv::vec3> to JPH::Array<JPH::Vec3>
static JPH::Array<JPH::Vec3> to_Jolt_Vec3_Array(const std::vector<cgv::vec3>& v) {
	// JPH::Array is basically just std::vector with a custom allocator, so conversion is straightforward.
	JPH::Array<JPH::Vec3> a;
	a.reserve(v.size());
	std::transform(v.begin(), v.end(), std::back_inserter(a), to_Jolt_Vec3);
	return a;
}

} // namespace convert
} // namespace physics
} // namespace cgv
