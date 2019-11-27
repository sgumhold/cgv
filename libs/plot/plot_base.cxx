#include "plot_base.h"
#include <cgv/render/shader_program.h>
#include <cgv/signal/rebind.h>
#include <cgv/render/attribute_array_binding.h>

namespace cgv {
	namespace plot {

std::vector<const char*> plot_base::font_names;
std::string plot_base::font_name_enum_def;

///
plot_base::tick_batch_info::tick_batch_info(int _ai, int _aj, bool _primary, unsigned _first_vertex, unsigned _first_label) 
	: ai(_ai), aj(_aj), primary(_primary), first_vertex(_first_vertex), first_label(_first_label)
{
}

/// set tick config defaults
tick_config::tick_config(bool primary)
{
	step = primary ? 5.0f : 1.0f;
	type = TT_LINE;
	line_width = primary ? 1.5f : 1.0f;
	length = primary ? 2.0f : 1.0f;
	label = primary;
	precision = -1;
}

axis_config::axis_config() : primary_ticks(true), secondary_ticks(false), color(0.3f,0.3f,0.3f)
{
	log_scale = false;
	line_width = 3.0f;
}

domain_config::domain_config(unsigned nr_axes) : color(0.85f,0.85f,0.85f), axis_configs(nr_axes)
{
	show_domain = true;
	fill = true;
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

///
void plot_base_config::set_colors(const rgb& base_color)
{
	stick_color = 0.25f*rgb(0, 0, 0) + 0.75f*base_color;
	point_color = base_color;
	bar_color = 0.5f*rgb(1, 1, 1) + 0.5f*base_color;
	bar_outline_color = base_color;
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

/// check whether tick information has to be updated
bool plot_base::tick_render_information_outofdate() const
{
	if (last_dom_cfg.axis_configs.size() != get_domain_config_ptr()->axis_configs.size())
		return true;
	if (last_dom_min != domain_min)
		return true;
	if (last_dom_max != domain_max)
		return true;
	for (unsigned ai = 0; ai < last_dom_cfg.axis_configs.size(); ++ai) {
		const axis_config& ac = last_dom_cfg.axis_configs[ai];
		const axis_config& ao = get_domain_config_ptr()->axis_configs[ai];
		if (ac.log_scale != ao.log_scale)
			return true;
		for (unsigned ti = 0; ti < 2; ++ti) {
			const tick_config& tc = ti == 0 ? ac.primary_ticks : ac.secondary_ticks;
			const tick_config& to = ti == 0 ? ao.primary_ticks : ao.secondary_ticks;
			if (tc.length != to.length)
				return true;
			if (tc.step != to.step)
				return true;
			if (tc.type != to.type)
				return true;
			if (tc.label != to.label)
				return true;
			if (tc.precision != to.precision)
				return true;
		}
	}
	if (last_dom_cfg.label_font_size!= get_domain_config_ptr()->label_font_size)
		return true;
	if (last_dom_cfg.label_font_index != get_domain_config_ptr()->label_font_index)
		return true;
	if (last_dom_cfg.label_ffa != get_domain_config_ptr()->label_ffa)
		return true;
	return false;
}

/// ensure that tick render information is current
void plot_base::ensure_tick_render_information()
{
	if (tick_render_information_outofdate()) {
		tick_vertices.clear();
		tick_labels.clear();
		tick_batches.clear();
		compute_tick_render_information();
		last_dom_cfg = *get_domain_config_ptr();
		last_dom_min = domain_min;
		last_dom_max = domain_max;
	}
}

float plot_base::log_conform_add(float v0, float v1, bool log_scale, float v_min, float v_max)
{
	if (!log_scale)
		return v0 + v1;
	float q = (v0 - 0.5f*(v_min+v_max)) / (v_max - v_min);
	return convert_from_log_space(q*(convert_to_log_space(v_max, v_min, v_max) - convert_to_log_space(v_min, v_min, v_max)) +
		0.5f*(convert_to_log_space(v_min, v_min, v_max) + convert_to_log_space(v_max, v_min, v_max)), v_min, v_max);
}

float plot_base::convert_to_log_space(float val, float min_val, float max_val)
{
	return log10(std::max(val, 1e-6f*max_val));
}

float plot_base::convert_from_log_space(float val, float min_val, float max_val)
{
	return pow(10.0f, val);
}

plot_base::vec3 plot_base::transform_to_world(const vecn& domain_point) const
{
	unsigned n = domain_point.size();
	vecn delta = extent;
	for (unsigned ai = 0; ai < n; ++ai) {
		float dom_min = domain_min(ai);
		float dom_max = domain_max(ai);
		if (get_domain_config_ptr()->axis_configs[ai].log_scale) {
			delta(ai) *= (convert_to_log_space(domain_point(ai), dom_min, dom_max) -
				0.5f*(convert_to_log_space(dom_min, dom_min, dom_max) + convert_to_log_space(dom_max, dom_min, dom_max))) /
				(convert_to_log_space(dom_max, dom_min, dom_max) - convert_to_log_space(dom_min, dom_min, dom_max));
		}
		else
			delta(ai) *= (domain_point(ai) - 0.5f*(dom_min + dom_max)) / (dom_max - dom_min);
	}
	vec3 d(n, &delta(0));
	return orientation.apply(d) + center_location;
}


void plot_base::collect_tick_geometry(int ai, int aj, const float* dom_min_pnt, const float* dom_max_pnt, const float* extent)
{
	int a0 = ai;
	int a1 = aj;
	if (a0 == 2)
		a0 = 1 - a1;
	if (a1 == 2)
		a1 = 1 - a0;
	axis_config& ac = get_domain_config_ptr()->axis_configs[ai];
	for (unsigned ti=0; ti<2; ++ti) {
		tick_config& tc = ti == 0 ? ac.primary_ticks : ac.secondary_ticks;
		if (tc.type == TT_NONE)
			return;
		tick_batches.push_back(tick_batch_info(ai, aj, ti == 0, (unsigned)tick_vertices.size(), (unsigned)tick_labels.size()));

		axis_config& ao = get_domain_config_ptr()->axis_configs[aj];
		// compute domain extent in both coordinate directions
		float dei = dom_max_pnt[ai] - dom_min_pnt[ai];
		float dej = dom_max_pnt[aj] - dom_min_pnt[aj];
		float dci = dom_min_pnt[ai] + 0.5f*dei;
		float dcj = dom_min_pnt[aj] + 0.5f*dej;

		float min_val = dom_min_pnt[ai];
		float max_val = dom_max_pnt[ai];
		if (ac.log_scale) {
			min_val = convert_to_log_space(min_val, dom_min_pnt[ai], dom_max_pnt[ai]);
			max_val = convert_to_log_space(max_val, dom_min_pnt[ai], dom_max_pnt[ai]);
		}
		int min_i = (int)((min_val - fmod(min_val, tc.step)) / tc.step);
		int max_i = (int)((max_val - fmod(max_val, tc.step)) / tc.step);

		// ignore secondary ticks on domain boundary
		if (ti == 1 && min_i * tc.step - min_val < std::numeric_limits<float>::epsilon())
			++min_i;
		if (ti == 1 && max_i * tc.step - max_val > -std::numeric_limits<float>::epsilon())
			--max_i;

		float dash_length = tc.length*0.01f*dej;
		if (extent[ai] < extent[aj])
			dash_length *= extent[ai] / extent[aj];

		float s_min = dom_min_pnt[aj];
		float s_max = dom_max_pnt[aj];

		for (int i = min_i; i <= max_i; ++i) {
			vec2 c;
			c[a0] = (float)(i*tc.step);
			if (ac.log_scale)
				c[a0] = convert_from_log_space(c[a0], dom_min_pnt[ai], dom_max_pnt[ai]);
			else // ignore secondary ticks on axes
				if (ti == 1 && fabs(c[ai]) < std::numeric_limits<float>::epsilon())
					continue;

			std::string label_str;
			if (tc.label)
				label_str = cgv::utils::to_string(c[a0]);
			switch (tc.type) {
			case TT_DASH:
				// generate label
				if (!label_str.empty()) {
					// left label
					c[a1] = log_conform_add(s_min, -0.5f*dash_length, ao.log_scale, dom_min_pnt[aj], dom_max_pnt[aj]);
					tick_labels.push_back(label_info(c, label_str, ai == 0 ? cgv::render::TA_TOP : cgv::render::TA_RIGHT));
					// right label
					c[a1] = log_conform_add(s_max,  0.5f*dash_length, ao.log_scale, dom_min_pnt[aj], dom_max_pnt[aj]);
					tick_labels.push_back(label_info(c, label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
				}
				// left tick
				c[a1] = s_min;
				tick_vertices.push_back(c);
				c[a1] = log_conform_add(s_min,  dash_length, ao.log_scale, dom_min_pnt[aj], dom_max_pnt[aj]);
				tick_vertices.push_back(c);

				// right tick
				c[a1] = s_max;
				tick_vertices.push_back(c);
				c[a1] = log_conform_add(s_max, -dash_length, ao.log_scale, dom_min_pnt[aj], dom_max_pnt[aj]);
				tick_vertices.push_back(c);

				// non log axis tick
				if (!ao.log_scale && s_min + 0.5f*dash_length < 0 && s_max - 0.5f*dash_length > 0) {
					c[a1] = -0.5f*dash_length;
					tick_vertices.push_back(c);
					c[a1] = 0.5f*dash_length;
					tick_vertices.push_back(c);
				}
				break;
			case TT_LINE:
			case TT_PLANE:
				// generate label
				if (!label_str.empty()) {
					// left label
					c[a1] = log_conform_add(s_min, -0.5f*dash_length, ao.log_scale, dom_min_pnt[aj], dom_max_pnt[aj]);
					tick_labels.push_back(label_info(c, label_str, ai == 0 ? cgv::render::TA_TOP : cgv::render::TA_RIGHT));
					// right label
					c[a1] = log_conform_add(s_max, 0.5f*dash_length, ao.log_scale, dom_min_pnt[aj], dom_max_pnt[aj]);
					tick_labels.push_back(label_info(c, label_str, ai == 0 ? cgv::render::TA_BOTTOM : cgv::render::TA_LEFT));
				}
				c[a1] = s_min; tick_vertices.push_back(c);
				if (tc.label) {
					c(a1) = log_conform_add(c(a1), -0.5f*dash_length, ao.log_scale, dom_min_pnt[aj], dom_max_pnt[aj]);
					tick_labels.push_back(label_info(c, label_str, ai == 0 ? cgv::render::TA_TOP : cgv::render::TA_RIGHT));
				}
				c[a1] = s_max; tick_vertices.push_back(c);
				break;
			}
		}
		tick_batches.back().vertex_count = (unsigned)(tick_vertices.size() - tick_batches.back().first_vertex);
		tick_batches.back().label_count = (unsigned)(tick_labels.size() - tick_batches.back().first_label);
	}
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

void plot_base::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i)
{
	prog.set_uniform(ctx, "orientation", orientation);
	prog.set_uniform(ctx, "extent", vec3(extent.size(),&extent(0)));
	prog.set_uniform(ctx, "domain_min_pnt", vec3(domain_min.size(), &domain_min(0)));
	prog.set_uniform(ctx, "domain_max_pnt", vec3(domain_max.size(), &domain_max(0)));
	prog.set_uniform(ctx, "center_location", center_location);
	prog.set_uniform(ctx, "offset_percentage", 0.0f);
	static const char* axis_name = "xyz";
	for (unsigned ai=0; ai<get_dim(); ++ai)
		prog.set_uniform(ctx, std::string(1, axis_name[ai])+"_axis_log_scale", get_domain_config_ptr()->axis_configs[ai].log_scale);

	if (i >= 0 && i < get_nr_sub_plots()) {
		prog.set_uniform(ctx, "point_color", ref_sub_plot_config(i).point_color);
		prog.set_uniform(ctx, "stick_color", ref_sub_plot_config(i).stick_color);
		prog.set_uniform(ctx, "bar_color", ref_sub_plot_config(i).bar_color);
		prog.set_uniform(ctx, "bar_outline_color", ref_sub_plot_config(i).bar_outline_color);
	}
}

/// set vertex shader input attributes based on attribute source information
size_t plot_base::set_attributes(cgv::render::context& ctx, int i, const std::vector< std::vector<vec2> >& samples)
{
	const auto& ass = attribute_sources[i];
	unsigned ai = 0;
	size_t count = -1;
	for (const auto& as : ass) {
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
		++ai;
	}
	return count;
}

//// set vertex shader input attributes based on attribute source information
size_t plot_base::set_attributes(cgv::render::context& ctx, int i, const std::vector< std::vector<vec3> >& samples)
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

/// set vertex shader input attributes 
void plot_base::set_attributes(cgv::render::context& ctx, const std::vector<vec2>& points)
{
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, &points[0][0], points.size(), 2*sizeof(float));
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 1, &points[0][1], points.size(), 2 * sizeof(float));
}

/// set vertex shader input attributes 
void plot_base::set_attributes(cgv::render::context& ctx, const std::vector<vec3>& points)
{
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 0, &points[0][0], points.size(), 3*sizeof(float));
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 1, &points[0][1], points.size(), 3*sizeof(float));
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, 2, &points[0][2], points.size(), 3*sizeof(float));
}

/// define a sub plot attribute ai from coordinate aj of the i-th internal sample container
void plot_base::set_sub_plot_attribute(unsigned i, unsigned ai, int subplot_index, size_t aj)
{
	assert(attribute_sources.size() > i);
	auto& ass = attribute_sources[i];
	while (ass.size() <= ai)
		ass.resize(ai + 1);
	ass[ai] = attribute_source(subplot_index, aj, 0, get_dim() * sizeof(float));
}
/// define a sub plot attribute from an external pointer
void plot_base::set_sub_plot_attribute(unsigned i, unsigned ai, const float* _pointer, size_t count, size_t stride)
{
	assert(attribute_sources.size() > i);
	auto& ass = attribute_sources[i];
	while (ass.size() <= ai)
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
	return;
	if (count_others < 3)
		prog.set_attribute(ctx, 2, 0.0f);
	if (count_others < 4)
		prog.set_attribute(ctx, 3, 0.0f);
	if (count_others < 5)
		prog.set_attribute(ctx, 4, 0.0f);
}

/// 
void plot_base::enable_attributes(cgv::render::context& ctx, unsigned count)
{
	cgv::render::attribute_array_binding::enable_global_array(ctx, 0);
	cgv::render::attribute_array_binding::enable_global_array(ctx, 1);
	if (count > 2)
		cgv::render::attribute_array_binding::enable_global_array(ctx, 2);
	if (count > 3)
		cgv::render::attribute_array_binding::enable_global_array(ctx, 3);
	if (count > 4)
		cgv::render::attribute_array_binding::enable_global_array(ctx, 4);
}
/// 
void plot_base::disable_attributes(cgv::render::context& ctx, unsigned count)
{
	cgv::render::attribute_array_binding::disable_global_array(ctx, 0);
	cgv::render::attribute_array_binding::disable_global_array(ctx, 1);
	if (count > 2)
		cgv::render::attribute_array_binding::disable_global_array(ctx, 2);
	if (count > 3)
		cgv::render::attribute_array_binding::disable_global_array(ctx, 3);
	if (count > 4)
		cgv::render::attribute_array_binding::disable_global_array(ctx, 4);
}

plot_base::plot_base(unsigned nr_axes) : dom_cfg(nr_axes), last_dom_cfg(0)
{
	dom_cfg_ptr = &dom_cfg;
	domain_min = vecn(nr_axes);
	domain_min.fill(0.0f);
	domain_max = vecn(nr_axes);
	domain_max.fill(1.0f);
	extent = vecn(nr_axes);
	extent.fill(1.0f);
	orientation = quat(1.0f, 0.0f, 0.0f, 0.0f);
	center_location = vec3(0.0f);
}

/// return domain shown in plot
const plot_base::box2 plot_base::get_domain() const
{
	return box2(vec2(2, &domain_min(0)), vec2(2, &domain_max(0)));
}

/// return domain shown in plot
const plot_base::box3 plot_base::get_domain3() const
{
	return box3(vec3(domain_min.size(), &domain_min(0)), vec3(domain_max.size(), &domain_max(0)));
}

/// set the domain in 2d
void plot_base::set_domain(const box2& dom)
{
	reinterpret_cast<vec2&>(domain_min(0)) = dom.get_min_pnt();
	reinterpret_cast<vec2&>(domain_max(0)) = dom.get_max_pnt();
}

/// set the domain for 3d plots
void plot_base::set_domain3(const box3& dom)
{
	unsigned n = std::min(get_dim(), 3u);
	for (unsigned ai = 0; ai < n; ++ai) {
		domain_min(ai) = dom.get_min_pnt()(ai);
		domain_max(ai) = dom.get_max_pnt()(ai);
	}
}
/// reference the domain min point
plot_base::vecn& plot_base::ref_domain_min()
{
	return domain_min;
}
/// reference the domain max point
plot_base::vecn& plot_base::ref_domain_max()
{
	return domain_max;
}

/// query the plot extend in 2D coordinates
const plot_base::vecn& plot_base::get_extent() const
{
	return extent;
}

/// set the plot extend in 2D coordinates
void plot_base::set_extent(const vecn& new_extent)
{
	extent = new_extent;
}

/// set the plot width to given value and if constrained == true the height, such that the aspect ration is the same as the aspect ratio of the domain
void plot_base::set_width(float new_width, bool constrained)
{
	extent(0) = new_width;
	if (constrained) {
		extent(1) = new_width * (domain_max(1)-domain_min(1)) / (domain_max(0) - domain_min(0));
	}
}

/// set the plot height to given value and if constrained == true the width, such that the aspect ration is the same as the aspect ratio of the domain
void plot_base::set_height(float new_height, bool constrained)
{
	extent(1) = new_height;
	if (constrained) {
		extent(0) = new_height * (domain_max(0) - domain_min(0)) / (domain_max(1) - domain_min(1));
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
	return transform_to_world(domain_min);
}

/// return the current plot center in 3D coordinates
const plot_base::vec3& plot_base::get_center() const
{
	return center_location;
}

/// return the i-th plot corner in 3D coordinates
plot_base::vec3 plot_base::get_corner(unsigned i) const
{
	unsigned n = extent.size();
	box3 B(vec3(n, &domain_min(0)), vec3(n, &domain_max(0)));
	vec3 c3 = B.get_corner(i);
	vecn c(n);
	for (unsigned j = 0; j < n; ++j)
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
	if (!found_sample) {
		if (adjust_min)
			domain_min(ai) = 0.0f;
		if (adjust_max)
			domain_max(ai) = 1.0f;
		return;
	}

	if (adjust_min && domain_min(ai) > samples_min)
		domain_min(ai) = samples_min;
	if (adjust_max && domain_max(ai) < samples_max)
		domain_max(ai) = samples_max;
	if (domain_min(ai) == domain_max(ai))
		domain_max(ai) += 1;
}

/// adjust tick marks of all axes based on maximum number of secondary ticks and domain min and max in coordinate of axis
void plot_base::adjust_tick_marks_to_domain(unsigned max_nr_secondary_ticks)
{
	for (unsigned ai = 0; ai < get_dim(); ++ai)
		adjust_tick_marks_to_domain_axis(ai, max_nr_secondary_ticks, domain_min(ai), domain_max(ai));
}

/// adjust the extent such that it has same aspect ration as domain
void plot_base::adjust_extent_to_domain_aspect_ratio(int preserve_ai)
{
	for (int ai = 0; ai < (int)get_dim(); ++ai) {
		if (ai == preserve_ai)
			continue;
		extent(ai) = extent(preserve_ai)*(domain_max(ai) - domain_min(ai)) / (domain_max(preserve_ai) - domain_min(preserve_ai));
	}
}

/// extend domain such that given axis is included
void plot_base::include_axis_to_domain(unsigned ai)
{
	for (unsigned aj = 0; aj < get_dim(); ++aj) {
		if (aj == ai)
			continue;
		if (domain_min(aj) > 0)
			domain_min(aj) = 0;
		if (domain_max(aj) < 0)
			domain_max(aj) = 0;
	}
}

/// adjust all axes of domain to data
void plot_base::adjust_domain_to_data(bool only_visible, bool adjust_x_axis, bool adjust_y_axis, bool adjust_z_axis)
{
	if (adjust_x_axis)
		adjust_domain_axis_to_data(0, true, true, only_visible);
	if (adjust_y_axis)
		adjust_domain_axis_to_data(1, true, true, only_visible);
	if (adjust_z_axis && get_dim() > 2)
		adjust_domain_axis_to_data(2, true, true, only_visible);
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

/// adjust tick marks of single axis based on maximum number of secondary ticks and domain min and max in coordinate of axis
void plot_base::adjust_tick_marks_to_domain_axis(unsigned ai, unsigned max_nr_secondary_ticks, float dom_min, float dom_max)
{
	axis_config& ac = get_domain_config_ptr()->axis_configs[ai];
	float de = dom_max - dom_min;
	if (ac.log_scale)
		de = convert_to_log_space(dom_max, dom_min, dom_max) - convert_to_log_space(dom_min, dom_min, dom_max);

	float reference_step = de / max_nr_secondary_ticks;
	float scale = (float)pow(10, -floor(log10(reference_step)));
	ac.primary_ticks.step   = 50.0f / scale;
	ac.secondary_ticks.step = 10.0f / scale;
	static float magic_numbers[9] = {
		1.5f,  5.0f, 1.0f,
		3.5f, 10.0f, 2.0f,
		7.5f, 20.0f, 5.0f
	};
	for (unsigned i=0; i<9; i += 3) 
		if (scale * reference_step < magic_numbers[i]) {
			ac.primary_ticks.step = magic_numbers[i + 1] / scale;
			ac.secondary_ticks.step = magic_numbers[i + 2] / scale;
			break;
		}
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


/// ensure tick computation
void plot_base::init_frame(cgv::render::context& ctx)
{
	ensure_tick_render_information();
}

void plot_base::create_plot_gui(cgv::base::base* bp, cgv::gui::provider& p)
{
	const char* axis_names = "xyz";

	ensure_font_names();

	if (p.begin_tree_node("dimensions", "heading", false, "level=3")) {
		p.align("\a");
		p.add_gui("center", center_location, "vector", "main_label='heading';gui_type='value_slider';options='min=-100;max=100;log=true;ticks=true'");
		p.add_gui("domain_min", domain_min, "vector", "main_label='heading';gui_type='value_slider';options='min=-10;max=10;log=true;ticks=true'");
		p.add_gui("domain_max", domain_max, "vector", "main_label='heading';gui_type='value_slider';options='min=-10;max=10;log=true;ticks=true'");
		//p.add_gui("domain", domain, "box", "main_label='heading';gui_type='value_slider';options='min=-10;max=10;step=0.1;log=true;ticks=true'");
		p.add_gui("extent", extent, "vector", "main_label='heading';gui_type='value_slider';options='min=0.01;max=100;step=0.001;log=true;ticks=true'");
		p.add_gui("orientation", reinterpret_cast<vec4&>(orientation), "direction", "main_label='heading';gui_type='value_slider'");
		p.align("\b");
	}


	bool open = p.begin_tree_node("domain", get_domain_config_ptr()->show_domain, false, "level=3;options='w=104';align=' '");
	p.add_member_control(bp, "show", get_domain_config_ptr()->show_domain, "toggle", "w=40", " ");
	p.add_member_control(bp, "fill", get_domain_config_ptr()->fill, "toggle", "w=40");
	if (open) {
		p.align("\a");
		p.add_member_control(bp, "fill color", get_domain_config_ptr()->color);
		for (unsigned i = 0; i < get_domain_config_ptr()->axis_configs.size(); ++i) {
			axis_config& ac = get_domain_config_ptr()->axis_configs[i];
			bool show = p.begin_tree_node(std::string(1, axis_names[i]) + " axis", ac.color, false, "level=3;options='w=142';align=' '");
			p.add_member_control(bp, "log", ac.log_scale, "toggle", "w=50");
			if (show) {
				p.align("\a");
				p.add_member_control(bp, "width", ac.line_width, "value_slider", "min=1;max=20;log=true;ticks=true");
				p.add_member_control(bp, "color", ac.color);
				char* tn[2] = { "primary tick", "secondary tick" };
				tick_config* tc[2] = { &ac.primary_ticks, &ac.secondary_ticks };
				for (unsigned ti = 0; ti < 2; ++ti) {
					bool vis = p.begin_tree_node(tn[ti], tc[ti]->label, false, "level=3;options='w=132';align=' '");
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
	bool show = p.begin_tree_node("points", pbc.show_points, false, "level=3;options='w=142';align=' '");
	p.add_member_control(bp, "show", pbc.show_points, "toggle", "w=50");
	if (show) {
		p.align("\a");
			p.add_member_control(bp, "size", pbc.point_size, "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "color", pbc.point_color);
		p.align("\b");
		p.end_tree_node(pbc.show_points);
	}
	show = p.begin_tree_node("sticks", pbc.show_sticks, false, "level=3;options='w=142';align=' '");
	p.add_member_control(bp, "show", pbc.show_sticks, "toggle", "w=50");
	if (show) {
		p.align("\a");
			p.add_member_control(bp, "width", pbc.stick_width, "value_slider", "min=1;max=20;log=true;ticks=true");
			p.add_member_control(bp, "color", pbc.stick_color);
		p.align("\b");
		p.end_tree_node(pbc.show_sticks);
	}
	show = p.begin_tree_node("bars", pbc.show_bars, false, "level=3;options='w=142';align=' '");
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
