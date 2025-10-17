#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/property_string.h>
#include <cgv/math/constants.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_gl/gl/gl_time_query.h>
#include <cgv_gpgpu/copy.h>
#include <cgv_gpgpu/copy_if.h>

class gpgpu_filter_test : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
public:
	gpgpu_filter_test() : cgv::base::node("GPGPU Filter Test")
	{
		random_engine.seed(42);
		spheres.style.surface_color = cgv::rgb(0.5f);
		spheres.style.map_color_to_material = cgv::render::CM_COLOR;
	}

	std::string get_type_name() const {
		return "gpgpu_filter_test";
	}

	void on_set(void* member_ptr)
	{
		post_redraw();
		update_member(member_ptr);
	}

	bool init(cgv::render::context& ctx)
	{
		if(!spheres.init(ctx)) {
			std::cout << "Error: could not initialize sphere render data" << std::endl;
			return false;
		}
		
		if(!time_query.init(ctx)) {
			std::cout << "Error: could not initialize GPU timer" << std::endl;
			return false;
		}

		// Define a data type that matches the 3d positions of the sphere geometry.
		// Note: Due to alignment rules for GPU buffers we cannot use sl::kVec3 directly but instead have to use a struct with three float members.
		sl::data_type vec3_type = { "vec3_type", {
			{ sl::Type::kFloat, "x" },
			{ sl::Type::kFloat, "y" },
			{ sl::Type::kFloat, "z" }
		} };

		// Define a boolean predicate that returns true for all input elements we want to keep and copy to the output buffer.
		// This string defines the body of a function with signature bool(value_type element, uint index), where value_type is equal to the value_type as given to the init method of copy_if.
		// The content needs to follow GLSL syntax.
		// In this case, value_type is equal to vec3_type as defined in the constructor and the predicate will return true for all positions within the unit sphere.
		std::string predicate = R"(
			vec3 position = vec3(element.x, element.y, element.z);
			return length(position) < 1.0;
		)";

		// Initialize the filter (copy_if) using the given value type and predicate.
		if(!filter.init(ctx, vec3_type, predicate)) {
			std::cout << "Error: could not initialize GPU filter algorithm" << std::endl;
			return false;
		}

		if(!copy_helper.init(ctx, vec3_type)) {
			std::cout << "Error: could not initialize GPU copy algorithm" << std::endl;
			return false;
		}

		return update_test();
	}

	void clear(cgv::render::context& ctx)
	{
		spheres.destruct(ctx);
		filtered_positions_buffer.destruct(ctx);
		copy_helper.destruct(ctx);
		filter.destruct(ctx);
		time_query.destruct(ctx);
	}

	void draw(cgv::render::context& ctx)
	{	
		spheres.render(ctx, 0, sphere_render_count);
	}

	void create_gui()
	{
		add_decorator("GPGPU Filter Test", "heading");

		add_member_control(this, "Measure time", measure_time, "check");

		add_member_control(this, "Primitive count", primitive_count, "value_slider", "min=1000;max=10000000;ticks=true");
		connect_copy(add_button("Generate")->click, cgv::signal::rebind(this, &gpgpu_filter_test::update_test));

		connect_copy(add_button("Filter")->click, cgv::signal::rebind(this, &gpgpu_filter_test::apply_filter));

		//std::string options = "min=" + std::to_string(radius_min) + ";max=" + std::to_string(radius_max) + ";step=0.0001;ticks=true";
		/*
		cgv::gui::property_string options;
		options.add("min", radius_min);
		options.add("max", radius_max);
		options.add("step", 0.0001f);
		add_decorator("Radius range", "heading", "level=3");
		add_member_control(this, "Min", threshold_min, "value_slider", options);
		add_member_control(this, "Max", threshold_max, "value_slider", options);
		*/

		if(begin_tree_node("Sphere style", spheres.style, false)) {
			align("\a");
			add_gui("", spheres.style);
			align("\b");
			end_tree_node(spheres.style);
		}
	}

	void apply_filter()
	{
		if(!get_context())
			return;

		cgv::render::context& ctx = *get_context();
		
		if(spheres.empty())
			return;

		if(!filter.is_initialized()) {
			std::cout << "Error: GPU filter algorithm is not initialized" << std::endl;
			return;
		}

		if(!copy_helper.is_initialized()) {
			std::cout << "Error: GPU copy algorithm is not initialized" << std::endl;
			return;
		}

		// Ensure the CPU side buffers are transferred to the GPU buffers before trying to access them with the GPU-based filter.
		cgv::render::sphere_renderer& sr = ref_sphere_renderer(ctx);
		spheres.early_transfer(ctx, sr);

		auto& aam = spheres.ref_attribute_array_manager();
		const cgv::render::vertex_buffer* positions_buffer = sr.get_vertex_buffer_ptr(ctx, aam, "position");
		
		if(positions_buffer) {
			time_query.begin_scope();
			// Filter the sphere positions from the render data and write the result to the scratch filtered positions buffer.
			filter.dispatch(ctx, cgv::gpgpu::begin(*positions_buffer), cgv::gpgpu::end<cgv::vec3>(*positions_buffer), cgv::gpgpu::begin(filtered_positions_buffer));
			// read back the count of filtered (copied) positions.
			size_t filtered_count = 0;
			filter.read_count(ctx, filtered_count);
			double time = time_query.end_scope_and_collect();
			std::cout << "Filtering done in " << (time / 1'000'000.0f) << " ms. Copied " << filtered_count << " elements." << std::endl;

			// Copy the filtered positions back to the sphere render data positions buffer.
			copy_helper.dispatch(ctx, cgv::gpgpu::begin(filtered_positions_buffer), cgv::gpgpu::begin(filtered_positions_buffer) + filtered_count, cgv::gpgpu::begin(*positions_buffer));
			sphere_render_count = filtered_count;
		}
		
		post_redraw();
	}

	bool update_test()
	{
		cgv::render::context* ctx = get_context();
		if(!ctx)
			return false;

		std::uniform_real_distribution<float> snorm_distr(-1.0f, 1.0f);
		const cgv::rgb green(0.0f, 1.0f, 0.0f);
		const cgv::rgb red(1.0f, 0.0f, 0.0f);

		spheres.clear();
		for(int i = 0; i < primitive_count; ++i) {
			cgv::vec3 position(
				snorm_distr(random_engine),
				snorm_distr(random_engine),
				snorm_distr(random_engine)
			);

			float t = 0.5f * snorm_distr(random_engine) + 0.5f;
			float radius = cgv::math::lerp(radius_min, radius_max, t);

			cgv::rgb color = (1.0f - t) * green + t * red;

			spheres.add(position, color, radius);
			//spheres.add_index(i);
		}

		// Create an empty buffer large enough to hold all filtered positions.
		filtered_positions_buffer.create_or_resize(*ctx, sizeof(cgv::vec3) * spheres.positions.size());

		// -1 means to render all spheres
		sphere_render_count = -1;
		
		post_redraw();
		return true;
	}

private:
	std::default_random_engine random_engine;

	int primitive_count = 10000;
	const float radius_min = 0.005f;
	const float radius_max = 0.05f;
	cgv::render::sphere_render_data<> spheres;
	cgv::render::vertex_buffer filtered_positions_buffer;
	int sphere_render_count = -1;

	cgv::gpgpu::copy copy_helper;
	cgv::gpgpu::copy_if filter;
	float threshold_min = radius_min;
	float threshold_max = radius_max;

	bool measure_time = false;
	cgv::render::gl::gl_time_query time_query;
};

#include <cgv/base/register.h>

/// register a factory to create new gpgpu filter tests
cgv::base::factory_registration<gpgpu_filter_test> gpgpu_filter_test_fac("New/GPGPU/Filter test");
