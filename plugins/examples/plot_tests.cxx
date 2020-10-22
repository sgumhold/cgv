#include <cgv/base/node.h>
#include <plot/plot2d.h>
#include <plot/plot3d.h>
#include <cgv/math/ftransform.h>
#include <cgv/render/drawable.h>
#include <cgv/render/render_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/texture.h>
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
			float y = 0.3f * j;
			for (i = 0; i < 30; ++i) {
				float x = 0.5f * i;
				float z = 0.1f * x * y;
				P1.push_back(vec3(x, y, z));
			}
		}
		for (j = 0; j < 30; ++j) {
			float y = 0.5f * j;
			for (i = 0; i < 50; ++i) {
				float x = 0.3f * i;
				float z = 0.1f * ((x - 7.0f) * (x - 7.0f) + (y - 7.0f) * (y - 3.0f));
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
	// plot that can manage several 2d sub plots
	cgv::plot::plot2d plot;
	// persistent vector with plot data
	std::vector<vec3> P;
	// GPU objects for offline 
	cgv::render::texture tex;
	cgv::render::render_buffer depth;
	cgv::render::frame_buffer fbo;
public:
	test_plot2d() : cgv::base::node("2d plot tester"), tex("[R,G,B,A]"), depth("[D]")
	{
		// compute vector of vec3 with x coordinates and function values of cos and sin
		unsigned i;
		for (i = 0; i < 50; ++i) {
			float x = 0.1f * i;
			P.push_back(vec3(x, cos(x), sin(x)));
		}

		// create two sub plots and configure their colors
		unsigned p1 = plot.add_sub_plot("cos");
		unsigned p2 = plot.add_sub_plot("sin");
		plot.set_sub_plot_colors(p1, rgb(1.0f, 0.0f, 0.1f));
		plot.set_sub_plot_colors(p2, rgb(0.1f, 0.0f, 1.0f));

		// attach sub plot attributes to previously created vector
		// CAREFUL: this creates references to P and P is not allowed to be deleted thereafter
		plot.set_sub_plot_attribute(p1, 0, &P[0][0], P.size(), sizeof(vec3));
		plot.set_sub_plot_attribute(p1, 1, &P[0][1], P.size(), sizeof(vec3));
		plot.set_sub_plot_attribute(p2, 0, &P[0][0], P.size(), sizeof(vec3));
		plot.set_sub_plot_attribute(p2, 1, &P[0][2], P.size(), sizeof(vec3));

		// adjust domain, tick marks and extent in world space (of offline rendering process)
		plot.adjust_domain_to_data();
		plot.adjust_tick_marks_to_domain();
		plot.set_extent(vecn(1.8f, 1.8f));
		plot.adjust_extent_to_domain_aspect_ratio();
		// scale up extent in y-direction where we want to use half the texture resolution
		vecn ex = plot.get_extent();
		ex(1) *= 2.0f;
		plot.set_extent(ex);
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
	std::string get_type_name() const
	{
		return "test_plot2d";
	}
	bool init(cgv::render::context& ctx)
	{
		// create GPU objects for offline rendering
		tex.create(ctx, cgv::render::TT_2D, 1024, 512);
		depth.create(ctx, 1024, 512);
		fbo.create(ctx, 1024, 512);
		fbo.attach(ctx, depth);
		fbo.attach(ctx, tex);

		// and init plot
		return plot.init(ctx) && fbo.is_complete(ctx);
	}
	void init_frame(cgv::render::context& ctx)
	{
		// first call init frame of plot
		plot.init_frame(ctx);
		
		if (!fbo.is_created())
			return;
		
		// if fbo is created, perform offline rendering with world space in the range [-1,1]² and white background
		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(cgv::math::identity4<double>());
		ctx.push_projection_matrix();
		ctx.set_projection_matrix(cgv::math::identity4<double>());
		fbo.enable(ctx);
		fbo.push_viewport(ctx);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		plot.draw(ctx);
		fbo.pop_viewport(ctx);
		fbo.disable(ctx);
		ctx.pop_projection_matrix();
		ctx.pop_modelview_matrix();

		// generate mipmaps in rendered texture and in case of success enable anisotropic filtering
		if (tex.generate_mipmaps(ctx))
			tex.set_min_filter(cgv::render::TF_ANISOTROP, 16.0f);
	}
	void draw(cgv::render::context& ctx)
	{
		// use default shader with texture support to draw offline rendered plot
		glDisable(GL_CULL_FACE);
		auto& prog = ctx.ref_default_shader_program(true);
		tex.enable(ctx);
		prog.enable(ctx);
		ctx.set_color(rgba(1, 1, 1, 1));
		ctx.push_modelview_matrix();
		// scale down in y-direction according to texture resolution
		ctx.mul_modelview_matrix(cgv::math::scale4<double>(1.0, 0.5, 1.0));
		ctx.tesselate_unit_square();
		ctx.pop_modelview_matrix();
		prog.disable(ctx);
		tex.disable(ctx);
		glEnable(GL_CULL_FACE);
	}
	void create_gui()
	{
		plot.create_gui(this, *this);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<test_plot2d> test_plot2d_fac("new/demo/test_plot2d", '2');
cgv::base::factory_registration<test_plot3d> test_plot3d_fac("new/demo/test_plot3d", '3');
