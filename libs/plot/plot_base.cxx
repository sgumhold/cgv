#include "plot_base.h"
#include <cgv/render/shader_program.h>
#include <cgv/signal/rebind.h>

namespace cgv {
	namespace plot {

std::vector<const char*> plot_base::font_names;
std::string plot_base::font_name_enum_def;

/// set tick config defaults
tick_config::tick_config(bool primary)
{
	step = primary ? 5.0f : 1.0f;
	type = TT_DASH;
	line_width = primary ? 4.0f : 2.0f;
	length = primary ? 2.5f : 1.5f;
	label = primary;
	precision = -1;
}

axis_config::axis_config() : primary_ticks(true), secondary_ticks(false), color(0.3f,0.3f,0.3f)
{
	log_scale = false;
	line_width = 3.0f;
}

domain_config::domain_config(unsigned nr_axes) : color(0.7f,0.7f,0.7f), axis_configs(nr_axes)
{
	show_domain = true;
	fill = false;
	label_font_index = -1;
	label_font_size = 16.0f;
	label_ffa = cgv::media::font::FFA_BOLD_ITALIC;
}

plot_base_config::plot_base_config(const std::string& _name) : name(_name)
{
	show_plot = true;

	point_size = 3;
	point_color = rgb(1,0,0);

	stick_width = 2;
	stick_color = rgb(1,1,0);

	bar_percentual_width = 0.75f;
	bar_outline_width = 1;
	bar_color = rgb(0,1,1);
	bar_outline_color = rgb(1,1,1);

	configure_chart(CT_BAR_CHART);
}

/// configure the sub plot to a specific chart type
void plot_base_config::configure_chart(ChartType chart_type)
{
	switch (chart_type) {
	case CT_POINT : 
	case CT_LINE_CHART:
		show_points = true;
		show_sticks = false;
		show_bars = false;
		break;
	case CT_BAR_CHART:
		show_points = false;
		show_sticks = false;
		show_bars = true;
		break;
	}
}


plot_base_config::~plot_base_config()
{
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
		get_domain_config_ptr()->label_font_index = 0;
		for (auto iter = font_names.begin(); iter != font_names.end(); ++iter)
			if (std::string(*iter) == "Times New Roman") {
				get_domain_config_ptr()->label_font_index = iter - font_names.begin();
				break;
			}
		on_font_selection();
	}
}

void plot_base::on_font_selection()
{
	label_font = cgv::media::font::find_font(font_names[get_domain_config_ptr()->label_font_index]);
	on_font_face_selection();
}

void plot_base::on_font_face_selection()
{
	label_font_face = label_font->get_font_face(get_domain_config_ptr()->label_ffa);
}

void plot_base::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i)
{
	prog.set_uniform(ctx, "point_color", ref_sub_plot_config(i).point_color);
	prog.set_uniform(ctx, "stick_color", ref_sub_plot_config(i).stick_color);
	prog.set_uniform(ctx, "bar_color", ref_sub_plot_config(i).bar_color);
	prog.set_uniform(ctx, "bar_outline_color", ref_sub_plot_config(i).bar_outline_color);
}

plot_base::plot_base(unsigned nr_axes) : dom_cfg(nr_axes)
{
	dom_cfg_ptr = &dom_cfg;
}

/// configure the label font
void plot_base::set_label_font(float font_size, cgv::media::font::FontFaceAttributes ffa, const std::string& font_name)
{
	get_domain_config_ptr()->label_font_size = font_size;
	get_domain_config_ptr()->label_ffa = ffa;
	if (!font_name.empty()) {
		for (auto iter = font_names.begin(); iter != font_names.end(); ++iter)
			if (font_name == *iter) {
				get_domain_config_ptr()->label_font_index = iter - font_names.begin();
				on_font_selection();
				break;
			}
	}
	else
		on_font_face_selection();
}

/// return const pointer to domain configuration
const domain_config* plot_base::get_domain_config_ptr() const
{
	return dom_cfg_ptr;
}

/// return pointer to domain configuration
domain_config* plot_base::get_domain_config_ptr()
{
	return dom_cfg_ptr;
}

/// set the domain configuration to an external configuration in order to synch several plots, if set to null, the internal domain config is used again
void plot_base::set_domain_config_ptr(domain_config* _new_ptr)
{
	if (_new_ptr)
		dom_cfg_ptr = _new_ptr;
	else
		dom_cfg_ptr = &dom_cfg;
}


unsigned plot_base::get_nr_sub_plots() const
{
	return configs.size();
}

plot_base_config& plot_base::ref_sub_plot_config(unsigned i)
{
	return *configs[i];
}

void plot_base::create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	const char* axis_names = "xyz";

	ensure_font_names();

	bool open = p.begin_tree_node("domain", get_domain_config_ptr()->show_domain, false, "level=3;w=70;align=' '");
	p.add_member_control(bp, "show", get_domain_config_ptr()->show_domain, "toggle", "w=40", " ");
	p.add_member_control(bp, "fill", get_domain_config_ptr()->fill, "toggle", "w=40");
	if (open) {
		p.align("\a");
		p.add_member_control(bp, "fill color", get_domain_config_ptr()->color);
		for (unsigned i = 0; i < get_domain_config_ptr()->axis_configs.size(); ++i) {
			axis_config& ac = get_domain_config_ptr()->axis_configs[i];
			bool show = p.begin_tree_node(std::string(1, axis_names[i]) + " axis", ac.color, false, "level=3;w=100;align=' '");
			p.add_member_control(bp, "log", ac.log_scale, "toggle", "w=50");
			if (show) {
				p.align("\a");
				p.add_member_control(bp, "width", ac.line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
				p.add_member_control(bp, "color", ac.color);
				char* tn[2] = { "primary tick", "secondary tick" };
				tick_config* tc[2] = { &ac.primary_ticks, &ac.secondary_ticks };
				for (unsigned ti = 0; ti < 2; ++ti) {
					bool vis = p.begin_tree_node(tn[ti], tc[ti]->label, false, "level=3;w=100;align=' '");
					p.add_member_control(bp, "label", tc[ti]->label, "toggle", "w=60");
					if (vis) {
						p.align("\a");
						p.add_member_control(bp, "type", tc[ti]->type, "dropdown", "enums='none,dash,line,plane'");
						p.add_member_control(bp, "step", tc[ti]->step, "value");
						p.add_member_control(bp, "width", tc[ti]->line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
						p.add_member_control(bp, "length", tc[ti]->length, "value_slider", "min=1;max=20;log=true;ticks=true");
						p.add_member_control(bp, "precision", tc[ti]->precision, "value_slider", "min=-1;max=5;ticks=true");
						p.align("\b");
						p.end_tree_node(tc[ti]->label);
					}
				}
				p.align("\b");
				p.end_tree_node(ac.color);
			}
		}
		p.add_member_control(bp, "font_size", get_domain_config_ptr()->label_font_size, "value_slider", "min=8;max=40;log=false;ticks=true");
		p.add_member_control(bp, "font", (cgv::type::DummyEnum&)get_domain_config_ptr()->label_font_index, "dropdown", font_name_enum_def);
		connect_copy(p.find_control((cgv::type::DummyEnum&)get_domain_config_ptr()->label_font_index)->value_change,
			cgv::signal::rebind(this, &plot_base::on_font_selection));
		p.add_member_control(bp, "face", get_domain_config_ptr()->label_ffa, "dropdown", "enums='normal,bold,italics,bold italics'");
		connect_copy(p.find_control(get_domain_config_ptr()->label_ffa)->value_change,
			cgv::signal::rebind(this, &plot_base::on_font_face_selection));
		p.align("\b");
		p.end_tree_node(get_domain_config_ptr()->show_domain);
	}
}

void plot_base::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	p.add_member_control(bp, "name", pbc.name);
	bool show = p.begin_tree_node("points", pbc.show_points, false, "level=3;w=100;align=' '");
	p.add_member_control(bp, "show", pbc.show_points, "toggle", "w=50");
	if (show) {
		p.align("\a");
			p.add_member_control(bp, "size", pbc.point_size, "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "color", pbc.point_color);
		p.align("\b");
		p.end_tree_node(pbc.show_points);
	}
	show = p.begin_tree_node("sticks", pbc.show_sticks, false, "level=3;w=100;align=' '");
	p.add_member_control(bp, "show", pbc.show_sticks, "toggle", "w=50");
	if (show) {
		p.align("\a");
			p.add_member_control(bp, "width", pbc.stick_width, "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "color", pbc.stick_color);
		p.align("\b");
		p.end_tree_node(pbc.show_sticks);
	}
	show = p.begin_tree_node("bars", pbc.show_bars, false, "level=3;w=100;align=' '");
	p.add_member_control(bp, "show", pbc.show_bars, "toggle", "w=50");
	if (show) {
		p.align("\a");
			p.add_member_control(bp, "width", pbc.bar_percentual_width, "value_slider", "min=0.01;max=1;log=true;ticks=true");
			p.add_member_control(bp, "fill", pbc.bar_color);
			p.add_member_control(bp, "outline_width", pbc.bar_outline_width, "value_slider", "min=0;max=20;log=true;ticks=true");
			p.add_member_control(bp, "outline", pbc.bar_outline_color);
		p.align("\b");
		p.end_tree_node(pbc.show_bars);
	}
}

void plot_base::create_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	create_plot_gui(bp, p);
	for (unsigned i=0; i<get_nr_sub_plots(); ++i) {
		plot_base_config& pbc = ref_sub_plot_config(i);
		bool show = p.begin_tree_node(std::string("configure ")+pbc.name, pbc.name, false, "level=3;w=100;align=' '");
		p.add_member_control(bp, "show", pbc.show_plot, "toggle", "w=50");
		if (show) {
			p.align("\a");
				create_config_gui(bp, p, i);
			p.align("\b");
			p.end_tree_node(pbc.name);
		}
	}
}


	}
}

#ifdef REGISTER_SHADER_FILES
#include <cgv/base/register.h>
#include <plot_shader_inc.h>
#endif
