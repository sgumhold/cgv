#include <cgv/base/node.h>
#include <plot/plot2d.h>
#include <plot/plot3d.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/provider.h>

class test_plot3d : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider
{
protected:
	cgv::plot::plot3d plot;
public:
	test_plot3d() : cgv::base::node("3d plot tester")
	{
		unsigned i, j;
		unsigned pi1 = plot.add_sub_plot("x*y");
		unsigned pi2 = plot.add_sub_plot("x²+y²");
		std::vector<vec3>& P1 = plot.ref_sub_plot_samples(pi1);
		std::vector<vec3>& P2 = plot.ref_sub_plot_samples(pi2);
		plot.set_samples_per_row(pi1, 30);
		plot.set_samples_per_row(pi2, 50);
		for (j = 0; j < 50; ++j) {
			float y = 0.3f*j;
			for (i = 0; i < 30; ++i) {
				float x = 0.5f*i;
				float z = 0.1f*x * y;
				P1.push_back(vec3(x, y, z));
			}
		}
		for (j = 0; j < 30; ++j) {
			float y = 0.5f*j;
			for (i = 0; i < 50; ++i) {
				float x = 0.3f*i;
				float z = 0.1f*((x-7.0f)*(x-7.0f)+(y-7.0f)*(y-3.0f));
				P2.push_back(vec3(x, y, z));
			}
		}
		plot.set_sub_plot_colors(0, rgb(1.0f, 0.0f, 0.1f));
		plot.set_sub_plot_colors(1, rgb(0.1f, 0.0f, 1.0f));

		plot.adjust_domain_to_data();
		plot.adjust_tick_marks_to_domain();
		plot.adjust_extent_to_domain_aspect_ratio();
	}
	std::string get_type_name() const
	{
		return "test_plot3d";
	}
	void on_set(void* member_ptr)
	{
		if ((member_ptr >= &plot.ref_domain_min()(0) && member_ptr < &plot.ref_domain_min()(0) + plot.get_dim()) ||
			(member_ptr >= &plot.ref_domain_max()(0) && member_ptr < &plot.ref_domain_max()(0) + plot.get_dim())) {
			plot.adjust_tick_marks_to_domain();
		}
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		return plot.init(ctx);
	}
	void init_frame(cgv::render::context& ctx)
	{
		plot.init_frame(ctx);
	}
	void draw(cgv::render::context& ctx)
	{
		plot.draw(ctx);
	}
	void create_gui()
	{
		plot.create_gui(this, *this);
	}
};

class test_plot2d : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider
{
protected:
	cgv::plot::plot2d plot;
	std::vector<vec3> P;
public:
	test_plot2d() : cgv::base::node("2d plot tester")
	{
		unsigned i;
		for (i = 0; i < 50; ++i) {
			float x = 0.1f*i;
			P.push_back(vec3(x, cos(x), sin(x)));
		}
		unsigned p1 = plot.add_sub_plot("cos");
		unsigned p2 = plot.add_sub_plot("sin");

		plot.set_sub_plot_colors(p1, rgb(1.0f, 0.0f, 0.1f));
		plot.set_sub_plot_colors(p2, rgb(0.1f, 0.0f, 1.0f));

		plot.set_sub_plot_attribute(p1, 0, &P[0][0], P.size(), sizeof(vec3));
		plot.set_sub_plot_attribute(p1, 1, &P[0][1], P.size(), sizeof(vec3));
		plot.set_sub_plot_attribute(p2, 0, &P[0][0], P.size(), sizeof(vec3));
		plot.set_sub_plot_attribute(p2, 1, &P[0][2], P.size(), sizeof(vec3));

		plot.adjust_domain_to_data();
		plot.adjust_tick_marks_to_domain();
		plot.adjust_extent_to_domain_aspect_ratio();
	}
	void on_set(void* member_ptr)
	{
		if ( (member_ptr >= &plot.ref_domain_min()(0) && member_ptr < &plot.ref_domain_min()(0) + plot.get_dim()) ||
			 (member_ptr >= &plot.ref_domain_max()(0) && member_ptr < &plot.ref_domain_max()(0) + plot.get_dim()) ) {
			plot.adjust_tick_marks_to_domain();		
		}
		update_member(member_ptr);
		post_redraw();
	}
	std::string get_type_name() const
	{
		return "test_plot2d";
	}
	bool init(cgv::render::context& ctx)
	{
		return plot.init(ctx);
	}
	void init_frame(cgv::render::context& ctx)
	{
		plot.init_frame(ctx);
	}
	void draw(cgv::render::context& ctx)
	{
		plot.draw(ctx);
	}
	void create_gui()
	{
		plot.create_gui(this, *this);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
extern cgv::base::factory_registration<test_plot2d> test_plot2d_fac("new/test_plot2d", '2');
extern cgv::base::factory_registration<test_plot3d> test_plot3d_fac("new/test_plot3d", '3');
