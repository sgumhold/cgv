#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/math/integer.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/attribute_array_manager.h>
#include <cgv_gl/sphere_renderer.h>

/// This example illustrates how to use a compute shader to perform massively parallel work on the GPU.
/// The scene consists of spheres placed on a regular grid inside a cube. Each sphere is assigned a random
/// velocity attribute. The compute shader updates the positions of the spheres based on their velocity in
/// parallel and without the need to copy geometry data between the CPU and GPU.
class compute_shader : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
public:
	compute_shader() : cgv::base::node("Compute Shader") {
		sphere_style.radius = 0.01f;
		// Connect an animation trigger to a timer event that is used to step the animation.
		cgv::signal::connect(cgv::gui::get_animation_trigger().shoot, this, &compute_shader::timer_event);
	}

	std::string get_type_name() const {
		return "compute_shader";
	}

	void on_set(void* member_ptr) {
		post_redraw();
		update_member(member_ptr);
	}

	void timer_event(double, double dt) {
		if(!animate || !get_context())
			return;
			
		cgv::render::context& ctx = *get_context();

		// Enable the attribute array containing the sphere geometry to be able to access the position and color vertex buffers.
		cgv::render::sphere_renderer& renderer = cgv::render::ref_sphere_renderer(ctx);
		renderer.enable_attribute_array_manager(ctx, sphere_geometry);

		const cgv::render::vertex_buffer* position_buffer = renderer.get_vertex_buffer_ptr(ctx, sphere_geometry, "position");
		if(!position_buffer) {
			std::cout << "Error: could not get position vertex buffer" << std::endl;
			return;
		}

		const cgv::render::vertex_buffer* color_buffer = renderer.get_vertex_buffer_ptr(ctx, sphere_geometry, "color");
		if(!color_buffer) {
			std::cout << "Error: could not get color vertex buffer" << std::endl;
			return;
		}

		if(compute_prog.is_created()) {
			// Bind the position and color vertex buffers as shader storage buffers to allow random read-write access by the compute shader.
			// The compute shader will interpret the color values as our velocities.
			position_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
			color_buffer->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

			compute_prog.enable(ctx);
			compute_prog.set_uniform(ctx, "count", static_cast<uint32_t>(sphere_count));
			compute_prog.set_uniform(ctx, "velocity", velocity);

			// Calculate the required number of groups to invoke enough threads to cover all positions.
			// The group size should be a power of two. The total number of threads is group_count * group_size.
			const size_t group_size = 64;
			const size_t group_count = cgv::math::div_round_up(sphere_count, group_size);

			// Dispatch group_count groups in 1 dimension since we are working on a 1D array.
			glDispatchCompute((GLsizei)group_count, 1, 1);

			// Wait for all writes to shader storage buffers to be completed.
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			compute_prog.disable(ctx);

			position_buffer->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
			color_buffer->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
		} else {
			std::cout << "Error: compute shader program is not initialized" << std::endl;
		}
				
		renderer.disable_attribute_array_manager(ctx, sphere_geometry);
		
		post_redraw();
	}

	bool init(cgv::render::context& ctx) {
		cgv::render::ref_sphere_renderer(ctx, 1);

		if(!sphere_geometry.init(ctx)) {
			std::cout << "Error: could not initialize sphere attribute array manager" << std::endl;
			return false;
		}

		// Load the compute shader program that is used in this example.
		if(!compute_prog.build_files(ctx, "compute", true)) {
			std::cerr << "ERROR in compute_shader::init() ... could not build shader program " << name << ".glcs" << std::endl;
			return false;
		}

		create_spheres();
		return true;
	}

	void clear(cgv::render::context& ctx) {
		cgv::render::ref_sphere_renderer(ctx, -1);
		sphere_geometry.destruct(ctx);
	}

	void draw(cgv::render::context& ctx) {
		cgv::render::sphere_renderer& renderer = cgv::render::ref_sphere_renderer(ctx);
		renderer.enable_attribute_array_manager(ctx, sphere_geometry);
		renderer.set_render_style(sphere_style);
		renderer.render(ctx, 0, sphere_count);
		renderer.disable_attribute_array_manager(ctx, sphere_geometry);
	}

	void create_gui() {
		add_decorator("Compute Shader", "heading", "level=2");

		add_member_control(this, "Animate", animate, "toggle");
		add_member_control(this, "Velocity", velocity, "value_slider", "min=0;max=1;step=0.0001");

		connect_copy(add_button("Reset")->click, cgv::signal::rebind(this, &compute_shader::create_spheres));
		
		if(begin_tree_node("Sphere style", sphere_style, false)) {
			align("\a");
			add_gui("sphere_style", sphere_style);
			align("\b");
			end_tree_node(sphere_style);
		}
	}

private:
	void create_spheres() {
		if(!get_context())
			return;
		
		cgv::render::context& ctx = *get_context();

		std::default_random_engine random_engine(42);
		std::uniform_real_distribution<float> unorm_distribution(0.0f, 1.0f);

		std::vector<cgv::vec3> positions;
		// Use the sphere colors to encode a scalar velocity attribute in range [0,1].
		// This has the neat side effect of coloring the spheres according to their velocity
		// with faster spheres being more red.
		std::vector<float> colors;
		positions.reserve(sphere_count);
		colors.reserve(sphere_count);

		const float stride = 1.0f / static_cast<float>(grid_resolution - 1);
		const float offset = -0.5f;

		cgv::ivec3 cell = { 0 };
		for(cell[2] = 0; cell[2] < grid_resolution; ++cell[2]) {
			for(cell[1] = 0; cell[1] < grid_resolution; ++cell[1]) {
				for(cell[0] = 0; cell[0] < grid_resolution; ++cell[0]) {
					cgv::vec3 position = stride * cgv::vec3(cell) + offset;
					positions.push_back(position);
					colors.push_back(unorm_distribution(random_engine));
				}
			}
		}

		cgv::render::sphere_renderer& renderer = cgv::render::ref_sphere_renderer(ctx);
		renderer.enable_attribute_array_manager(ctx, sphere_geometry);
		renderer.set_position_array(ctx, positions);
		renderer.set_color_array(ctx, colors);
		renderer.disable_attribute_array_manager(ctx, sphere_geometry);
		
		post_redraw();
	}

	bool animate = false;

	float velocity = 0.5f;

	const size_t grid_resolution = 32;
	const size_t sphere_count = grid_resolution * grid_resolution * grid_resolution;

	cgv::render::shader_program compute_prog;

	cgv::render::attribute_array_manager sphere_geometry;
	cgv::render::sphere_render_style sphere_style;	
};

#include <cgv/base/register.h>

/// register a factory to create new compute_shader instances
cgv::base::factory_registration<compute_shader> compute_shader_fac("New/GPGPU/Compute Shader");
