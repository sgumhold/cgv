#include <cgv/base/node.h>
#include <cgv/data/bvh.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/math/intersection.h>
#include <cgv/math/random.h>
#include <cgv/media/named_colors.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_gl/surface_render_data.h>

// This example illustrates how to use the Bounding Volume Hierarchy (BVH) acceleration data structure
// from cgv::data to perform efficient raycast tests against many primitives.

// Define a custom sphere primitive that implements the intersectable interface.
struct Sphere : public cgv::data::ray_intersectable {
	cgv::vec3 center = { 0.0f };
	float radius = 0.0f;
	cgv::rgb color = { 0.0f };

	cgv::box3 get_bounds() const override {
		return { center - radius, center + radius };
	}

	bool intersect(const cgv::ray3& ray, cgv::data::ray_intersection_info& info) const override {
		// Use the intersection routine provided by cgv::math
		return cgv::math::first_ray_sphere_intersection(ray, center, radius, info.t, &info.normal) > 0;
	}
};

// Define a custom triangle primitive that implements the intersectable interface.
struct Triangle : public cgv::data::ray_intersectable {
	cgv::vec3 p0 = { 0.0f };
	cgv::vec3 p1 = { 0.0f };
	cgv::vec3 p2 = { 0.0f };
	cgv::rgb color = { 0.0f };

	cgv::box3 get_bounds() const override {
		cgv::box3 bounds;
		bounds.add_point(p0);
		bounds.add_point(p1);
		bounds.add_point(p2);
		return bounds;
	}

	bool intersect(const cgv::ray3& ray, cgv::data::ray_intersection_info& info) const override {
		// Use the intersection routine provided by cgv::math
		return cgv::math::ray_triangle_intersection(ray, p0, p1, p2, info.t, nullptr, &info.normal) > 0;
	}

	cgv::vec3 get_normal() const {
		return normalize(cross(p1 - p0, p2 - p0));
	}
};

class raycasting :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider {
protected:
	/// the current view
	cgv::render::view* view = nullptr;

	/// the number of primitives to create
	size_t primitive_count = 10000;
	/// the array of spheres in the scene
	std::vector<Sphere> spheres;
	/// the array of triangles in the scene
	std::vector<Triangle> triangles;

	/// the scene represented using a BVH
	cgv::data::bvh scene;

	/// Render data for the spheres
	cgv::render::sphere_render_data<> sphere_geometry;
	/// Render data for the triangles
	cgv::render::surface_render_data<> triangle_geometry;

	/// The current mouse position in screen coordinates
	cgv::vec2 mouse_screen_position = { -1.0f };

public:
	raycasting() : cgv::base::node("Raycasting Test")
	{
		// Illuminate triangles from both sides
		triangle_geometry.style.illumination_mode = cgv::render::IlluminationMode::IM_TWO_SIDED;
	}

	std::string get_type_name() const
	{
		return "raycasting";
	}

	bool init(cgv::render::context& ctx)
	{
		// Initialize all classes holding GPU geometry data
		if(!sphere_geometry.init(ctx))
			return false;
		
		if(!triangle_geometry.init(ctx))
			return false;

		create_scene();

		return true;
	}

	void clear(cgv::render::context& ctx)
	{
		sphere_geometry.destruct(ctx);
		triangle_geometry.destruct(ctx);
	}

	void init_frame(cgv::render::context& ctx)
	{
		if(!view)
			view = find_view_as_node();

		// If we have a valid mouse position we generate a ray from the view eye point through the pixel pointed to by
		// the mouse screen coordinate and test for any intersections with the scene geometry.
		if(view && mouse_screen_position.x() >= 0.0f && mouse_screen_position.y() >= 0.0f) {
			cgv::uvec2 viewport_size = { ctx.get_width(), ctx.get_height() };
			// Generate a ray using the current view configuration. This assumes the model matrix is unused and equivalent to an identity matrix.
			cgv::ray3 mouse_ray(mouse_screen_position, viewport_size, cgv::vec3(view->get_eye()), ctx.get_projection_matrix() * ctx.get_modelview_matrix());
			
			// Reset all colors to default
			sphere_geometry.colors.clear();
			sphere_geometry.fill_colors(cgv::media::colors::gray);
			triangle_geometry.colors.clear();
			triangle_geometry.fill_colors(cgv::media::colors::gray);

			// Intersect the ray with the scene. The result contains a flag indicating whether the ray actually hit a primitive
			// and if this is true, the intersection information as well as the primitive index and a pointer tot he primitive.
			cgv::data::bvh_result result = scene.closest_intersection(mouse_ray);
			if(result.is_hit) {
				if(result.primitive_index < spheres.size()) {
					sphere_geometry.colors[result.primitive_index] = cgv::media::colors::green;
				} else {
					// Subtract the sphere count from the primitve index to get the triangle index since during construction all triangles were placed after all spheres.
					size_t triangle_index = result.primitive_index - spheres.size();
					// Set the color for all three triangle vertices.
					triangle_geometry.colors[3 * triangle_index + 0] = cgv::media::colors::green;
					triangle_geometry.colors[3 * triangle_index + 1] = cgv::media::colors::green;
					triangle_geometry.colors[3 * triangle_index + 2] = cgv::media::colors::green;
				}
			}

			sphere_geometry.set_out_of_date();
			triangle_geometry.set_out_of_date();
		}
	}

	void draw(cgv::render::context& ctx)
	{	
		if(!view)
			return;

		sphere_geometry.render(ctx);
		triangle_geometry.render(ctx);
	}

	bool handle(cgv::gui::event& event)
	{
		if(event.get_kind() == cgv::gui::EventId::EID_MOUSE) {
			cgv::gui::mouse_event& mouse_event = dynamic_cast<cgv::gui::mouse_event&>(event);

			if(mouse_event.get_action() == cgv::gui::MouseAction::MA_MOVE) {
				if(auto ctx = get_context()) {
					// Get the mouse screen position by inverting the y component to transform from FLTK coordinates to OpenGL coordinates.
					mouse_screen_position = { static_cast<float>(mouse_event.get_x()), static_cast<float>(ctx->get_height() - mouse_event.get_y() - 1) };
					post_redraw();
				}
			}
		}

		return false;
	}

	void on_set(void* member_ptr)
	{
		post_redraw();
		update_member(member_ptr);
	}

	void create_gui()
	{
		add_decorator("Raycasting", "heading");
	}

	void create_scene()
	{
		cgv::math::random rng;

		auto get_random_vec3 = [&rng]() {
			cgv::vecn v(3u, 0.0f);
			rng.uniform(v);
			return 2.0f * cgv::vec3::from_vec(v) - 1.0f;
		};

		// Generate randomly placed spheres and triangles
		for(size_t i = 0; i < primitive_count; ++i) {
			bool is_sphere = false;
			rng.uniform(is_sphere);
			if(is_sphere) {
				Sphere sphere;
				sphere.center = get_random_vec3();
				rng.uniform(sphere.radius);
				sphere.radius = 0.02f * (sphere.radius + 0.1f);
				sphere.color = cgv::media::colors::gray;
				spheres.push_back(sphere);
			} else {
				cgv::vec3 position = get_random_vec3();

				Triangle triangle;
				triangle.p0 = position += 0.05f * get_random_vec3();
				triangle.p1 = position += 0.05f * get_random_vec3();
				triangle.p2 = position += 0.05f * get_random_vec3();
				triangle.color = cgv::media::colors::gray;
				triangles.push_back(triangle);
			}
		}

		// Collect pointers to the generated spheres and triangles as pointers to ray_intersectables. This is only possible because we do not
		// alter the respective lists after building the BVH and the pointers will stay valid. Pointers to all triangles are appended to
		// the list after all sphere pointers. We later need to pay attention to this when retrieving the hit primitive index.
		//
		// Note: This is only one possible way of using the BVH. The call-site could also directly manage a list of polymorphic ray_intersectable instances
		// and pass this to the build function.
		std::vector<const cgv::data::ray_intersectable*> primitives;
		primitives.reserve(spheres.size() + triangles.size());
		std::transform(spheres.begin(), spheres.end(), std::back_inserter(primitives), [](const Sphere& sphere) { return &sphere; });
		std::transform(triangles.begin(), triangles.end(), std::back_inserter(primitives), [](const Triangle& triangle) { return &triangle; });
		// Build the BVH using a maximum depth of 8. The BVH will store pointers to the primitives but does not take ownership of them.
		// We need to ensure the objects live as long as we use the BVH.
		scene.build(primitives, 8);

		// Update the GPU-side geometry for rendering.
		sphere_geometry.clear();

		for(const Sphere& sphere : spheres)
			sphere_geometry.add(sphere.center, sphere.color, sphere.radius);

		triangle_geometry.clear();

		for(const Triangle& triangle : triangles) {
			triangle_geometry.add_triangle(triangle.p0, triangle.p1, triangle.p2);
			triangle_geometry.add_triangle_normal(triangle.get_normal());
			triangle_geometry.add_triangle_color(cgv::media::colors::gray);
		}
	}

	void stream_help(std::ostream& os) {}
};

#include <cgv/base/register.h>

/// register a factory to create new raycasting tests
cgv::base::factory_registration<raycasting> raycasting_fac("New/Demo/Raycasting");
