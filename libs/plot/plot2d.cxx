#include "plot2d.h"
#include <libs/cgv_gl/gl/gl.h>
#include <cgv/media/color_scale.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/math/ftransform.h>
#include <libs/tt_gl_font/tt_gl_font.h>

namespace cgv {
	namespace plot {

/** extend common plot configuration with parameters specific to 1d plot */
plot2d_config::plot2d_config(const std::string& _name) : plot_base_config(_name, 2)
{
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
	multi_y_axis_mode = false;
	dz = 0.0f;
	layer_depth = 0.00001f;
	disable_depth_mask = false;
	//legend_components = LC_ANY;
	rrs.illumination_mode = cgv::render::IM_OFF;
	rrs.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
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
	attribute_source_arrays.push_back(attribute_source_array());
	attribute_source_arrays.back().attribute_sources.push_back(attribute_source(i, 0, 0, 2 * sizeof(float)));
	attribute_source_arrays.back().attribute_sources.push_back(attribute_source(i, 1, 0, 2 * sizeof(float)));
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
	aam_domain.init(ctx);
	aam_tick_labels.init(ctx);
	if (!line_prog.build_program(ctx, "plot2d_line.glpr", true)) {
		std::cerr << "could not build GLSL program from plot2d_line.glpr" << std::endl;
		return false;
	}
	if (!point_prog.build_program(ctx, "plot2d_point.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_point.glpr" << std::endl;
		return false;
	}
	if (!rectangle_prog.build_program(ctx, "plot2d_rect.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_rect.glpr" << std::endl;
		return false;
	}
	return plot_base::init(ctx);
}

void plot2d::clear(cgv::render::context& ctx)
{
	cgv::render::ref_rectangle_renderer(ctx, -1);
	aam_domain.destruct(ctx);
	aam_tick_labels.destruct(ctx);
	point_prog.destruct(ctx);
	line_prog.destruct(ctx);
	rectangle_prog.destruct(ctx);
	plot_base::clear(ctx);
}

bool plot2d::draw_point_plot(cgv::render::context& ctx, int i, int layer_idx)
{
	// skip unvisible and empty sub plots
	if (!ref_sub_plot2d_config(i).show_plot)
		return false;
	GLsizei count = (GLsizei)enable_attributes(ctx, i, samples);
	bool result = false;
	if (count > 0) {
		const plot2d_config& spc = ref_sub_plot2d_config(i);
		if (spc.show_points && point_prog.is_linked()) {
			set_plot_uniforms(ctx, point_prog);
			set_mapping_uniforms(ctx, point_prog);
			point_prog.set_uniform(ctx, "depth_offset", -layer_idx * layer_depth);
			point_prog.set_uniform(ctx, "reference_point_size", get_domain_config_ptr()->reference_size);
			point_prog.set_uniform(ctx, "blend_width_in_pixel", get_domain_config_ptr()->blend_width_in_pixel);
			point_prog.set_uniform(ctx, "halo_width_in_pixel", 0.0f);
			point_prog.set_uniform(ctx, "percentual_halo_width", spc.point_halo_width.size / spc.point_size.size);
			point_prog.set_uniform(ctx, "viewport_height", (float)ctx.get_height());
			point_prog.set_uniform(ctx, "measure_point_size_in_pixel", false);
			point_prog.set_uniform(ctx, "screen_aligned", false);
			point_prog.set_uniform(ctx, "color_index", spc.point_color.color_idx);
			point_prog.set_uniform(ctx, "secondary_color_index", spc.point_halo_color.color_idx);
			point_prog.set_uniform(ctx, "opacity_index", spc.point_color.opacity_idx);
			point_prog.set_uniform(ctx, "secondary_opacity_index", spc.point_halo_color.opacity_idx);
			point_prog.set_uniform(ctx, "size_index", spc.point_size.size_idx);
			point_prog.set_uniform(ctx, "secondary_size_index", spc.point_halo_width.size_idx);
			point_prog.set_attribute(ctx, point_prog.get_color_index(), spc.point_color.color);
			point_prog.set_attribute(ctx, "secondary_color", spc.point_halo_color.color);
			point_prog.set_attribute(ctx, "size", spc.point_size.size);
			point_prog.enable(ctx);
			draw_sub_plot_samples(count, spc);
			point_prog.disable(ctx);
			result = true;
		}
	}
	disable_attributes(ctx, i);
	return result;
}
bool plot2d::draw_line_plot(cgv::render::context& ctx, int i, int layer_idx)
{
	// skip unvisible and empty sub plots
	if (!ref_sub_plot2d_config(i).show_plot)
		return false;
	GLsizei count = (GLsizei)enable_attributes(ctx, i, samples);
	bool result = false;
	if (count > 0) {
		const plot2d_config& spc = ref_sub_plot2d_config(i);
		if (spc.show_lines && line_prog.is_linked()) {
			set_plot_uniforms(ctx, line_prog);
			set_mapping_uniforms(ctx, line_prog);
			line_prog.set_uniform(ctx, "depth_offset", -layer_idx * layer_depth);
			line_prog.set_uniform(ctx, "reference_line_width", get_domain_config_ptr()->reference_size);
			line_prog.set_uniform(ctx, "blend_width_in_pixel", get_domain_config_ptr()->blend_width_in_pixel);
			line_prog.set_uniform(ctx, "halo_width_in_pixel", 0.0f);
			line_prog.set_uniform(ctx, "halo_color_strength", 1.0f);
			line_prog.set_uniform(ctx, "halo_color", spc.line_halo_color.color);
			line_prog.set_uniform(ctx, "percentual_halo_width", spc.line_halo_width.size / spc.line_width.size);
			line_prog.set_uniform(ctx, "viewport_height", (float)ctx.get_height());
			line_prog.set_uniform(ctx, "measure_line_width_in_pixel", false);
			line_prog.set_uniform(ctx, "screen_aligned", false);
			line_prog.set_uniform(ctx, "color_index", spc.line_color.color_idx);
			line_prog.set_uniform(ctx, "secondary_size_index", spc.line_halo_width.size_idx);
			line_prog.set_uniform(ctx, "secondary_color_index", spc.line_halo_color.color_idx);
			line_prog.set_uniform(ctx, "opacity_index", spc.line_color.opacity_idx);
			line_prog.set_uniform(ctx, "secondary_opacity_index", spc.line_halo_color.opacity_idx);
			line_prog.set_uniform(ctx, "size_index", spc.line_width.size_idx);
			line_prog.set_attribute(ctx, line_prog.get_color_index(), spc.line_color.color);
			line_prog.set_attribute(ctx, "secondary_color", spc.line_halo_color.color);
			line_prog.set_attribute(ctx, "size", spc.line_width.size);
			line_prog.enable(ctx);
			if (strips[i].empty())
				draw_sub_plot_samples(count, spc, true);
			else {
				unsigned fst = 0;
				for (unsigned j = 0; j < strips[i].size(); ++j) {
					glDrawArrays(GL_LINE_STRIP, fst, strips[i][j]);
					fst += strips[i][j];
				}
			}
			line_prog.disable(ctx);
			result = true;
		}
	}
	disable_attributes(ctx, i);
	return result;
}
bool plot2d::draw_stick_plot(cgv::render::context& ctx, int i, int layer_idx)
{
	// skip unvisible and empty sub plots
	if (!ref_sub_plot2d_config(i).show_plot)
		return false;
	GLsizei count = (GLsizei)enable_attributes(ctx, i, samples);
	bool result = false;
	if (count > 0) {
		const plot2d_config& spc = ref_sub_plot2d_config(i);
		if (spc.show_sticks && rectangle_prog.is_linked()) {
			// configure vertex shader
			rectangle_prog.set_uniform(ctx, "color_index", spc.stick_color.color_idx);
			rectangle_prog.set_uniform(ctx, "secondary_color_index", -1);
			rectangle_prog.set_uniform(ctx, "opacity_index", spc.stick_color.opacity_idx);
			rectangle_prog.set_uniform(ctx, "secondary_opacity_index", -1);
			rectangle_prog.set_uniform(ctx, "size_index", spc.stick_width.size_idx);
			rectangle_prog.set_uniform(ctx, "secondary_size_index", -1);
			rectangle_prog.set_uniform(ctx, "rectangle_base_window", spc.stick_base_window);
			rectangle_prog.set_uniform(ctx, "rectangle_coordinate_index", spc.stick_coordinate_index);
			rectangle_prog.set_uniform(ctx, "rectangle_border_width", 0.0f);
			rectangle_prog.set_attribute(ctx, rectangle_prog.get_color_index(), spc.stick_color.color);
			rectangle_prog.set_attribute(ctx, "size", spc.stick_width.size * get_domain_config_ptr()->reference_size);
			rectangle_prog.set_attribute(ctx, "depth_offset", -layer_idx * layer_depth);
			// configure geometry shader
			rectangle_prog.set_uniform(ctx, "border_mode", spc.stick_coordinate_index == 0 ? 2 : 1);

			rectangle_prog.enable(ctx);
			draw_sub_plot_samples(count, spc);
			rectangle_prog.disable(ctx);
			result = true;
		}
	}
	disable_attributes(ctx, i);
	return result;
}
void plot2d::configure_bar_plot(cgv::render::context& ctx)
{
	// prepare rectangle program to draw bar and stick plots
	set_plot_uniforms(ctx, rectangle_prog);
	set_mapping_uniforms(ctx, rectangle_prog);
	// configure geometry shader
	rectangle_prog.set_uniform(ctx, "pixel_blend", 1.0f);
	rectangle_prog.set_uniform(ctx, "viewport_height", (float)ctx.get_height());
	// configure fragment shader
	rectangle_prog.set_uniform(ctx, "use_texture", false);
	// configure side shader
	rectangle_prog.set_uniform(ctx, "culling_mode", 0);
	rectangle_prog.set_uniform(ctx, "map_color_to_material", 7);
	rectangle_prog.set_uniform(ctx, "illumination_mode", 0);
}
bool plot2d::draw_bar_plot(cgv::render::context& ctx, int i, int layer_idx)
{
	// skip unvisible and empty sub plots
	if (!ref_sub_plot2d_config(i).show_plot)
		return false;
	bool result = false;
	GLsizei count = (GLsizei)enable_attributes(ctx, i, samples);
	if (count > 0) {
		const plot2d_config& spc = ref_sub_plot2d_config(i);
		if (spc.show_bars && rectangle_prog.is_linked()) {
			// configure vertex shader
			rectangle_prog.set_uniform(ctx, "color_index", spc.bar_color.color_idx);
			rectangle_prog.set_uniform(ctx, "secondary_color_index", spc.bar_outline_color.color_idx);
			rectangle_prog.set_uniform(ctx, "opacity_index", spc.bar_color.opacity_idx);
			rectangle_prog.set_uniform(ctx, "secondary_opacity_index", spc.bar_outline_color.opacity_idx);
			rectangle_prog.set_uniform(ctx, "size_index", spc.bar_percentual_width.size_idx);
			rectangle_prog.set_uniform(ctx, "secondary_size_index", spc.bar_outline_width.size_idx);
			rectangle_prog.set_uniform(ctx, "rectangle_base_window", spc.bar_base_window);
			rectangle_prog.set_uniform(ctx, "rectangle_coordinate_index", spc.bar_coordinate_index);
			rectangle_prog.set_uniform(ctx, "rectangle_border_width", spc.bar_outline_width.size * get_domain_config_ptr()->reference_size);
			rectangle_prog.set_attribute(ctx, rectangle_prog.get_color_index(), spc.bar_color.color);
			rectangle_prog.set_attribute(ctx, "secondary_color", spc.bar_outline_color.color);
			rectangle_prog.set_attribute(ctx, "size", get_extent()[0] / count * spc.bar_percentual_width.size);
			rectangle_prog.set_attribute(ctx, "depth_offset", -layer_idx * layer_depth);
			// configure geometry shader
			rectangle_prog.set_uniform(ctx, "border_mode", spc.bar_coordinate_index == 0 ? 2 : 1);

			rectangle_prog.enable(ctx);
			draw_sub_plot_samples(count, spc);
			rectangle_prog.disable(ctx);
			result = true;
		}
	}
	disable_attributes(ctx, i);
	return result;
}
int plot2d::draw_sub_plots_jointly(cgv::render::context& ctx, int layer_idx)
{
	// first draw all bar plots
	unsigned i;
	for (i = 0; i < get_nr_sub_plots(); ++i) {
		draw_bar_plot(ctx, i, layer_idx);
		++layer_idx;
	}
	// next draw stick plots
	for (i = 0; i < get_nr_sub_plots(); ++i) {
		draw_stick_plot(ctx, i, layer_idx);
		++layer_idx;
	}
	// then we draw line plots
	for (i = 0; i < get_nr_sub_plots(); ++i) {
		draw_line_plot(ctx, i, layer_idx);
		++layer_idx;
	}
	// finally draw point plots
	for (i = 0; i < get_nr_sub_plots(); ++i) {
		draw_point_plot(ctx, i, layer_idx);
		++layer_idx;
	}
	return layer_idx;
}
void plot2d::draw_domain(cgv::render::context& ctx, int si, bool no_fill)
{
	tick_labels.clear();
	tick_batches.clear();
	std::vector<box2> R; R.resize(5);
	std::vector<rgb> C; C.resize(5);
	std::vector<float> D; D.resize(5);
	vec2 extent = vec2::from_vec(get_extent());
	float rs = get_domain_config_ptr()->reference_size;
	if (get_domain_config_ptr()->fill && !no_fill) {
		R[0] = box2(-0.5f * extent, 0.5f * extent);
		C[0] = get_domain_config_ptr()->color;
		D[0] = 0.0f;
	}
	for (unsigned ai = 0; ai < 2; ++ai) {
		axis_config& ac = get_domain_config_ptr()->axis_configs[ai];
		axis_config& ao = get_domain_config_ptr()->axis_configs[1 - ai];
		float lw = rs * ac.line_width;
		rgb col = get_domain_config_ptr()->fill ? ac.color : get_domain_config_ptr()->color;
		vec2 axis_extent(0.0f);
		axis_extent[ai] = extent[ai];
		axis_extent[1 - ai] = lw;
		box2 axis_box(-0.5f * axis_extent, 0.5f * axis_extent);
		axis_box.ref_min_pnt()[1 - ai] += 0.5f * (extent[1 - ai] - lw);
		axis_box.ref_max_pnt()[1 - ai] += 0.5f * (extent[1 - ai] - lw);
		R[2 * ai+1] = axis_box;
		C[2 * ai+1] = col;
		D[2 * ai+1] = -layer_depth;
		axis_box.ref_min_pnt()[1 - ai] -= extent[1 - ai] - lw;
		axis_box.ref_max_pnt()[1 - ai] -= extent[1 - ai] - lw;
		R[2 * ai + 2] = axis_box;
		C[2 * ai + 2] = col;
		D[2 * ai + 2] = -layer_depth;
		// axis line
		if (ao.get_attribute_min() < 0 && ao.get_attribute_max() > 0) {
			float axis_plot = ao.plot_space_from_attribute_space(0.0f);
			axis_box.ref_min_pnt()[1 - ai] = axis_plot - 0.5f * lw;
			axis_box.ref_max_pnt()[1 - ai] = axis_plot + 0.5f * lw;
			R.push_back(axis_box);
			C.push_back(ac.color);
			D.push_back(-2 * layer_depth);
		}
	}
	for (unsigned ti = 0; ti < 2; ++ti) {
		for (unsigned ai = 0; ai < 2; ++ai) {
			axis_config& ac = get_domain_config_ptr()->axis_configs[ai];
			float acw = 1.5f * ac.line_width * get_domain_config_ptr()->reference_size;
			axis_config& ao = get_domain_config_ptr()->axis_configs[1 - ai];
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
				// ignore secondary ticks on primary ticks
				if (ti == 1 && fabs(fmod(c_tick, ac.primary_ticks.step)) < 0.00001f)
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
					if (!label_str.empty()) {
						mx[1 - ai] += acw;
						tick_labels.push_back(label_info(mx.to_vec(), label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
						if (ti == 1)
							tick_labels.back().scale = 0.75f;
					}
					mn[1 - ai] = he - dl;
					mx[1 - ai] = he;
					R.push_back(box2(mn, mx));
					C.push_back(ac.color);
					D.push_back(d);
					if (!label_str.empty()) {
						mn[1 - ai] -= acw;
						tick_labels.push_back(label_info(mn.to_vec(), label_str, ai == 0 ? cgv::render::TA_TOP : cgv::render::TA_RIGHT));
						if (ti == 1)
							tick_labels.back().scale = 0.75f;
					}
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
						mn[1 - ai] += acw;
						mx[1 - ai] -= acw;
						mn[ai] += 2.5f * lw;
						mx[ai] += 2.5f * lw;
						tick_labels.push_back(label_info(mx.to_vec(), label_str, cgv::render::TextAlignment(ai == 0 ? cgv::render::TA_TOP+cgv::render::TA_LEFT : cgv::render::TA_RIGHT+cgv::render::TA_BOTTOM)));
						if (ti == 1)
							tick_labels.back().scale = 0.75f;
						tick_labels.push_back(label_info(mn.to_vec(), label_str, cgv::render::TextAlignment(ai == 0 ? cgv::render::TA_BOTTOM+cgv::render::TA_LEFT : cgv::render::TA_LEFT+ cgv::render::TA_BOTTOM)));
						if (ti == 1)
							tick_labels.back().scale = 0.75f;
					}
					break;
				}
			}
			tick_batches.back().label_count = (unsigned)(tick_labels.size() - tick_batches.back().first_label);
		}
	}
	auto& rr = cgv::render::ref_rectangle_renderer(ctx);
	rr.set_render_style(rrs);
	rr.enable_attribute_array_manager(ctx, aam_domain);
	rr.set_rectangle_array(ctx, R);
	rr.set_color_array(ctx, C);
	rr.set_depth_offset_array(ctx, D);
	size_t offset = (get_domain_config_ptr()->fill && !no_fill) ? 0 : 1;
	rr.render(ctx, offset, R.size()-offset);
	rr.disable_attribute_array_manager(ctx, aam_domain);
}

void plot2d::draw_tick_labels(cgv::render::context& ctx, int si)
{
	if (tick_labels.empty() || label_font_face.empty())
		return;
	cgv::tt_gl_font_face_ptr ff = dynamic_cast<cgv::tt_gl_font_face*>(&(*label_font_face));
	if (!ff) {
		ctx.enable_font_face(label_font_face, get_domain_config_ptr()->label_font_size);
		for (const auto& tbc : tick_batches) if (tbc.label_count > 0) {
			ctx.set_color(get_domain_config_ptr()->axis_configs[tbc.ai].color);
			for (unsigned i = tbc.first_label; i < tbc.first_label + tbc.label_count; ++i) {
				const label_info& li = tick_labels[i];
				ctx.set_cursor(li.position, li.label, li.align);
				ctx.output_stream() << li.label;
				ctx.output_stream().flush();
			}
		}
		return;
	}
	else {
		float rs = 0.2f*get_domain_config_ptr()->reference_size;
		ctx.enable_font_face(ff, 5*get_domain_config_ptr()->label_font_size);
		std::vector<cgv::render::textured_rectangle> Q;
		std::vector<rgba> C;
		for (const auto& tbc : tick_batches) if (tbc.label_count > 0) {
			for (unsigned i = tbc.first_label; i < tbc.first_label + tbc.label_count; ++i) {
				const label_info& li = tick_labels[i];
				vec2 pos = vec2::from_vec(li.position);
				pos = ff->align_text(pos, li.label, li.align, li.scale*rs, true);
				unsigned cnt = ff->text_to_quads(pos, li.label, Q, li.scale*rs, true);
				for (unsigned i = 0; i < cnt; ++i)
					C.push_back(get_domain_config_ptr()->axis_configs[tbc.ai].color);
			}
		}
		if (si != -1) {
			vec2 p(0.0f, 0.1f);
			auto& spc = ref_sub_plot2d_config(si);
			vec2 pos = ff->align_text(p, spc.name, cgv::render::TA_NONE, 1.3f * rs, true);
			unsigned cnt = ff->text_to_quads(pos, spc.name, Q, 1.3f * rs, true);
			for (unsigned i = 0; i < cnt; ++i)
				C.push_back(rgb(spc.ref_color.color[0], spc.ref_color.color[1], spc.ref_color.color[2]));
			p[1] -= 1.3f * rs * 20 * get_domain_config_ptr()->label_font_size;
		}
		if (Q.empty())
			return;
		auto& rr = cgv::render::ref_rectangle_renderer(ctx);
		cgv::render::rectangle_render_style rrs = cgv::ref_rectangle_render_style();
		rrs.default_depth_offset = -4 * layer_depth;
		rr.set_render_style(rrs);
		rr.enable_attribute_array_manager(ctx, aam_tick_labels);
		rr.set_textured_rectangle_array(ctx, Q);
		rr.set_color_array(ctx, C);
		ff->ref_texture(ctx).enable(ctx);
		rr.render(ctx, 0, Q.size());
		ff->ref_texture(ctx).disable(ctx);
		rr.disable_attribute_array_manager(ctx, aam_tick_labels);
	}
}

void plot2d::draw(cgv::render::context& ctx)
{
	// store to be changed opengl state
	GLboolean line_smooth = glIsEnabled(GL_LINE_SMOOTH); 
	GLboolean blend = glIsEnabled(GL_BLEND); 
	GLboolean cull_face = glIsEnabled(GL_CULL_FACE);
	GLenum blend_src, blend_dst, depth;
	glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
	glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
	glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth));

	// update state
	glEnable(GL_LINE_SMOOTH);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	// place plot with modelview matrix
	ctx.push_modelview_matrix();
	mat4 R;
	orientation.put_homogeneous_matrix(R);
	ctx.mul_modelview_matrix(cgv::math::translate4<float>(center_location) * R);

	// configure bar prog only once
	configure_bar_plot(ctx);
	// draw all subplots jointly in one plane
	if (dz == 0.0f) {
		if (get_domain_config_ptr()->show_domain) {
			draw_domain(ctx);
			draw_tick_labels(ctx);
		}
		if (legend_components != LC_HIDDEN)
			draw_legend(ctx, -5 * layer_depth);
		if (disable_depth_mask)
			glDepthMask(GL_FALSE);
		else
			glDepthFunc(GL_LEQUAL);
		draw_sub_plots_jointly(ctx, 6);
		if (disable_depth_mask)
			glDepthMask(GL_TRUE);
		else
			glDepthFunc(GL_LESS);
	}
	// draw subplots with offset in back to front order
	else {
		auto M = ctx.get_modelview_matrix();
		double plot_pos_eye_z = M(2, 3);
		double plot_z_eye_z = M(2, 2);
		int i_begin = 0, i_end = get_nr_sub_plots(), i_delta = 1;
		// check if we can not use default order
		if (plot_pos_eye_z * plot_z_eye_z * dz > 0) {
			i_begin = i_end - 1;
			i_end = -1;
			i_delta = -1;
			ctx.mul_modelview_matrix(cgv::math::translate4<float>(vec3(0, 0, (get_nr_sub_plots()-1) * dz)));
		}
		// traverse all subplots in back to front order
		bool fst = true;
		for (int i = i_begin; i != i_end; i += i_delta) {
			if (get_domain_config_ptr()->show_domain) {
				draw_domain(ctx, i, !fst);
				draw_tick_labels(ctx, i);
			}
			if (legend_components != LC_HIDDEN)
				draw_legend(ctx, -5 * layer_depth);
			if (disable_depth_mask)
				glDepthMask(GL_FALSE);
			else
				glDepthFunc(GL_LEQUAL);
			draw_bar_plot(ctx, i, 6);
			draw_stick_plot(ctx, i, 7);
			draw_line_plot(ctx, i, 8);
			draw_point_plot(ctx, i, 9);
			if (disable_depth_mask)
				glDepthMask(GL_TRUE);
			else
				glDepthFunc(GL_LESS);
			ctx.mul_modelview_matrix(cgv::math::translate4<float>(vec3(0, 0, i_delta*dz)));
			fst = false;
		}
	}

	ctx.pop_modelview_matrix();

	// recover opengl state
	if (!line_smooth)
		glDisable(GL_LINE_SMOOTH);
	if (!blend)
		glDisable(GL_BLEND);
	if (cull_face)
		glEnable(GL_CULL_FACE);
	glDepthFunc(depth);
	glBlendFunc(blend_src, blend_dst);
}

/// create the gui for a point subplot
void plot2d::create_point_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	auto& p2dc = reinterpret_cast<plot2d_config&>(pbc);
	plot_base::create_point_config_gui(bp, p, pbc);
}

/// create the gui for a stick subplot
void plot2d::create_stick_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	auto& p2dc = reinterpret_cast<plot2d_config&>(pbc);
	plot_base::create_stick_config_gui(bp, p, pbc);
}

/// create the gui for a bar subplot
void plot2d::create_bar_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	auto& p2dc = reinterpret_cast<plot2d_config&>(pbc);
	plot_base::create_bar_config_gui(bp, p, pbc);
}

void plot2d::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot_base::create_config_gui(bp, p, i);
}

void plot2d::create_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	p.add_decorator("plot2d", "heading");
	p.add_member_control(bp, "multi_y_axis_mode", multi_y_axis_mode, "toggle");
	p.add_member_control(bp, "dz", dz, "value_slider", "min=-1;max=1;step=0.1;ticks=true");
	plot_base::create_gui(bp, p);
	p.add_member_control(bp, "disable_depth_mask", disable_depth_mask, "toggle");
	if (p.begin_tree_node("rectangle", rrs, false, "level=3")) {
		p.align("\a");
		p.add_member_control(bp, "layer_depth", layer_depth, "value_slider", "min=0.000001;max=0.01;step=0.0000001;log=true;ticks=true");
		p.add_gui("rectangle style", rrs);
		p.align("\b");
	}
}

	}
}