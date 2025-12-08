#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/property_string.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_gl/gl/gl_time_query.h>
#include <cgv_gpgpu/select_if.h>

/// This example illustrates how to perform stream compaction, i.e. filtering of an array, on the GPU using the gpgpu::select_if algorithm.
class stream_compaction : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
public:
	stream_compaction() : cgv::base::node("Stream Compaction")
	{
		random_engine.seed(42);
		spheres.style.surface_color = cgv::rgb(0.5f);
		spheres.style.map_color_to_material = cgv::render::CM_COLOR;
	}

	std::string get_type_name() const {
		return "stream_compaction";
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

		// Define the value type to match the radius values in floating point format.
		sl::data_type value_type = sl::Type::kFloat;

		// Define the arguments used by the predicate. Since the predicate depends on uniform, outside variables we have to define them before usage.
		// The following defines two uniform single-valued arguments of type float. The arguments are then available to use in the predicate functin by their given name.
		// Is is considered good practice to give arguments a prefix, here "a_", to differentiate them from other identifiers.
		// In general, arguments can be of type uniform, buffer, texture or image.
		cgv::gpgpu::argument_definitions arguments = {
			{ sl::Type::kFloat, "a_radius_min" },
			{ sl::Type::kFloat, "a_radius_max" }
		};

		// Define a boolean predicate that returns true for all input elements we want to keep and copy to the output buffer.
		// This string defines the body of a function with signature bool(value_type element, uint index), where value_type is equal to the value_type as given to the init method of copy_if.
		// The content needs to follow GLSL syntax.
		// In this case, value_type is equal to float as defined in the constructor and the predicate will return true for all radii
		// that are in the range a_radius_min to a_radius_max inclusive.
		std::string predicate = R"(
			return a_radius_min <= element && element <= a_radius_max;
		)";

		// Initialize the filter (select_if) using the given value type, arguments and predicate.
		if(!filter.init(ctx, sl::Type::kFloat, arguments, predicate)) {
			std::cout << "Error: could not initialize GPU filter algorithm" << std::endl;
			return false;
		}

		return create_spheres();
	}

	void clear(cgv::render::context& ctx)
	{
		spheres.destruct(ctx);
		filter.destruct(ctx);
		time_query.destruct(ctx);
	}

	void draw(cgv::render::context& ctx)
	{	
		apply_filter();
		spheres.render(ctx, 0, sphere_render_count);
	}

	void create_gui()
	{
		add_decorator("GPGPU Filtering", "heading");

		add_member_control(this, "Measure time", measure_time, "check");

		add_member_control(this, "Primitive count", primitive_count, "value_slider", "min=1000;max=1000000;ticks=true");
		connect_copy(add_button("Generate")->click, cgv::signal::rebind(this, &stream_compaction::create_spheres));

		cgv::gui::property_string options;
		options.add("min", radius_min);
		options.add("max", radius_max);
		options.add("step", 0.0001f);
		add_decorator("Radius range", "heading", "level=3");
		add_member_control(this, "Min", threshold_min, "value_slider", options);
		add_member_control(this, "Max", threshold_max, "value_slider", options);
		
		if(begin_tree_node("Sphere style", spheres.style, false)) {
			align("\a");
			add_gui("", spheres.style);
			align("\b");
			end_tree_node(spheres.style);
		}
	}

private:
	bool create_spheres() {
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
			cgv::rgb color = cgv::media::lerp(green, red, t);

			spheres.add(position, color, radius);
			// Add indices that are later filtered and used to draw the reduced amount of spheres.
			spheres.add_index(i);
		}

		// A value of -1 means to render all spheres.
		sphere_render_count = -1;

		post_redraw();
		return true;
	}

	void apply_filter() {
		if(!get_context())
			return;

		cgv::render::context& ctx = *get_context();

		if(spheres.empty())
			return;

		if(!filter.is_initialized()) {
			std::cout << "Error: GPU filter algorithm is not initialized" << std::endl;
			return;
		}

		// Ensure the CPU side buffers are transferred to the GPU buffers before trying to access them with the GPU-based filter.
		cgv::render::sphere_renderer& sr = ref_sphere_renderer(ctx);
		spheres.early_transfer(ctx, sr);

		auto& aam = spheres.ref_attribute_array_manager();
		const cgv::render::vertex_buffer* radius_buffer = sr.get_vertex_buffer_ptr(ctx, aam, "radius");
		const cgv::render::vertex_buffer* index_buffer = sr.get_index_buffer_ptr(aam);

		if(!radius_buffer || !index_buffer) {
			std::cout << "Error: could not get required buffers for filtering" << std::endl;
			return;
		}

		// Bind the required arguments. The argument names have to match the names used in the argument definition during initialization of the algorithm.
		// The argument_binding_list determines the bindings between named arguments and their actual values.
		cgv::gpgpu::argument_binding_list arguments;
		arguments.bind_uniform("a_radius_min", threshold_min);
		arguments.bind_uniform("a_radius_max", threshold_max);

		if(measure_time)
			time_query.begin_scope();

		// Filter the spheres by their radii and write the resulting indices to the index buffer.
		filter.dispatch(ctx, cgv::gpgpu::begin(*radius_buffer), cgv::gpgpu::end<cgv::vec3>(*radius_buffer), cgv::gpgpu::begin(*index_buffer), arguments);
		// Read back the count of filtered indices.
		size_t filtered_count = 0;
		filter.read_count(ctx, filtered_count);
		if(measure_time) {
			double time = time_query.end_scope_and_collect();
			std::cout << "Filtering done in " << (time / 1'000'000.0f) << " ms. Copied " << filtered_count << " elements." << std::endl;
		}

		sphere_render_count = (int)filtered_count;
	}

	std::default_random_engine random_engine;

	int primitive_count = 10000;
	const float radius_min = 0.005f;
	const float radius_max = 0.05f;
	cgv::render::sphere_render_data<> spheres;
	int sphere_render_count = -1;

	cgv::gpgpu::select_if filter;
	float threshold_min = radius_min;
	float threshold_max = radius_max;

	bool measure_time = false;
	cgv::render::gl::gl_time_query time_query;
};

#include <cgv/base/register.h>

/// register a factory to create new stream_compaction instances
cgv::base::factory_registration<stream_compaction> stream_compaction_fac("New/GPGPU/Stream Compaction");
