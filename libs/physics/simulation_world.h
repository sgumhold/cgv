#pragma once

#include <iostream>
#include <thread>

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation.
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include "rigid_body.h"

#include "lib_begin.h"

namespace cgv {
namespace physics {

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Layers {

static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::ObjectLayer NUM_LAYERS = 2;

};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
	bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
		switch(inObject1) {
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers {

static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr JPH::uint NUM_LAYERS(2);

};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];

public:
	BPLayerInterfaceImpl() {
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	JPH::uint GetNumBroadPhaseLayers() const override {
		return BroadPhaseLayers::NUM_LAYERS;
	}

	JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
	bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
		switch(inLayer1) {
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

class CGV_API simulation_world {
public:
	struct physics_system_creation_settings {
		bool use_fixed_memory_preallocation = true;
		unsigned preallocated_memory_size = 10 * 1024 * 1024;
		// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
		// Note: For smaller systems this can be reduced to, e.g., 1024, but the default value is good for most situations.
		unsigned max_number_bodies = 65536;
		// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
		unsigned number_body_mutexes = 0;
		// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
		// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
		// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
		// Note: For smaller systems this can be reduced to, e.g., 1024, but the default value is good for most situations.
		unsigned max_number_body_pairs = 65536;
		// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
		// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		// Note: For smaller systems this can be reduced to, e.g., 1024, but the default value is good for most situations.
		unsigned max_number_contact_constraints = 10240;
	};

	// This determines the number of collision steps to perform during every update of the physics system and can be adjusted any time.
	// If you take larger time steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
	int collision_steps = 1;

private:
	// An allocator used for temporary memory allocations during the physics update
	JPH::TempAllocator* temp_allocator = nullptr;
	// A Job system to execute physics jobs on multiple threads
	JPH::JobSystemThreadPool* job_system = nullptr;

	// Create mapping table from object layer to broadphase layer
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	BPLayerInterfaceImpl broad_phase_layer_interface;

	// Create class that filters object vs broadphase layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

	// Create class that filters object vs object layers
	// Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	// The actual physics system that manages all subsystems.
	//JPH::PhysicsSystem* physics_system = nullptr;
	JPH::PhysicsSystem* physics_system = nullptr;
	
	// A list of all registered rigid bodies
	std::vector<rigid_body> rigid_bodies;

	/// Remove all rigid bodies in the given range from the body interface and delete them.
	void remove_and_delete_rigid_bodies(std::vector<rigid_body>::iterator first, std::vector<rigid_body>::iterator last);

public:
	/// Construct the physics simulation world. This method actually does nothing. Call init to construct the members of the physics context.
	simulation_world() {}
	/// Destruct the physics simulation world which destructs the JPH::PhysicsSystem and associate object instances.
	~simulation_world();

	/// Initialize the underlying JPH::PhysicsSystem and associate object instances using the given creation settings.
	bool init(physics_system_creation_settings creation_settings);

	/// A body activation listener gets notified when bodies activate and go to sleep
	/// Note that this is called from a job so whatever you do here needs to be thread safe.
	/// PhysicsSystem will take a reference to this so this instance needs to stay alive!
	/// Registering one is entirely optional. For an example see https://github.com/jrouwe/JoltPhysics/tree/master/HelloWorld
	void set_body_activation_listener(JPH::BodyActivationListener* listener) {
		physics_system->SetBodyActivationListener(listener);
	}

	/// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	/// Note that this is called from a job so whatever you do here needs to be thread safe.
	/// PhysicsSystem will take a reference to this so this instance needs to stay alive!
	/// Registering one is entirely optional. For an example see https://github.com/jrouwe/JoltPhysics/tree/master/HelloWorld
	void set_contact_listener(JPH::ContactListener* listener) {
		physics_system->SetContactListener(listener);
	}

	/// Return a pointer to the physics system.
	/// Only valid if init() was called beforehand.
	JPH::PhysicsSystem* get_physics_system() const {
		return physics_system;
	}

	/// Return the locking body interface of the physics system that enables accessing bodies from multiple threads.
	/// The body interface is the main way to interact with the bodies in the physics system.
	/// Shorthand for calling get_physics_system()->GetBodyInterface().
	/// Will crash if simulation_world::init() was not called beforehand!
	JPH::BodyInterface& get_body_interface_no_lock() const {
		return physics_system->GetBodyInterface();
	}

	/// Return the non-locking body interface of the physics system that only allows accessing bodies from a single thread.
	/// The body interface is the main way to interact with the bodies in the physics system.
	/// Shorthand for calling get_physics_system()->GetBodyInterfaceNoLock().
	/// Will crash if simulation_world::init() was not called beforehand!
	JPH::BodyInterface& get_body_interface() const {
		return physics_system->GetBodyInterfaceNoLock();
	}

	/// @brief Create a new rigid_body and add it to the physics system.
	/// 
	/// As an optional step before starting the physics simulation you can optimize the broad phase by calling: physics_system->OptimizeBroadPhase();
	/// This improves collision detection performance. Usually this is only done after startup and when many bodies are present in the system.
	/// You should definitely not call this every frame. To keep the broad phase efficient it is helpful to insert all new objects in batches instead
	/// of 1 at a time to keep the broad phase efficient.
	/// 
	/// @param collision_shape_settings The settings used by JoltPhysics to create the collision shape.
	/// @param shape_representation The visual representation of the rigid_body. Can be different from the collision shape.
	/// @param activate If true, the body is awake upon creation and will be updated in the simulation, otherwise the body will sleep until a first contact is registered.
	/// @return True if the body was successfully created, false if we ran out of available memory for physics bodies.
	bool create_and_add_rigid_body(const JPH::BodyCreationSettings& collision_shape_settings, const std::shared_ptr<const abstract_shape_representation> shape_representation, bool activate = true);

	/// Return a const reference to the list of registered rigid_bodies.
	const std::vector<rigid_body>& ref_rigid_bodies() const { return rigid_bodies; }

	/// Update the world by stepping the physics system with the given time step in seconds.
	/// The delta_time should be at least 1 / 60th of a second. Any periods longer that that
	/// require more than 1 collision_steps to produce a stable simulation.
	void update(float delta_time);

	/// Erase all registered rigid bodies.
	void erase_rigid_bodies();

	/// Convenience method to erase all rigid bodies that are in the given layer.
	void erase_rigid_bodies_by_layer(JPH::ObjectLayer layer);

	/// Convenience method to erase all rigid bodies that have the given motion type.
	void erase_rigid_bodies_by_motion_type(JPH::EMotionType motion_type);

	/// Convenience method to erase all rigid bodies that have the given activation state.
	/// Careful: Static bodies are inactive by convention and will also be removed this way.
	/// If you need to be more specific with your selection consider using erase_rigid_bodies_if.
	void erase_rigid_bodies_by_activation_state(bool active);

	/// Provides a filter interface to erase rigid bodies where the given predicate evaluates to true.
	/// The predicate receives a const reference to the JPH::Body and a shared_ptr to a const
	/// abstract_shape_representation to allow filtering by a large set of properties. It can be supplied as a
	/// lambda closure in the form:
	///		[](const JPH::Body&, const std::shared_ptr<const abstract_shape_representation>) -> bool {}
	/// 
	/// The following example removes sleeping bodies but keeps the ones marked as static (shape representation is not used here):
	///		[](const JPH::Body& body, const auto representation) {
	///			return !body->IsActive() && body->GetObjectLayer() == physics::Layers::MOVING;
	///		});
	template<class Pred>
	void erase_rigid_bodies_if(Pred predicate) {
		// Reorder the list of rigid bodies so all bodies fulfilling the predicate are sorted to the front while all remaining bodies are sorted to the end.
		// The returned iterator points to the first body that does not fulfill the predicate.
		auto it = std::partition(rigid_bodies.begin(), rigid_bodies.end(),
								 [this, &predicate](const rigid_body& rb) {
									 // Make sure the body is locked before accessing it. GetBodyLockInterface() returns the
									 // locking interface suitable for multi-threaded access. If no multi-threading is used
									 // we can also use the non-locking interface through GetBodyLockInterfaceNoLock();
									 // BodyLockRead is used to only allow read access.
									 JPH::BodyLockRead lock(physics_system->GetBodyLockInterface(), rb.get_body_id());
									 // Check if body id is still valid.
									 if(lock.Succeeded())
										 return !predicate(lock.GetBody(), rb.get_shape_representation());
									 // If the body is no longer valid it is sorted to the back.
									 return false;
								 });

		// Remove all bodies starting at the returned iterator.
		remove_and_delete_rigid_bodies(it, rigid_bodies.end());
	}

	/// Provides a convenience interface to execute a function for each rigid body.
	/// The function receives a reference to the JPH::Body and a shared_ptr to a const
	/// abstract_shape_representation to allow direct modification of the body. The interface
	/// is especially useful to, e.g., first check if the body meets certain criteria and then
	/// calling modifying methods on the instance.
	/// It can be supplied as a lambda closure in the form:
	///		[](JPH::Body&, const std::shared_ptr<const abstract_shape_representation>) {}
	template<class Func>
	void for_each_rigid_body(Func function) const {
		std::for_each(rigid_bodies.begin(), rigid_bodies.end(),
								 [this, &function](const rigid_body& rb) {
									 // Make sure the body is locked before accessing it. GetBodyLockInterface() returns the
									 // locking interface suitable for multi-threaded access. If no multi-threading is used
									 // we can also use the non-locking interface through GetBodyLockInterfaceNoLock();
									 // BodyLockWrite is used to allow modifications to the body.
									 JPH::BodyLockWrite lock(physics_system->GetBodyLockInterface(), rb.get_body_id());
									 // Check if body id is still valid and call the functor.
									 if(lock.Succeeded())
										 function(lock.GetBody(), rb.get_shape_representation());
								 });
	}

	/// Provides a convenience interface to return a list of JPH::BodyIDs for all rigid bodies
	/// that fulfill the given predicate. The predicate receives a const reference to the JPH::Body 
	/// and a shared_ptr to a const abstract_shape_representation.
	/// It can be supplied as a lambda closure in the form:
	///		[](const JPH::Body&, const std::shared_ptr<const abstract_shape_representation>) {}
	template<class Pred>
	std::vector<JPH::BodyID> transform_rigid_bodies_to_id_if(Pred predicate) const {
		std::vector<JPH::BodyID> ids;
		ids.reserve(rigid_bodies.size() / 2);

		std::for_each(rigid_bodies.begin(), rigid_bodies.end(),
					  [this, &predicate, &ids](const rigid_body& rb) {
						  // Make sure the body is locked before accessing it. GetBodyLockInterface() returns the
						  // locking interface suitable for multi-threaded access. If no multi-threading is used
						  // we can also use the non-locking interface through GetBodyLockInterfaceNoLock();
						  JPH::BodyLockRead lock(physics_system->GetBodyLockInterface(), rb.get_body_id());
						  // Check if body id is still valid and call the functor.
						  if(lock.Succeeded()) {
							  if(predicate(lock.GetBody(), rb.get_shape_representation()))
								 ids.push_back(rb.get_body_id());
						  }
					  });

		return ids;
	}
};

} // namespace physics
} // namespace cgv

#include <cgv/config/lib_end.h>
