#include "physics_viewer.h"

#include <cgv/gui/trigger.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/render/shader_library.h>

// Jolt
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>

#include <physics/convert_types.h>

using namespace cgv::render;

physics_viewer::physics_viewer() : application_plugin("Physics Viewer") {

	// Seed the random generator with a constant value so that every run produces the same results
	rng.set_seed(42ull);

	// Define the simulation world boundaries
	simulation_bounds = cgv::box3(cgv::vec3(-100.0f, -5.0f, -100.0f), cgv::vec3(100.0f, 50.0f, 100.0f));

	// Setup a timer event to step the physics simulation
	connect(cgv::gui::get_animation_trigger().shoot, this, &physics_viewer::timer_event);

	help.add_line("Adding Bodies:");
	help.add_bullet_point("1 - Box");
	help.add_bullet_point("2 - Capsule");
	help.add_bullet_point("3 - Tapered capsule");
	help.add_bullet_point("4 - Cylinder");
	help.add_bullet_point("5 - Sphere");
	help.add_bullet_point("6 - Convex mesh");
	help.add_bullet_point("7 - Mesh (static)");
	help.add_bullet_point("G - Generate selected body");
	help.add_bullet_point("R - Generate random body");

	help.add_line("Simulation Control:");
	help.add_bullet_point(". - Step simulation");
	help.add_bullet_point("Spacebar - Play/Pause simulation");
}

bool physics_viewer::self_reflect(cgv::reflect::reflection_handler& rh) {

	return false;
}

bool physics_viewer::init(context& ctx) {

	bool success = true;
	success &= shader_library::load(ctx, background_prog, "background.glpr");
	success &= taa.init(ctx);
	success &= physics_world.init(cgv::physics::simulation_world::physics_system_creation_settings());
	success &= physics_renderer.init(ctx, physics_world);

	// Load and construct two example meshes
	const auto construct_mesh_from_file = [&ctx](const std::string& filename, cgv::media::mesh::simple_mesh<>& mesh, cgv::render::mesh_render_info& mesh_info) {
		if(mesh.read(filename)) {
			mesh_info.construct(ctx, mesh);
			mesh_info.bind(ctx, ctx.ref_surface_shader_program(true), true);
		}

		if(!mesh_info.is_constructed()) {
			std::cout << "Error: Could not construct the mesh from file " << filename << std::endl;
			return false;
		}

		return true;
	};

	success &= construct_mesh_from_file("./resources/icosphere.obj", icosphere_mesh, icosphere_mesh_info);
	success &= construct_mesh_from_file("./resources/bunny_low_poly.obj", bunny_mesh, bunny_mesh_info);

	create_scene();

	return success;
}

void physics_viewer::clear(context& ctx) {

	taa.destruct(ctx);

	background_prog.destruct(ctx);

	physics_renderer.destruct(ctx);
}

void physics_viewer::on_set(void* member_ptr) {

	handle_member_change(cgv::utils::pointer_test(member_ptr));
	update_member(member_ptr);
	post_redraw();
}

void physics_viewer::handle_member_change(const cgv::utils::pointer_test& m) {}

bool physics_viewer::handle_event(cgv::gui::event& e) {

	if(e.get_kind() == cgv::gui::EID_MOUSE) {
		// Currently unused
		//auto& me = static_cast<cgv::gui::mouse_event&>(e);
	} else if(e.get_kind() == cgv::gui::EID_KEY) {
		auto& ke = static_cast<cgv::gui::key_event&>(e);
		if(ke.get_action() == cgv::gui::KA_RELEASE)
			return false;

		switch(ke.get_key()) {
		case '1':
			generate_random_box();
			return true;
		case '2':
			generate_random_capsule();
			return true;
		case '3':
			generate_random_tapered_capsule();
			return true;
		case '4':
			generate_random_cylinder();
			return true;
		case '5':
			generate_random_sphere();
			return true;
		case '6':
			generate_random_convex_mesh_instance();
			return true;
		case '7':
			generate_random_mesh_instance();
			return true;
		case 'G':
			spawn_shape(false);
			return true;
		case 'R':
			spawn_shape(true);
			return true;
		case '.':
			step_simulation();
			return true;
		case cgv::gui::KEY_Space:
			run = !run;
			update_member(&run);
			return true;
		default:
			break;
		}
	}

	return false;
}

void physics_viewer::init_frame(context& ctx) {
	
	taa.ensure(ctx);
}

void physics_viewer::draw(context& ctx) {

	taa.begin(ctx);

	draw_background(ctx);
	physics_renderer.draw(ctx);
	
	taa.end(ctx);
}

void physics_viewer::draw_background(context& ctx) {

	glDisable(GL_DEPTH_TEST);

	const float gamma = ctx.get_gamma();

	background_prog.enable(ctx);
	background_prog.set_uniform(ctx, "w", static_cast<int>(ctx.get_width()));
	background_prog.set_uniform(ctx, "h", static_cast<int>(ctx.get_height()));
	background_prog.set_uniform(ctx, "color_1", cgv::rgba(pow(background_color_bottom, gamma), 1.0f));
	background_prog.set_uniform(ctx, "color_2", cgv::rgba(pow(background_color_top, gamma), 1.0f));
	background_prog.set_uniform(ctx, "mode", 2);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	background_prog.disable(ctx);

	glEnable(GL_DEPTH_TEST);
}

void physics_viewer::after_finish(cgv::render::context& ctx) {
	
	if(initialize_view_ptr()) {
		taa.set_view(view_ptr);

		background_color_top = cgv::rgb(0.1f, 0.13f, 0.22f);
		background_color_bottom = cgv::rgb(0.33f, 0.40f, 0.52f);

		view_ptr->set_eye_keep_view_angle(cgv::dvec3(0.0f, 6.0f, 12.0f));
		post_redraw();
	}
}

void physics_viewer::create_gui() {

	add_decorator("Physics Demo", "heading");
	help.create_gui(this);
	add_member_control(this, "Run", run, "toggle");
	connect_copy(add_button("Step")->click, cgv::signal::rebind(this, &physics_viewer::step_simulation));

	add_decorator("Scene Control", "heading", "level=4");
	connect_copy(add_button("Clear Scene")->click, cgv::signal::rebind(this, &physics_viewer::clear_scene));
	connect_copy(add_button("Create Floor")->click, cgv::signal::rebind(this, &physics_viewer::create_floor));
	connect_copy(add_button("Remove Moving Layer")->click, cgv::signal::rebind(this, &physics_viewer::remove_moving_bodies));
	connect_copy(add_button("Remove Sleeping Bodies")->click, cgv::signal::rebind(this, &physics_viewer::remove_sleeping_bodies));
	connect_copy(add_button("Remove Dynamic Bodies")->click, cgv::signal::rebind(this, &physics_viewer::remove_dynamic_bodies));

	add_decorator("Spawn Random Shapes", "heading", "level=4");
	add_member_control(this, "Type", selected_shape_type, "dropdown", "enums='Box, Capsule, Tapered Capsule, Cylinder (unstable), Sphere, Convex Mesh, Mesh'");
	connect_copy(add_button("Spawn Selected")->click, cgv::signal::rebind(this, &physics_viewer::spawn_shape, cgv::signal::const_expression<bool>(false)));
	connect_copy(add_button("Spawn Random")->click, cgv::signal::rebind(this, &physics_viewer::spawn_shape, cgv::signal::const_expression<bool>(true)));

	add_decorator("Body Control", "heading", "level=4");
	connect_copy(add_button("Activate Moving Layer")->click, cgv::signal::rebind(this, &physics_viewer::activate_bodies));
	connect_copy(add_button("Apply Impulse")->click, cgv::signal::rebind(this, &physics_viewer::apply_impulse));
}

void physics_viewer::timer_event(double t, double dt) {

	if(run) {
		step_physics_world(static_cast<float>(dt));
		physics_renderer.update();
		post_redraw();
	}
}

void physics_viewer::step_physics_world(float dt) {

	// Step the world
	physics_world.update(static_cast<float>(dt));

	// Use the body erase interface to remove bodies that are outside of the simulation bounds
	physics_world.erase_rigid_bodies_if(
		[this](const JPH::Body& body, const auto& representation) {
			auto position = cgv::physics::convert::to_vec3(body.GetCenterOfMassPosition());
			return !simulation_bounds.inside(position);
		});
}

void physics_viewer::step_simulation() {

	// This method only exists to allow stepping through a key event and gui button.
	if(!run) {
		step_physics_world(1.0f / 60.0f);
		physics_renderer.update();
		post_redraw();
	}
}

cgv::rgb physics_viewer::generate_random_color() {

	float hue = 0.0f;
	rng.uniform(hue);
	return static_cast<cgv::rgb>(cgv::media::color<float, cgv::media::ColorModel::HLS>(hue, 0.5f, 1.0f));
}

cgv::vec3 physics_viewer::generate_spawn_position() {

	cgv::vec3 position;
	rng.uniform(-position_deviation, position_deviation, position);
	position.y() = spawn_height;
	return position;
}

cgv::quat physics_viewer::generate_random_orientation() {

	cgv::quat orientation;
	rng.uniform_quat_orientation(orientation);
	return orientation;
}

physics_viewer::body_creation_info physics_viewer::create_box(const cgv::vec3& extent) {

	auto box = std::make_shared<cgv::physics::box_representation>();
	box->extent = extent;
	return { new JPH::BoxShape(cgv::physics::convert::to_Jolt_Vec3(0.5f * extent)), box };
}

physics_viewer::body_creation_info physics_viewer::create_capsule(float height, float radius) {
	
	auto capsule = std::make_shared<cgv::physics::capsule_representation>();
	capsule->height = height;
	capsule->base_radius = radius;
	return { new JPH::CapsuleShape(0.5f * height, radius), capsule };
}

physics_viewer::body_creation_info physics_viewer::create_tapered_capsule(float height, float base_radius, float top_radius) {

	auto tapered_capsule = std::make_shared<cgv::physics::tapered_capsule_representation>();
	tapered_capsule->height = height;
	tapered_capsule->base_radius = base_radius;
	tapered_capsule->top_radius = top_radius;
	JPH::TaperedCapsuleShapeSettings settings(0.5f * height, top_radius, base_radius);
	auto result = settings.Create();
	temp_shape_ref = result.Get();
	return { temp_shape_ref.GetPtr(), tapered_capsule };
}

physics_viewer::body_creation_info physics_viewer::create_cylinder(float height, float radius) {
	
	auto cylinder = std::make_shared<cgv::physics::cylinder_representation>();
	cylinder->height = height;
	cylinder->base_radius = radius;
	return { new JPH::CylinderShape(0.5f * height, radius), cylinder };
}

physics_viewer::body_creation_info physics_viewer::create_sphere(float radius) {

	auto sphere = std::make_shared<cgv::physics::sphere_representation>();
	sphere->radius = radius;
	return { new JPH::SphereShape(radius), sphere };
}

void physics_viewer::add_body(body_creation_info creation_info, JPH::EMotionType motion_type, JPH::ObjectLayer layer, bool activate) {

	JPH::BodyCreationSettings settings(creation_info.collision_shape,
									   cgv::physics::convert::to_Jolt_Vec3(creation_info.position),
									   cgv::physics::convert::to_Jolt_Quat(creation_info.orientation),
									   motion_type,
									   layer);

	physics_world.create_and_add_rigid_body(settings, creation_info.representation, activate);
}

void physics_viewer::clear_scene() {

	physics_world.erase_rigid_bodies();
	physics_renderer.update();
	post_redraw();
}

void physics_viewer::remove_moving_bodies() {

	physics_world.erase_rigid_bodies_by_layer(cgv::physics::Layers::MOVING);
	physics_renderer.update();
	post_redraw();
}

void physics_viewer::remove_sleeping_bodies() {

	physics_world.erase_rigid_bodies_if(
		[](const JPH::Body& body, const auto representation) {
			return !body.IsActive() && body.GetObjectLayer() == cgv::physics::Layers::MOVING;
		});

	physics_renderer.update();
	post_redraw();
}

void physics_viewer::remove_dynamic_bodies() {

	physics_world.erase_rigid_bodies_by_motion_type(JPH::EMotionType::Dynamic);
	physics_renderer.update();
	post_redraw();
}

void physics_viewer::activate_bodies() {

	std::vector<JPH::BodyID> body_ids = physics_world.transform_rigid_bodies_to_id_if(
		[](const JPH::Body& body, const auto& representation) {
			return body.IsDynamic() && body.GetObjectLayer() == cgv::physics::Layers::MOVING && !body.IsActive();
		});

	if(!body_ids.empty())
		physics_world.get_body_interface().ActivateBodies(body_ids.data(), static_cast<int>(body_ids.size()));

	post_redraw();
}

void physics_viewer::apply_impulse() {

	const JPH::Vec3 impulse(0.0f, 10000.0f, 0.0f);

	std::for_each(physics_world.ref_rigid_bodies().begin(), physics_world.ref_rigid_bodies().end(),
				  [this, &impulse](const cgv::physics::rigid_body& rigid_body) {
					  physics_world.get_body_interface().AddImpulse(rigid_body.get_body_id(), impulse);
				  });

	post_redraw();
}

void physics_viewer::create_scene() {

	create_floor();

	const std::vector<cgv::vec3> positions = {
		{ 0.0f, 2.0f, 0.0f },
		{ 0.5f, 3.0f, 0.0f },
		{ -0.3f, 4.0f, 0.3f },
	};

	for(const auto& position : positions) {
		auto creation_info = create_sphere(0.5f);
		creation_info.position = position;
		creation_info.representation->color = generate_random_color();
		add_body(creation_info, JPH::EMotionType::Dynamic, cgv::physics::Layers::MOVING);
	}

	physics_renderer.update();
}

void physics_viewer::create_floor() {

	const cgv::vec3 position(0.0f, -0.5f, 0.0f);
	const cgv::vec3 extent(20.0f, 1.0f, 20.0f);

	const float thickness = 0.2f;
	const float height = 1.0f;

	std::vector<std::pair<cgv::vec3, cgv::vec3>> boxes;
	boxes.push_back({ position, extent });

	cgv::vec3 wall_position(-0.5f * extent.x() + 0.5f * thickness, height, 0.0f);
	wall_position += position;

	cgv::vec3 wall_extent(thickness, height, extent.z());

	boxes.push_back({ wall_position, wall_extent });
	boxes.push_back({ wall_position + cgv::vec3(extent.x() - thickness, 0.0f, 0.0f), wall_extent });

	std::swap(wall_position.x(), wall_position.z());
	std::swap(wall_extent.x(), wall_extent.z());

	boxes.push_back({ wall_position, wall_extent });
	boxes.push_back({ wall_position + cgv::vec3(0.0f, 0.0f, extent.z() - thickness), wall_extent });;

	for(const auto& box : boxes) {
		auto creation_info = create_box(box.second);
		creation_info.position = box.first;
		add_body(creation_info, JPH::EMotionType::Static, cgv::physics::Layers::NON_MOVING, false);
	}

	physics_renderer.update();
}

void physics_viewer::add_random_body(body_creation_info creation_info) {

	creation_info.position = generate_spawn_position();
	creation_info.orientation = generate_random_orientation();
	creation_info.representation->color = generate_random_color();
	add_body(creation_info, JPH::EMotionType::Dynamic, cgv::physics::Layers::MOVING);

	physics_renderer.update();
	post_redraw();
}

void physics_viewer::generate_random_box() {

	cgv::vec3 extent;
	rng.uniform(0.5f, 2.0f, extent);

	add_random_body(create_box(extent));
}

void physics_viewer::generate_random_capsule() {

	float height = 1.0f;
	rng.uniform(1.5f, 3.0f, height);

	float radius = 1.0f;
	rng.uniform(0.1f, 0.5f, radius);

	add_random_body(create_capsule(height, radius));
}

void physics_viewer::generate_random_tapered_capsule() {

	float height = 1.0f;
	rng.uniform(1.5f, 3.0f, height);

	float base_radius = 1.0f;
	rng.uniform(0.1f, 0.5f, base_radius);

	float top_radius = 1.0f;
	rng.uniform(0.25f, 0.75f, top_radius);

	add_random_body(create_tapered_capsule(height, base_radius, top_radius));
}

void physics_viewer::generate_random_cylinder() {

	float height = 1.0f;
	rng.uniform(1.5f, 3.0f, height);

	float radius = 1.0f;
	rng.uniform(0.1f, 0.5f, radius);

	add_random_body(create_cylinder(height, radius));
}

void physics_viewer::generate_random_sphere() {

	float radius = 1.0f;
	rng.uniform(0.25f, 0.75f, radius);

	add_random_body(create_sphere(radius));
}

void physics_viewer::generate_random_convex_mesh_instance() {

	auto represenation = std::make_shared<cgv::physics::mesh_representation>();
	represenation->mesh_info = &icosphere_mesh_info;

	JPH::ConvexHullShapeSettings settings(cgv::physics::convert::to_Jolt_Vec3_Array(icosphere_mesh.get_positions()));
	auto result = settings.Create();
	temp_shape_ref = result.Get();

	if(result.HasError())
		std::cout << "Error: " << result.GetError() << std::endl;
	else
		add_random_body({ temp_shape_ref.GetPtr(), represenation });
}

void physics_viewer::generate_random_mesh_instance() {

	float scale = 1.0f;
	rng.uniform(1.0f, 3.0f, scale);

	auto represenation = std::make_shared<cgv::physics::mesh_representation>();
	represenation->mesh_info = &bunny_mesh_info;
	represenation->scale = scale;



	// TODO: Create a method for this in simple_mesh?
	std::vector<cgv::media::mesh::simple_mesh<>::idx_type> vertex_indices;
	std::vector<cgv::math::fvec<cgv::media::mesh::simple_mesh<>::idx_type, 4>> unique_quadruples;
	bunny_mesh.merge_indices(vertex_indices, unique_quadruples);

	std::vector<cgv::media::mesh::simple_mesh<>::idx_type> triangle_element_buffer;
	bunny_mesh.extract_triangle_element_buffer(vertex_indices, triangle_element_buffer);

	auto& positions = bunny_mesh.get_positions();
	JPH::TriangleList triangles;

	for(size_t i = 0; i < triangle_element_buffer.size(); i += 3)
		triangles.emplace_back(cgv::physics::convert::to_Jolt_Vec3(positions[triangle_element_buffer[i + 0]]),
							   cgv::physics::convert::to_Jolt_Vec3(positions[triangle_element_buffer[i + 1]]),
							   cgv::physics::convert::to_Jolt_Vec3(positions[triangle_element_buffer[i + 2]]));



	JPH::MeshShapeSettings settings(triangles);
	auto result = settings.Create();
	temp_shape_ref = result.Get();

	JPH::Shape* shape = new JPH::ScaledShape(temp_shape_ref.GetPtr(), cgv::physics::convert::to_Jolt_Vec3({ scale }));

	if(result.HasError()) {
		std::cout << "Error: " << result.GetError() << std::endl;
	} else {
		// Mesh shapes are added as static bodies because they have limited functionality in the physics system.
		// They could be used as dynamic bodies but are not allowed to collide with other mesh shapes.
		body_creation_info creation_info(shape, represenation);

		auto position = generate_spawn_position();
		position.y() = 0.0f;
		creation_info.position = position;

		float angle = 0.0f;
		rng.uniform(0.0f, 3.1415f, angle);
		cgv::quat orientation(cgv::quat::AxisEnum::Y_AXIS, angle);

		creation_info.orientation = orientation;
		creation_info.representation->color = generate_random_color();
		add_body(creation_info, JPH::EMotionType::Static, cgv::physics::Layers::NON_MOVING, false);

		physics_renderer.update();
		post_redraw();
	}
}

void physics_viewer::spawn_shape(bool random) {

	ShapeType type = selected_shape_type;

	if(random) {
		unsigned index = 0;
		rng.uniform(0, shape_type_count - 1, index);
		type = static_cast<ShapeType>(index);
	}

	switch(type) {
	case ShapeType::kBox:
		generate_random_box();
		break;
	case ShapeType::kCapsule:
		generate_random_capsule();
		break;
	case ShapeType::kTaperedCapsule:
		generate_random_tapered_capsule();
		break;
	case ShapeType::kCylinder:
		generate_random_cylinder();
		break;
	case ShapeType::kSphere:
		generate_random_sphere();
		break;
	case ShapeType::kConvexMesh:
		generate_random_convex_mesh_instance();
		break;
	case ShapeType::kMesh:
		generate_random_mesh_instance();
		break;
	default:
		break;
	}
}

#include <cgv/base/register.h>

cgv::base::object_registration<physics_viewer> physics_viewer_reg("physics_viewer");
cgv::base::registration_order_definition ro_def("stereo_view_interactor;physics_viewer");
