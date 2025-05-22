#include <numeric>
#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/math/constants.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_gl/gl/gl_time_query.h>
#include <cgv_gpgpu/visibility_sort.h>

#include <cgv_gpgpu/radix_sort_4x.h>
#include <cgv_gpgpu/radix_sort_onesweep.h>
#include <cgv_gpgpu/sequence.h>
#include <cgv_gpgpu/transform.h>

class visibility_sorting : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
protected:
	cgv::render::view* view_ptr = nullptr;

	unsigned n = 10000;

	cgv::render::sphere_render_style sphere_style;
	cgv::render::sphere_render_data<cgv::rgba> spheres;

	bool do_sort = true;

	cgv::gpgpu::visibility_sort visibility_sorter;

	bool use_new_algorithms = false;

	cgv::gpgpu::transform distance_transform;
	cgv::gpgpu::sequence generate_indices;
	cgv::gpgpu::radix_sort* sort = nullptr;
	cgv::render::vertex_buffer distance_buffer;

	bool measure_time = false;
	cgv::render::gl::gl_time_query time_query;

public:
	visibility_sorting() : cgv::base::node("Visibility Sorting Test")
	{
		view_ptr = nullptr;
		n = 10000;
		sphere_style.radius = 0.01f;
		sphere_style.surface_color = cgv::rgb(1.0f, 0.5f, 0.2f);
		sphere_style.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
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

		visibility_sorter.destruct(ctx);

		distance_transform.destruct(ctx);
		generate_indices.destruct(ctx);
		sort->destruct(ctx);
		delete sort;

		time_query.destruct(ctx);
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_sphere_renderer(ctx, 1);
		if(!spheres.init(ctx))
			return false;
		
		visibility_sorter.set_sort_order(cgv::gpgpu::visibility_sort::SO_DESCENDING);

		sl::data_type vec3_t = { "vec3_t", {
			{ sl::Type::kFloat, "x" },
			{ sl::Type::kFloat, "y" },
			{ sl::Type::kFloat, "z" }
		}};

		cgv::gpgpu::argument_definitions arguments = { { sl::Type::kVec3, "u_eye_pos" } };

		std::string operation = R"(
			vec3 pos = vec3(element.x, element.y, element.z);
			vec3 eye_to_pos = pos - u_eye_pos;
			return dot(eye_to_pos, eye_to_pos);
		)";

		sort = new cgv::gpgpu::radix_sort_onesweep();

		distance_transform.init(ctx, vec3_t, { sl::Type::kFloat }, arguments, operation);

		generate_indices.init(ctx, sl::Type::kUInt);

		create_data();

		view_ptr = find_view_as_node();

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

		ctx.push_depth_test_state();
		ctx.disable_depth_test();

		ctx.push_blend_state();
		ctx.enable_blending();
		ctx.set_blend_func_back_to_front();
		
		auto& sr = ref_sphere_renderer(ctx);
		spheres.early_transfer(ctx, sr);

		if(do_sort) {
			if(measure_time)
				time_query.begin_scope();

			sort_spheres(ctx, sr);

			if(measure_time) {
				double time = time_query.end_scope_and_collect();
				std::cout << "Sorting done in " << (time / 1'000'000.0f) << " ms -> " << static_cast<float>(n) / (time / 1000.0f) << " M/s" << std::endl;
			}
		}

		spheres.render(ctx, sr, sphere_style);

		ctx.pop_blend_state();
		ctx.pop_depth_test_state();
	}
	void sort_spheres(cgv::render::context& ctx, cgv::render::sphere_renderer& sr) {
		auto& aam = spheres.ref_attribute_array_manager();
		const cgv::render::vertex_buffer* position_buffer_ptr = sr.get_vertex_buffer_ptr(ctx, aam, "position");
		const cgv::render::vertex_buffer* index_buffer_ptr = sr.get_index_buffer_ptr(aam);

		if(position_buffer_ptr && index_buffer_ptr) {
			if(use_new_algorithms) {
				if(distance_transform.is_initialized()) {
					cgv::gpgpu::argument_binding_list arguments = {
						{ "u_eye_pos", cgv::vec3(view_ptr->get_eye()) }
					};

					distance_transform.dispatch(ctx, *position_buffer_ptr, distance_buffer, spheres.indices.size(), arguments);
				} else {
					std::cout << "Warning: GPU transform is not initialized." << std::endl;
				}

				if(generate_indices.is_initialized())
					generate_indices.dispatch(ctx, *index_buffer_ptr, spheres.indices.size(), 0u, 1u);
				else
					std::cout << "Warning: GPU sequence is not initialized." << std::endl;

				if(sort->is_initialized())
					sort->dispatch(ctx, distance_buffer, *index_buffer_ptr);
				else
					std::cout << "Warning: GPU sort is not initialized." << std::endl;
			} else {
				if(visibility_sorter.is_initialized())
					visibility_sorter.execute(ctx, *position_buffer_ptr, *index_buffer_ptr, view_ptr->get_eye(), view_ptr->get_view_dir());
				else
					std::cout << "Warning: GPU visibility sort is not initialized." << std::endl;
			}
		}
	}
	void create_gui()
	{
		add_decorator("Visibility Sorting", "heading");

		add_member_control(this, "N", n, "value_slider", "min=1000;max=10000000;ticks=true");
		connect_copy(add_button("Generate")->click, cgv::signal::rebind(this, &visibility_sorting::create_data));

		add_member_control(this, "Sort", do_sort, "check");
		add_member_control(this, "Use new algorithms", use_new_algorithms, "check");
		add_member_control(this, "Measure time", measure_time, "check");

		if(begin_tree_node("Sphere Style", sphere_style, false)) {
			align("\a");
			add_gui("", sphere_style);
			align("\b");
			end_tree_node(sphere_style);
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
		const float sphere_radius = std::pow(sphere_volume / (4.0f/3.0f * PI), 1.0f/3.0f); // r = cube_root_of(V / (4/3*pi));
		
		sphere_style.radius = sphere_radius;

		// Make room for enough sphere indices.
		spheres.indices.resize(spheres.size());
		// Fill sphere indices with numbers from 0 to n-1.
		// This is not necessary when sorting, but we still need valid indices in case we do not sort the spheres.
		std::iota(spheres.indices.begin(), spheres.indices.end(), 0);

		if(!visibility_sorter.init(ctx, spheres.indices.size()))
			std::cout << "Error: Could not initialize GPU sorter!" << std::endl;

		if(!sort->init(ctx, sl::Type::kFloat, sl::Type::kUInt, cgv::gpgpu::radix_sort::Order::kDescending, spheres.indices.size()))
			std::cout << "Error: Could not initialize GPU sort!" << std::endl;

		distance_buffer.create_or_resize(ctx, sizeof(uint32_t) * spheres.indices.size());

		spheres.set_out_of_date();
		post_redraw();
	}
};

#include <cgv/base/register.h>

/// register a factory to create new visibility sorting tests
cgv::base::factory_registration<visibility_sorting> test_visibility_sorting_fac("New/GPGPU/Visibility Sorting");
