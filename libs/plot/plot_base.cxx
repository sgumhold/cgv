#include "plot_base.h"
#include <cgv/render/shader_program.h>
#include <cgv/signal/rebind.h>

namespace cgv {
	namespace plot {

std::vector<const char*> plot_base::font_names;
std::string plot_base::font_name_enum_def;

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
	label_ticks[0] = false;
	label_ticks[1] = true;
	label_font_size = 12;
	label_ffa = cgv::media::font::FFA_ITALIC;
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


void plot_base::ensure_font_names()
{
	if (font_names.empty()) {
		cgv::media::font::enumerate_font_names(font_names);
		if (font_names.empty())
			return;
		font_name_enum_def = std::string("enums='") + font_names[0];
		for (unsigned i = 1; i < font_names.size(); ++i) {
			font_name_enum_def += ',';
			font_name_enum_def += font_names[i];
		}
		font_name_enum_def += "'";
	}
	if (!label_font) {
		label_font_index = 0;
		for (auto iter = font_names.begin(); iter != font_names.end(); ++iter)
			if (std::string(*iter) == "Times New Roman") {
				label_font_index = iter - font_names.begin();
				break;
			}
			
		on_font_selection();
	}
}
void plot_base::on_font_selection()
{
	label_font      = cgv::media::font::find_font(font_names[label_font_index]);
	on_font_face_selection();
}

void plot_base::on_font_face_selection()
{
	label_font_face = label_font->get_font_face(label_ffa);
}

/// configure the label font
void plot_base::set_label_font(float font_size, cgv::media::font::FontFaceAttributes ffa, const std::string& font_name)
{
	label_font_size = font_size;
	label_ffa = ffa;
	if (!font_name.empty()) {
		for (auto iter = font_names.begin(); iter != font_names.end(); ++iter)
			if (font_name == *iter) {
				label_font_index = iter - font_names.begin();
				on_font_selection();
				break;
			}
	}
	else
		on_font_face_selection();
}

void plot_base::create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	const char* axis_names = "xyz";

	ensure_font_names();

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
			p.add_member_control(bp, "font_size", label_font_size, "value_slider", "min=8;max=40;log=false;ticks=true");
			p.add_member_control(bp, "font", (cgv::type::DummyEnum&)label_font_index, "dropdown", font_name_enum_def);
			connect_copy(p.find_control((cgv::type::DummyEnum&)label_font_index)->value_change, cgv::signal::rebind(this, &plot_base::on_font_selection));
			p.add_member_control(bp, "face", label_ffa, "dropdown", "enums='normal,bold,italics,bold italics'");
			connect_copy(p.find_control(label_ffa)->value_change, cgv::signal::rebind(this, &plot_base::on_font_face_selection));
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