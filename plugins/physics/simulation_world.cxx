#include "simulation_world.h"

using namespace JPH;

/*
* TODOs:
*
* Consider:
* // Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
* // You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
* // Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
* physics_system->OptimizeBroadPhase();
*
* Shall we detect when all bodies are sleeping and skip the simulation update until they awaken again?
* // Now we're ready to simulate the body, keep simulating until it goes to sleep
* body_interface.IsActive(body_id);
*
*/

namespace physics {

simulation_world::~simulation_world() {
	for(const auto& body : rigid_bodies) {
		// Remove the body from the physics system. Note that the body itself keeps all of its state and can be re-added at any time.
		body_interface->RemoveBody(body.get_body_id());
		// Destroy the body. After this the body ID is no longer valid.
		body_interface->DestroyBody(body.get_body_id());
	}

	rigid_bodies.clear();

	// Unregisters all types with the factory and cleans up the default material
	UnregisterTypes();

	delete Factory::sInstance;
	Factory::sInstance = nullptr;

	delete job_system;
	delete temp_allocator;

	delete physics_system;
}

bool simulation_world::init(physics_system_creation_settings creation_settings) {
	// Register allocation hook
	// Important: This must happen before trying to create any other JoltPhysics objects!
	RegisterDefaultAllocator();

	// Create a factory
	Factory::sInstance = new Factory();

	// Register all Jolt physics types
	RegisterTypes();

	// We need a temp allocator for temporary allocations during the physics update. We're
	// pre-allocating 10 MB to avoid having to do allocations during the physics update.
	// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
	// malloc / free.
	if(creation_settings.use_fixed_memory_preallocation)
		temp_allocator = new TempAllocatorImpl(creation_settings.preallocated_memory_size);
	else
		temp_allocator = new TempAllocatorMalloc();

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	job_system = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	// Now we can create and initialize the actual physics system using the given configuration.
	physics_system = new PhysicsSystem();
	physics_system->Init(
		creation_settings.max_number_bodies,
		creation_settings.number_body_mutexes,
		creation_settings.max_number_body_pairs,
		creation_settings.max_number_contact_constraints,
		broad_phase_layer_interface,
		object_vs_broadphase_layer_filter,
		object_vs_object_layer_filter
	);

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	body_interface = &physics_system->GetBodyInterface();

	return true;
}

bool simulation_world::create_and_add_rigid_body(const BodyCreationSettings& collision_shape_settings, const std::shared_ptr<const abstract_shape_representation> shape_representation, bool activate) {
	BodyID id = body_interface->CreateAndAddBody(collision_shape_settings, activate ? EActivation::Activate : EActivation::DontActivate);

	// If we run out of available bodies the returned ID is invalid.
	if(id.IsInvalid())
		return false;

	rigid_bodies.push_back({ id, shape_representation });
	return true;
}

const Body* simulation_world::get_body_by_id(BodyID id) const {
	// GetBodyLockInterface returns the locking interface suitable for multi-threaded access.
	// If no multi-threading is used we can also use the non-locking interface through GetBodyLockInterfaceNoLock();
	BodyLockRead lock(physics_system->GetBodyLockInterface(), id);
	// Check if body id is still valid.
	if(lock.Succeeded())
		return &lock.GetBody();

	return nullptr;
}

void simulation_world::update(float delta_time) {
	// Step the world
	physics_system->Update(delta_time, collision_steps, temp_allocator, job_system);
}

void simulation_world::erase_rigid_bodies() {
	remove_and_delete_rigid_bodies(rigid_bodies.begin(), rigid_bodies.end());
}

void simulation_world::erase_rigid_bodies_by_layer(ObjectLayer layer) {
	erase_rigid_bodies_if(
		[&layer](const auto& body, const auto& representation) {
			return body->GetObjectLayer() == layer;
		});
}

void simulation_world::erase_rigid_bodies_by_motion_type(EMotionType motion_type) {
	erase_rigid_bodies_if(
		[&motion_type](const auto& body, const auto& representation) {
			return body->GetMotionType() == motion_type;
		});
}

void simulation_world::erase_rigid_bodies_by_activation_state(bool active) {
	erase_rigid_bodies_if(
		[&active](const auto& body, const auto& representation) {
			return body->IsActive() == active;
		});
}

void simulation_world::remove_and_delete_rigid_bodies(std::vector<rigid_body>::iterator first, std::vector<rigid_body>::iterator last) {
	std::vector<BodyID> body_ids;
	std::transform(first, last, std::back_inserter(body_ids), [](const rigid_body& rb) { return rb.get_body_id(); });

	if(!body_ids.empty()) {
		body_interface->RemoveBodies(body_ids.data(), static_cast<int>(body_ids.size()));
		body_interface->DestroyBodies(body_ids.data(), static_cast<int>(body_ids.size()));

		rigid_bodies.erase(first, last);
	}
}

} // namespace physics
