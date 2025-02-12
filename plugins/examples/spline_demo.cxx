#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/find_action.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/application.h>
#include <cgv/render/drawable.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/view.h>
#include <cgv/render/texture.h>
#include <cgv/data/data_view.h>
#include <cgv_gl/point_renderer.h>
#include <cgv_gl/line_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/color_scale.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv_gl/gl/gl.h>
#include <vector>
#include <fstream>


using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::math;

class spline_demo : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
protected:
	bool is_open = true;
	bool clamp_start = true;
	bool clamp_end = true;
	unsigned degree = 3;
	unsigned max_nr_segments = 0;
	unsigned nr_steps = 20;
	cgv::media::ColorScale cs = cgv::media::CS_HUE;
	std::vector<float> knot_vector;
	std::vector<float> segment_widths;
	float range;
	void recompute_segment_colors()
	{
		segment_colors.resize(segment_widths.size());
		if (segment_colors.empty())
			return;
		if (segment_colors.size() == 1) {
			segment_colors[0] = cgv::media::color_scale(0.0, cs);
			return;
		}
		for (size_t i = 0; i < segment_colors.size(); ++i) {
			double x = double(i) / segment_colors.size();
			if (is_open) {
				float v = 0.3f + 0.4f * (i & 1);
				if (i+1 < degree || i+degree > segment_colors.size()) {
					segment_colors[i] = cgv::rgb(v, v, v);
					continue;
				}
				x = double(i+1 - degree) / (segment_colors.size()+2 - 2 * degree);
			}
			segment_colors[i] = cgv::media::color_scale(x, cs);
		}
	}
	void recompute_segment_widths()
	{
		unsigned N = (unsigned)polygon.size();
		bool did_resize = false;
		if (is_open) {
			unsigned W = N + degree - 2;
			if (segment_widths.size() != W) {
				segment_widths.resize(W);
				did_resize = true;
			}
			for (unsigned i = 0; i < W; ++i) {
				if (is_open && ((clamp_start && (i+1 < degree)) || (clamp_end && (i+1 >= N))))
					segment_widths[i] = 0;
				else
					segment_widths[i] = 1;
			}
		}
		else {
			if (segment_widths.size() != N+1) {
				segment_widths.resize(N+1);
				std::fill(segment_widths.begin(), segment_widths.end(), 1.0f);
				did_resize = true;
			}
		}
		if (did_resize) {
			recompute_segment_colors();
			post_recreate_gui();
		}
		else
			for (auto& sw : segment_widths)
				update_member(&sw);
		update_knot_vector_from_segment_widths();
	}
	void update_knot_vector_from_segment_widths()
	{
		unsigned N = (unsigned)polygon.size();
		unsigned K = is_open ? N + degree - 1 : N + 1;
		bool did_resize = false;
		if (knot_vector.size() != K) {
			knot_vector.resize(K);
			did_resize = true;
		}
		knot_vector.front() = 0.0f;
		for (unsigned i = 1; i < K; ++i)
			knot_vector[i] = knot_vector[i-1] + segment_widths[i - 1];

		if (!did_resize) {
			for (auto& nv : knot_vector)
				update_member(&nv);
		}
		else
			post_recreate_gui();


		recompute_spline();
	}
	void recompute_knot_vector()
	{
		unsigned K = (unsigned)polygon.size();
		unsigned m = is_open ? K + degree - 1 : K;
		knot_vector.resize(m);
		float v = 0;
		for (unsigned i = 0; i < m; ++i) {
			knot_vector[i] = v;
			if (is_open && ((clamp_start && (i+1 < degree)) || (clamp_end && (i >= K))))
				continue;
			v += 1;
		}
		recompute_spline();
		post_recreate_gui();
		post_redraw();
	}
	cgv::vec2 eval_spline(float t, unsigned k)
	{
		// short name for knots u[k]
		const auto& u = knot_vector;
		// K is number of knots indexed with k
		unsigned K = unsigned(u.size());
		// d is spline degree
		unsigned D = unsigned(degree);
		// N is number of control points indexed with i
		unsigned N = unsigned(polygon.size());
		// extract influencing D+1 deBoor points indexed with j
		std::vector<cgv::vec2> d(D+1, cgv::vec2(0.0f));
		for (unsigned j = 0; j <= D; ++j)
			d[j] = polygon[(N+k+j-D)%N];
		// iterate recursion index r = 1 ... D 
		for (unsigned r = 1; r <= D; ++r) {
			// iterate deBoor point index j = D ... r downwards to support in place computation
			for (unsigned j = D; j >= r; --j) {
				// compute knot index range
				unsigned i0 = (K + k + j - D) % K;
				unsigned i1 = (K + k + j + 1 - r) % K;
				float alpha = (t - u[i0])/(u[i1]-u[i0]);
				d[j] = (1 - alpha) * d[j-1] + alpha*d[j];
			}
		}
		return d[D];
	}
	cgv::vec2 eval_spline_sg(float t, unsigned j)
	{
		// short name for knots u[k]
		const auto& u = knot_vector;
		// K is number of knots indexed with k
		unsigned K = unsigned(u.size());
		// d is spline degree
		unsigned D = unsigned(degree);
		// N is number of control points indexed with i
		unsigned N = unsigned(polygon.size());
		// extract influencing D+1 deBoor points indexed with j
		std::vector<cgv::vec2> d(D+1);
		std::copy(polygon.begin() + j, polygon.begin() + j + D + 1, d.begin());
		// iterate recursion index r = 1 ... D 
		for (unsigned r = 1; r <= D; ++r) {
			// iterate deBoor point index i = D ... r downwards to support in place computation
			for (unsigned i = D; i >= r; --i) {
				// compute knot index range
				unsigned k0 = j + i - 1;
				unsigned k1 = j + D + i - r;
				float alpha = (t - u[k0])/(u[k1]-u[k0]);
				d[i] = (1 - alpha) * d[i-1] + alpha*d[i];
			}
		}
		return d[D];
	}

	void recompute_spline()
	{
		sampled_spline.clear();
		spline_colors.clear();
		if (polygon.empty())
			return;
		bool tmp_is_open = is_open;
		std::vector<cgv::vec2> tmp_polygon;
		std::vector<float> tmp_knot_vector;
		if (!is_open) {
			tmp_knot_vector = knot_vector;
			tmp_polygon = polygon;
			for (unsigned i = 0; i < degree+2; ++i)
				polygon.push_back(polygon[i]);
			for (unsigned i = 0; i < degree; ++i)
				knot_vector.push_back(knot_vector.back() + knot_vector[i + 1] - knot_vector[i]);
			for (unsigned i = 0; i < degree; ++i)
				knot_vector.insert(knot_vector.begin(), knot_vector.front() -
					*(tmp_knot_vector.end()-i-1) + *(tmp_knot_vector.end()-i-2));			
			is_open = true;
		}
		// short name for knots u[k]
		const auto& u = knot_vector;
		// K is number of knots indexed with k
		unsigned K = unsigned(u.size());

		unsigned j = 0;
		for (unsigned k = 0; k < K; ++k) {
			if (k > K - 2*degree && !tmp_is_open)
				break;
			if (is_open && k+1 < degree)
				continue;
			if (is_open && k+degree+1 > K)
				continue;
			if (++j >= max_nr_segments)
				if (max_nr_segments > 0)
					break;

			if (u[k] == u[(k + 1) % K])
				continue;

			for (unsigned s = 0; s < nr_steps; ++s) {
				float lambda = float(s) / (nr_steps - 1);
				float t = (1 - lambda)*u[k] + lambda*u[(k+1)%K];
				spline_colors.push_back(segment_colors[k]);
				//sampled_spline.push_back(eval_spline(t, k));
				sampled_spline.push_back(eval_spline_sg(t, j-1));
			}
		}
		spline_vbo_changed = true;
		if (!tmp_is_open) {
			is_open = false;
			knot_vector = tmp_knot_vector;
			polygon = tmp_polygon;
		}
		post_redraw();
	}
	cgv::vec2 last_pos;
	cgv::vec2 p_edge;
	int selected_index;
	int edge_index;
	/// style used for point rendering
	point_render_style prs;
	/// style used for cone rendering
	cone_render_style crs;
	/// style used for sphere rendering
	sphere_render_style srs;
	/// style used for line rendering
	line_render_style lrs;
	/// style used for rectangle rendering
	rectangle_render_style rrs;
	/// whether tiangle vbo needs to be recomputed
	bool pnt_vbo_changed;
	bool spline_vbo_changed;
	/// remember number of vertices stored in pnt_vbo
	size_t pnt_vbo_count;
	/// vertex buffer to store polygon corners together with point colors
	vertex_buffer pnt_vbo;
	size_t spline_vbo_count;
	vertex_buffer spline_vbo;
	/// array manager for point rendering
	attribute_array_manager pnt_aam;
	/// array manager for line rendering
	attribute_array_manager line_aam;
	/// array manager for line rendering
	attribute_array_manager spline_aam;
public:
	std::vector<cgv::rgb>  segment_colors;
	std::vector<cgv::vec2> polygon;
	std::vector<cgv::vec2> sampled_spline;
	std::vector<cgv::rgb>  colors;
	std::vector<cgv::rgb>  spline_colors;
	cgv::render::view*     view_ptr;

	// polygon rasterization
	bool read_polygon(const std::string& file_name)
	{
		std::ifstream is(file_name.c_str());
		if (is.fail())
			return false;
		float x, y;
		cgv::box2 B;
		B.invalidate();
		while (!is.eof()) {
			is >> x >> y;
			if (is.fail())
				break;
			polygon.push_back(cgv::vec2(x, y));
			B.add_point(polygon.back());
		}
		float scale = 2.0f / B.get_extent()[B.get_max_extent_coord_index()];
		cgv::vec2 ctr = B.get_center();
		for (auto& p : polygon) 
			p = scale*(p - ctr);
	
		return true;
	}
	void generate_circle(unsigned n)
	{
		polygon.resize(n);
		for (unsigned i = 0; i < n; ++i) {
			float angle = float(2 * M_PI*i / n);
			polygon[i] = cgv::vec2(cos(angle), sin(angle));
		}
	}
	void set_colors()
	{
		colors.resize(polygon.size());
		std::fill(colors.begin(), colors.end(), cgv::rgb(0.5f, 0.5f, 0.5f));
	}
	spline_demo() : node("spline_demo"), 
		pnt_vbo(VBT_VERTICES, VBU_DYNAMIC_DRAW), 
		spline_vbo(VBT_VERTICES, VBU_DYNAMIC_DRAW)
	{
		//if (!read_polygon("S:/develop/projects/git/cgv/plugins/examples/poly.txt"))
			generate_circle(8);
	
		recompute_segment_widths();
		set_colors();
		view_ptr = 0;
		selected_index = -1;
		edge_index = -1;
		pnt_vbo_changed = true;
		spline_vbo_changed = true;
		spline_vbo_count = pnt_vbo_count = 0;
		prs.point_size = 12;
		prs.default_depth_offset = -0.000003f;
		prs.blend_points = true;
		prs.measure_point_size_in_pixel = false;
		lrs.blend_lines = true;
		lrs.measure_line_width_in_pixel = false;
		lrs.reference_line_width = 0.01f;
		lrs.blend_width_in_pixel = 1.0f;
		rrs.blend_rectangles = true;
		rrs.map_color_to_material = CM_COLOR_AND_OPACITY;
	}
	/// return name of type
	std::string get_type_name() const 
	{ 
		return "spline_demo"; 
	}
	void stream_help(std::ostream& os)
	{
	}
	bool init(context& ctx)
	{
		view_ptr = find_view_as_node();
		if (!view_ptr)
			return false;
		ref_point_renderer(ctx, 1);
		ref_line_renderer(ctx, 1);
		ref_rectangle_renderer(ctx, 1);
		pnt_aam.init(ctx);
		line_aam.init(ctx);
		spline_aam.init(ctx);
		ctx.ref_default_shader_program();
		ctx.ref_default_shader_program(true);
		return true;
	}
	void clear(context& ctx)
	{
		pnt_vbo.destruct(ctx);
		spline_vbo.destruct(ctx);
		pnt_aam.destruct(ctx);
		line_aam.destruct(ctx);
		spline_aam.destruct(ctx);
		ref_point_renderer(ctx, -1);
		ref_line_renderer(ctx, -1);
		ref_rectangle_renderer(ctx, -1);
	}
	void init_frame(context& ctx)
	{
		if (!pnt_vbo.is_created() || pnt_vbo_changed) {
			// collect geometry data in cpu vectors
			std::vector<cgv::vec2> P = polygon;
			std::vector<cgv::rgb>  C = colors;
			if (selected_index != -1)
				C[selected_index] = cgv::rgb(1.0f, 0.0f, 1.0f);
			else if (edge_index != -1) {
				P.push_back(p_edge);
				C.push_back(cgv::rgb(1.0f, 0.0f, 1.0f));
			}
			pnt_vbo_count = P.size();
			// ensure that point buffer has necessary size
			size_t necessary_size = P.size() * sizeof(cgv::vec2) + C.size() * sizeof(cgv::rgb);
			if (pnt_vbo.get_size_in_bytes() < necessary_size)
				pnt_vbo.destruct(ctx);
			if (!pnt_vbo.is_created())
				pnt_vbo.create(ctx, necessary_size);
			// copy data into point buffer
			pnt_vbo.replace(ctx, 0, &P.front(), P.size());
			pnt_vbo.replace(ctx, P.size() * sizeof(cgv::vec2), &C.front(), C.size());
			pnt_vbo_changed = false;
			// update attribute managers
			auto& pr = ref_point_renderer(ctx);
			pr.enable_attribute_array_manager(ctx, pnt_aam);
			pr.set_position_array<cgv::vec2>(ctx, pnt_vbo, 0, pnt_vbo_count);
			pr.set_color_array<cgv::rgb>(ctx, pnt_vbo, pnt_vbo_count * sizeof(cgv::vec2), pnt_vbo_count);
			pr.disable_attribute_array_manager(ctx, pnt_aam);

			auto& lr = ref_line_renderer(ctx);
			lr.enable_attribute_array_manager(ctx, line_aam);
			lr.set_position_array<cgv::vec2>(ctx, pnt_vbo, 0, pnt_vbo_count);
			lr.disable_attribute_array_manager(ctx, line_aam);
		}
		if (!spline_vbo.is_created() || spline_vbo_changed) {
			// collect geometry data in cpu vectors
			const std::vector<cgv::vec2>& P = sampled_spline;
			const std::vector<cgv::rgb>&  C = spline_colors;
			spline_vbo_count = P.size();
			// ensure that point buffer has necessary size
			size_t necessary_size = P.size() * sizeof(cgv::vec2) + C.size() * sizeof(cgv::rgb);
			if (spline_vbo.get_size_in_bytes() < necessary_size)
				spline_vbo.destruct(ctx);
			if (!spline_vbo.is_created())
				spline_vbo.create(ctx, necessary_size);
			// copy data into point buffer
			spline_vbo.replace(ctx, 0, &P.front(), P.size());
			spline_vbo.replace(ctx, P.size() * sizeof(cgv::vec2), &C.front(), C.size());
			spline_vbo_changed = false;
			// update attribute managers
			auto& lr = ref_line_renderer(ctx);
			lr.enable_attribute_array_manager(ctx, spline_aam);
			lr.set_position_array<cgv::vec2>(ctx, spline_vbo, 0, spline_vbo_count);
			lr.set_color_array<cgv::rgb>(ctx, spline_vbo, P.size() * sizeof(cgv::vec2), spline_vbo_count);
			lr.disable_attribute_array_manager(ctx, spline_aam);
		}
	}
	void draw(context& ctx)
	{
		static cgv::box2 rectangle(cgv::vec2(-2, -2), cgv::vec2(2, 2));
		static cgv::vec4 texcoord(0, 0, 1, 1);
		auto& rr = ref_rectangle_renderer(ctx);
		rr.set_render_style(rrs);
		rr.set_rectangle(ctx, rectangle);
		rr.set_depth_offset(ctx, 0.000001f);
		rr.set_color(ctx, cgv::rgb(1, 1, 1));
		ctx.set_color(cgv::rgb(1, 1, 1));
		rr.render(ctx, 0, 1);

		// prepare line rendering
		auto& lr = ref_line_renderer(ctx);
		lr.set_render_style(lrs);
		// draw polygon
		if (polygon.size() > 0) {
			lr.enable_attribute_array_manager(ctx, line_aam);
			lr.set_color(ctx, cgv::rgb(0.8f, 0.5f, 0.0f));
			ctx.set_color(cgv::rgb(0.8f, 0.5f, 0));
			lr.set_line_width(ctx, 5.0f);
			// collect indices of to render polygon as triangle strip
			std::vector<GLuint> I;
			size_t n = polygon.size()+(is_open?0:1);
			for (GLuint i = 0; i < n; ++i)
				I.push_back(i % polygon.size());
			// set indices and render in indexed mode
			lr.set_indices(ctx, I, true);
			lr.set_depth_offset(ctx, -0.000001f);
			lr.render(ctx, 0, I.size(), true);
			lr.disable_attribute_array_manager(ctx, line_aam);
		}
		if (sampled_spline.size() > 0) {
			lr.enable_attribute_array_manager(ctx, spline_aam);
			// configure color and line width
			lr.set_color(ctx, cgv::rgb(0.8f, 0, 0.7f));
			ctx.set_color(cgv::rgb(0.8f, 0, 0.7f));
			lr.set_line_width(ctx, 2.0f);
			lr.set_depth_offset(ctx, -0.000002f);
			// collect indices of to render polygon as triangle strip
			std::vector<GLuint> I;
			size_t n = sampled_spline.size() + (is_open ? 0 : 1);
			for (GLuint i = 0; i < n; ++i)
				I.push_back(i % sampled_spline.size());
			// set indices and render in indexed mode
			lr.set_indices(ctx, I, true);
			lr.render(ctx, 0, I.size(), true);
			lr.disable_attribute_array_manager(ctx, spline_aam);
		}		
		// draw points
		if (pnt_vbo_count > 0) {
			auto& pr = ref_point_renderer(ctx);
			pr.set_render_style(prs);
			pr.enable_attribute_array_manager(ctx, pnt_aam);
			pr.render(ctx, 0, pnt_vbo_count);
			pr.disable_attribute_array_manager(ctx, pnt_aam);
		}
	}
	bool handle(event& e)
	{
		//if (e.get_kind() == EID_KEY) {
		//	key_event& ke = static_cast<key_event&>(e);
		//	if (ke.get_action() == KA_RELEASE)
		//		return false;
		//	switch (ke.get_key()) {
		//	}
		//}
		//else 
		if (e.get_kind() == EID_MOUSE) {
			mouse_event& me = static_cast<mouse_event&>(e);
			int y_gl = get_context()->get_height() - 1 - me.get_y();
			switch (me.get_action()) {
			case MA_MOVE :
				if (view_ptr) {
					float epsilon = (float)(prs.point_size * view_ptr->get_y_extent_at_focus() / get_context()->get_height());
					cgv::dvec3 p_d;
					if (get_world_location(me.get_x(), y_gl, *view_ptr, p_d)) {
						cgv::vec2 p((float)p_d.x(),(float)p_d.y());
						int min_index = -1;
						float min_dist = 0;
						for (unsigned i = 0; i < polygon.size(); ++i) {
							float dist = (polygon[i] - p).length();
							if (min_index == -1 || dist < min_dist) {
								min_dist = dist;
								min_index = i;
							}
						}
						if (min_dist > epsilon)
							min_index = -1;
						
						//std::cout << "<" << epsilon << "> " << p << "[" << selected_index << "|" << min_dist << "|" << min_index << "]";
						//if (min_index != -1)
						//	std::cout << " = " << polygon[min_index];

						if (min_index != selected_index) {
							selected_index = min_index;
							on_set(&selected_index);
						}
						// if no point found, check for edges
						if (selected_index == -1) {
							int min_index = -1;
							cgv::vec2 last_p_edge = p_edge;
							float min_dist = 0;
							for (unsigned i = 0; i < polygon.size(); ++i) {
								const cgv::vec2& p0 = polygon[i];
								const cgv::vec2& p1 = polygon[(i + 1) % polygon.size()];
								cgv::vec2 d = p1 - p0;
								float lambda = dot(p - p0, d) / dot(d, d);
								if (lambda > 0 && lambda < 1) {
									cgv::vec2 pp = p0 + lambda*d;
									float dist = (pp - p).length();
									if (dist < epsilon) {
										if (min_index == -1 || dist < min_dist) {
											min_dist = dist;
											min_index = i;
											p_edge = pp;
										}
									}
								}
							}
							if (min_dist > epsilon)
								min_index = -1;

							//std::cout << " (" << edge_index << "|" << min_dist << "|" << min_index << ")";
							//if (min_index != -1)
							//	std::cout << " = " << p_edge;

							if (min_index != edge_index) {
								edge_index = min_index;
								on_set(&edge_index);
							}
							on_set(&p_edge);
						}
						else {
							if (edge_index != -1) {
								edge_index = -1;
								on_set(&edge_index);
							}
						}
						//std::cout << std::endl;
					}
				}
				break;
			case MA_DRAG :
				if (me.get_button_state() == MB_LEFT_BUTTON) {
					if (selected_index != -1) {
						cgv::dvec3 p_d;
						if (get_world_location(me.get_x(), y_gl, *view_ptr, p_d)) {
							cgv::vec2 new_pos((float)p_d.x(), (float)p_d.y());
							cgv::vec2 diff = new_pos - last_pos;
							polygon[selected_index] += diff;
							on_set(&polygon[selected_index]);
							recompute_spline();
							last_pos = new_pos;
						}
					}
					return true;
				}
				break;
			case MA_PRESS:
				if (me.get_button() == MB_LEFT_BUTTON) {
					if (selected_index != -1) {
						cgv::dvec3 p_d;
						if (get_world_location(me.get_x(), y_gl, *view_ptr, p_d)) {
							last_pos = cgv::dvec2(p_d.x(), p_d.y());
						}
					}
					else if (edge_index != -1) {
						cgv::dvec3 p_d;
						if (get_world_location(me.get_x(), y_gl, *view_ptr, p_d)) {
							p_edge = last_pos = cgv::dvec2(p_d.x(), p_d.y());
						}
						else 
							last_pos = p_edge;

						if (edge_index == polygon.size() - 1) {
							polygon.push_back(p_edge);
							colors.push_back(cgv::rgb(0.5f, 0.5f, 0.5f));
						}
						else {
							polygon.insert(polygon.begin() + edge_index + 1, p_edge);
							colors.insert(colors.begin() + edge_index + 1, cgv::rgb(0.5f, 0.5f, 0.5f));
						}
						recompute_segment_widths();
						selected_index = edge_index + 1;
						edge_index = -1;
						on_set(&polygon[edge_index + 1]);
						on_set(&edge_index);
						on_set(&selected_index);
					}
					return true;
				}
				else if (me.get_button() == MB_RIGHT_BUTTON) {
					if (selected_index != -1) {
						if (polygon.size() > 3) {
							polygon.erase(polygon.begin() + selected_index);
							colors.erase(colors.begin() + selected_index);
							selected_index = -1;
							on_set(&polygon[0]);
							on_set(&selected_index);
							recompute_segment_widths();
						}
						return true;
					}
				}
				break;
			case MA_RELEASE:
				break;
			}
		}
		return false;
	}
	std::string get_menu_path() const 
	{
		return "example/spline_demo"; 
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &degree || member_ptr == &is_open || member_ptr == &clamp_start || member_ptr == &clamp_end) {
			recompute_segment_widths();
		}
		if ((member_ptr >= &segment_widths[0] && member_ptr < &segment_widths[0] + segment_widths.size())) {
			update_knot_vector_from_segment_widths();
		}
		if (member_ptr >= &knot_vector[0] && member_ptr < &knot_vector[0] + knot_vector.size()) {
			unsigned i = unsigned(reinterpret_cast<float*>(member_ptr) - &knot_vector.front());
			if (i > 0) {
				segment_widths[i - 1] = knot_vector[i] - knot_vector[i - 1];
				update_member(&segment_widths[i - 1]);
			}
			if (i < segment_widths.size() && i+1 < knot_vector.size()) {
				segment_widths[i] = knot_vector[i+1] - knot_vector[i];
				update_member(&segment_widths[i - 1]);
			}
			recompute_spline();
		}
		if (member_ptr == &nr_steps || member_ptr == &max_nr_segments) {
			recompute_spline();
		}
		if (member_ptr >= &p_edge && member_ptr < &p_edge + 1 ||
			member_ptr == &edge_index || member_ptr == &selected_index) {
			pnt_vbo_changed = true;
		}
		if (member_ptr >= &polygon[0] && member_ptr < &polygon[0] + polygon.size()) {
			set_colors();
			recompute_spline();
			pnt_vbo_changed = true;
		}
		update_member(member_ptr);
		post_redraw();
	}
	void create_gui() 
	{	
		add_decorator("spline demo", "heading");
		add_member_control(this, "is_open", is_open, "toggle");
		add_member_control(this, "clamp_start", clamp_start, "toggle");
		add_member_control(this, "clamp_end", clamp_end, "toggle");
		add_member_control(this, "degree", degree, "value_slider", "min=1;max=10;ticks=true");
		add_member_control(this, "nr_steps", nr_steps, "value_slider", "min=10;max=1000;log=true;ticks=true");
		add_member_control(this, "max_nr_segments", max_nr_segments, "value_slider", "min=0;max=10;ticks=true");
		if (begin_tree_node("segment", segment_widths, true)) {
			align("\a");
			for (size_t i = 0; i < segment_widths.size(); ++i) {
				add_member_control(this, std::string("width") + cgv::utils::to_string(i, 2),
					segment_widths[i], "value_slider", "min=0;max=1;ticks=true;w=150", " ");
				add_member_control(this, "", segment_colors[i], "", "w=42");
			}
			align("\b");
			end_tree_node(segment_widths);
		}
		if (begin_tree_node("knot vector", knot_vector, true)) {
			align("\a");
			for (size_t i = 0; i < knot_vector.size(); ++i) {
				add_member_control(this, std::string("u") + cgv::utils::to_string(i, 2),
					knot_vector[i], "value_slider", "min=0;max=12;ticks=true");

			}
			align("\b");
			end_tree_node(knot_vector);
		}
		if (begin_tree_node("line style", lrs, false)) {
			align("\a");
			add_gui("render style", lrs);
			align("\b");
			end_tree_node(lrs);
		}
	}
};

#include <cgv/base/register.h>

factory_registration<spline_demo> spline_demo_fac("New/Algorithms/Spline Demo", 'S', true);

