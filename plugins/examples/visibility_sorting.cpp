#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_render_data.h>
#include <cgv_gl/gl/gl_time_query.h>
#include <cgv_gpgpu/visibility_sort.h>

class visibility_sorting : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
protected:
	cgv::render::view* view_ptr;

	unsigned n;

	cgv::render::sphere_render_style sphere_style;
	cgv::render::sphere_render_data<cgv::rgba> spheres;

	cgv::gpgpu::visibility_sort visibility_sorter;
	bool do_sort;
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

		time_query.destruct(ctx);
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_sphere_renderer(ctx, 1);
		if(!spheres.init(ctx))
			return false;
		
		visibility_sorter.set_sort_order(cgv::gpgpu::visibility_sort::SO_DESCENDING);

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
		if(!view_ptr || spheres.empty()) return;

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		// use back-to-front blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto& sr = ref_sphere_renderer(ctx);
		spheres.early_transfer(ctx, sr);

		auto& aam = spheres.ref_attribute_array_manager();
		const cgv::render::vertex_buffer* position_buffer_ptr = sr.get_vertex_buffer_ptr(ctx, aam, "position");
		const cgv::render::vertex_buffer* index_buffer_ptr = sr.get_index_buffer_ptr(aam);
		
		if(visibility_sorter.is_initialized()) {
			if(position_buffer_ptr && index_buffer_ptr && do_sort) {
				time_query.begin();
				visibility_sorter.execute(ctx, *position_buffer_ptr, *index_buffer_ptr, view_ptr->get_eye(), view_ptr->get_view_dir());
				double time = time_query.end();
				std::cout << "Sorting done in " << (time / 1'000'000.0f) << " ms -> " << static_cast<float>(n) / (time / 1000.0f) << " M/s" << std::endl;
			}
		} else {
			std::cout << "Warning: GPU visibility sort is not initialized." << std::endl;
		}

		spheres.render(ctx, sr, sphere_style);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void create_gui()
	{
		add_decorator("Visibility Sorting", "heading");

		add_member_control(this, "N", n, "value_slider", "min=1000;max=10000000;ticks=true");
		connect_copy(add_button("Generate")->click, cgv::signal::rebind(this, &visibility_sorting::create_data));

		add_member_control(this, "Sort", do_sort, "check");

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

		spheres.indices.resize(spheres.size());

		if(!visibility_sorter.init(ctx, spheres.indices.size()))
			std::cout << "Error: Could not initialize GPU sorter!" << std::endl;

		spheres.set_out_of_date();
		post_redraw();
	}
};

#include <cgv/base/register.h>

/// register a factory to create new visibility sorting tests
cgv::base::factory_registration<visibility_sorting> test_visibility_sorting_fac("New/GPGPU/Visibility Sorting");
