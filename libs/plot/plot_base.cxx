#include "plot_base.h"
#include <cgv/render/shader_program.h>

namespace cgv {
	namespace plot {

axis_config::axis_config()
{
	log_scale = false;
	ticks[0].step = 1;
	ticks[0].type = TT_DASH;
	ticks[1].step = 5;
	ticks[1].type = TT_DASH;
}

plot_base_config::plot_base_config()
{
	show_plot = true;

	show_points = false;
	point_size = 3;
	point_color = Clr(1,0,0);

	show_sticks = false;
	stick_width = 2;
	stick_color = Clr(1,1,0);

	show_bars = false;
	bar_percentual_width = 0.5f;
	bar_outline_width = 1;
	bar_color = Clr(0,1,1);
	bar_outline_color = Clr(1,1,1);
}

plot_base_config::~plot_base_config()
{
}

plot_base::plot_base()
{
	plot_scale = 1;
	show_axes = true;
	axis_line_width = 2;
	axis_color = Clr(0.5f,0.5f,0.5f);
	tick_line_width[0] = 1;
	tick_line_width[1] = 2;
	tick_length[0] = 1;
	tick_length[1] = 1.5f;
	label_ticks[0] = true;
	label_ticks[1] = true;
}

unsigned plot_base::get_nr_sub_plots() const
{
	return configs.size();
}

void plot_base::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i)
{
	prog.set_uniform(ctx, "point_color", (cgv::math::fvec<Clr::component_type, Clr::nr_components>&) ref_sub_plot_config(i).point_color);
	prog.set_uniform(ctx, "stick_color", (cgv::math::fvec<Clr::component_type, Clr::nr_components>&) ref_sub_plot_config(i).stick_color);
	prog.set_uniform(ctx, "bar_color", (cgv::math::fvec<Clr::component_type, Clr::nr_components>&) ref_sub_plot_config(i).bar_color);
	prog.set_uniform(ctx, "bar_outline_color", (cgv::math::fvec<Clr::component_type, Clr::nr_components>&) ref_sub_plot_config(i).bar_outline_color);

	prog.set_uniform(ctx, "plot_scale", plot_scale);
}

plot_base_config& plot_base::ref_sub_plot_config(unsigned i)
{
	return *configs[i];
}

void plot_base::create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	const char* axis_names = "xyz";

	bool open = p.begin_tree_node("axes config", show_axes, false, "level=3;w=100;align=' '");
	p.add_member_control(bp, "axes", show_axes, "toggle", "w=50");
	if (open) {
		p.align("\a");
			p.add_member_control(bp, "width", axis_line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "color", axis_color);
			for (unsigned i=0; i<get_nr_axes(); ++i) {
				axis_config& ac = ref_axis_config(i);
				p.add_decorator(std::string(1, axis_names[i])+" axis", "heading", "level=3");
				p.add_member_control(bp, "log_scale", ac.log_scale, "toggle");
				p.add_member_control(bp, "primary tick_step", ac.ticks[0].step, "value");
				p.add_member_control(bp, "primary tick_type", ac.ticks[0].type, "dropdown", "enums='none,dash,line,plane'");
				p.add_member_control(bp, "secondary tick_step", ac.ticks[1].step, "value");
				p.add_member_control(bp, "secondary tick_type", ac.ticks[1].type, "dropdown", "enums='none,dash,line,plane'");
			}
		p.align("\b");
		p.end_tree_node(show_axes);
	}

	if (p.begin_tree_node("ticks config", tick_line_width[0], false, "level=3")) {
		p.align("\a");
			p.add_decorator("primary ticks", "heading", "level=3");
			p.add_member_control(bp, "width", tick_line_width[0], "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "length", tick_length[0], "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "label", label_ticks[0], "check");

			p.add_decorator("secondary ticks", "heading", "level=3");
			p.add_member_control(bp, "width", tick_line_width[1], "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "length", tick_length[1], "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "label", label_ticks[1], "check");
		p.align("\b");
		p.end_tree_node(tick_line_width[0]);
	}
}

void plot_base::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot_base_config& pbc = ref_sub_plot_config(i);

	p.add_decorator("points", "heading", "level=3;w=100", " ");
	p.add_member_control(bp, "show", pbc.show_points, "toggle", "w=50");
	p.add_member_control(bp, "size", pbc.point_size, "value_slider", "min=1;max=20;log=true;ticks=true");
	p.add_member_control(bp, "color", pbc.point_color);

	p.add_decorator("sticks", "heading", "level=3;w=100", " ");
	p.add_member_control(bp, "show", pbc.show_sticks, "toggle", "w=50");
	p.add_member_control(bp, "width", pbc.stick_width, "value_slider", "min=1;max=20;log=true;ticks=true");
	p.add_member_control(bp, "color", pbc.stick_color);

	p.add_decorator("bars", "heading", "level=3;w=100", " ");
	p.add_member_control(bp, "show", pbc.show_bars, "toggle", "w=50");
	p.add_member_control(bp, "width", pbc.bar_percentual_width, "value_slider", "min=0.01;max=1;log=true;ticks=true");
	p.add_member_control(bp, "fill", pbc.bar_color);
	p.add_member_control(bp, "outline_width", pbc.bar_outline_width, "value_slider", "min=0;max=20;log=true;ticks=true");
	p.add_member_control(bp, "outline", pbc.bar_outline_color);
}

void plot_base::create_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	create_plot_gui(bp, p);
	if (p.begin_tree_node("configure sub plots", configs, false, "level=3")) {
		p.align("\a");
		for (unsigned i=0; i<get_nr_sub_plots(); ++i) {
			plot_base_config& pbc = ref_sub_plot_config(i);
			bool open = p.begin_tree_node(pbc.name, pbc.name, false, "level=3;w=100;align=' '");
			p.add_member_control(bp, "show", pbc.show_plot, "toggle", "w=50");
			if (open) {
				p.align("\a");
				create_config_gui(bp, p, i);
				p.align("\b");
				p.end_tree_node(pbc.name);
			}
		}
		p.align("\b");
		p.end_tree_node(configs);
	}
}


	}
}