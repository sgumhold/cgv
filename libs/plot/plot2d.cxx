#include "plot2d.h"
#include <libs/cgv_gl/gl/gl.h>
#include <cgv/media/color_scale.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace plot {

void plot2d::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i)
{
	plot_base::set_uniforms(ctx, prog, i);
	/*
	if (i >= 0 && i < get_nr_sub_plots()) {
		prog.set_uniform(ctx, "line_color", ref_sub_plot2d_config(i).line_color);
		float w1 = ref_sub_plot2d_config(i).bar_percentual_width*(float)(
			get_domain_config_ptr()->axis_configs[0].get_attribute_extent() / 
			(ref_sub_plot_samples(i).size() - 1.0f));
		float w2 = w1;
		if (ref_sub_plot_strips(i).size() > 1)
			w2 *= ref_sub_plot_strips(i).size();
		prog.set_uniform(ctx, "bar_width", w2);
	}
	*/
}

/** extend common plot configuration with parameters specific to 1d plot */
plot2d_config::plot2d_config(const std::string& _name) : plot_base_config(_name)
{
	reference_point_size = 0.001f;
	blend_width_in_pixel = 1.0;
	halo_width_in_pixel = 0.0f;
	percentual_halo_width = -0.35f;
	halo_color = rgba(0.5f, 0.5f, 0.5f, 1.0f);

	stick_coordinate_index = 1;
	stick_base_window = 0.0f;

	bar_width = 0.001f;
	bar_coordinate_index = 1;
	bar_base_window = 0.0;

	configure_chart(CT_LINE_CHART);
};

/// configure the sub plot to a specific chart type
void plot2d_config::configure_chart(ChartType chart_type)
{
	plot_base_config::configure_chart(chart_type);
	show_lines = chart_type == CT_LINE_CHART;
}

/// construct empty plot with default domain [0..1,0..1]
plot2d::plot2d(unsigned nr_attributes) : plot_base(2, nr_attributes)
{
	view_ptr = 0;
	legend_components = LC_ANY;
	rrs.illumination_mode = cgv::render::IM_OFF;
	layer_depth = 0.00001f;
	auto& acs = get_domain_config_ptr()->axis_configs;
	acs[0].name = "x";
	acs[1].name = "y";
	for (unsigned ai =0; ai < nr_attributes; ++ai)
		acs[ai+2].name = std::string("attribute_")+cgv::utils::to_string(ai);
}

bool plot2d::compute_sample_coordinate_interval(int i, int ai, float& samples_min, float& samples_max)
{
	// compute bounding box
	bool found_sample = false;
	float min_value, max_value;
	for (unsigned j = 0; j < samples[i].size(); ++j) {
		if (found_sample) {
			min_value = std::min(min_value, samples[i][j](ai));
			max_value = std::max(max_value, samples[i][j](ai));
		}
		else {
			min_value = samples[i][j](ai);
			max_value = samples[i][j](ai);
			found_sample = true;
		}
	}
	if (found_sample) {
		samples_min = min_value;
		samples_max = max_value;
		return true;
	}
	return false;
}


unsigned plot2d::add_sub_plot(const std::string& name)
{
	// determine index of new sub plot
	unsigned i = get_nr_sub_plots();

	// create new config
	if (i == 0)
		configs.push_back(new plot2d_config(name));
	else {
		configs.push_back(new plot2d_config(ref_sub_plot2d_config(i - 1)));
		ref_sub_plot_config(i).name = name;
	}

	// create new point container
	samples.push_back(std::vector<plot2d::vec2>());
	strips.push_back(std::vector<unsigned>());
	attribute_sources.push_back(std::vector<attribute_source>());
	attribute_sources.back().push_back(attribute_source(i, 0, 0, 2 * sizeof(float)));
	attribute_sources.back().push_back(attribute_source(i, 1, 0, 2 * sizeof(float)));
	// return sub plot index
	return i;
}

void plot2d::delete_sub_plot(unsigned i)
{
	delete configs[i];
	configs[i] = 0;
	configs.erase(configs.begin() + i);
	samples.erase(samples.begin() + i);
	strips.erase(strips.begin() + i);
}

/// return a reference to the plot base configuration of the i-th plot
plot2d_config& plot2d::ref_sub_plot2d_config(unsigned i)
{
	return static_cast<plot2d_config&>(ref_sub_plot_config(i));
}

std::vector<plot2d::vec2>& plot2d::ref_sub_plot_samples(unsigned i)
{
	return samples[i];
}

/// return the strip definition of the i-th sub plot
std::vector<unsigned>& plot2d::ref_sub_plot_strips(unsigned i)
{
	return strips[i];
}

bool plot2d::init(cgv::render::context& ctx)
{
	cgv::render::ref_rectangle_renderer(ctx, 1);
	//view_ptr = find_view_as_node();

	if (!prog.build_program(ctx, "plot2d.glpr", true)) {
		std::cerr << "could not build GLSL program from plot2d.glpr" << std::endl;
		return false;
	}
	if (!point_prog.build_program(ctx, "plot2d_point.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_point.glpr" << std::endl;
		return false;
	}
	if (!stick_prog.build_program(ctx, "plot2d_stick.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_stick.glpr" << std::endl;
		return false;
	}
	if (!bar_prog.build_program(ctx, "plot2d_bar.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_bar.glpr" << std::endl;
		return false;
	}
	if (!bar_outline_prog.build_program(ctx, "plot2d_bar_outline.glpr")) {
		std::cerr << "could not build GLSL program from bar_outline_prog.glpr" << std::endl;
		return false;
	}
	return plot_base::init(ctx);
}

void plot2d::clear(cgv::render::context& ctx)
{
	cgv::render::ref_rectangle_renderer(ctx, -1);
	point_prog.destruct(ctx);
	prog.destruct(ctx);
	stick_prog.destruct(ctx);
	bar_prog.destruct(ctx);
	bar_outline_prog.destruct(ctx);
}

void plot2d::draw_sub_plot(cgv::render::context& ctx, unsigned i)
{
	unsigned nr_attributes = (unsigned)attribute_sources[i].size();
	enable_attributes(ctx, nr_attributes);
	GLsizei count = (GLsizei)set_attributes(ctx, i, samples);
	if (count > 0) {
		const plot2d_config& spc = ref_sub_plot2d_config(i);
		float rs = get_domain_config_ptr()->reference_size;

		if (spc.show_bars && bar_prog.is_linked()) {
			set_default_attributes(ctx, bar_prog, nr_attributes);
			set_uniforms(ctx, bar_prog, i);
			bar_prog.set_uniform(ctx, "bar_width", spc.bar_width);
			bar_prog.set_uniform(ctx, "bar_base_window", spc.bar_base_window);
			bar_prog.set_uniform(ctx, "bar_coordinate_index", spc.bar_coordinate_index);
			bar_prog.set_uniform(ctx, "color_index", 0);
			bar_prog.set_uniform(ctx, "secondary_color_index", -1);
			bar_prog.set_uniform(ctx, "opacity_index", 0);
			bar_prog.set_uniform(ctx, "secondary_opacity_index", -1);
			bar_prog.set_uniform(ctx, "size_index", 0);
			bar_prog.set_attribute(ctx, "color", spc.bar_color);
			glDisable(GL_CULL_FACE);
				bar_prog.enable(ctx);
					draw_sub_plot_samples(count, spc);
				bar_prog.disable(ctx);
			glEnable(GL_CULL_FACE);

			if (spc.bar_outline_width > 0 && bar_outline_prog.is_linked()) {
				glLineWidth(spc.bar_outline_width);
				set_default_attributes(ctx, bar_outline_prog, nr_attributes);
				set_uniforms(ctx, bar_outline_prog, i);
				bar_outline_prog.set_uniform(ctx, "bar_width", spc.bar_width);
				bar_outline_prog.set_uniform(ctx, "bar_base_window", spc.bar_base_window);
				bar_outline_prog.set_uniform(ctx, "bar_coordinate_index", spc.bar_coordinate_index);
				bar_outline_prog.set_uniform(ctx, "color_index", 1);
				bar_outline_prog.set_uniform(ctx, "secondary_color_index", -1);
				bar_outline_prog.set_uniform(ctx, "opacity_index", 1);
				bar_outline_prog.set_uniform(ctx, "secondary_opacity_index", -1);
				bar_outline_prog.set_uniform(ctx, "size_index", 1);
				bar_outline_prog.set_attribute(ctx, "color", spc.bar_outline_color);
				bar_outline_prog.enable(ctx);
					draw_sub_plot_samples(count, spc);
				bar_outline_prog.disable(ctx);
			}
		}
		if (spc.show_sticks && stick_prog.is_linked()) {
			glLineWidth(spc.stick_width);
			set_default_attributes(ctx, stick_prog, nr_attributes);
			set_uniforms(ctx, stick_prog, i);
			stick_prog.set_uniform(ctx, "stick_coordinate_index", spc.stick_coordinate_index);
			stick_prog.set_uniform(ctx, "stick_base_window", spc.stick_base_window);
			stick_prog.set_uniform(ctx, "color_index", 0);
			stick_prog.set_uniform(ctx, "secondary_color_index", -1);
			stick_prog.set_uniform(ctx, "opacity_index", 0);
			stick_prog.set_uniform(ctx, "secondary_opacity_index", -1);
			stick_prog.set_uniform(ctx, "size_index", 0);
			stick_prog.set_attribute(ctx, "color", spc.stick_color);
			stick_prog.enable(ctx);
				draw_sub_plot_samples(count, spc);
			stick_prog.disable(ctx);
		}
		if (spc.show_lines && prog.is_linked()) {
			glLineWidth(spc.line_width);
			set_default_attributes(ctx, prog, nr_attributes);
			set_uniforms(ctx, prog, i);
			prog.set_uniform(ctx, "feature_offset", 0.001f * get_extent().length());
			prog.set_uniform(ctx, "color_index", 0);
			prog.set_uniform(ctx, "secondary_color_index", -1);
			prog.set_uniform(ctx, "opacity_index", 0);
			prog.set_uniform(ctx, "secondary_opacity_index", -1);
			prog.set_uniform(ctx, "size_index", 0);
			prog.set_attribute(ctx, "color", spc.line_color);
			prog.enable(ctx);
			if (strips[i].empty())
				draw_sub_plot_samples(count, spc, true);
			else {
				unsigned fst = 0;
				for (unsigned j = 0; j < strips[i].size(); ++j) {
					glDrawArrays(GL_LINE_STRIP, fst, strips[i][j]);
					fst += strips[i][j];
				}
			}
			prog.disable(ctx);
		}
		if (spc.show_points && point_prog.is_linked()) {
			double y_view_angle = 45.0f;
			if (view_ptr)
				y_view_angle = view_ptr->get_y_view_angle();
			float pixel_extent_per_depth = (float)(2.0 * tan(0.5 * 0.0174532925199 * y_view_angle) / ctx.get_height());
			set_default_attributes(ctx, point_prog, nr_attributes);
			set_uniforms(ctx, point_prog, i);
			point_prog.set_uniform(ctx, "depth_offset", -5*int(i+1) * layer_depth);
			point_prog.set_uniform(ctx, "reference_point_size", rs);
			point_prog.set_uniform(ctx, "blend_width_in_pixel", spc.blend_width_in_pixel);
			point_prog.set_uniform(ctx, "halo_width_in_pixel", spc.halo_width_in_pixel);
			point_prog.set_uniform(ctx, "percentual_halo_width", spc.percentual_halo_width);
			point_prog.set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
			point_prog.set_uniform(ctx, "feature_offset", 0.001f * get_extent().length());
			point_prog.set_uniform(ctx, "color_index", 0);
			point_prog.set_uniform(ctx, "secondary_color_index", 1);
			point_prog.set_uniform(ctx, "opacity_index", 0);
			point_prog.set_uniform(ctx, "secondary_opacity_index", 1);
			point_prog.set_uniform(ctx, "size_index", 0);
			point_prog.set_attribute(ctx, "color", spc.point_color);
			point_prog.set_attribute(ctx, "secondary_color", spc.halo_color);
			point_prog.set_attribute(ctx, "size", spc.point_size);
			point_prog.enable(ctx);
				draw_sub_plot_samples(count, spc);
			point_prog.disable(ctx);
		}
	}
	disable_attributes(ctx, unsigned(attribute_sources[i].size()));
}

void plot2d::draw_domain(cgv::render::context& ctx)
{
	tick_labels.clear();
	tick_batches.clear();
	std::vector<box2> R;
	std::vector<rgb> C;
	std::vector<float> D;
	vec2 extent = vec2::from_vec(get_extent());
	float rs = get_domain_config_ptr()->reference_size;
	if (get_domain_config_ptr()->fill) {
		R.push_back(box2(-0.5f * extent, 0.5f * extent));
		C.push_back(get_domain_config_ptr()->color);
		D.push_back(0.0f);
	}
	for (unsigned ai = 0; ai < 2; ++ai) {
		axis_config& ac = get_domain_config_ptr()->axis_configs[ai];
		axis_config& ao = get_domain_config_ptr()->axis_configs[1-ai];
		float lw = rs * ac.line_width;
		rgb col = get_domain_config_ptr()->fill ? ac.color : get_domain_config_ptr()->color;
		vec2 axis_extent(0.0f);
		axis_extent[ai] = extent[ai];
		axis_extent[1-ai] = lw;
		box2 axis_box(-0.5f * axis_extent, 0.5f * axis_extent);
		axis_box.ref_min_pnt()[1 - ai] += 0.5f * (extent[1 - ai]-lw);
		axis_box.ref_max_pnt()[1 - ai] += 0.5f * (extent[1 - ai]-lw);
		R.push_back(axis_box);
		C.push_back(col);
		D.push_back(-layer_depth);
		axis_box.ref_min_pnt()[1 - ai] -= extent[1 - ai]-lw;
		axis_box.ref_max_pnt()[1 - ai] -= extent[1 - ai]-lw;
		R.push_back(axis_box);
		C.push_back(col);
		D.push_back(-layer_depth);
		// axis line
		if (ao.get_attribute_min() < 0 && ao.get_attribute_max() > 0) {
			float axis_plot = ao.plot_space_from_attribute_space(0.0f);
			axis_box.ref_min_pnt()[1 - ai] = axis_plot - 0.5f * lw;
			axis_box.ref_max_pnt()[1 - ai] = axis_plot + 0.5f * lw;
			R.push_back(axis_box);
			C.push_back(ac.color);
			D.push_back(-2*layer_depth);
		}

		for (unsigned ti = 0; ti < 2; ++ti) {
			tick_config& tc = ti == 0 ? ac.primary_ticks : ac.secondary_ticks;
			if (tc.type == TT_NONE)
				continue;
			tick_batches.push_back(tick_batch_info(ai, 1 - ai, ti == 0, 0, (unsigned)tick_labels.size()));
			float min_tick = ac.tick_space_from_attribute_space(ac.get_attribute_min());
			float max_tick = ac.tick_space_from_attribute_space(ac.get_attribute_max());
			int min_i = (int)ceil(min_tick / tc.step - std::numeric_limits<float>::epsilon());
			int max_i = (int)((max_tick - fmod(max_tick, tc.step)) / tc.step);
			// ignore secondary ticks on domain boundary
			if (ti == 1 && min_i * tc.step - min_tick < std::numeric_limits<float>::epsilon())
				++min_i;
			if (ti == 1 && max_i * tc.step - max_tick > -std::numeric_limits<float>::epsilon())
				--max_i;
			float lw = 0.5f * get_domain_config_ptr()->reference_size * tc.line_width;
			float dl = 0.5f * get_domain_config_ptr()->reference_size * tc.length;
			float he = 0.5f * ao.extent;
			float d = -int(ti + 3) * layer_depth;
			bool includes_zero = ao.get_attribute_min() < 0.0f && ao.get_attribute_max() > 0.0f;
			float z_plot = includes_zero ? ao.plot_space_from_attribute_space(0.0f) : 0.0f;
			for (int i = min_i; i <= max_i; ++i) {
				float c_tick = (float)(i * tc.step);
				float c_attr = ac.attribute_space_from_tick_space(c_tick);
				// ignore secondary ticks on axes
				if (fabs(c_attr) < std::numeric_limits<float>::epsilon())
					continue;
				std::string label_str;
				if (tc.label)
					label_str = cgv::utils::to_string(c_attr);
				float c_plot = ac.plot_space_from_window_space(ac.window_space_from_tick_space(c_tick));
				vec2 mn, mx;
				mn[ai] = c_plot - lw;
				mx[ai] = c_plot + lw;
				switch (tc.type) {
				case TT_DASH:
					mn[1 - ai] = -he;
					mx[1 - ai] = -he + dl;
					R.push_back(box2(mn, mx));
					C.push_back(ac.color);
					D.push_back(d);
					if (!label_str.empty())
						tick_labels.push_back(label_info(mx, label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
					mn[1 - ai] = he - dl;
					mx[1 - ai] = he;
					R.push_back(box2(mn, mx));
					C.push_back(ac.color);
					D.push_back(d);
					if (!label_str.empty())
						tick_labels.push_back(label_info(mn, label_str, ai == 0 ? cgv::render::TA_TOP : cgv::render::TA_RIGHT));
					if (includes_zero) {
						mn[1 - ai] = z_plot - dl;
						mx[1 - ai] = z_plot + dl;
						R.push_back(box2(mn, mx));
						C.push_back(ac.color);
						D.push_back(d);
					}
					break;
				case TT_LINE:
				case TT_PLANE:
					mn[1 - ai] = -he;
					mx[1 - ai] = he;
					R.push_back(box2(mn, mx));
					C.push_back(ac.color);
					D.push_back(d);
					if (!label_str.empty()) {
						tick_labels.push_back(label_info(mn, label_str, ai == 0 ? cgv::render::TA_TOP : cgv::render::TA_RIGHT));
						tick_labels.push_back(label_info(mx, label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
					}
					break;
				}
			}
			tick_batches.back().label_count = (unsigned)(tick_labels.size() - tick_batches.back().first_label);
		}
	}
	auto& rr = cgv::render::ref_rectangle_renderer(ctx);
	rr.set_render_style(rrs);
	rr.set_rectangle_array(ctx, R);
	rr.set_color_array(ctx, C);
	rr.set_depth_offset_array(ctx, D);
	rr.render(ctx, 0, R.size());
}

void plot2d::draw_tick_labels(cgv::render::context& ctx)
{
	if (tick_labels.empty())
		return;
	for (const auto& tbc : tick_batches) if (tbc.label_count > 0) {
		ctx.set_color(get_domain_config_ptr()->axis_configs[tbc.ai].color);
		for (unsigned i = tbc.first_label; i < tbc.first_label + tbc.label_count; ++i) {
			const label_info& li = tick_labels[i];
			ctx.set_cursor(vec3(li.position, 0.0f).to_vec(), li.label, li.align);
			ctx.output_stream() << li.label;
			ctx.output_stream().flush();
		}
	}
}

void plot2d::draw(cgv::render::context& ctx)
{
	GLboolean line_smooth = glIsEnabled(GL_LINE_SMOOTH); glEnable(GL_LINE_SMOOTH);
	GLboolean point_smooth = glIsEnabled(GL_POINT_SMOOTH); glEnable(GL_POINT_SMOOTH);
	GLboolean blend = glIsEnabled(GL_BLEND); glEnable(GL_BLEND);
	GLenum blend_src, blend_dst, depth;
	glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
	glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
	glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth));
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);


	ctx.push_modelview_matrix();
	mat4 R;
	orientation.put_homogeneous_matrix(R);
	ctx.mul_modelview_matrix(cgv::math::translate4<float>(center_location) * R);
	if (get_domain_config_ptr()->show_domain) {
		draw_domain(ctx);
		ctx.enable_font_face(label_font_face, get_domain_config_ptr()->label_font_size);
		draw_tick_labels(ctx);
	}
	if (legend_components != LC_HIDDEN)
		draw_legend(ctx, -4 * layer_depth);
	for (unsigned i = 0; i < samples.size(); ++i) {
		// skip unvisible and empty sub plots
		if (!ref_sub_plot2d_config(i).show_plot)
			continue;
		draw_sub_plot(ctx, i);
	}


	ctx.pop_modelview_matrix();


	if (!line_smooth)
		glDisable(GL_LINE_SMOOTH);
	if (!point_smooth)
		glDisable(GL_POINT_SMOOTH);
	if (!blend)
		glDisable(GL_BLEND);
	glDepthFunc(depth);
	glBlendFunc(blend_src, blend_dst);

	return;

	//GLboolean line_smooth = glIsEnabled(GL_LINE_SMOOTH); glEnable(GL_LINE_SMOOTH);
	//GLboolean point_smooth = glIsEnabled(GL_POINT_SMOOTH); glEnable(GL_POINT_SMOOTH);
	//GLboolean blend = glIsEnabled(GL_BLEND); glEnable(GL_BLEND);
	//GLenum blend_src, blend_dst, depth;
	glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
	glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
	glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth));
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	enable_attributes(ctx, nr_attributes);
	for (unsigned i = 0; i<samples.size(); ++i) {
		// skip unvisible and empty sub plots
		if (!ref_sub_plot2d_config(i).show_plot)
			continue;
		draw_sub_plot(ctx, i);
	}
	if (!line_smooth)
		glDisable(GL_LINE_SMOOTH);
	if (!point_smooth)
		glDisable(GL_POINT_SMOOTH);
	if (!blend)
		glDisable(GL_BLEND);
	glDepthFunc(depth);
	glBlendFunc(blend_src, blend_dst);

	draw_legend(ctx);
}

/// create the gui for a point subplot
void plot2d::create_point_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	auto& p2dc = reinterpret_cast<plot2d_config&>(pbc);
	p.add_member_control(bp, "reference_point_size", p2dc.reference_point_size, "value_slider", "min=0.0001;max=1;ticks=true;log=true");
	p.add_member_control(bp, "blend_width_in_pixel", p2dc.blend_width_in_pixel, "value_slider", "min=0;max=2;ticks=true");
	p.add_member_control(bp, "halo_width_in_pixel", p2dc.halo_width_in_pixel, "value_slider", "min=0;max=20;ticks=true;log=true");
	p.add_member_control(bp, "percentual_halo_width", p2dc.percentual_halo_width, "value_slider", "min=-0.9;max=0.9;ticks=true");
	p.add_member_control(bp, "halo_color", p2dc.halo_color);
}

/// create the gui for a stick subplot
void plot2d::create_stick_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	auto& p2dc = reinterpret_cast<plot2d_config&>(pbc);
	p.add_member_control(bp, "crd_idx", p2dc.stick_coordinate_index, "value_slider", "min=0;max=1;ticks=true");
	p.add_member_control(bp, "base", p2dc.stick_base_window, "value_slider", "min=0;max=1;ticks=true");
}

/// create the gui for a bar subplot
void plot2d::create_bar_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	auto& p2dc = reinterpret_cast<plot2d_config&>(pbc);
	p.add_member_control(bp, "width", p2dc.bar_width, "value_slider", "min=0;max=1;step=0.0001ticks=true;log=true");
	p.add_member_control(bp, "crd_idx", p2dc.bar_coordinate_index, "value_slider", "min=0;max=1;ticks=true");
	p.add_member_control(bp, "base", p2dc.bar_base_window, "value_slider", "min=0;max=1;ticks=true");
}

void plot2d::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot_base::create_config_gui(bp, p, i);
	if (p.begin_tree_node("rectangle", rrs, false, "level=3")) {
		p.align("\a");
		p.add_member_control(bp, "layer_depth", layer_depth, "value_slider", "min=0.000001;max=0.01;step=0.0000001;log=true;ticks=true");
		p.add_gui("rectangle style", rrs);
		p.align("\b");
	}
}

	}
}