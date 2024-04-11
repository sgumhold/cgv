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
#include <cgv_gl/rectangle_renderer.h>
#include <cgv/math/ftransform.h>
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

enum PolygonOrientation
{
	PO_CCW,
	PO_CW
};

enum PolygonCornerStatus
{
	PCS_CONVEX,
	PCS_CONCAVE,
	PCS_EAR
};

struct polygon_node
{
	unsigned idx;
	PolygonCornerStatus status;
	polygon_node(unsigned _idx = 0, PolygonCornerStatus _status = PCS_CONVEX) : idx(_idx), status(_status) {}
};

class ear_cutting : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
protected:
	cgv::vec2 last_pos;
	cgv::vec2 p_edge;
	unsigned nr_steps;
	bool wireframe;
	float lambda;
	int selected_index;
	int edge_index;
	/// style used for point rendering
	point_render_style prs;
	/// style used for line rendering
	line_render_style lrs;
	/// style used for rectangle rendering
	rectangle_render_style rrs;
	/// whether tiangle vbo needs to be recomputed
	bool pnt_vbo_changed;
	/// remember number of vertices stored in pnt_vbo
	size_t pnt_vbo_count;
	/// vertex buffer to store polygon corners together with point colors
	vertex_buffer pnt_vbo;
	/// array manager for point rendering
	attribute_array_manager pnt_aam;
	/// array manager for line rendering
	attribute_array_manager line_aam;
	/// whether tiangle vbo needs to be recomputed
	bool tgl_vbo_changed;
	/// remember number of vertices stored in pnt_vbo
	size_t tgl_vbo_count;
	/// vertex buffer to corners of shrinked triangles and per triangle colors
	vertex_buffer tgl_vbo;
	/// attribute array binding for triangle rendering
	attribute_array_binding tgl_aab;
public:
	std::vector<cgv::vec2>         polygon;
	std::vector<cgv::rgb>          colors;
	PolygonOrientation        orientation;
	std::vector<cgv::ivec3>        triangles;
	std::vector<polygon_node> nodes;
	cgv::render::view*        view_ptr;

	// polygon rasterization
	bool tex_outofdate;
	cgv::render::texture tex;
	bool show_rasterization;
	std::vector<cgv::rgb8> img;
	cgv::rgb8 background_color, background_color2;
	size_t img_width, img_height;
	cgv::box2 img_extent;
	bool synch_img_dimensions;
	bool validate_pixel_location(const cgv::vec2& p) const { return p(0) >= 0 && p(0) < int(img_width) && p(1) >= 0 && p(1) < int(img_height); }
	size_t linear_index(const cgv::ivec2& p) const { return img_width*p(1) + p(0); }
	static cgv::ivec2 round(const cgv::vec2& p) { return cgv::ivec2(int(floor(p(0)+0.5f)), int(floor(p(1)+0.5f))); }
	void set_pixel(const cgv::ivec2& p, const cgv::rgb8& c) { if (validate_pixel_location(p)) img[linear_index(p)] = c; }
	const cgv::rgb8& get_pixel(const cgv::ivec2& p) const { return img[linear_index(p)]; }
	cgv::vec2 pixel_from_world(const cgv::vec2& p) const { return cgv::vec2(float(img_width), float(img_height))*(p - img_extent.get_min_pnt()) / img_extent.get_extent(); }
	cgv::vec2 world_from_pixel(const cgv::vec2& p) const { return p*img_extent.get_extent() / cgv::vec2(float(img_width), float(img_height)) + img_extent.get_min_pnt(); }
	void clear_image(const cgv::rgb8& c) { std::fill(img.begin(), img.end(), c); }
	void clear_image(const cgv::rgb8& c1, const cgv::rgb8& c2) {
		for (unsigned y = 0; y < img_height; ++y)
			for (unsigned x = 0; x < img_width; ++x)
				set_pixel(cgv::ivec2(x, y), ((x & 1) == (y & 1)) ? c1 : c2);
	}
	void rasterize_polygon(const std::vector<cgv::vec2>& polygon, const cgv::rgb8& c) {
		for (const auto& p : polygon)
			set_pixel(pixel_from_world(p), c);
	}
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
	PolygonOrientation determine_orientation() const
	{
		float cp_sum = 0;
		for (unsigned i = 0; i < polygon.size(); ++i) {
			const cgv::vec2& p0 = polygon[i];
			const cgv::vec2& p1 = polygon[(i+1)%polygon.size()];
			cp_sum += p0(0)*p1(1) - p0(1)*p1(0);
		}
		return cp_sum < 0 ? PO_CW : PO_CCW;
	}
	PolygonCornerStatus classify_node(unsigned ni) const
	{		
		const cgv::vec2& p = polygon[nodes[ni].idx];
		const cgv::vec2& pp = polygon[nodes[(ni + nodes.size() - 1) % nodes.size()].idx];
		const cgv::vec2& pn = polygon[nodes[(ni + 1) % nodes.size()].idx];
		cgv::vec2 dn = pn - p;
		cgv::vec2 dp = p - pp;
		float s = dp(0)*dn(1) - dp(1)*dn(0);
		if (orientation == PO_CCW)
			return ((s <= 0) ? PCS_CONCAVE : PCS_CONVEX);
		else
			return ((s >= 0) ? PCS_CONCAVE : PCS_CONVEX);
	}
	void classify_nodes()
	{
		for (unsigned ni = 0; ni<nodes.size(); ++ni)
			nodes[ni].status = classify_node(ni);
	}
	static float cpn(const cgv::vec2& p0, const cgv::vec2& p1, const cgv::vec2& p2)
	{
		cgv::vec2 d01 = p1 - p0;
		cgv::vec2 d02 = p2 - p0;
		return d01(0)*d02(1) - d01(1)*d02(0);
	}
	static bool compare_sign(float a, float b) 
	{
		if (a < 0)
			return b <= 0;
		if (a > 0)
			return b >= 0;
		return true;
	}
	static bool is_inside(const cgv::vec2& p, const cgv::vec2& p0, const cgv::vec2& p1, const cgv::vec2& p2) 
	{
		float n012 = cpn(p0, p1, p2);
		float n0   = cpn(p, p1, p2);
		float n1   = cpn(p0, p, p2);
		float n2   = cpn(p0, p1, p);

		return compare_sign(n012, n0) && compare_sign(n012, n1) && compare_sign(n012, n2);
	}
	bool is_ear(unsigned ni) const
	{
		const cgv::vec2& p = polygon[nodes[ni].idx];
		const cgv::vec2& pp = polygon[nodes[(ni + nodes.size() - 1) % nodes.size()].idx];
		const cgv::vec2& pn = polygon[nodes[(ni + 1) % nodes.size()].idx];
		for (unsigned nj = 0; nj < nodes.size(); ++nj) {
			if (nj == (ni + 1) % nodes.size())
				continue;
			if (ni == (nj + 1) % nodes.size())
				continue;
			if (nodes[nj].status == PCS_CONCAVE) {
				const cgv::vec2& pc = polygon[nodes[nj].idx];
				if (is_inside(pc, pp, p, pn))
					return false;
			}
		}
		return true;
	}
	void find_ears()
	{
		for (unsigned ni=0; ni<nodes.size(); ++ni)
			if (nodes[ni].status == PCS_CONVEX) {
				if (is_ear(ni))
					nodes[ni].status = PCS_EAR;
			}
	}
	void init_ear_cutting()
	{
		triangles.clear();
		orientation = determine_orientation();
		nodes.clear();
		for (unsigned i = 0; i < polygon.size(); ++i)
			nodes.push_back(polygon_node(i));
	}
	void set_colors()
	{
		colors.resize(polygon.size());
		std::fill(colors.begin(), colors.end(), cgv::rgb(0.5f, 0.5f, 0.5f));
		for (unsigned ni = 0; ni < nodes.size(); ++ni)
			switch (nodes[ni].status) {
			case PCS_CONVEX: colors[nodes[ni].idx] = cgv::rgb(0, 0, 1); break;
			case PCS_CONCAVE: colors[nodes[ni].idx] = cgv::rgb(1, 0, 0); break;
			case PCS_EAR: colors[nodes[ni].idx] = cgv::rgb(0, 1, 0); break;
			}
	}
	void perform_ear_cutting()
	{
		init_ear_cutting();
		for (unsigned i = 0; i < nr_steps; ++i) {
			classify_nodes();
			find_ears();
			// find ear
			unsigned ni;
			for (ni = 0; ni < nodes.size(); ++ni)
				if (nodes[ni].status == PCS_EAR)
					break;
			//
			if (ni == nodes.size())
				break;

			//
			triangles.push_back(cgv::ivec3(nodes[(ni + nodes.size() - 1) % nodes.size()].idx, nodes[ni].idx, nodes[(ni + 1) % nodes.size()].idx));
			nodes.erase(nodes.begin() + ni);
		}
		if (nodes.size() == 2)
			nodes.clear();
	}
	void reallocate_image()
	{
		img.resize(img_width*img_height);
		clear_image(background_color, background_color2);
		tex_outofdate = true;
	}
	ear_cutting() : node("ear_cutting"), pnt_vbo(VBT_VERTICES, VBU_DYNAMIC_DRAW), tgl_vbo(VBT_VERTICES, VBU_DYNAMIC_DRAW)
	{
		show_rasterization = false;
		tex.set_mag_filter(cgv::render::TF_NEAREST);
		img_width = img_height = 64;
		img_extent.ref_min_pnt() = cgv::vec2(-2, -2);
		img_extent.ref_max_pnt() = cgv::vec2( 2,  2);
		synch_img_dimensions = true;
		background_color = cgv::rgb8(255, 255, 128);
		background_color2 = cgv::rgb8(255, 128, 255);
		reallocate_image();

		//if (!read_polygon("S:/develop/projects/git/cgv/plugins/examples/poly.txt"))
			generate_circle(8);

		init_ear_cutting();
		classify_nodes();
		find_ears();
		set_colors();
		nr_steps = 0;
		lambda = 0.1f;
		view_ptr = 0;
		selected_index = -1;
		wireframe = false;
		edge_index = -1;
		pnt_vbo_changed = true;
		tgl_vbo_changed = true;
		pnt_vbo_count = tgl_vbo_count = 0;
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
		return "ear_cutting"; 
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
		tgl_aab.create(ctx);
		ctx.ref_default_shader_program();
		ctx.ref_default_shader_program(true);
		return true;
	}
	void clear(context& ctx)
	{
		pnt_vbo.destruct(ctx);
		tgl_vbo.destruct(ctx);
		pnt_aam.destruct(ctx);
		line_aam.destruct(ctx);
		tgl_aab.destruct(ctx);
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
			pr.set_color_array<cgv::rgb>(ctx, pnt_vbo, pnt_vbo_count*sizeof(cgv::vec2), pnt_vbo_count);
			pr.disable_attribute_array_manager(ctx, pnt_aam);

			auto& lr = ref_line_renderer(ctx);
			lr.enable_attribute_array_manager(ctx, line_aam);
			lr.set_position_array<cgv::vec2>(ctx, pnt_vbo, 0, pnt_vbo_count);
			lr.disable_attribute_array_manager(ctx, line_aam);

		}
		if ((!tgl_vbo.is_created() || tgl_vbo_changed) && !triangles.empty()) {
			// collect geometry data in cpu vectors
			std::vector<cgv::vec2> P;
			std::vector<cgv::rgb> C;
			cgv::rgb c0(0.0f, 0.0f, 1.0f);
			cgv::rgb c1(1.0f, 1.0f, 0.0f);
			float scale = 1.0f / (polygon.size() - 2);
			for (unsigned ti = 0; ti < triangles.size(); ++ti) {
				const auto& t = triangles[ti];
				cgv::vec2 ctr = 0.3333333333333f * (polygon[t[0]] + polygon[t[1]] + polygon[t[2]]);
				float lambda_c = scale * ti;
				cgv::rgb c = (1 - lambda_c) * c0 + lambda_c * c1;
				for (unsigned i = 0; i < 3; ++i) {
					P.push_back((1 - lambda) * polygon[t[i]] + lambda * ctr);
					C.push_back(c);
				}
			}
			tgl_vbo_count = P.size();
			// ensure that point buffer has necessary size
			size_t necessary_size = P.size() * sizeof(cgv::vec2) + C.size() * sizeof(cgv::rgb);
			if (tgl_vbo.get_size_in_bytes() < necessary_size)
				tgl_vbo.destruct(ctx);
			if (!tgl_vbo.is_created())
				tgl_vbo.create(ctx, necessary_size);
			// copy data into point buffer
			tgl_vbo.replace(ctx, 0, &P.front(), P.size());
			tgl_vbo.replace(ctx, P.size() * sizeof(cgv::vec2), &C.front(), C.size());
			tgl_vbo_changed = false;
			// update attribute array binding
			auto& prog = ctx.ref_default_shader_program();
			cgv::vec2 tmp;
			cgv::rgb clr;
			tgl_aab.set_attribute_array(ctx, prog.get_position_index(), get_element_type(tmp), tgl_vbo, 0, tgl_vbo_count);
			tgl_aab.set_attribute_array(ctx, prog.get_color_index(), get_element_type(clr), tgl_vbo, tgl_vbo_count * sizeof(cgv::vec2), tgl_vbo_count);
		}
		if (tex_outofdate) {
			if (tex.is_created())
				tex.destruct(ctx);
			cgv::data::data_format df("uint8[R,G,B]");
			df.set_width(unsigned(img_width));
			df.set_height(unsigned(img_height));
			cgv::data::data_view dv(&df, &img[0]);
			tex.create(ctx, dv);
			tex_outofdate = false;
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
		if (show_rasterization) {
			rr.set_texcoord(ctx, texcoord);
			tex.enable(ctx);
			rr.render(ctx, 0, 1);
			tex.disable(ctx);
		}
		else
			rr.render(ctx, 0, 1);

		// draw triangles
		if (!triangles.empty()) {
			shader_program& prog = ctx.ref_default_shader_program();
			tgl_aab.enable(ctx);
			prog.enable(ctx);
			if (wireframe) {
				tgl_aab.disable_array(ctx, prog.get_color_index());
				ctx.set_color(cgv::rgb(0, 0, 0));
				std::vector<GLuint> I;
				for (GLuint vi = 0; vi < tgl_vbo_count; vi += 3) {
					if (!I.empty())
						I.push_back(123456);
					I.push_back(vi);
					I.push_back(vi + 1);
					I.push_back(vi + 2);
					I.push_back(vi);
				}
				glEnable(GL_PRIMITIVE_RESTART);
				glPrimitiveRestartIndex(123456);
				glDrawElements(GL_LINE_STRIP, (GLsizei)I.size(), GL_UNSIGNED_INT, I.data());
				glDisable(GL_PRIMITIVE_RESTART);
				tgl_aab.enable_array(ctx, prog.get_color_index());
			}
			else
				glDrawArrays(GL_TRIANGLES, 0, (GLsizei)tgl_vbo_count);
			prog.disable(ctx);
			tgl_aab.disable(ctx);
		}
		
		// prepare line rendering
		auto& lr = ref_line_renderer(ctx);
		lr.set_render_style(lrs);
		lr.enable_attribute_array_manager(ctx, line_aam);
		// draw polygon
		if (polygon.size() > 0) {
			lr.set_color(ctx, cgv::rgb(0.8f, 0.5f, 0));
			ctx.set_color(cgv::rgb(0.8f, 0.5f, 0));
			lr.set_line_width(ctx, 5.0f);
			// collect indices of to render polygon as triangle strip
			std::vector<GLuint> I;
			for (GLuint i = 0; i <= polygon.size(); ++i)
				I.push_back(i % polygon.size());
			// set indices and render in indexed mode
			lr.set_indices(ctx, I, true);
			lr.set_depth_offset(ctx, -0.000001f);
			lr.render(ctx, 0, I.size(), true);
		}
		// draw unprocessed polygon
		if (!nodes.empty()) {
			// configure color and line width
			lr.set_color(ctx, cgv::rgb(0.8f, 0, 0.7f));
			ctx.set_color(cgv::rgb(0.8f, 0, 0.7f));
			lr.set_line_width(ctx, 2.0f);
			lr.set_depth_offset(ctx, -0.000002f);
			// collect indices of to render line loop as triangle strip
			std::vector<GLuint> I;
			for (const auto& n : nodes)
				I.push_back(n.idx);
			I.push_back(nodes.front().idx);
			// set indices and render in indexed mode
			lr.set_indices(ctx, I, true);
			lr.render(ctx, 0, I.size(), true);
		}
		// finish line rendering
		lr.disable_attribute_array_manager(ctx, line_aam);
		
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
		if (e.get_kind() == EID_KEY) {
			key_event& ke = static_cast<key_event&>(e);
			if (ke.get_action() == KA_RELEASE)
				return false;
			switch (ke.get_key()) {
			case 'N':
				if (nr_steps < polygon.size() - 2)
					++nr_steps;
				on_set(&nr_steps);
				return true;
			case 'W':
				wireframe = !wireframe;
				on_set(&wireframe);
				return true;
			case 'P':
				if (nr_steps > 0)
					--nr_steps;
				on_set(&nr_steps);
				return true;
			}
		}
		else if (e.get_kind() == EID_MOUSE) {
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
							on_set(polygon[selected_index]);
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
						selected_index = edge_index + 1;
						edge_index = -1;
						on_set(polygon[edge_index + 1]);
						on_set(&edge_index);
						on_set(&selected_index);
						if (find_control(nr_steps))
							find_control(nr_steps)->set("max", polygon.size() - 2);
						if (nr_steps >= polygon.size() / 2) {
							++nr_steps;
							on_set(&nr_steps);
						}
					}
					return true;
				}
				else if (me.get_button() == MB_RIGHT_BUTTON) {
					if (selected_index != -1) {
						if (polygon.size() > 3) {
							polygon.erase(polygon.begin() + selected_index);
							colors.erase(colors.begin() + selected_index);
							selected_index = -1;
							on_set(polygon[0]);
							on_set(&selected_index);
							if (find_control(nr_steps))
								find_control(nr_steps)->set("max", polygon.size() - 2);
							if (nr_steps > polygon.size() - 2) {
								nr_steps = unsigned(polygon.size() - 2);
								on_set(&nr_steps);
							}
							else {
								if (nr_steps >= polygon.size() / 2) {
									--nr_steps;
									on_set(&nr_steps);
								}

							}
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
		return "example/ear_cutting"; 
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &img_height || member_ptr == &img_width) {
			if (synch_img_dimensions) {
				if (member_ptr == &img_width) {
					img_height = img_width;
					update_member(&img_height);
				}
				else {
					img_width = img_height;
					update_member(&img_width);
				}
			}
			reallocate_image();
			rasterize_polygon(polygon, cgv::rgb8(255, 0, 0));
			tex_outofdate = true;
		}
		if (member_ptr == &background_color || member_ptr == &background_color2) {
			clear_image(background_color, background_color2);
			rasterize_polygon(polygon, cgv::rgb8(255, 0, 0));
			tex_outofdate = true;
		}
		if (member_ptr == &nr_steps) {
			perform_ear_cutting();
			classify_nodes();
			find_ears();
			set_colors();
			pnt_vbo_changed = tgl_vbo_changed = true;
		}
		if (member_ptr >= &p_edge && member_ptr < &p_edge + 1 ||
			member_ptr == &edge_index || member_ptr == &selected_index) {
			pnt_vbo_changed = true;
		}
		if (member_ptr == &lambda) {
			tgl_vbo_changed = true;
		}
		if (member_ptr >= &polygon[0] && member_ptr < &polygon[0] + polygon.size()) {
			perform_ear_cutting();
			classify_nodes();
			find_ears();
			set_colors();
			clear_image(background_color, background_color2);
			rasterize_polygon(polygon, cgv::rgb8(255, 0, 0));
			pnt_vbo_changed = tgl_vbo_changed = tex_outofdate = true;
		}
		update_member(member_ptr);
		post_redraw();
	}
	void create_gui() 
	{	
		add_decorator("ear cutting", "heading");
		add_member_control(this, "nr_steps", nr_steps, "value_slider", "min=0;max=100;ticks=true");
		find_control(nr_steps)->set("max", polygon.size() - 2);
		add_member_control(this, "lambda", lambda, "value_slider", "min=0;max=0.5;ticks=true");
		add_member_control(this, "wireframe", wireframe, "check");

		if (begin_tree_node("rasterization", synch_img_dimensions)) {
			align("\a");
			add_member_control(this, "show_rasterization", show_rasterization, "toggle");
			add_member_control(this, "synch_img_dimensions", synch_img_dimensions, "toggle");
			add_member_control(this, "img_width", img_width, "value_slider", "min=2;max=1024;log=true;ticks=true");
			add_member_control(this, "img_height", img_height, "value_slider", "min=2;max=1024;log=true;ticks=true");
			add_member_control(this, "background_color", background_color);
			add_member_control(this, "background_color2", background_color2);
			align("\b");
			end_tree_node(synch_img_dimensions);
		}
		if (begin_tree_node("style", pnt_aam)) {
			align("\a");
			if (begin_tree_node("points", prs)) {
				align("\a");
				add_gui("points", prs);
				align("\b");
				end_tree_node(prs);
			}
			if (begin_tree_node("lines", lrs)) {
				align("\a");
				add_gui("lines", lrs);
				align("\b");
				end_tree_node(lrs);
			}
			if (begin_tree_node("rectangles", rrs)) {
				align("\a");
				add_gui("rectangles", rrs);
				align("\b");
				end_tree_node(rrs);
			}
		}
	}
};

#include <cgv/base/register.h>

factory_registration<ear_cutting> ec_fac("New/Algorithms/Ear Cutting", 'E', true);

