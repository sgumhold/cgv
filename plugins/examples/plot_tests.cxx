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
	// persistent vector with plot data
	std::vector<cgv::vec4> P1, P2;

public:
	test_plot3d() : cgv::base::node("3D Plot Test"), plot(1)
	{
		unsigned i, j;
		unsigned pi1 = plot.add_sub_plot("x*y");
		unsigned pi2 = plot.add_sub_plot("x+y");
		plot.set_samples_per_row(pi1, 30);
		plot.set_samples_per_row(pi2, 50);
		for (j = 0; j < 50; ++j) {
			float y = 0.3f * j;
			for (i = 0; i < 30; ++i) {
				float x = 0.5f * i;
				float z = 0.1f * x * y;
				float w = 0.1f * ((x - 7.0f) * (x - 7.0f) + (y - 7.0f) * (y - 3.0f));
				P1.push_back(cgv::vec4(x, y, z, w));
			}
		}
		for (j = 0; j < 30; ++j) {
			float y = 0.5f * j;
			for (i = 0; i < 50; ++i) {
				float x = 0.3f * i;
				float z = 0.1f * ((x - 7.0f) * (x - 7.0f) + (y - 7.0f) * (y - 3.0f));
				float w = 0.1f * x * y;
				P2.push_back(cgv::vec4(x, y, z, w));
			}
		}
		for (unsigned c = 0; c < 4; ++c) {
			plot.set_sub_plot_attribute(0, c, &P1[0][c], P1.size(), sizeof(cgv::vec4));
			plot.set_sub_plot_attribute(1, c, &P2[0][c], P2.size(), sizeof(cgv::vec4));
		}
		plot.set_sub_plot_colors(0, cgv::rgb(1.0f, 0.0f, 0.1f));
		plot.set_sub_plot_colors(1, cgv::rgb(0.1f, 0.0f, 1.0f));

		plot.legend_components = cgv::plot::LegendComponent(cgv::plot::LC_PRIMARY_COLOR + cgv::plot::LC_PRIMARY_OPACITY);
		plot.color_mapping[0] = 3;
		plot.opacity_mapping[0] = 3;

		plot.adjust_domain_to_data();
		plot.adjust_tick_marks();
		plot.adjust_extent_to_domain_aspect_ratio();
	}
	std::string get_type_name() const
	{
		return "test_plot3d";
	}
	void on_set(void* member_ptr)
	{
		//if ((member_ptr >= &plot.ref_domain_min()(0) && member_ptr < &plot.ref_domain_min()(0) + plot.get_dim()) ||
		//	(member_ptr >= &plot.ref_domain_max()(0) && member_ptr < &plot.ref_domain_max()(0) + plot.get_dim())) {
		//	plot.adjust_tick_marks();
		//}
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		plot.set_view_ptr(find_view_as_node());
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
	std::vector<cgv::vec4> P;
	// GPU objects for offline 
	cgv::render::texture tex;
	cgv::render::render_buffer depth;
	cgv::render::frame_buffer fbo;
	// whether to use offscreen rendering
	bool render_offscreen;

public:
	test_plot2d() : cgv::base::node("2D Plot Test"), tex("[R,G,B,A]"), depth("[D]"), plot("trigonometry", 2)
	{
		// compute vector of vec3 with x coordinates and function values of cos and sin
		unsigned i;
		for (i = 1; i < 50; ++i) {
			float x = 0.1f * i;
			P.push_back(cgv::vec4(x, cos(x), sin(x), cos(x)*cos(x)));
		}
		// create two sub plots and configure their colors
		unsigned p1 = plot.add_sub_plot("cos");
		unsigned p2 = plot.add_sub_plot("sin");
		unsigned p3 = plot.add_sub_plot("cos");
		//plot.set_sub_plot_colors(p1, rgb(1.0f, 0.0f, 0.1f)); // will be set later to the attribute with index 2

		plot.set_sub_plot_colors(p2, cgv::rgb(0.1f, 0.0f, 1.0f));
		plot.set_sub_plot_colors(p3, cgv::rgb(0.0f, 1.0f, 0.1f));

		// attach sub plot attributes to previously created vector
		// CAREFUL: this creates references to P and P is not allowed to be deleted thereafter
		plot.set_sub_plot_attribute(p1, 0, &P[0][0], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p1, 1, &P[0][1], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p1, 2, &P[0][2], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p1, 3, &P[0][3], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p2, 0, &P[0][0], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p2, 1, &P[0][2], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p2, 2, &P[0][0], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p2, 3, &P[0][3], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p3, 0, &P[0][0], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p3, 1, &P[0][3], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p3, 2, &P[0][1], P.size(), sizeof(cgv::vec4));
		plot.set_sub_plot_attribute(p3, 3, &P[0][2], P.size(), sizeof(cgv::vec4));

		plot.legend_components = cgv::plot::LegendComponent(cgv::plot::LC_PRIMARY_COLOR + cgv::plot::LC_PRIMARY_OPACITY);

		plot.color_mapping[0] = 2;
		plot.color_scale_index[0] = cgv::media::CS_HUE;
		plot.opacity_mapping[0] = 3;

		plot.ref_sub_plot2d_config(0).set_color_indices(0);
		plot.ref_sub_plot2d_config(0).line_halo_color.color_idx = 0;

		// adjust domain, tick marks and extent in world space (of offline rendering process)
		plot.adjust_domain_to_data();
		plot.adjust_tick_marks();
		plot.adjust_extent_to_domain_aspect_ratio();
		// scale up extent in y-direction where we want to use half the texture resolution
		//vecn ex = plot.get_extent();
		//ex(1) *= 2.0f;
		//plot.set_extent(ex);

		render_offscreen = false;
	}
	void on_set(void* member_ptr)
	{
		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const
	{
		return "test_plot2d";
	}
	bool init(cgv::render::context& ctx)
	{
		// create GPU objects for offline rendering
		tex.create(ctx, cgv::render::TT_2D, 2048, 1024);
		depth.create(ctx, 2048, 1024);
		fbo.create(ctx, 2048, 1024);
		fbo.attach(ctx, depth);
		fbo.attach(ctx, tex);

		plot.set_view_ptr(find_view_as_node());
		// and init plot
		return plot.init(ctx) && fbo.is_complete(ctx);
	}
	void init_frame(cgv::render::context& ctx)
	{
		// first call init frame of plot
		plot.init_frame(ctx);
		if (!fbo.is_created() || !render_offscreen)
			return;

		auto ex = plot.get_extent();
		fbo.enable(ctx);
		fbo.push_viewport(ctx);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(cgv::math::identity4<double>());
		ctx.push_projection_matrix();
		ctx.set_projection_matrix(cgv::math::ortho4<double>(-ex[0]/2, ex[0]/2, -ex[1]/2, ex[1]/2, -1, 1));
		glDepthMask(GL_FALSE);
		plot.draw(ctx);
		glDepthMask(GL_TRUE);
		ctx.pop_projection_matrix();
		ctx.pop_modelview_matrix();

		fbo.pop_viewport(ctx);
		fbo.disable(ctx);
		// generate mipmaps in rendered texture and in case of success enable anisotropic filtering
		if (tex.generate_mipmaps(ctx))
			tex.set_min_filter(cgv::render::TF_ANISOTROP, 16.0f);
	}
	void draw(cgv::render::context& ctx)
	{
		if (render_offscreen && fbo.is_created()) {
			// use default shader with texture support to draw offline rendered plot
			glDisable(GL_CULL_FACE);
			auto& prog = ctx.ref_default_shader_program(true);
			tex.enable(ctx);
			prog.enable(ctx);
			prog.set_uniform(ctx, "gamma", 1.0f);
			ctx.set_color(cgv::rgba(1, 1, 1, 1));
			ctx.push_modelview_matrix();
			ctx.mul_modelview_matrix(cgv::math::scale4<double>(plot.get_extent()(0)/2, plot.get_extent()(1)/2, 1.0));
			ctx.tesselate_unit_square();
			ctx.pop_modelview_matrix();
			prog.disable(ctx);
			tex.disable(ctx);
			glEnable(GL_CULL_FACE);
		}
		else
			plot.draw(ctx);
	}
	void create_gui()
	{
		add_decorator("plot2d Test", "heading");
		add_member_control(this, "Render Offscreen", render_offscreen, "toggle");
		plot.create_gui(this, *this);
	}
};

#include <cgv/base/register.h>

/// register a factory to create new cubes
cgv::base::factory_registration<test_plot2d> test_plot2d_fac("New/Demo/Test Plot2d", '2');
cgv::base::factory_registration<test_plot3d> test_plot3d_fac("New/Demo/Test Plot3d", '3');
