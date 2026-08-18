#include <numeric>
#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/math/constants.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_gl/gl/gl_time_query.h>
#include <cgv_gpgpu/visibility_sort.h>

/// This example illustrates how to use the gpgpu::visibility_sorting class to render transparent objects.
/// 
/// The scene consists of spheres generated uniformly within a cube. Each sphere is assigned a random
/// color and opacity. The visibility sorting routine is set up to take sphere positions together with
/// the view eye position to first calculate the spheres' distances to the eye. Then an index array where
/// each index represents a sphere is sorted by using the distance value as the sort key to arrange the
/// spheres in descending order for every frame. The index array is used for indexed rendering of the
/// spheres in back-to-front order which, together with the back-to-front blend equation will result in
/// correct blending of the transparent spheres.
/// Note that this only governs the render order on a per-object basis and may still produce popping artifacts
/// whenever two spheres overlap and the view angle changes.
class visibility_sorting : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
public:
	visibility_sorting() : cgv::base::node("Visibility Sorting")
	{
		view_ptr = nullptr;
		n = 10000;
		spheres.style.radius = 0.01f;
		spheres.style.surface_color = cgv::rgb(1.0f, 0.5f, 0.2f);
		spheres.style.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
		do_sort = true;
	}

	void on_set(void* member_ptr)
	{
		post_redraw();
		update_member(member_ptr);
	}

	std::string get_type_name() const
	{
		return "visibility_sorting";
	}

	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_sphere_renderer(ctx, -1);
		spheres.destruct(ctx);

		sort.destruct(ctx);

		time_query.destruct(ctx);
	}

	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_sphere_renderer(ctx, 1);
		if(!spheres.init(ctx))
			return false;
		
		// Define the data type of the sphere positions. We have to take care of alignment restrictions of GPU buffers and cannot use vec3 as the type directly.
		// Instead we need to define a proxy by using a struct of three separate floats to mimick the densely packed array from the sphere geometry.
		sl::data_type vec3_t = { "vec3_t", {
			{ sl::Type::Float, "x" },
			{ sl::Type::Float, "y" },
			{ sl::Type::Float, "z" }
		}};

		// Define the operation that transforms sphere positions to distances. This defines a GLSL code snippet that can access teh current element, i.e, sphere position,
		// and the eye position available as the uniform 'u_eye_pos'.
		std::string operation = R"(
			vec3 pos = vec3(element.x, element.y, element.z);
			vec3 eye_to_pos = pos - u_eye_pos;
			return dot(eye_to_pos, eye_to_pos);
		)";

		// Initialize the sorting routine using our custom position type and unsigned integers for indices.
		sort.init(ctx, vec3_t, sl::Type::UInt, 1, operation, cgv::gpgpu::SortOrder::Descending);

		create_data();

		time_query.init(ctx);

		return true;
	}

	void init_frame(cgv::render::context& ctx)
	{
		if(!view_ptr)
			view_ptr = find_view_as_node();
	}

	void draw(cgv::render::context& ctx)
	{	
		if(!view_ptr || spheres.empty())
			return;

		// Setup the render pass to use back-to-front blending
		ctx.push_depth_test_state();
		ctx.disable_depth_test();

		ctx.push_blend_state();
		ctx.enable_blending();
		ctx.set_blend_func_back_to_front();
		
		// Initiate a transfer of the CPU side geometry data into the GPU side buffers.
		// The render_data classes will do this automatically whenever the render method is called.
		// However, we need to access the data before rendering so we manually need to ensure it
		// is available to the GPU.
		auto& sr = ref_sphere_renderer(ctx);
		spheres.early_transfer(ctx, sr);

		// Sort the spheres and measure the duration if requested
		if(do_sort) {
			if(measure_time)
				time_query.begin_scope();

			sort_spheres(ctx, sr);

			if(measure_time) {
				double time = time_query.end_scope_and_collect();
				std::cout << "Sorting done in " << (time / 1'000'000.0f) << " ms -> " << static_cast<float>(n) / (time / 1000.0f) << " M/s" << std::endl;
			}
		}

		spheres.render(ctx);

		ctx.pop_blend_state();
		ctx.pop_depth_test_state();
	}

	void sort_spheres(cgv::render::context& ctx, cgv::render::sphere_renderer& sr) {
		auto& aam = spheres.ref_attribute_array_manager();
		const cgv::render::vertex_buffer* position_buffer_ptr = sr.get_vertex_buffer_ptr(ctx, aam, "position");
		const cgv::render::vertex_buffer* index_buffer_ptr = sr.get_index_buffer_ptr(aam);

		if(position_buffer_ptr && index_buffer_ptr) {
			if(sort.is_initialized())
				sort.execute(ctx, cgv::gpgpu::begin(*position_buffer_ptr), cgv::gpgpu::end<cgv::vec3>(*position_buffer_ptr), cgv::gpgpu::begin(*index_buffer_ptr), view_ptr->get_eye());
			else
				std::cout << "Warning: GPU visibility sort process is not initialized." << std::endl;
		}
	}

	void create_gui()
	{
		add_decorator("Visibility Sorting", "heading");

		add_member_control(this, "N", n, "value_slider", "min=1000;max=10000000;ticks=true");
		connect_copy(add_button("Generate")->click, cgv::signal::rebind(this, &visibility_sorting::create_data));

		add_member_control(this, "Sort", do_sort, "check");
		add_member_control(this, "Measure time", measure_time, "check");

		if(begin_tree_node("Sphere Style", spheres.style, false)) {
			align("\a");
			add_gui("", spheres.style);
			align("\b");
			end_tree_node(spheres.style);
		}
	}

	void create_data()
	{
		auto& ctx = *get_context();

		spheres.clear();

		std::mt19937 rng(42);
		std::uniform_real_distribution<float> pos_distr(-1.0f, 1.0f);
		std::uniform_real_distribution<float> col_distr(0.2f, 0.9f);

		for(unsigned i = 0; i < n; ++i) {
			cgv::vec3 pos(
				pos_distr(rng),
				pos_distr(rng),
				pos_distr(rng)
			);

			cgv::rgba col(
				col_distr(rng),
				col_distr(rng),
				col_distr(rng),
				col_distr(rng)
			);

			spheres.add(pos, col);
		}

		// Compute sphere radius based on metric derived from densest sphere packing
		const float box_volume = 8.0f;
		const float packing_density_percentage = 0.635f; // 63.5% of box volume for random packing of equal spheres
		
		const float sphere_volume = (box_volume * packing_density_percentage) / n;
		const float sphere_radius = std::pow(sphere_volume / (4.0f/3.0f * static_cast<float>(cgv::math::constants::pi)), 1.0f/3.0f); // r = cube_root_of(V / (4/3*pi));
		
		spheres.style.radius = sphere_radius;

		// Make room for enough sphere indices.
		spheres.indices.resize(spheres.size());
		// Fill sphere indices with numbers from 0 to n-1.
		// This is not necessary when sortingas the visibility_sorting algorithm will generate indices, but we still need valid indices in case we do not sort the spheres.
		std::iota(spheres.indices.begin(), spheres.indices.end(), 0);

		// Resize the sort to the new sphere count
		if(!sort.resize(ctx, spheres.indices.size()))
			std::cout << "Error: Could not resize GPU visibility sort!" << std::endl;

		spheres.set_out_of_date();
		post_redraw();
	}

private:
	cgv::render::view* view_ptr = nullptr;

	/// The number of spheres to generate
	unsigned n = 10000;

	/// Store sphere geometry using full RGBA color to store opacity
	cgv::render::sphere_render_data<cgv::rgba> spheres;

	/// A flag indicating if the spheres are to be sorted or not
	bool do_sort = true;

	/// The sorting routine
	cgv::gpgpu::visibility_sort sort;

	bool measure_time = false;
	cgv::render::gl::gl_time_query time_query;
};

#include <cgv/base/register.h>

/// register a factory to create new visibility sorting tests
cgv::base::factory_registration<visibility_sorting> test_visibility_sorting_fac("New/GPGPU/Visibility Sorting");
