#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/find_action.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/application.h>
#include <cgv/render/drawable.h>
#include <cgv/render/view.h>
#include <cgv_gl/point_renderer.h>
#include <cgv/math/vec.h>
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
public:
	typedef cgv::math::fvec<float, 2> vec2;
	typedef cgv::math::fvec<float, 3> vec3;
	typedef cgv::math::fvec<double, 3> vec3d;
	typedef cgv::math::fvec<unsigned, 3> ivec3;
	typedef cgv::media::color<float> clr_type;
	typedef cgv::media::axis_aligned_box<float, 2> box2;
protected:
	vec3 last_pos;
	vec3 p_edge;
	unsigned nr_steps;
	bool wireframe;
	float lambda;
	point_render_style prs;
	point_renderer pnt_renderer;
	int selected_index;
	int edge_index;
public:
	std::vector<vec2> polygon;
	std::vector<vec3> points;
	std::vector<clr_type> colors;
	PolygonOrientation orientation;
	std::vector<ivec3> triangles;
	std::vector<polygon_node> nodes;
	cgv::render::view* view_ptr;
	bool read_polygon(const std::string& file_name)
	{
		std::ifstream is(file_name.c_str());
		if (is.fail())
			return false;
		float x, y;
		box2 B;
		B.invalidate();
		while (!is.eof()) {
			is >> x >> y;
			if (is.fail())
				break;
			polygon.push_back(vec2(x, y));
			B.add_point(polygon.back());
		}
		float scale = 2.0f / B.get_extent()[B.get_max_extent_coord_index()];
		vec2 ctr = B.get_center();
		for (auto& p : polygon) 
			p = scale*(p - ctr);
	
		return true;
	}
	void generate_circle(unsigned n)
	{
		polygon.resize(n);
		for (unsigned i = 0; i < n; ++i) {
			float angle = float(2 * M_PI*i / n);
			polygon[i] = vec2(cos(angle), sin(angle));
		}
	}
	PolygonOrientation determine_orientation() const
	{
		float cp_sum = 0;
		for (unsigned i = 0; i < polygon.size(); ++i) {
			const vec2& p0 = polygon[i];
			const vec2& p1 = polygon[(i+1)%polygon.size()];
			cp_sum += p0(0)*p1(1) - p0(1)*p1(0);
		}
		return cp_sum < 0 ? PO_CW : PO_CCW;
	}
	PolygonCornerStatus classify_node(unsigned ni) const
	{		
		const vec2& p = polygon[nodes[ni].idx];
		const vec2& pp = polygon[nodes[(ni + nodes.size() - 1) % nodes.size()].idx];
		const vec2& pn = polygon[nodes[(ni + 1) % nodes.size()].idx];
		vec2 dn = pn - p;
		vec2 dp = p - pp;
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
	static float cpn(const vec2& p0, const vec2& p1, const vec2& p2)
	{
		vec2 d01 = p1 - p0;
		vec2 d02 = p2 - p0;
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
	static bool is_inside(const vec2& p, const vec2& p0, const vec2& p1, const vec2& p2) 
	{
		float n012 = cpn(p0, p1, p2);
		float n0   = cpn(p, p1, p2);
		float n1   = cpn(p0, p, p2);
		float n2   = cpn(p0, p1, p);

		return compare_sign(n012, n0) && compare_sign(n012, n1) && compare_sign(n012, n2);
	}
	bool is_ear(unsigned ni) const
	{
		const vec2& p = polygon[nodes[ni].idx];
		const vec2& pp = polygon[nodes[(ni + nodes.size() - 1) % nodes.size()].idx];
		const vec2& pn = polygon[nodes[(ni + 1) % nodes.size()].idx];
		for (unsigned nj = 0; nj < nodes.size(); ++nj) {
			if (nj == (ni + 1) % nodes.size())
				continue;
			if (ni == (nj + 1) % nodes.size())
				continue;
			if (nodes[nj].status == PCS_CONCAVE) {
				const vec2& pc = polygon[nodes[nj].idx];
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
		std::fill(colors.begin(), colors.end(), clr_type(0.5f, 0.5f, 0.5f));
		for (unsigned ni = 0; ni < nodes.size(); ++ni)
			switch (nodes[ni].status) {
			case PCS_CONVEX: colors[nodes[ni].idx] = clr_type(0, 0, 1); break;
			case PCS_CONCAVE: colors[nodes[ni].idx] = clr_type(1, 0, 0); break;
			case PCS_EAR: colors[nodes[ni].idx] = clr_type(0, 1, 0); break;
			}
//		colors[nodes[0].idx] = clr_type(0, 0, 0);
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
			triangles.push_back(ivec3(nodes[(ni + nodes.size() - 1) % nodes.size()].idx, nodes[ni].idx, nodes[(ni + 1) % nodes.size()].idx));
			nodes.erase(nodes.begin() + ni);
		}
		if (nodes.size() == 2)
			nodes.clear();
	}

	ear_cutting() : node("ear_cutting")
	{
		if (!read_polygon("S:/develop/projects/git/cgv/plugins/examples/poly.txt"))
			generate_circle(8);

		init_ear_cutting();
		classify_nodes();
		find_ears();
		set_colors();
		nr_steps = 0;
		lambda = 0.1f;
		prs.illumination_mode = IM_OFF;
		prs.culling_mode = CM_OFF;
		prs.orient_splats = false;
		prs.point_size = 16;
		view_ptr = 0;
		selected_index = -1;
		wireframe = false;
		edge_index = -1;
		for (unsigned i = 0; i < polygon.size(); ++i) {
			points.push_back(vec3(polygon[i](0), polygon[i](1), 0));
		}
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
		std::vector<cgv::render::view*> views;
		cgv::base::find_interface<cgv::render::view>(cgv::base::base_ptr(this), views);
		if (views.empty())
			return false;
		view_ptr = views[0];
		if (pnt_renderer.init(ctx)) {
			pnt_renderer.set_reference_point_size(0.005f);
			pnt_renderer.set_y_view_angle(float(view_ptr->get_y_view_angle()));
			pnt_renderer.set_render_style(prs);
			return true;
		}
		return false;
	}
	void clear(context& ctx)
	{
		pnt_renderer.clear(ctx);
	}
	void draw_polygon()
	{
		glBegin(GL_LINE_LOOP);
		for (const auto& p : polygon)
			glVertex2fv(p);
		glEnd();
	}
	void draw_unprocessed_polygon()
	{
		glBegin(GL_LINE_LOOP);
		for (const auto& n : nodes)
			glVertex2fv(polygon[n.idx]);
		glEnd();
	}
	void draw_triangles()
	{
		//clr_type c0(0.5f, 0.8f, 0.6f);
		//clr_type c1(1.0f, 0.3f, 1.0f);
		clr_type c0(0.0f, 0.0f, 1.0f);
		clr_type c1(1.0f, 1.0f, 0.0f);
		float scale = 1.0f / (polygon.size() - 2);
		if (wireframe) {
			glColor3f(0,0,0);
			for (const auto& t : triangles) {
				vec2 ctr = 0.3333333333333f*(polygon[t[0]] + polygon[t[1]] + polygon[t[2]]);
				glBegin(GL_LINE_LOOP);
				for (unsigned i = 0; i<3; ++i)
					glVertex2fv((1 - lambda)*polygon[t[i]] + lambda*ctr);
				glEnd();
			}
		}
		else {
			glColor3f(0.5f, 0.8f, 0.6f);
			glBegin(GL_TRIANGLES);
			for (unsigned ti = 0; ti < triangles.size(); ++ti) {
				const auto& t = triangles[ti];
				float lambda_c = scale*ti;
				clr_type c = (1 - lambda_c)*c0 + lambda_c*c1;
				glColor3fv(&c[0]);
				vec2 ctr = 0.3333333333333f*(polygon[t[0]] + polygon[t[1]] + polygon[t[2]]);
				for (unsigned i = 0; i < 3; ++i)
					glVertex2fv((1 - lambda)*polygon[t[i]] + lambda*ctr);
			}
			glEnd();
		}
	}
	void draw_points(context& ctx)
	{
		clr_type tmp(1,0,1);
		if (selected_index != -1)
			std::swap(tmp, colors[selected_index]);
		else if (edge_index != -1) {
			points.push_back(p_edge);
			colors.push_back(tmp);
		}
		pnt_renderer.set_color_array(ctx, colors);
		pnt_renderer.set_position_array(ctx, points);
		pnt_renderer.validate_and_enable(ctx);
		glDrawArrays(GL_POINTS, 0, points.size());
		pnt_renderer.disable(ctx);

		if (selected_index != -1)
			std::swap(tmp, colors[selected_index]);
		else if (edge_index != -1) {
			points.pop_back();
			colors.pop_back();
		}
	}
	void draw(context& ctx)
	{
		draw_points(ctx);
		glLineWidth(2);
		glColor3f(0.8f, 0, 0.7f);
		draw_unprocessed_polygon();
		glLineWidth(5);
		glColor3f(0.8f, 0.5f, 0);
		draw_polygon();
		glLineWidth(1);
		glColor3f(0, 0, 0);
		draw_triangles();
		glColor3f(0.9f, 1, 1);
		glPushMatrix();
		glScaled(2, 2, 2);
		ctx.tesselate_unit_square();
		glPopMatrix();
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
			switch (me.get_action()) {
			case MA_MOVE :
				if (view_ptr) {
					float epsilon = (float)(prs.point_size * view_ptr->get_y_extent_at_focus() / get_context()->get_height());
					vec3d p_d;
					if (get_world_location(me.get_x(), me.get_y(), *view_ptr, p_d)) {
						vec3 p = p_d;
						int last_selected_index = selected_index;
						selected_index = -1;
						float min_dist = 0;
						for (unsigned i = 0; i < polygon.size(); ++i) {
							float dist = (points[i] - p).length();
							if (dist <= epsilon) {
								if (selected_index == -1 || dist < min_dist) {
									min_dist = dist;
									selected_index = i;
								}
							}
						}
						if (last_selected_index != selected_index)
							on_set(&selected_index);

						// if no point found, check for edges
						if (selected_index == -1) {
							int last_edge_index = edge_index;
							edge_index = -1;
							vec3 last_p_edge = p_edge;
							float min_dist = 0;
							for (unsigned i = 0; i < polygon.size(); ++i) {
								const vec3& p0 = points[i];
								const vec3& p1 = points[(i + 1) % points.size()];
								vec3 d = p1 - p0;
								float lambda = dot(p - p0, d) / dot(d, d);
								if (lambda > 0 && lambda < 1) {
									vec3 pp = p0 + lambda*d;
									float dist = (pp - p).length();
									if (dist < epsilon) {
										if (edge_index == -1 || dist < min_dist) {
											min_dist = dist;
											edge_index = i;
											p_edge = pp;
											p_edge(2) = 0;
										}
									}
								}
							}
							if (last_edge_index != edge_index)
								on_set(&edge_index);
							else if ((last_p_edge - p_edge).length() > 0.5f*epsilon / prs.point_size) 
								on_set(&p_edge);
						}
						else {
							if (edge_index != -1)
								edge_index = -1;
						}
					}
				}
				break;
			case MA_DRAG :
				if (me.get_button_state() == MB_LEFT_BUTTON) {
					if (selected_index != -1) {
						vec3d p_d;
						if (get_world_location(me.get_x(), me.get_y(), *view_ptr, p_d)) {
							vec3 new_pos = p_d;
							vec3 diff = new_pos - last_pos;
							diff(2) = 0;
							points[selected_index] += diff;
							polygon[selected_index] += vec2(diff);
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
						vec3d p_d;
						if (get_world_location(me.get_x(), me.get_y(), *view_ptr, p_d)) {
							last_pos = p_d;
						}
					}
					else if (edge_index != -1) {
						vec3d p_d;
						if (get_world_location(me.get_x(), me.get_y(), *view_ptr, p_d)) {
							p_edge = last_pos = p_d;
						}
						else 
							last_pos = p_edge;

						if (edge_index == points.size() - 1) {
							points.push_back(p_edge);
							polygon.push_back(vec2(p_edge));
							colors.push_back(clr_type(0.5f, 0.5f, 0.5f));
						}
						else {
							points.insert(points.begin() + edge_index + 1, p_edge);
							polygon.insert(polygon.begin() + edge_index + 1, vec2(p_edge));
							colors.insert(colors.begin() + edge_index + 1, clr_type(0.5f, 0.5f, 0.5f));
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
							points.erase(points.begin() + selected_index);
							colors.erase(colors.begin() + selected_index);
							selected_index = -1;
							on_set(polygon[0]);
							on_set(&selected_index);
							if (find_control(nr_steps))
								find_control(nr_steps)->set("max", polygon.size() - 2);
							if (nr_steps > polygon.size() - 2) {
								nr_steps = polygon.size() - 2;
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
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const 
	{
		return "example/ear_cutting"; 
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &nr_steps) {
			perform_ear_cutting();
			classify_nodes();
			find_ears();
			set_colors();
		}
		if (member_ptr >= &polygon[0] && member_ptr < &polygon[0] + polygon.size()) {
			perform_ear_cutting();
			classify_nodes();
			find_ears();
			set_colors();
		}
		update_member(member_ptr);
		post_redraw();
	}
	/// you must overload this for gui creation
	void create_gui() 
	{	
		add_decorator("ear cutting", "heading");
		add_member_control(this, "nr_steps", nr_steps, "value_slider", "min=0;max=100;ticks=true");
		find_control(nr_steps)->set("max", polygon.size() - 2);
		add_member_control(this, "lambda", lambda, "value_slider", "min=0;max=0.5;ticks=true");
		add_member_control(this, "wireframe", wireframe, "check");
		if (begin_tree_node("point rendering", prs)) {
			align("\a");
			add_gui("points", prs);
			align("\b");
		}
	}

};

#include <cgv/base/register.h>

extern factory_registration<ear_cutting> ec_fac("new/ear_cutting", 'E', true);

