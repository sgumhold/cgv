#include "plot3d.h"
#include <libs/cgv_gl/gl/gl.h>
#include <cgv/math/ftransform.h>

namespace cgv {
	namespace plot {


void plot3d_config::set_colors(const rgb& base_color)
{
	plot_base_config::set_colors(base_color);
	surface_color = 0.1f * rgb(1, 1, 1) + 0.9f * base_color;
}

/// overloaded in derived classes to compute complete tick render information
void plot3d::compute_tick_render_information()
{
	/*
	collect_tick_geometry(0, 1, &domain_min(0), &domain_max(0), &extent(0));
	collect_tick_geometry(0, 2, &domain_min(0), &domain_max(0), &extent(0));
	collect_tick_geometry(1, 0, &domain_min(0), &domain_max(0), &extent(0));
	collect_tick_geometry(1, 2, &domain_min(0), &domain_max(0), &extent(0));
	collect_tick_geometry(2, 0, &domain_min(0), &domain_max(0), &extent(0));
	collect_tick_geometry(2, 1, &domain_min(0), &domain_max(0), &extent(0));
	*/
}

plot3d_config::plot3d_config(const std::string& _name) : plot_base_config(_name, 3)
{
	show_points = true;
	show_lines = true;
	show_line_orientation = true;
	show_bars = false;
	samples_per_row = 0;
	show_surface = true;
	wireframe = false;
	surface_color = rgb(0.7f,0.4f,0);
	face_illumination = PFI_PER_FACE;
	bar_percentual_depth = 1.0f;
}

bool plot3d::compute_sample_coordinate_interval(int i, int ai, float& samples_min, float& samples_max)
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

plot3d::plot3d(unsigned nr_attributes) : plot_base(3, nr_attributes)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	acs[0].name = "x"; acs[0].color = rgb(0.4f, 0.2f, 0.2f);
	acs[1].name = "y"; acs[1].color = rgb(0.2f, 0.4f, 0.2f);
	acs[2].name = "z"; acs[2].color = rgb(0.2f, 0.2f, 0.4f);
	for (unsigned ai = 0; ai < nr_attributes; ++ai)
		acs[ai + 3].name = std::string("attribute_") + cgv::utils::to_string(ai);

	brs.culling_mode = cgv::render::CM_FRONTFACE;
	brs.map_color_to_material = cgv::render::CM_COLOR;
	brs.illumination_mode = cgv::render::IM_TWO_SIDED;

	get_domain_config_ptr()->blend_width_in_pixel = 0.0f;

	legend_location[2] = 0.01f;
}

unsigned plot3d::add_sub_plot(const std::string& name)
{
	// determine index of new sub plot
	unsigned i = get_nr_sub_plots();

	// create new config
	if (i == 0)
		configs.push_back(new plot3d_config(name));
	else {
		configs.push_back(new plot3d_config(ref_sub_plot3d_config(i - 1)));
		ref_sub_plot_config(i).name = name;
	}

	// create new point container
	samples.push_back(std::vector<vec3>());
	attribute_source_arrays.push_back(attribute_source_array());
	attribute_source_arrays.back().attribute_sources.push_back(attribute_source(i, 0, 0, 3 * sizeof(float)));
	attribute_source_arrays.back().attribute_sources.push_back(attribute_source(i, 1, 0, 3 * sizeof(float)));
	attribute_source_arrays.back().attribute_sources.push_back(attribute_source(i, 2, 0, 3 * sizeof(float)));

	// return sub plot index
	return i;
}

void plot3d::delete_sub_plot(unsigned i)
{
	delete configs[i];
	configs[i] = 0;
	configs.erase(configs.begin() + i);
	samples.erase(samples.begin() + i);
}

/// set the number of samples of the i-th sub plot to N
void plot3d::set_samples_per_row(unsigned i, unsigned N)
{
	ref_sub_plot3d_config(i).samples_per_row = N;
}

/// return the number of samples per row
unsigned plot3d::get_samples_per_row(unsigned i) const
{
	return const_cast<plot3d*>(this)->ref_sub_plot3d_config(i).samples_per_row;
}

/// return a reference to the plot base configuration of the i-th plot
plot3d_config& plot3d::ref_sub_plot3d_config(unsigned i)
{
	return static_cast<plot3d_config&>(ref_sub_plot_config(i));
}


/// return the samples of the i-th sub plot
std::vector<vec3>& plot3d::ref_sub_plot_samples(unsigned i)
{
	return samples[i];
}


bool plot3d::init(cgv::render::context& ctx)
{
	bool success = true;
	if (!sphere_prog.build_program(ctx, "plot3d_sphere.glpr")) {
		success = false;
		std::cerr << "could not build GLSL program from plot3d_sphere.glpr" << std::endl;
	}
	else
		sphere_prog.allow_context_to_set_color(false);
	if (!stick_prog.build_program(ctx, "plot3d_stick.glpr")) {
		success = false;
		std::cerr << "could not build GLSL program from plot3d_stick.glpr" << std::endl;
	}
	else
		stick_prog.allow_context_to_set_color(false);
	if (!tick_label_prog.build_program(ctx, "plot3d_tick_label.glpr")) {
		success = false;
		std::cerr << "could not build GLSL program from plot3d_tick_label.glpr" << std::endl;
	}
	else
		tick_label_prog.allow_context_to_set_color(false);
	if (!box_prog.build_program(ctx, "plot3d_box.glpr")) {
		std::cerr << "could not build GLSL program from plot3d_box.glpr" << std::endl;
		success = false;
	}
	else
		box_prog.allow_context_to_set_color(false);
	if (!wirebox_prog.build_program(ctx, "plot3d_box_wire.glpr")) {
		success = false;
		std::cerr << "could not build GLSL program from plot3d_box_wire.glpr" << std::endl;
	}
	else
		wirebox_prog.allow_context_to_set_color(false);
	if (!tube_prog.build_program(ctx, "plot3d_tube.glpr", true)) {
		std::cerr << "could not build GLSL program from plot3d_tube.glpr" << std::endl;
		success = false;
	}
	else {
		tube_prog.set_uniform(ctx, "map_color_to_material", 3);
		tube_prog.allow_context_to_set_color(false);
	}

	//if (!surface_prog.is_created()) {
	//	if (!surface_prog.build_program(ctx, "plot3d_surface.glpr")) {
	//		std::cerr << "could not build GLSL program from plot3d_surface.glpr" << std::endl;
	//	}
	//}
	aam_domain.init(ctx);
	cgv::render::ref_box_renderer(ctx, 1);
	cgv::render::ref_cone_renderer(ctx, 1);
	return plot_base::init(ctx);
}

void plot3d::draw_sub_plots(cgv::render::context& ctx)
{
	float rs = 0.2f*get_domain_config_ptr()->reference_size;
	vecn extent = this->extent.to_vec();
	double y_view_angle = 45.0f;
	if (view_ptr)
		y_view_angle = view_ptr->get_y_view_angle();
	float pixel_extent_per_depth = (float)(2.0 * tan(0.5 * 0.0174532925199 * y_view_angle) / ctx.get_height());

	for (unsigned i = 0; i < get_nr_sub_plots(); ++i) {
		size_t count = enable_attributes(ctx, i, samples);
		if (count > 0) {
			const plot3d_config& spc = ref_sub_plot3d_config(i);
			if (spc.show_plot) {
				if (spc.show_points) {
					set_plot_uniforms(ctx, sphere_prog);
					set_mapping_uniforms(ctx, sphere_prog);
					sphere_prog.set_uniform(ctx, "radius_scale", spc.point_size.size * rs);
					sphere_prog.set_uniform(ctx, "map_color_to_material", 7);
					sphere_prog.set_uniform(ctx, "blend_width_in_pixel", get_domain_config_ptr()->blend_width_in_pixel);
					sphere_prog.set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
					sphere_prog.set_uniform(ctx, "halo_width_in_pixel", 0.0f);
					sphere_prog.set_uniform(ctx, "halo_color_strength", 1.0f);
					sphere_prog.set_uniform(ctx, "percentual_halo_width", spc.point_halo_width.size / spc.point_size.size);
					sphere_prog.set_uniform(ctx, "color_index", spc.point_color.color_idx);
					sphere_prog.set_uniform(ctx, "secondary_color_index", spc.point_halo_color.color_idx);
					sphere_prog.set_uniform(ctx, "opacity_index", spc.point_color.opacity_idx);
					sphere_prog.set_uniform(ctx, "secondary_opacity_index", spc.point_halo_color.opacity_idx);
					sphere_prog.set_uniform(ctx, "size_index", spc.point_size.size_idx);
					sphere_prog.set_uniform(ctx, "secondary_size_index", spc.point_halo_width.size_idx);
					sphere_prog.set_attribute(ctx, sphere_prog.get_color_index(), spc.point_color.color);
					sphere_prog.set_attribute(ctx, "secondary_color", spc.point_halo_color.color);
					sphere_prog.set_attribute(ctx, "size", spc.point_size.size);
					sphere_prog.enable(ctx);
					draw_sub_plot_samples(int(count), spc);
					sphere_prog.disable(ctx);
				}
				if (spc.show_bars) {
					unsigned N = (unsigned)count;
					unsigned M = (unsigned)count;
					if (spc.samples_per_row > 0) {
						N = spc.samples_per_row;
						M /= spc.samples_per_row;
					}
					float box_width = spc.bar_percentual_width.size * extent((spc.bar_coordinate_index + 1) % 3) / N;
					float box_depth = spc.bar_percentual_depth.size * extent((spc.bar_coordinate_index + 2) % 3) / M;
					if (spc.bar_outline_width.size > 0) {
						//glLineWidth(spc.bar_outline_width.size);
						set_plot_uniforms(ctx, wirebox_prog);
						set_mapping_uniforms(ctx, wirebox_prog);
						wirebox_prog.set_uniform(ctx, "box_width", box_width);
						wirebox_prog.set_uniform(ctx, "box_depth", box_depth);
						wirebox_prog.set_uniform(ctx, "box_coordinate_index", spc.bar_coordinate_index);
						wirebox_prog.set_uniform(ctx, "box_base_window", spc.bar_base_window);
						wirebox_prog.set_uniform(ctx, "color_index", spc.bar_outline_color.color_idx);
						wirebox_prog.set_uniform(ctx, "secondary_color_index", -1);
						wirebox_prog.set_uniform(ctx, "opacity_index", spc.bar_outline_color.opacity_idx);
						wirebox_prog.set_uniform(ctx, "secondary_opacity_index", -1);
						wirebox_prog.set_uniform(ctx, "size_index", spc.bar_percentual_width.size_idx);
						wirebox_prog.set_uniform(ctx, "secondary_size_index", spc.bar_percentual_depth.size_idx);
						wirebox_prog.set_attribute(ctx, wirebox_prog.get_color_index(), spc.bar_outline_color.color);
						wirebox_prog.enable(ctx);
						draw_sub_plot_samples(int(count), spc);
						wirebox_prog.disable(ctx);
					}
					set_plot_uniforms(ctx, box_prog);
					set_mapping_uniforms(ctx, box_prog);
					box_prog.set_uniform(ctx, "map_color_to_material", 7);
					box_prog.set_uniform(ctx, "box_width", box_width);
					box_prog.set_uniform(ctx, "box_depth", box_depth);
					box_prog.set_uniform(ctx, "box_base_window", spc.bar_base_window);
					box_prog.set_uniform(ctx, "box_coordinate_index", spc.bar_coordinate_index);
					box_prog.set_uniform(ctx, "color_index", spc.bar_color.color_idx);
					box_prog.set_uniform(ctx, "secondary_color_index", -1);
					box_prog.set_uniform(ctx, "opacity_index", spc.bar_color.opacity_idx);
					box_prog.set_uniform(ctx, "secondary_opacity_index", -1);
					box_prog.set_uniform(ctx, "size_index", spc.bar_percentual_width.size_idx);
					box_prog.set_uniform(ctx, "secondary_size_index", spc.bar_percentual_depth.size_idx);
					box_prog.set_attribute(ctx, box_prog.get_color_index(), spc.bar_color.color);
					box_prog.enable(ctx);
					draw_sub_plot_samples(int(count), spc);
					box_prog.disable(ctx);
				}
				if (spc.show_sticks) {
					set_plot_uniforms(ctx, stick_prog);
					set_mapping_uniforms(ctx, stick_prog);
					stick_prog.set_uniform(ctx, "radius_scale", 1.0f);
					stick_prog.set_uniform(ctx, "stick_coordinate_index", spc.stick_coordinate_index);
					stick_prog.set_uniform(ctx, "stick_base_window", spc.stick_base_window);
					stick_prog.set_uniform(ctx, "map_color_to_material", 7);
					stick_prog.set_uniform(ctx, "color_index", spc.stick_color.color_idx);
					stick_prog.set_uniform(ctx, "secondary_color_index", -1);
					stick_prog.set_uniform(ctx, "opacity_index", spc.stick_color.opacity_idx);
					stick_prog.set_uniform(ctx, "secondary_opacity_index", -1);
					stick_prog.set_uniform(ctx, "size_index", spc.stick_width.size_idx);
					stick_prog.set_uniform(ctx, "secondary_size_index", -1);
					stick_prog.set_attribute(ctx, stick_prog.get_color_index(), spc.stick_color.color);
					stick_prog.set_attribute(ctx, "secondary_color", spc.stick_color.color);
					stick_prog.set_attribute(ctx, "size", spc.stick_width.size * rs);
					stick_prog.set_attribute(ctx, "secondary_size", spc.stick_width.size * rs);
					stick_prog.enable(ctx);
					draw_sub_plot_samples(int(count), spc);
					stick_prog.disable(ctx);
				}
				if (spc.show_lines) {
					set_plot_uniforms(ctx, tube_prog);
					set_mapping_uniforms(ctx, tube_prog);
					tube_prog.set_uniform(ctx, "radius_scale", 1.0f);
					tube_prog.set_uniform(ctx, "map_color_to_material", 7);
					tube_prog.set_uniform(ctx, "color_index", spc.line_color.color_idx);
					tube_prog.set_uniform(ctx, "secondary_color_index", -1);
					tube_prog.set_uniform(ctx, "opacity_index", spc.line_color.opacity_idx);
					tube_prog.set_uniform(ctx, "secondary_opacity_index", -1);
					tube_prog.set_uniform(ctx, "size_index", spc.line_width.size_idx);
					tube_prog.set_uniform(ctx, "secondary_size_index", -1);
					tube_prog.set_attribute(ctx, tube_prog.get_color_index(), spc.line_color.color);
					tube_prog.set_attribute(ctx, "size", spc.line_width.size * rs);
					tube_prog.enable(ctx);
					draw_sub_plot_samples(int(count), spc, true);
					tube_prog.disable(ctx);
				}
			}
		}
		disable_attributes(ctx, i);
	}
}

void plot3d::draw_domain(cgv::render::context& ctx)
{
	tick_labels.clear();
	tick_batches.clear();
	const domain_config& dc = *get_domain_config_ptr();
	vecn E = get_extent();
	set_extent(extent.to_vec());
	if (dc.fill) {
		vec3 origin(0.0f);
		cgv::render::box_renderer& br = cgv::render::ref_box_renderer(ctx);
		brs.surface_color = get_domain_config_ptr()->color;
		bool tmp = br.ref_prog().does_context_set_color();
		br.ref_prog().allow_context_to_set_color(false);
		br.set_render_style(brs);
		br.set_position(ctx, origin);
		br.set_extent(ctx, extent);
		br.set_position_is_center(true);
		br.set_color(ctx, dc.color);
		br.render(ctx, 0, 1);
		br.ref_prog().allow_context_to_set_color(tmp);
	}
	// draw axes
	std::vector<vec3> P;
	std::vector<rgb> C;
	std::vector<float> R;
	float rs = get_domain_config_ptr()->reference_size;
	for (unsigned ai = 0; ai < 3; ++ai) {
		int aj = (ai + 1) % 3;
		int ak = (ai + 2) % 3;
		axis_config& ac = get_domain_config_ptr()->axis_configs[ai];
		axis_config& ao = get_domain_config_ptr()->axis_configs[aj];
		axis_config& ap = get_domain_config_ptr()->axis_configs[ak];
		float lw = rs * ac.line_width;
		rgb col = ac.color;
		vec3 D = 0.5f * extent;
		unsigned cnt = 4;
		float c[5][2] = { {-D[aj],-D[ak]}, {D[aj],-D[ak]}, {D[aj],D[ak]}, {-D[aj],D[ak]} };
		// axis line
		if (ao.get_attribute_min() < 0 && ao.get_attribute_max() > 0 &&
			ap.get_attribute_min() < 0 && ap.get_attribute_max() > 0) {
			c[cnt][0] = ao.plot_space_from_attribute_space(0.0f);
			c[cnt][1] = ap.plot_space_from_attribute_space(0.0f);
			++cnt;
		}
		vec3 p;
		for (unsigned ci = 0; ci < cnt; ++ci) {
			p[ai] = -D[ai];
			p[aj] = c[ci][0];
			p[ak] = c[ci][1];
			P.push_back(p);
			C.push_back(col);
			R.push_back(lw);
			p[ai] = D[ai];
			P.push_back(p);
			C.push_back(col);
			R.push_back(lw);

			for (unsigned ti = 0; ti < 2; ++ti) {
				tick_config& tc = ti == 0 ? ac.primary_ticks : ac.secondary_ticks;
				if (tc.type == TT_NONE)
					continue;
				tick_batches.push_back(tick_batch_info(ai, aj, ti == 0, 0, (unsigned)tick_labels.size()));
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
				for (int i = min_i; i <= max_i; ++i) {
					float c_tick = (float)(i * tc.step);
					float c_attr = ac.attribute_space_from_tick_space(c_tick);
					std::string label_str;
					if (tc.label)
						label_str = cgv::utils::to_string(c_attr);
					float c_plot = ac.plot_space_from_window_space(ac.window_space_from_tick_space(c_tick));
					switch (tc.type) {
					case TT_DASH:
						p[ai] = c_plot;
						p[aj] = c[ci][0]-dl;
						p[ak] = c[ci][1];
						P.push_back(p);
						C.push_back(col);
						R.push_back(lw);
						p[aj] = c[ci][0]+dl;
						P.push_back(p);
						C.push_back(col);
						R.push_back(lw);
						if (!label_str.empty())
							tick_labels.push_back(label_info(p.to_vec(), label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
						p[aj] = c[ci][0];
						p[ak] = c[ci][1]-dl;
						P.push_back(p);
						C.push_back(col);
						R.push_back(lw);
						p[ak] = c[ci][1]+dl;
						P.push_back(p);
						C.push_back(col);
						R.push_back(lw);
//						if (!label_str.empty())
//							tick_labels.push_back(label_info(p.to_vec(), label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
						break;
					case TT_LINE:
					case TT_PLANE:
						if (ci < 4) {
							p[ai] = c_plot;
							p[aj] = c[ci][0];
							p[ak] = c[ci][1];
							if (!label_str.empty())
								tick_labels.push_back(label_info(p.to_vec(), label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
							P.push_back(p);
							C.push_back(col);
							R.push_back(lw);
							p[aj] = c[(ci + 1) % 4][0];
							p[ak] = c[(ci + 1) % 4][1];
							P.push_back(p);
							C.push_back(col);
							R.push_back(lw);
						}
						else {
							p[ai] = c_plot;
							p[aj] = c[ci][0];
							p[ak] = c[ci][1];
							if (!label_str.empty())
								tick_labels.push_back(label_info(p.to_vec(), label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
							p[ak] = -D[ak];
							P.push_back(p);
							C.push_back(col);
							R.push_back(lw);
							p[ak] = D[ak];
							P.push_back(p);
							C.push_back(col);
							R.push_back(lw);
							p[aj] = -D[aj];
							p[ak] = c[ci][1];
							P.push_back(p);
							C.push_back(col);
							R.push_back(lw);
							p[aj] = D[aj];
							P.push_back(p);
							C.push_back(col);
							R.push_back(lw);
						}
						break;
					}
				}
				tick_batches.back().label_count = (unsigned)(tick_labels.size() - tick_batches.back().first_label);
			}
		}
	}
	set_extent(E);

	auto& rcr = cgv::render::ref_cone_renderer(ctx);
	rcr.set_render_style(rcrs);
	rcr.enable_attribute_array_manager(ctx, aam_domain);
	rcr.set_position_array(ctx, P);
	rcr.set_color_array(ctx, C);
	rcr.set_radius_array(ctx, R);
	rcr.render(ctx, 0, P.size());
	rcr.disable_attribute_array_manager(ctx, aam_domain);
}

void plot3d::draw_ticks(cgv::render::context& ctx)
{
	if (tick_labels.empty())
		return;
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
}

void plot3d::draw(cgv::render::context& ctx)
{	
	prepare_extents();

	GLboolean blend = glIsEnabled(GL_BLEND); 
	GLenum blend_src, blend_dst, depth;
	glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
	glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
	glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	ctx.push_modelview_matrix();
	mat4 R;
	orientation.put_homogeneous_matrix(R);
	ctx.mul_modelview_matrix(cgv::math::translate4<float>(center_location) * R);
	if (get_domain_config_ptr()->show_domain) {
		draw_domain(ctx);
		draw_ticks(ctx);
	}
	if (legend_components != LC_HIDDEN)
		draw_legend(ctx);

	draw_sub_plots(ctx);
	ctx.pop_modelview_matrix();

	if (!blend)
		glDisable(GL_BLEND);
	glDepthFunc(depth);
	glBlendFunc(blend_src, blend_dst);
}

void plot3d::clear(cgv::render::context& ctx)
{
	sphere_prog.destruct(ctx);
	box_prog.destruct(ctx);
	wirebox_prog.destruct(ctx);
	stick_prog.destruct(ctx);
	tube_prog.destruct(ctx);
//	surface_prog.destruct(ctx);
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_cone_renderer(ctx, -1);
	aam_domain.destruct(ctx);
	plot_base::clear(ctx);
}

void plot3d::create_line_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	plot_base::create_line_config_gui(bp, p, pbc);
	plot3d_config& p3bc = reinterpret_cast<plot3d_config&>(pbc);
	p.add_member_control(bp, "show_orientation", p3bc.show_line_orientation, "check");
}
void plot3d::create_bar_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	plot_base::create_bar_config_gui(bp, p, pbc);
	plot3d_config& p3bc = reinterpret_cast<plot3d_config&>(pbc);
	add_mapped_size_control(p, bp, "depth", p3bc.bar_percentual_depth, "min=0.01;max=1;log=true;ticks=true");
}

void plot3d::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot_base::create_config_gui(bp, p, i);
	plot3d_config& pbc = ref_sub_plot3d_config(i);
	bool show = p.begin_tree_node("surface", pbc.show_surface, false, "level=3;w=100;align=' '");
	p.add_member_control(bp, "show", pbc.show_surface, "toggle", "w=50");
	if (show) {
		p.align("\a");
		p.add_view("samples per row", pbc.samples_per_row);
		p.add_member_control(bp, "wireframe", pbc.wireframe, "check");
		p.add_member_control(bp, "color", pbc.surface_color);
		p.add_member_control(bp, "wireframe", pbc.face_illumination, "dropdown", "enums='none,face,vertex'");
		p.align("\b");
		p.end_tree_node(pbc.show_surface);
	}
}

void plot3d::create_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	p.add_decorator("plot3d", "heading");
	plot_base::create_gui(bp, p);
	if (p.begin_tree_node("rounded cones", rcrs)) {
		p.align("\a");
		p.add_gui("rcrs", rcrs);
		p.align("\b");
		p.end_tree_node(rcrs);
	}
}

	}
}