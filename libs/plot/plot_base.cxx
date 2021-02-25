#include "plot_base.h"
#include <cgv/render/shader_program.h>
#include <cgv/signal/rebind.h>
#include <cgv/media/color_scale.h>
#include <cgv/math/ftransform.h>
#include <cgv/render/attribute_array_binding.h>
#include <libs/cgv_gl/gl/gl.h>
#include <cgv/render/color_scale.h>
#include <algorithm>

namespace cgv {
	namespace plot {

std::vector<const char*> plot_base::font_names;
std::string plot_base::font_name_enum_def;

///
plot_base::tick_batch_info::tick_batch_info(int _ai, int _aj, bool _primary, unsigned _first_vertex, unsigned _first_label) 
	: ai(_ai), aj(_aj), primary(_primary), first_vertex(_first_vertex), first_label(_first_label)
{
}

domain_config::domain_config(unsigned nr_axes) : color(0.85f,0.85f,0.85f), axis_configs(nr_axes)
{
	show_domain = true;
	fill = true;
	reference_size = 0.001f;
	blend_width_in_pixel = 1.0f;
	label_font_index = -1;
	label_font_size = 16.0f;
	label_ffa = cgv::media::font::FFA_BOLD_ITALIC;
}

plot_base_config::plot_base_config(const std::string& _name, unsigned dim) : name(_name)
{
	show_plot = true;
	begin_sample = 0;
	end_sample = size_t(-1);
	ref_size = 3;
	ref_color = rgb(1, 0, 0);
	ref_opacity = 1.0f;
	bar_percentual_width = 0.75f;
	point_halo_width = 1.0f;
	point_halo_color = rgba(0.5f, 0.5f, 0.5f, 1.0f);
	stick_coordinate_index = dim-1;
	stick_base_window = 0.0f;
	bar_coordinate_index = dim-1;
	bar_base_window = 0.0;
	sub_plot_influence = SPI_ALL;
	set_colors(ref_color.color);
	set_opacities(ref_opacity.opacity);
	set_sizes(ref_size.size);
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
///
void plot_base_config::set_colors(const rgb& base_color)
{
	ref_color.color = base_color;
	if ((sub_plot_influence & SPI_POINT) != 0)
		point_color.color = base_color;
	if ((sub_plot_influence & SPI_POINT_HALO) != 0) {
		cgv::media::color<float, cgv::media::HLS> hls_base(base_color);
		hls_base.S() *= 0.6f;
		point_halo_color.color = hls_base;
	}
	if ((sub_plot_influence & SPI_LINE) != 0)
		line_color.color = 0.25f * rgb(1, 1, 1) + 0.75f * base_color;
	if ((sub_plot_influence & SPI_STICK) != 0)
		stick_color.color = 0.25f * rgb(0, 0, 0) + 0.75f * base_color;
	if ((sub_plot_influence & SPI_BAR) != 0)
		bar_color.color = 0.5f * rgb(1, 1, 1) + 0.5f * base_color;
	if ((sub_plot_influence & SPI_BAR_OUTLINE) != 0)
		bar_outline_color.color = base_color;
}

void plot_base_config::set_color_indices(int idx)
{
	if ((sub_plot_influence & SPI_POINT) != 0)
		point_color.color_idx = idx;
	if ((sub_plot_influence & SPI_POINT_HALO) != 0)
		point_halo_color.color_idx = idx;
	if ((sub_plot_influence & SPI_LINE) != 0)
		line_color.color_idx = idx;
	if ((sub_plot_influence & SPI_STICK) != 0)
		stick_color.color_idx = idx;
	if ((sub_plot_influence & SPI_BAR) != 0)
		bar_color.color_idx = idx;
	if ((sub_plot_influence & SPI_BAR_OUTLINE) != 0)
		bar_outline_color.color_idx = idx;
}

void plot_base_config::set_sizes(float _size)
{
	ref_size.size = _size;
	if ((sub_plot_influence & SPI_POINT) != 0)
		point_size.size = _size;
	if ((sub_plot_influence & SPI_POINT_HALO) != 0)
		point_halo_width.size = 0.2f * _size;
	if ((sub_plot_influence & SPI_LINE) != 0)
		line_width.size = 0.5f * _size;
	if ((sub_plot_influence & SPI_STICK) != 0)
		stick_width.size = 0.4f * _size;
	if ((sub_plot_influence & SPI_BAR_OUTLINE) != 0)
		bar_outline_width.size = 0.2f * _size;
}

void plot_base_config::set_size_indices(int idx)
{
	if ((sub_plot_influence & SPI_POINT) != 0)
		point_size.size_idx = idx;
	if ((sub_plot_influence & SPI_POINT_HALO) != 0)
		point_halo_width.size_idx = idx;
	if ((sub_plot_influence & SPI_LINE) != 0)
		line_width.size_idx = idx;
	if ((sub_plot_influence & SPI_STICK) != 0)
		stick_width.size_idx = idx;
	if ((sub_plot_influence & SPI_BAR_OUTLINE) != 0)
		bar_outline_width.size_idx = idx;
}

void plot_base_config::set_opacities(float _opa)
{
	ref_opacity = _opa;
	if ((sub_plot_influence & SPI_POINT) != 0)
		point_color.color.alpha() = _opa;
	if ((sub_plot_influence & SPI_POINT_HALO) != 0)
		point_halo_color.color.alpha() = _opa;
	if ((sub_plot_influence & SPI_LINE) != 0)
		line_color.color.alpha() = pow(_opa, 0.8f);
	if ((sub_plot_influence & SPI_STICK) != 0)
		stick_color.color.alpha() = pow(_opa, 0.9f);
	if ((sub_plot_influence & SPI_BAR) != 0) 
		bar_outline_color.color.alpha() = pow(_opa, 0.7f);
	if ((sub_plot_influence & SPI_BAR_OUTLINE) != 0)
		bar_color.color.alpha() = pow(_opa, 0.7f);
}

void plot_base_config::set_opacity_indices(int idx)
{
	if ((sub_plot_influence & SPI_POINT) != 0)
		point_color.opacity_idx = idx;
	if ((sub_plot_influence & SPI_POINT_HALO) != 0)
		point_halo_color.opacity_idx = idx;
	if ((sub_plot_influence & SPI_LINE) != 0)
		line_color.opacity_idx = idx;
	if ((sub_plot_influence & SPI_STICK) != 0)
		stick_color.opacity_idx = idx;
	if ((sub_plot_influence & SPI_BAR) != 0)
		bar_outline_color.opacity_idx = idx;
	if ((sub_plot_influence & SPI_BAR_OUTLINE) != 0)
		bar_color.opacity_idx = idx;
}

plot_base_config::~plot_base_config()
{
}

/// constructor for empty sources
attribute_source::attribute_source() : source(AS_NONE), pointer(0), offset(0), count(0), stride(0)
{}

/// constructor for source from sample container
attribute_source::attribute_source(int _sub_plot_index, size_t ai, size_t _count, size_t _stride) :
	source(AS_SAMPLE_CONTAINER), sub_plot_index(_sub_plot_index), offset(ai), count(_count), stride(_stride) {}
/// constructor for source from external data
attribute_source::attribute_source(const float* _pointer, size_t _count, size_t _stride) :
	source(AS_POINTER), pointer(_pointer), offset(0), count(_count), stride(_stride) {}
/// constructor for source from vbo
attribute_source::attribute_source(const cgv::render::vertex_buffer* _vbo_ptr, size_t _offset, size_t _count, size_t _stride) :
source(AS_VBO), vbo_ptr(_vbo_ptr), offset(_offset), count(_count), stride(_stride) {}
/// copy constructor has no magic inside
attribute_source::attribute_source(const attribute_source& as)
{
	source = as.source;
	switch (source) {
	case AS_NONE: pointer = 0; offset = count = stride = 0; break;
	case AS_SAMPLE_CONTAINER: sub_plot_index = as.sub_plot_index; break;
	case AS_POINTER: pointer = as.pointer; break;
	case AS_VBO: vbo_ptr = as.vbo_ptr; break;
	}
	offset = as.offset;
	count  = as.count;
	stride = as.stride;
}

void plot_base::draw_sub_plot_samples(int count, const plot_base_config& spc, bool strip)
{
	if (spc.begin_sample >= spc.end_sample) {
		glDrawArrays(strip ? GL_LINE_STRIP : GL_POINTS, GLint(spc.begin_sample), GLsizei(count - spc.begin_sample));
		if (strip) {
			GLint indices[2] = { count - 1, 0 };
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, indices);
		}
		if (spc.end_sample > 0)
			glDrawArrays(strip ? GL_LINE_STRIP : GL_POINTS, 0, GLsizei(spc.end_sample));
	}
	else {
		if (spc.end_sample == size_t(-1))
			glDrawArrays(strip ? GL_LINE_STRIP : GL_POINTS, GLint(spc.begin_sample), GLsizei(count - spc.begin_sample));
		else
			if (spc.begin_sample < spc.end_sample)
				glDrawArrays(strip ? GL_LINE_STRIP : GL_POINTS, GLint(spc.begin_sample), GLsizei(spc.end_sample - spc.begin_sample));
	}
}


///
plot_base::vec3 plot_base::world_space_from_plot_space(const vecn& pnt_plot) const
{
	vec3 pnt(pnt_plot(0), pnt_plot(1), pnt_plot.size() >= 3 ? pnt_plot(2) : 0.0f);
	return orientation.apply(pnt) + center_location;
}

/// transform from attribute space to world space
plot_base::vec3 plot_base::transform_to_world(const vecn& pnt_attr) const
{
	unsigned n = pnt_attr.size();
	vecn pnt_plot(n);
	const auto& acs = get_domain_config_ptr()->axis_configs;
	for (unsigned ai = 0; ai < n; ++ai) {
		pnt_plot(ai) =
			acs[ai].plot_space_from_window_space(
				acs[ai].window_space_from_tick_space(
					acs[ai].tick_space_from_attribute_space(pnt_attr[ai])));
	}
	return world_space_from_plot_space(pnt_plot);
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
				get_domain_config_ptr()->label_font_index = (unsigned)(iter - font_names.begin());
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

void plot_base::set_plot_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog)
{
	vec3 extent(0.0f);
	vecn attribute_min(8u, 0.0f), attribute_max(8u, 1.0f), axis_log_minimum(8u, 0.000001f);
	cgv::math::vec<int> axis_log_scale(8u, 0);
	for (unsigned ai = 0; ai < get_domain_config_ptr()->axis_configs.size(); ++ai) {
		const auto& ac = get_domain_config_ptr()->axis_configs[ai];
		attribute_min(ai) = ac.get_attribute_min();
		attribute_max(ai) = ac.get_attribute_max();
		if (ai < get_dim())
			extent(ai) = ac.extent;
		axis_log_scale(ai) = ac.get_log_scale() ? 1 : 0;
		axis_log_minimum(ai) = ac.get_log_minimum();
	}
	vec3 E(extent.size(), &extent(0));
	prog.set_uniform(ctx, "extent", E);
	prog.set_uniform_array(ctx, "attribute_min", attribute_min);
	prog.set_uniform_array(ctx, "attribute_max", attribute_max);
	prog.set_uniform_array(ctx, "axis_log_scale", axis_log_scale);
	prog.set_uniform_array(ctx, "axis_log_minimum", axis_log_minimum);
	prog.set_uniform(ctx, "orientation", orientation);
	prog.set_uniform(ctx, "center_location", center_location);
}

/// set the uniforms for defining the mappings to visual variables
void plot_base::set_mapping_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog)
{
	prog.set_uniform_array(ctx, "color_mapping", color_mapping, MAX_NR_COLOR_MAPPINGS);
	prog.set_uniform_array(ctx, "opacity_mapping", opacity_mapping, MAX_NR_COLOR_MAPPINGS);
	prog.set_uniform_array(ctx, "size_mapping", size_mapping, MAX_NR_COLOR_MAPPINGS);
	if (prog.get_uniform_location(ctx, "color_scale_gamma[0]") != -1) {
		cgv::render::configure_color_scale(ctx, prog, color_scale_index, window_zero_position);
		prog.set_uniform_array(ctx, "color_scale_gamma", color_scale_gamma, MAX_NR_COLOR_MAPPINGS);
	}
	if (prog.get_uniform_location(ctx, "opacity_gamma[0]") != -1) {
		prog.set_uniform_array(ctx, "opacity_gamma", opacity_gamma, MAX_NR_OPACITY_MAPPINGS);
		prog.set_uniform_array(ctx, "opacity_min", opacity_min, MAX_NR_OPACITY_MAPPINGS);
		prog.set_uniform_array(ctx, "opacity_max", opacity_max, MAX_NR_OPACITY_MAPPINGS);
		int opa[MAX_NR_OPACITY_MAPPINGS];
		for (int oi=0;oi< MAX_NR_OPACITY_MAPPINGS;++oi)
			opa[oi] = opacity_is_bipolar[oi] ? 1 : 0;
		prog.set_uniform_array(ctx, "opacity_is_bipolar", opa, MAX_NR_OPACITY_MAPPINGS);
		prog.set_uniform_array(ctx, "opacity_window_zero_position", opacity_window_zero_position, MAX_NR_OPACITY_MAPPINGS);
	}
	if (prog.get_uniform_location(ctx, "size_mapping[0]") != -1) {
		prog.set_uniform_array(ctx, "size_gamma", size_gamma, MAX_NR_SIZE_MAPPINGS);
		prog.set_uniform_array(ctx, "size_min", size_min, MAX_NR_SIZE_MAPPINGS);
		prog.set_uniform_array(ctx, "size_max", size_max, MAX_NR_SIZE_MAPPINGS);
	}
}

/// set vertex shader input attributes 
void plot_base::set_attributes(cgv::render::context& ctx, const std::vector<vec2>& points)
{
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, &points[0][0], points.size(), sizeof(vec2));
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 1, &points[0][1], points.size(), sizeof(vec2));
}

/// set vertex shader input attributes 
void plot_base::set_attributes(cgv::render::context& ctx, const std::vector<vec3>& points)
{
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, &points[0][0], points.size(), sizeof(vec3));
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 1, &points[0][1], points.size(), sizeof(vec3));
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 2, &points[0][2], points.size(), sizeof(vec3));
}

/// set vertex shader input attributes based on attribute source information
size_t plot_base::set_attributes(cgv::render::context& ctx, int i, const std::vector<std::vector<vec2>>& samples)
{
	const auto& ass = attribute_sources[i];
	unsigned ai = 0;
	size_t count = -1;
	for (ai = 0; ai < ass.size(); ++ai) {
		const auto& as = ass[ai];
		switch (as.source) {
		case AS_SAMPLE_CONTAINER: {
			int j = as.sub_plot_index == -1 ? i : as.sub_plot_index;
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, ai,
				&samples[j][0][as.offset], samples[j].size(), sizeof(vec2));
			count = std::min(samples[j].size(), count);
			break;
		}
		case AS_POINTER:
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, ai,
				as.pointer, as.count, (unsigned) as.stride);
			count = std::min(as.count, count);
			break;
		case AS_VBO:
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, ai, *as.vbo_ptr,
				cgv::render::element_descriptor_traits<float>::get_type_descriptor(*as.pointer),
				as.count, as.offset, (unsigned) as.stride);
			count = std::min(as.count, count);
			break;
		}
	}
	return count;
}

/// set vertex shader input attributes based on attribute source information
size_t plot_base::set_attributes(cgv::render::context& ctx, int i, const std::vector<std::vector<vec3>>& samples)
{
	const auto& ass = attribute_sources[i];
	unsigned ai = 0;
	size_t count = -1;
	for (const auto& as : ass) {
		switch (as.source) {
		case AS_SAMPLE_CONTAINER: {
			int j = as.sub_plot_index == -1 ? i : as.sub_plot_index;
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, ai,
				&samples[j][0][as.offset], samples[j].size(), sizeof(vec3));
			count = std::min(samples[j].size(), count);
			break;
		}
		case AS_POINTER:
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, ai,
				as.pointer, as.count, (unsigned) as.stride);
			count = std::min(as.count, count);
			break;
		case AS_VBO:
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, ai, *as.vbo_ptr,
				cgv::render::element_descriptor_traits<float>::get_type_descriptor(*as.pointer),
				as.count, as.offset, (unsigned)as.stride);
			count = std::min(as.count, count);
			break;
		}
		++ai;
	}
	return count;
}

/// define a sub plot attribute ai from coordinate aj of the i-th internal sample container
void plot_base::set_sub_plot_attribute(unsigned i, unsigned ai, int subplot_index, size_t aj)
{
	assert(attribute_sources.size() > i);
	auto& ass = attribute_sources[i];
	if (ass.size() <= ai)
		ass.resize(ai + 1);
	ass[ai] = attribute_source(subplot_index, aj, 0, get_dim() * sizeof(float));
}
/// define a sub plot attribute from an external pointer
void plot_base::set_sub_plot_attribute(unsigned i, unsigned ai, const float* _pointer, size_t count, size_t stride)
{
	assert(attribute_sources.size() > i);
	auto& ass = attribute_sources[i];
	if (ass.size() <= ai)
		ass.resize(ai + 1);
	ass[ai] = attribute_source(_pointer, count, stride);
}
/// define a sub plot attribute from a vbo (attribute must be stored in float type in vbo)
void plot_base::set_sub_plot_attribute(unsigned i, unsigned ai, const cgv::render::vertex_buffer* _vbo_ptr, size_t _offset, size_t _count, size_t _stride)
{
	assert(attribute_sources.size() > i);
	auto& ass = attribute_sources[i];
	while (ass.size() <= ai)
		ass.resize(ai + 1);
	ass[ai] = attribute_source(_vbo_ptr, _offset, _count, _stride);
}

void plot_base::set_default_attributes(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned count_others)
{
	for (unsigned i=0; i<count_others; ++i)
		prog.set_attribute(ctx, get_dim()+i, 0.0f);
}

/// 
void plot_base::enable_attributes(cgv::render::context& ctx, unsigned count)
{
	for (unsigned ai=0; ai<count; ++ai)
		cgv::render::attribute_array_binding::enable_global_array(ctx, ai);
}
/// 
void plot_base::disable_attributes(cgv::render::context& ctx, unsigned count)
{
	for (unsigned ai = 0; ai < count; ++ai)
		cgv::render::attribute_array_binding::disable_global_array(ctx, ai);
}

plot_base::plot_base(unsigned _dim, unsigned _nr_attributes) : dom_cfg(_dim+_nr_attributes)
{
	dim = _dim;
	view_ptr = 0;
	nr_attributes = _nr_attributes;
	dom_cfg_ptr = &dom_cfg;
	legend_components = LC_HIDDEN;
	legend_location = vec3(0.8f, 0.6f, 0.0f);
	legend_extent = vec2(0.05f,0.7f);
	legend_color = rgba(0.3f, 0.2f, 0.8f, 1.0f);
	orientation = quat(1.0f, 0.0f, 0.0f, 0.0f);
	center_location = vec3(0.0f);

	for (int ci = 0; ci < MAX_NR_COLOR_MAPPINGS; ++ci) {
		color_mapping[ci] = -1;
		color_scale_index[ci] = cgv::media::CS_TEMPERATURE;
		color_scale_gamma[ci] = 1;
		window_zero_position[ci] = 0.5f;
	}
	for (int oi = 0; oi < MAX_NR_OPACITY_MAPPINGS; ++oi) {
		opacity_mapping[oi] = -1;
		opacity_gamma[oi] = 1.0f;
		opacity_is_bipolar[oi] = false;
		opacity_window_zero_position[oi] = 0.5;
		opacity_min[oi] = 0.1f;
		opacity_max[oi] = 1.0f;
	}

	for (int si = 0; si < MAX_NR_SIZE_MAPPINGS; ++si) {
		size_mapping[si] = -1;
		size_min[si] = 0.1f;
		size_max[si] = 1.0f;
		size_gamma[si] = 1.0f;
	}
}

/// set the view ptr
void plot_base::set_view_ptr(cgv::render::view* _view_ptr)
{
	view_ptr = _view_ptr;
}

void plot_base::draw_legend(cgv::render::context& ctx, float depth_offset)
{
	legend_prog.set_uniform(ctx, "extent", vec3::from_vec(get_extent()));
	set_mapping_uniforms(ctx, legend_prog);
	ctx.push_modelview_matrix();
	// draw legend
	std::vector<vec3> P;
	std::vector<float> V;
	vec3 pmin = legend_location - vec3(0.5f * legend_extent, 0.0f);
	vec3 pmax = legend_location + vec3(0.5f * legend_extent, 0.0f);
	P.push_back(pmin);
	P.push_back(vec3(pmax(0), pmin(1), pmin(2)));
	P.push_back(vec3(pmin(0), pmax(1), pmin(2)));
	P.push_back(pmax);
	V.push_back(0.0f);
	V.push_back(0.0f);
	V.push_back(1.0f);
	V.push_back(1.0f);
	legend_prog.enable(ctx);
	ctx.set_color(legend_color);
	cgv::render::configure_color_scale(ctx, legend_prog, color_scale_index, window_zero_position);
	int pos_idx = legend_prog.get_attribute_location(ctx, "position");
	int val_idx = legend_prog.get_attribute_location(ctx, "value");
	legend_prog.set_uniform(ctx, "depth_offset", depth_offset);
	cgv::render::attribute_array_binding::enable_global_array(ctx, pos_idx);
	cgv::render::attribute_array_binding::enable_global_array(ctx, val_idx);
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, pos_idx, P);
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, val_idx, V);
	if ((legend_components & LC_PRIMARY_COLOR) != 0) {
		legend_prog.set_uniform(ctx, "color_index", 0);
		legend_prog.set_uniform(ctx, "opacity_index", -1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		ctx.mul_modelview_matrix(cgv::math::translate4<float>(vec3(1.1f*legend_extent(0), 0.0f, 0.0f)));
	}
	if ((legend_components & LC_PRIMARY_OPACITY) != 0) {
		legend_prog.set_uniform(ctx, "color_index", -1);
		legend_prog.set_uniform(ctx, "opacity_index", 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		ctx.mul_modelview_matrix(cgv::math::translate4<float>(vec3(1.1f*legend_extent(0), 0.0f, 0.0f)));
	}
	if ((legend_components & LC_SECONDARY_COLOR) != 0) {
		legend_prog.set_uniform(ctx, "color_index", 1);
		legend_prog.set_uniform(ctx, "opacity_index", -1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		ctx.mul_modelview_matrix(cgv::math::translate4<float>(vec3(1.1f*legend_extent(0), 0.0f, 0.0f)));
	}
	if ((legend_components & LC_SECONDARY_OPACITY) != 0) {
		legend_prog.set_uniform(ctx, "color_index", -1);
		legend_prog.set_uniform(ctx, "opacity_index", 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		ctx.mul_modelview_matrix(cgv::math::translate4<float>(vec3(1.1f*legend_extent(0), 0.0f, 0.0f)));
	}
	cgv::render::attribute_array_binding::disable_global_array(ctx, pos_idx);
	cgv::render::attribute_array_binding::disable_global_array(ctx, val_idx);
	legend_prog.disable(ctx);
	ctx.pop_modelview_matrix();
}


/// return domain shown in plot
const plot_base::box2 plot_base::get_domain() const
{
	const auto& acs = get_domain_config_ptr()->axis_configs;
	return box2(vec2(acs[0].get_attribute_min(),acs[1].get_attribute_min()), 
		        vec2(acs[0].get_attribute_max(), acs[1].get_attribute_max()));
}

/// return domain shown in plot
const plot_base::box3 plot_base::get_domain3() const
{
	const auto& acs = get_domain_config_ptr()->axis_configs;
	return box3(vec3(acs[0].get_attribute_min(), acs[1].get_attribute_min(), get_dim() == 2 ? 0 : acs[2].get_attribute_min()), 
		        vec3(acs[0].get_attribute_max(), acs[1].get_attribute_max(), get_dim() == 2 ? 0 : acs[2].get_attribute_max()));
}

/// set the domain in 2d
void plot_base::set_domain(const box2& dom)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	acs[0].set_attribute_range(dom.get_min_pnt()(0), dom.get_max_pnt()(0));
	acs[1].set_attribute_range(dom.get_min_pnt()(1), dom.get_max_pnt()(1));
}

/// set the domain for 3d plots
void plot_base::set_domain3(const box3& dom)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	unsigned n = std::min(get_dim(), 3u);
	for (unsigned ai = 0; ai < n; ++ai)
		acs[ai].set_attribute_range(dom.get_min_pnt()(ai), dom.get_max_pnt()(ai));
}
/// query the plot extend in 2D coordinates
plot_base::vecn plot_base::get_extent() const
{
	const auto& acs = get_domain_config_ptr()->axis_configs;
	vecn extent(get_dim());
	for (unsigned ai = 0; ai < get_dim(); ++ai)
		extent(ai) = acs[ai].extent;
	return extent;
}

/// set the plot extend in 2D coordinates
void plot_base::set_extent(const vecn& new_extent)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	unsigned n = std::min(get_dim(), new_extent.size());
	for (unsigned ai = 0; ai < n; ++ai)
		acs[ai].extent = new_extent(ai);
}

/// set the plot width to given value and if constrained == true the height, such that the aspect ration is the same as the aspect ratio of the domain
void plot_base::set_width(float new_width, bool constrained)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	acs[0].extent = new_width;
	if (constrained) {
		acs[1].extent = new_width *
			(acs[1].tick_space_from_attribute_space(acs[1].get_attribute_max()) - acs[1].tick_space_from_attribute_space(acs[1].get_attribute_max())) / 
			(acs[0].tick_space_from_attribute_space(acs[0].get_attribute_max()) - acs[0].tick_space_from_attribute_space(acs[0].get_attribute_max()));
	}
}

/// set the plot height to given value and if constrained == true the width, such that the aspect ration is the same as the aspect ratio of the domain
void plot_base::set_height(float new_height, bool constrained)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	acs[1].extent = new_height;
	if (constrained) {
		acs[0].extent = new_height *
			(acs[0].tick_space_from_attribute_space(acs[0].get_attribute_max()) - acs[0].tick_space_from_attribute_space(acs[0].get_attribute_max())) /
			(acs[1].tick_space_from_attribute_space(acs[1].get_attribute_max()) - acs[1].tick_space_from_attribute_space(acs[1].get_attribute_max()));
	}
}

/// set new orientation quaternion
void plot_base::set_orientation(const quat& q)
{
	orientation = q;
}

/// place the origin of the plot in 3D to the given location
void plot_base::place_origin(const vec3& new_origin_location)
{
	center_location += new_origin_location - get_origin();
}

/// place the plot extent center in 3D to the given location (this might can change the current origin location) 
void plot_base::place_center(const vec3& new_center_location)
{
	center_location = new_center_location;
}

/// place a corner (0 .. lower left, 1 .. lower right, 2 .. upper left, 3 .. upper right) to a given 3D location ((this might can change the current origin / center location) 
void plot_base::place_corner(unsigned corner_index, const vec3& new_corner_location)
{
	center_location += new_corner_location - get_corner(corner_index);
}

/// return the current origin in 3D coordinates
plot_base::vec3 plot_base::get_origin() const
{
	return transform_to_world(get_domain3().get_min_pnt().to_vec());
}

/// get current orientation quaternion
const plot_base::quat& plot_base::get_orientation() const
{
	return orientation;
}

/// return the current plot center in 3D coordinates
const plot_base::vec3& plot_base::get_center() const
{
	return center_location;
}

/// return the i-th plot corner in 3D coordinates
plot_base::vec3 plot_base::get_corner(unsigned i) const
{
	box3 B = get_domain3();
	vec3 c3 = B.get_corner(i);
	vecn c(get_dim());
	for (unsigned j = 0; j < get_dim(); ++j)
		c(j) = c3(j);
	return transform_to_world(c);
}

/// return true world direction of x, y or z axis
const plot_base::vec3 plot_base::get_axis_direction(unsigned ai) const
{
	vec3 a(0.0f);
	a(ai) = 1.0f;
	return orientation.apply(a);
}

/// adjust the domain with respect to \c ai th axis to the data
void plot_base::adjust_domain_axis_to_data(unsigned ai, bool adjust_min, bool adjust_max, bool only_visible)
{
	bool found_sample = false;
	float samples_min, samples_max;
	for (unsigned i = 0; i < configs.size(); ++i) {
		if (only_visible && !ref_sub_plot_config(i).show_plot)
			continue;
		if (attribute_sources.size() <= i)
			continue;
		if (attribute_sources[i].size() <= ai)
			continue;
		const attribute_source& as = attribute_sources[i][ai];
		if (as.source == AS_NONE)
			continue;
		bool new_found_sample = false;
		float new_samples_min, new_samples_max;
		switch (as.source) {
		case AS_SAMPLE_CONTAINER:
		{
			int j = as.sub_plot_index == -1 ? i : as.sub_plot_index;
			new_found_sample = compute_sample_coordinate_interval(j, (int)as.offset, new_samples_min, new_samples_max);
			break;
		}
		case AS_POINTER:
		{
			const float* ptr = as.pointer;
			for (unsigned j = 0; j < as.count; ++j) {
				if (new_found_sample) {
					new_samples_min = std::min(new_samples_min, *ptr);
					new_samples_max = std::max(new_samples_max, *ptr);
				}
				else {
					new_samples_min = new_samples_max = *ptr;
					new_found_sample = true;
				}
				reinterpret_cast<const char*&>(ptr) += as.stride;
			}
			break;
		}
		case AS_VBO:
			std::cerr << "VBO case not implemented for adjustment of domain axes." << std::endl;
			break;
		}
		if (new_found_sample) {
			if (found_sample) {
				samples_min = std::min(samples_min, new_samples_min);
				samples_max = std::max(samples_max, new_samples_max);
			}
			else {
				samples_min = new_samples_min;
				samples_max = new_samples_max;
				found_sample = true;
			}
		}
	}
	auto& acs = get_domain_config_ptr()->axis_configs;
	if (!found_sample) {
		if (adjust_min)
			acs[ai].set_attribute_minimum(0.0f);
		if (adjust_max)
			acs[ai].set_attribute_maximum(1.0f);
		return;
	}
	if (adjust_min) 
		acs[ai].set_attribute_minimum(samples_min);
	if (adjust_max) 
		acs[ai].set_attribute_maximum(samples_max);
	if (acs[ai].get_attribute_min() == acs[ai].get_attribute_max())
		acs[ai].set_attribute_maximum(acs[ai].get_attribute_max() + 1);
}

/// adjust tick marks of all axes based on maximum number of secondary ticks and domain min and max in coordinate of axis
void plot_base::adjust_tick_marks(unsigned max_nr_secondary_ticks, bool adjust_to_attribute_ranges)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	for (unsigned ai = 0; ai < acs.size(); ++ai) {
		acs[ai].adjust_tick_marks_to_range(max_nr_secondary_ticks);
		if (!adjust_to_attribute_ranges && ai + 1 == get_dim())
			break;
	}
}

/// adjust the extent such that it has same aspect ration as domain
void plot_base::adjust_extent_to_domain_aspect_ratio(int preserve_ai)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	for (int ai = 0; ai < (int)get_dim(); ++ai) {
		if (ai == preserve_ai)
			continue;
		acs[ai].extent = acs[preserve_ai].extent*acs[ai].get_attribute_extent()/acs[preserve_ai].get_attribute_extent();
	}
}

/// extend domain such that given axis is included
void plot_base::include_axis_to_domain(unsigned ai)
{
	auto& acs = get_domain_config_ptr()->axis_configs;
	for (unsigned aj = 0; aj < get_dim(); ++aj) {
		if (aj == ai)
			continue;
		if (acs[aj].get_attribute_min() > 0)
			acs[aj].set_attribute_minimum(0);
		if (acs[aj].get_attribute_max() < 0)
			acs[aj].set_attribute_maximum(0);
	}
}

/// adjust all axes of domain to data
void plot_base::adjust_domain_to_data(bool only_visible)
{
	unsigned n = unsigned(get_domain_config_ptr()->axis_configs.size());
	for (unsigned ai=0; ai < n; ++ai)
		adjust_domain_axis_to_data(ai, true, true, only_visible);
}

/// configure the label font
void plot_base::set_label_font(float font_size, cgv::media::font::FontFaceAttributes ffa, const std::string& font_name)
{
	get_domain_config_ptr()->label_font_size = font_size;
	get_domain_config_ptr()->label_ffa = ffa;
	if (!font_name.empty()) {
		for (auto iter = font_names.begin(); iter != font_names.end(); ++iter)
			if (font_name == *iter) {
				get_domain_config_ptr()->label_font_index = (unsigned)(iter - font_names.begin());
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
	return (unsigned) configs.size();
}

plot_base_config& plot_base::ref_sub_plot_config(unsigned i)
{
	return *configs[i];
}

/// set the colors for all plot features as variation of the given color
void plot_base::set_sub_plot_colors(unsigned i, const rgb& base_color)
{
	ref_sub_plot_config(i).set_colors(base_color);
}

bool plot_base::init(cgv::render::context& ctx)
{
	if (!legend_prog.build_program(ctx, "plot_legend.glpr", true)) {
		std::cerr << "could not build GLSL program from plot_legend.glpr" << std::endl;
		return false;
	}
	return true;
}

void plot_base::create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	const char* axis_names = "xyz";

	ensure_font_names();
	p.add_member_control(bp, "reference_size", get_domain_config_ptr()->reference_size, "value_slider", "min=0.0001;step=0.00001;max=1;log=true;ticks=true");
	p.add_member_control(bp, "blend_width_in_pixel", get_domain_config_ptr()->blend_width_in_pixel, "value_slider", "min=0.0001;step=0.0;max=3.0;ticks=true");

	if (p.begin_tree_node("visual variables", dim, false, "level=3")) {
		p.align("\a");
		bool show;
		unsigned nr = std::max(MAX_NR_COLOR_MAPPINGS, std::max(MAX_NR_SIZE_MAPPINGS, MAX_NR_OPACITY_MAPPINGS));
		static const char* prefixes[] = { "primary_", "secondary_", "ternary_" };
		for (unsigned idx = 0; idx < nr; ++idx) {
			std::string prefix(prefixes[idx]);
			if (idx < MAX_NR_COLOR_MAPPINGS) {
				show = p.begin_tree_node(prefix + "color", color_mapping[idx], false, "level=3;align=' ';options='w=100'");
				p.add_member_control(bp, "", (cgv::type::DummyEnum&)color_mapping[idx], "dropdown", "w=92;enums='off=-1,attr0,attr1,attr2,attr3,attr4,attr5,attr6,attr7'");
				if (show) {
					p.align("\a");
					p.add_member_control(bp, prefix + "color_scale", (cgv::type::DummyEnum&)color_scale_index[idx], "dropdown", cgv::media::get_color_scale_enum_definition());
					p.add_member_control(bp, prefix + "color_gamma", color_scale_gamma[idx], "value_slider", "min=0.1;step=0.01;max=10;log=true;ticks=true");
					p.add_member_control(bp, prefix + "window_zero_position", window_zero_position[idx], "value_slider", "min=0;max=1;ticks=true");
					p.align("\b");
					p.end_tree_node(color_mapping[idx]);
				}
			}
			if (idx < MAX_NR_OPACITY_MAPPINGS) {
				show = p.begin_tree_node(prefix + "opacity", opacity_mapping[idx], false, "level=3;align=' ';options='w=100'");
				p.add_member_control(bp, "", (cgv::type::DummyEnum&)opacity_mapping[idx], "dropdown", "w=92;enums='off=-1,attr0,attr1,attr2,attr3,attr4,attr5,attr6,attr7'");
				if (show) {
					p.align("\a");
					p.add_member_control(bp, prefix + "opacity_gamma", opacity_gamma[idx], "value_slider", "min=0.1;step=0.01;max=10;log=true;ticks=true");
					p.add_member_control(bp, prefix + "opacity_is_bipolar", opacity_is_bipolar[idx], "check");
					p.add_member_control(bp, prefix + "opacity_window_zero_position", opacity_window_zero_position[idx], "value_slider", "min=0;max=1;ticks=true");
					p.add_member_control(bp, prefix + "opacity_min", opacity_min[idx], "value_slider", "min=0;step=0.01;max=1;ticks=true");
					p.add_member_control(bp, prefix + "opacity_max", opacity_max[idx], "value_slider", "min=0;step=0.01;max=1;ticks=true");
					p.align("\b");
					p.end_tree_node(opacity_mapping[idx]);
				}
			}
			if (idx < MAX_NR_SIZE_MAPPINGS) {
				show = p.begin_tree_node(prefix + "size", size_mapping[idx], false, "level=3;align=' ';options='w=100'");
				p.add_member_control(bp, "", (cgv::type::DummyEnum&)size_mapping[idx], "dropdown", "w=92;enums='off=-1,attr0,attr1,attr2,attr3,attr4,attr5,attr6,attr7'");
				if (show) {
					p.align("\a");
					p.add_member_control(bp, prefix+"size_gamma", size_gamma[idx], "value_slider", "min=0.1;step=0.01;max=10;log=true;ticks=true");
					p.add_member_control(bp, prefix+"size_min", size_min[idx], "value_slider", "min=0.1;step=0.01;max=10;log=true;ticks=true");
					p.add_member_control(bp, prefix+"size_max", size_max[idx], "value_slider", "min=0.1;step=0.01;max=10;log=true;ticks=true");
					p.align("\b");
					p.end_tree_node(size_mapping);
				}
			}
		}
		p.align("\b");
		p.end_tree_node(dim);
	}
	if (p.begin_tree_node("legend", legend_components, false, "level=3")) {
		p.align("\a");
		p.add_member_control(bp, "legend_color", legend_color);
		p.add_gui("legend_components", legend_components, "bit_field_control", "enums='primary color=1,secondary color=2,primary opacity=4,secondary opacity=8,primary size=16,secondary size=32'");
		p.add_gui("center", legend_location, "vector", "main_label='heading';gui_type='value_slider';options='min=-1.2;max=1.2;log=true;ticks=true'");
		p.add_gui("extent", legend_extent, "vector", "main_label='heading';gui_type='value_slider';options='min=0.01;max=10;step=0.001;log=true;ticks=true'");
		p.align("\b");
		p.end_tree_node(legend_components);
	}

	if (p.begin_tree_node("placement", center_location, false, "level=3")) {
		p.align("\a");
		p.add_gui("center", center_location, "vector", "main_label='heading';gui_type='value_slider';options='min=-100;max=100;log=true;ticks=true'");
		p.add_gui("orientation", reinterpret_cast<vec4&>(orientation), "direction", "main_label='heading';gui_type='value_slider'");
		p.align("\b");
		p.end_tree_node(center_location);
	}
	bool open = p.begin_tree_node("domain", get_domain_config_ptr()->show_domain, false, "level=3;options='w=104';align=' '");
	p.add_member_control(bp, "show", get_domain_config_ptr()->show_domain, "toggle", "w=40", " ");
	p.add_member_control(bp, "fill", get_domain_config_ptr()->fill, "toggle", "w=40");
	if (open) {
		p.align("\a");
		p.add_member_control(bp, "fill color", get_domain_config_ptr()->color);
		for (unsigned i = 0; i < get_domain_config_ptr()->axis_configs.size(); ++i)
			get_domain_config_ptr()->axis_configs[i].create_gui(bp, p);
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

void plot_base::update_ref_opacity(unsigned i, cgv::gui::provider& p)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	pbc.set_opacities(pbc.ref_opacity.opacity);
	p.update_member(&pbc.point_color.color.alpha());
	p.update_member(&pbc.point_halo_color.color.alpha());
	p.update_member(&pbc.line_color.color.alpha());
	p.update_member(&pbc.stick_color.color.alpha());
	p.update_member(&pbc.bar_color.color.alpha());
	p.update_member(&pbc.bar_outline_color.color.alpha());
}

void plot_base::update_ref_opacity_index(unsigned i, cgv::gui::provider& p)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	pbc.set_opacity_indices(pbc.ref_opacity.opacity_idx);
	p.update_member(&pbc.point_color.opacity_idx);
	p.update_member(&pbc.point_halo_color.opacity_idx);
	p.update_member(&pbc.line_color.opacity_idx);
	p.update_member(&pbc.stick_color.opacity_idx);
	p.update_member(&pbc.bar_color.opacity_idx);
	p.update_member(&pbc.bar_outline_color.opacity_idx);
}

void plot_base::update_ref_color(unsigned i, cgv::gui::provider& p)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	pbc.set_colors(pbc.ref_color.color);
	p.update_member(&pbc.point_color.color);
	p.update_member(&pbc.point_halo_color.color);
	p.update_member(&pbc.line_color.color);
	p.update_member(&pbc.stick_color.color);
	p.update_member(&pbc.bar_color.color);
	p.update_member(&pbc.bar_outline_color.color);
}

void plot_base::update_ref_color_index(unsigned i, cgv::gui::provider& p)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	pbc.set_color_indices(pbc.ref_color.color_idx);
	p.update_member(&pbc.point_color.color_idx);
	p.update_member(&pbc.point_halo_color.color_idx);
	p.update_member(&pbc.line_color.color_idx);
	p.update_member(&pbc.stick_color.color_idx);
	p.update_member(&pbc.bar_color.color_idx);
	p.update_member(&pbc.bar_outline_color.color_idx);
}

void plot_base::update_ref_size(unsigned i, cgv::gui::provider& p)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	pbc.set_sizes(pbc.ref_size.size);
	p.update_member(&pbc.point_size.size);
	p.update_member(&pbc.point_halo_width.size);
	p.update_member(&pbc.line_width.size);
	p.update_member(&pbc.stick_width.size);
	p.update_member(&pbc.bar_outline_width.size);
}

void plot_base::update_ref_size_index(unsigned i, cgv::gui::provider& p)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	p.update_member(&pbc.point_size.size_idx);
	p.update_member(&pbc.point_halo_width.size_idx);
	p.update_member(&pbc.line_width.size_idx);
	p.update_member(&pbc.stick_width.size_idx);
	p.update_member(&pbc.bar_outline_width.size_idx);
}

void plot_base::create_base_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	p.add_member_control(bp, "name", pbc.name);
	p.add_gui("sub plot influence", pbc.sub_plot_influence, "bit_field_control", 
		"gui_type='toggle';align='';options='w=32';enums='pnt=1,halo=2,line=4,stk=8,bar=16,out=32'");
	p.align("\n");
	connect_copy(p.add_member_control(bp, "color", pbc.ref_color.color, "", "w=100", " ")->value_change,
		cgv::signal::rebind(this, &plot_base::update_ref_color, cgv::signal::_c(i), cgv::signal::_r(p)));
	connect_copy(p.add_member_control(bp, "", (cgv::type::DummyEnum&)pbc.ref_color.color_idx, "dropdown", "w=92;enums='off=-1,primary=0,secondary=1'")->value_change,
		cgv::signal::rebind(this, &plot_base::update_ref_color_index, cgv::signal::_c(i), cgv::signal::_r(p)));

	connect_copy(p.add_member_control(bp, "opacity", pbc.ref_opacity.opacity, "value_slider", "min=0;max=1;ticks=true;w=100", " ")->value_change,
		cgv::signal::rebind(this, &plot_base::update_ref_opacity, cgv::signal::_c(i), cgv::signal::_r(p)));
	connect_copy(p.add_member_control(bp, "", (cgv::type::DummyEnum&)pbc.ref_opacity.opacity_idx, "dropdown", "w=92;enums='off=-1,primary=0,secondary=1'")->value_change,
		cgv::signal::rebind(this, &plot_base::update_ref_opacity_index, cgv::signal::_c(i), cgv::signal::_r(p)));

	connect_copy(p.add_member_control(bp, "size", pbc.ref_size.size, "value_slider", "min=1;max=20;log=true;ticks=true;w=100", " ")->value_change,
		cgv::signal::rebind(this, &plot_base::update_ref_size, cgv::signal::_c(i), cgv::signal::_r(p)));
	connect_copy(p.add_member_control(bp, "", (cgv::type::DummyEnum&)pbc.ref_size.size_idx, "dropdown", "w=92;enums='off=-1,primary=0,secondary=1'")->value_change,
		cgv::signal::rebind(this, &plot_base::update_ref_size_index, cgv::signal::_c(i), cgv::signal::_r(p)));

	p.add_member_control(bp, "begin", pbc.begin_sample, "value_slider", "min=0;ticks=true")->set("max", attribute_sources[i].front().count - 1);
	p.add_member_control(bp, "end", pbc.end_sample, "value_slider", "min=-1;ticks=true")->set("max", attribute_sources[i].front().count - 1);
}

void plot_base::add_mapped_size_control(cgv::gui::provider& p, cgv::base::base* bp, const std::string& name, mapped_size& ms, std::string options)
{
	if (options.empty())
		options = "min=1;max=20;log=true;ticks=true;w=100";
	else
		options += ";w=140";
	p.add_member_control(bp, name, ms.size, "value_slider", options, " ");
	p.add_member_control(bp, "si", (cgv::type::DummyEnum&)ms.size_idx, "dropdown", "enums='off=-1,primary,secondary';w=52");
}

void plot_base::add_mapped_rgb_control(cgv::gui::provider& p, cgv::base::base* bp, const std::string& name, mapped_rgb& ms)
{
	p.add_member_control(bp, name, ms.color, "", "w=140", " ");
	p.add_member_control(bp, "ci", (cgv::type::DummyEnum&)ms.color_idx, "dropdown", "enums='off=-1,primary,secondary';w=52");
}
void plot_base::add_mapped_rgba_control(cgv::gui::provider& p, cgv::base::base* bp, const std::string& name, mapped_rgba& ms)
{
	p.add_member_control(bp, name, ms.color, "", "w=80", " ");
	p.add_member_control(bp, "ci", (cgv::type::DummyEnum&)ms.color_idx, "dropdown", "enums='off=-1,primary,secondary';w=42", " ");
	p.add_member_control(bp, "oi", (cgv::type::DummyEnum&)ms.opacity_idx, "dropdown", "enums='off=-1,primary,secondary';w=42");
}
void plot_base::add_mapped_opacity_control(cgv::gui::provider& p, cgv::base::base* bp, const std::string& name, mapped_opacity& ms)
{
	p.add_member_control(bp, name, ms.opacity, "", "w=140", " ");
	p.add_member_control(bp, "oi", (cgv::type::DummyEnum&)ms.opacity_idx, "dropdown", "enums='off=-1,primary,secondary';w=52");
}

void plot_base::create_point_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	add_mapped_rgba_control(p, bp, "color", pbc.point_color);
	add_mapped_rgba_control(p, bp, "halo_color", pbc.point_halo_color);
	add_mapped_size_control(p, bp, "size", pbc.point_size);
	add_mapped_size_control(p, bp, "halo_width", pbc.point_halo_width, "min=-8;max=8;ticks=true");
}
void plot_base::create_line_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	add_mapped_rgba_control(p, bp, "color", pbc.line_color);
	add_mapped_size_control(p, bp, "width", pbc.line_width);
}
void plot_base::create_stick_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	p.add_member_control(bp, "crd_idx", pbc.stick_coordinate_index, "value_slider", "min=0;max=1;ticks=true")
		->set("max", get_dim()-1);
	p.add_member_control(bp, "base", pbc.stick_base_window, "value_slider", "min=0;max=1;ticks=true");
	add_mapped_rgba_control(p, bp, "color", pbc.stick_color);
	add_mapped_size_control(p, bp, "width", pbc.stick_width);
}
void plot_base::create_bar_config_gui(cgv::base::base* bp, cgv::gui::provider& p, plot_base_config& pbc)
{
	p.add_member_control(bp, "crd_idx", pbc.bar_coordinate_index, "value_slider", "min=0;max=1;ticks=true")
		->set("max", get_dim() - 1);
	p.add_member_control(bp, "base", pbc.bar_base_window, "value_slider", "min=0;max=1;ticks=true");
	add_mapped_rgba_control(p, bp, "color", pbc.bar_color);
	add_mapped_rgba_control(p, bp, "outline", pbc.bar_outline_color);
	add_mapped_size_control(p, bp, "outline_width", pbc.bar_outline_width);
	add_mapped_size_control(p, bp, "width", pbc.bar_percentual_width, "min=0.01;max=1;log=true;ticks=true");
}
void plot_base::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot_base_config& pbc = ref_sub_plot_config(i);
	bool show = p.begin_tree_node(pbc.name, pbc.show_plot, true, "level=3;options='w=142';align=' '");
	p.add_member_control(bp, "show", pbc.show_points, "toggle", "w=50");
	if (show) {
		p.align("\a");
		create_base_config_gui(bp, p, i);
		p.align("\b");
		p.end_tree_node(pbc.show_plot);
	}
	show = p.begin_tree_node("points", pbc.show_points, false, "level=3;options='w=142';align=' '");
	p.add_member_control(bp, "show", pbc.show_points, "toggle", "w=50");
	if (show) {
		p.align("\a");
		create_point_config_gui(bp, p, pbc);
		p.align("\b");
		p.end_tree_node(pbc.show_points);
	}
	show = p.begin_tree_node("lines", pbc.show_lines, false, "level=3;options='w=142';align=' '");
	p.add_member_control(bp, "show", pbc.show_lines, "toggle", "w=50");
	if (show) {
		p.align("\a");
		create_line_config_gui(bp, p, pbc);
		p.align("\b");
		p.end_tree_node(pbc.show_lines);
	}
	show = p.begin_tree_node("sticks", pbc.show_sticks, false, "level=3;options='w=142';align=' '");
	p.add_member_control(bp, "show", pbc.show_sticks, "toggle", "w=50");
	if (show) {
		p.align("\a");
		create_stick_config_gui(bp, p, pbc);
		p.align("\b");
		p.end_tree_node(pbc.show_sticks);
	}
	show = p.begin_tree_node("bars", pbc.show_bars, false, "level=3;options='w=142';align=' '");
	p.add_member_control(bp, "show", pbc.show_bars, "toggle", "w=50");
	if (show) {
		p.align("\a");
		create_bar_config_gui(bp, p, pbc);
		p.align("\b");
		p.end_tree_node(pbc.show_bars);
	}
}

void plot_base::create_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	create_plot_gui(bp, p);
	for (unsigned i=0; i<get_nr_sub_plots(); ++i) {
		plot_base_config& pbc = ref_sub_plot_config(i);
		bool show = p.begin_tree_node(std::string("configure ")+pbc.name, pbc.name, false, "level=3;options='w=142';align=' '");
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
