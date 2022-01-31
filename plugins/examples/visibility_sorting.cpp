#include <random>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv_glutil/sphere_render_data.h>
#include <cgv_glutil/gpu_sorter.h>
#include <cgv_glutil/radix_sort_4way.h>

class visibility_sorting : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
protected:
	cgv::render::view* view_ptr;

	unsigned n;

	cgv::render::sphere_render_style sphere_style;
	cgv::glutil::sphere_render_data<rgba> rd;

	cgv::glutil::gpu_sorter* sorter;
	bool do_sort;

public:
	visibility_sorting() : cgv::base::node("visibility sorting test")
	{
		view_ptr = nullptr;
		n = 10000;
		sphere_style.radius = 0.01f;
		sphere_style.surface_color = rgb(1.0f, 0.5f, 0.2f);
		sphere_style.map_color_to_material = CM_COLOR_AND_OPACITY;
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
		rd.destruct(ctx);

		delete sorter;
		sorter = nullptr;
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_sphere_renderer(ctx, 1);
		if(!rd.init(ctx))
			return false;
		
		sorter = new cgv::glutil::radix_sort_4way();
		sorter->set_sort_order(cgv::glutil::gpu_sorter::SO_DESCENDING);

		create_data();

		view_ptr = find_view_as_node();

		return true;
	}
	void init_frame(cgv::render::context& ctx)
	{
		if(!view_ptr)
			view_ptr = find_view_as_node();
	}
	void draw(cgv::render::context& ctx)
	{	
		if(!view_ptr || rd.ref_pos().size() == 0) return;

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		// use back-to-front blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto& sr = ref_sphere_renderer(ctx);
		rd.early_transfer(ctx, sr);

		int pos_handle = 0;
		int idx_handle = 0;
		auto& aam = rd.ref_aam();
		
		pos_handle = sr.get_vbo_handle(ctx, aam, "position");
		idx_handle = sr.get_index_buffer_handle(aam);
		

		if(pos_handle > 0 && idx_handle > 0 && do_sort) {
			sorter->sort(ctx, pos_handle, idx_handle, view_ptr->get_eye(), view_ptr->get_view_dir());
		}

		rd.render(ctx, sr, sphere_style);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void create_gui()
	{
		add_decorator("visibility sorting", "heading");

		add_member_control(this, "n", n, "value_slider", "min=1000;max=10000000;ticks=true");
		connect_copy(add_button("generate")->click, cgv::signal::rebind(this, &visibility_sorting::create_data));

		add_member_control(this, "sort", do_sort, "check");

		if(begin_tree_node("sphere style", sphere_style, true)) {
			align("\a");
			add_gui("", sphere_style);
			align("\b");
			end_tree_node(sphere_style);
		}
	}
	void create_data()
	{
		auto& ctx = *get_context();

		rd.clear();

		std::mt19937 rng(42);
		std::uniform_real_distribution<float> pos_distr(-1.0f, 1.0f);
		std::uniform_real_distribution<float> col_distr(0.2f, 0.9f);

		for(unsigned i = 0; i < n; ++i) {
			float x = 0.5f * static_cast<float>(i + 1);
			float y = 0.3f * static_cast<float>(i);

			vec3 pos(
				pos_distr(rng),
				pos_distr(rng),
				pos_distr(rng)
			);

			rgba col(
				col_distr(rng),
				col_distr(rng),
				col_distr(rng),
				col_distr(rng)
			);

			rd.add(pos);
			rd.add(col);
		}

		rd.ref_idx().resize(rd.ref_pos().size());

		if(!sorter->init(ctx, rd.ref_idx().size()))
			std::cout << "Could not initialize GPU sorter" << std::endl;

		rd.set_out_of_date();
		post_redraw();
	}
};

#include <cgv/base/register.h>

/// register a factory to create new visibility sorting tests
cgv::base::factory_registration<visibility_sorting> test_visibility_sorting_fac("new/GPGPU/visibility_sorting");
