#pragma once

#include <random>

#include <cgv/gui/help_message.h>
#include <cgv/math/random.h>
#include <cgv_app/application_plugin.h>
#include <cgv_gl/box_render_data.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_post/temporal_anti_aliasing.h>

#include <physics/simulation_world.h>
#include <physics/renderer.h>

class physics_viewer : public cgv::app::application_plugin {
public:
	struct body_creation_info {
		cgv::vec3 position = { 0.0f };
		cgv::quat orientation = {};
		JPH::Shape* collision_shape = nullptr;
		std::shared_ptr<cgv::physics::abstract_shape_representation> representation;

		body_creation_info(JPH::Shape* collision_shape, std::shared_ptr<cgv::physics::abstract_shape_representation> representation) : collision_shape(collision_shape), representation(representation) {}
	};

protected:
	enum class ShapeType {
		kBox,
		kCapsule,
		kTaperedCapsule,
		kCylinder,
		kSphere,
		kConvexMesh,
		kMesh
	};

	const unsigned shape_type_count = 7;
	ShapeType selected_shape_type = ShapeType::kBox;

	cgv::post::temporal_anti_aliasing taa;

	cgv::render::shader_program background_prog;
	cgv::rgb background_color_top = { 0.0f };
	cgv::rgb background_color_bottom = { 0.0f };

	const float position_deviation = 5.0f;
	const float spawn_height = 4.0f;

	// This is used to hold temporary references to shapes that are manually created instead of using the convenience construtors.
	// Since the shape instance is created in the scope of a method it would be invalidated on return and before the physics systems
	// body interface can take over ownership. TODO: Check if this produces memory leaks! Find a a better solution!
	JPH::Ref<JPH::Shape> temp_shape_ref;

	cgv::media::mesh::simple_mesh<> icosphere_mesh;
	cgv::media::mesh::simple_mesh<> bunny_mesh;
	cgv::render::mesh_render_info icosphere_mesh_info;
	cgv::render::mesh_render_info bunny_mesh_info;

	cgv::math::random rng;

	cgv::physics::simulation_world physics_world;
	cgv::physics::renderer physics_renderer;
	cgv::box3 simulation_bounds;

	bool run = false;

	cgv::gui::help_message help;

	void post_redraw() {
		taa.reset();
		drawable::post_redraw();
	}

	void draw_background(cgv::render::context& ctx);

	void timer_event(double t, double dt);
	void step_physics_world(float dt);
	void step_simulation();

	cgv::rgb generate_random_color();
	cgv::vec3 generate_spawn_position();
	cgv::quat generate_random_orientation();

	body_creation_info create_box(const cgv::vec3& extent);
	body_creation_info create_capsule(float height, float radius);
	body_creation_info create_tapered_capsule(float height, float base_radius, float top_radius);
	body_creation_info create_cylinder(float height, float radius);
	body_creation_info create_sphere(float radius);

	void add_body(body_creation_info creation_info, JPH::EMotionType motion_type, JPH::ObjectLayer layer, bool activate = true);

	void clear_scene();
	void remove_moving_bodies();
	void remove_sleeping_bodies();
	void remove_dynamic_bodies();
	void activate_bodies();
	void apply_impulse();

	void create_scene();
	void create_floor();

	void add_random_body(body_creation_info creation_info);
	void generate_random_box();
	void generate_random_capsule();
	void generate_random_tapered_capsule();
	void generate_random_cylinder();
	void generate_random_sphere();
	void generate_random_convex_mesh_instance();
	void generate_random_mesh_instance();
	void spawn_shape(bool random);

public:
	physics_viewer();
	std::string get_type_name() const { return "physics_viewer"; }
	
	void stream_stats(std::ostream& os) {};
	void stream_help(std::ostream& os) {};
	bool self_reflect(cgv::reflect::reflection_handler& _rh);

	bool init(cgv::render::context& ctx);
	void clear(cgv::render::context& ctx);

	void on_set(void* member_ptr);
	void handle_member_change(const cgv::utils::pointer_test& m);
	bool handle_event(cgv::gui::event& e);
	
	void init_frame(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	void after_finish(cgv::render::context& ctx);

	void create_gui();
};
