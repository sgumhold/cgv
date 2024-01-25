#include "gl_delaunay_mesh_draw.h"
#include <cgv/utils/stopwatch.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/find_action.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/render/view.h>
#include <iostream>

using namespace cgv::render::gl;
using namespace cgv::signal;

gl_delaunay_mesh_draw::gl_delaunay_mesh_draw() : node("delaunay")
{
	show_corner_ds = false;
	show_points = true;
	show_voronoi_vertices = false;
	show_voronoi_edges = false;
	show_delaunay_triangles = true;
	show_vertex_labels = false;
	show_reconstruction = true;
	debug_point_location = false;
	debug_flipable = false;
	debug_trace = false;
	debug_nearest_neighbor = false;
	debug_next_insertion = false;
	debug_largest_circle = false;
	set_random_seed = true;
	tm = 0;
	nr_to_be_added = 1;
	tm_bak = 0;
	s  = 312;
	primitive_scale = 1.0;
	hierarchy_factor = 32;
	sample_sampling = tm_type::ST_RANDOM;
	sample_distribution = tm_type::DT_UNIFORM;
	sample_generator = tm_type::GT_RANDOM;
	sample_shape = tm_type::ST_SQUARE;
	sample_strategy = tm_type::SS_REJECTION;
	sample_size = 100;
	sample_shuffle = true;

	init_mesh(25);
	measure_time();
}

void gl_delaunay_mesh_draw::init_mesh(unsigned int n)
{
	if (tm)
		delete tm;
	if (tm_bak)
		delete tm_bak;
	tm_bak = 0;
	if (set_random_seed)
		srand(s);
	tm = new tm_type();
	tm->generate_sample_data_set(n, sample_sampling, sample_distribution, sample_generator, sample_shape, sample_strategy, sample_shuffle);
	tm->set_hierarchy_factor(hierarchy_factor);
	vi = 0;
	vc = 0;
	first_other_vi = -1;
	ti_lc = -1;
	show_reconstruction = false;
	update_member(&show_reconstruction);
	post_redraw();
}

bool gl_delaunay_mesh_draw::init(context& ctx)
{

	std::vector<cgv::render::view*> views;
	cgv::base::find_interface(base_ptr(this),views);
	for (unsigned i = 0; i < views.size(); ++i) {
		views[i]->set_eye_keep_extent(cgv::render::view::vec3(0,0,10));
		views[i]->set_y_extent_at_focus(2);
	}
	return true;
}

std::string gl_delaunay_mesh_draw::get_type_name() const 
{
	return "gl_delaunay_mesh_draw"; 
}


void gl_delaunay_mesh_draw::draw_edge(unsigned int ci) const
{
	glArrayElement(tm->vi_of_ci(tm->next(ci)));
	glArrayElement(tm->vi_of_ci(tm->prev(ci)));
}

void gl_delaunay_mesh_draw::draw_triangle(unsigned int ci) const
{
	glArrayElement(tm->vi_of_ci(pli.ci));
	glArrayElement(tm->vi_of_ci(tm->next(pli.ci)));
	glArrayElement(tm->vi_of_ci(tm->prev(pli.ci)));
}

#define sqr(x) ((x)*(x))

void gl_delaunay_mesh_draw::compute_draw_data()
{
	if ( (show_voronoi_vertices || show_voronoi_edges) && tm->get_nr_triangles() > 0) {
		unsigned int T = tm->get_nr_triangles();
		vv.resize(T);
		unsigned int ti;
		for (ti=0; ti<T; ++ti)
			vv[ti] = tm->compute_circum_center(tm->ci_of_ti(ti));
	}
	if (debug_trace && vi < tm->get_nr_vertices() && vi > 0 && vc < tm->get_nr_vertices() && vc != vi-1)
		tm->trace_segment(vc, vi-1, trace_hits);

	if (debug_point_location && tm->get_nr_triangles() > 0) {
		pli = tm->localize_point(tm->p_of_vi(vi),
			tm->ci_of_vi(tm->find_nearest_neighbor(tm->p_of_vi(vi))));
	}
	if (debug_nearest_neighbor && tm->get_nr_triangles() > 0)
		vnn = tm->find_nearest_neighbor(tm->p_of_vi(vi));
	compute_largest_circle();
}

void gl_delaunay_mesh_draw::compute_largest_circle()
{
	if (debug_largest_circle && tm->get_nr_triangles() > 0) {
		unsigned int T = tm->get_nr_triangles();
		unsigned int ti;
		tm_type::point_type c;
		r_lc = 0;
		ti_lc = -1;
		for (ti=0; ti<T; ++ti) {
			const tm_type::point_type& p = tm->p_of_vi(tm->vi_of_ci(tm->ci_of_ti(ti)));
			const tm_type::point_type* cp = &c;
			if (ti >= vv.size())
				c = tm->compute_circum_center(tm->ci_of_ti(ti));
			else
				cp = &vv[ti];
			double r = sqrt(sqr(cp->x()-p.x())+sqr(cp->y()-p.y()));
			if (r > r_lc || ti_lc == -1) {
				tm_type::point_location_info pli = tm->localize_point(*cp, tm->ci_of_ti(ti));
				if (!pli.is_outside) {
					r_lc = r;
					ti_lc = ti;
					c_lc = *cp;
				}
			}
		}
	}
}

void gl_delaunay_mesh_draw::clear_draw_data()
{
	vv.clear();
	trace_hits.clear();
	vnn = -1;
}

void gl_delaunay_mesh_draw::draw_text_elements(context& ctx) const
{
	if (show_vertex_labels) {
		glColor3f(0.7f,0.7f,0.7f);
		for (unsigned int vj=0; vj < (vi==0?tm->get_nr_vertices():vi); ++vj) {
			const tm_type::point_type& p0 = tm->p_of_vi(vj);
			ctx.set_cursor(cgv::math::vec<double>(p0.x(),p0.y(),0));
			ctx.output_stream() << "v" << vj;
			ctx.output_stream().flush();
		}
	}
}
void gl_delaunay_mesh_draw::draw_point_elements() const
{
	glVertexPointer(2, gl_traits<coord_type>::type, 0, &(tm->p_of_vi(0).x()));
	glEnableClientState(GL_VERTEX_ARRAY);
	if (debug_next_insertion) {
		glPointSize(3*(float)primitive_scale);
		glColor3f(0.8f, 0.0f, 0.6f);
		glDrawArrays(GL_POINTS, vi, (tm->get_nr_triangles() == 0) ? 3 : 1);
	}
	if (debug_nearest_neighbor && tm->get_nr_triangles() > 0) {
		glPointSize(3*(float)primitive_scale);
		glColor3f(1.0f, 1.0f, 1.0f);
		glDrawArrays(GL_POINTS, vnn, 1);
	}
	if (debug_trace && vi < tm->get_nr_vertices() && vi > 0 && vc < tm->get_nr_vertices() && vc != vi-1) {
		// draw constraint vertices
		glPointSize(2*(float)primitive_scale);
		glColor3f(0.5f, 0.5f, 0.5f);
		glDrawArrays(GL_POINTS, vc, 1);
		glDrawArrays(GL_POINTS, vi-1, 1);
		// draw incident vertices
		glColor3f(0.9f,0.6f,0.3f);
		for (unsigned int i=0; i<trace_hits.size(); ++i) 
			if (trace_hits[i].type == tm_type::INCIDENT_VERTEX) 
			glDrawArrays(GL_POINTS, trace_hits[i].vi_of_incident_vertex, 1);
	}
	if (show_points) {
		glPointSize(2*(float)primitive_scale);
		glColor3f(1.0f, 0.0f, 0.0f);
		if (first_other_vi == -1)
			glDrawArrays(GL_POINTS, 0, vi == 0 ? tm->get_nr_vertices() : vi);
		else {
			glDrawArrays(GL_POINTS, 0, first_other_vi);
			glColor3f(1.0f, 1.0f, 0.0f);
			glDrawArrays(GL_POINTS, first_other_vi, tm->get_nr_vertices()-first_other_vi);
		}
	}
	if (show_voronoi_vertices && tm->get_nr_triangles() > 0) {
		glVertexPointer(2, gl_traits<coord_type>::type, 0, &(vv[0].x()));
		glPointSize(2*(float)primitive_scale);
		glColor3f(0.0f, 1.0f, 0.0f);
		glDrawArrays(GL_POINTS, 0, tm->get_nr_triangles());
	}
	glDisableClientState(GL_VERTEX_ARRAY);
}

void gl_delaunay_mesh_draw::draw_line_elements() const
{
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (debug_largest_circle && ti_lc != -1) {
		glLineWidth(1.5f*(float)primitive_scale);
		glColor3f(1.0f,1.0f,1.0f);
		glBegin(GL_LINE_LOOP);
		for (unsigned i=0; i<360; ++i) {
			double a = 0.01745329252*i;
			glVertex2d(c_lc.x()+r_lc*cos(a),c_lc.y()+r_lc*sin(a));
		}
		glEnd();
	}
	if (show_corner_ds) {
		glLineWidth(0.5f*(float)primitive_scale);
		glColor3f(1.0f,1.0f,1.0f);
		glBegin(GL_LINES);
		for (unsigned int ci=0; ci<tm->get_nr_corners(); ++ci) {
			if (tm->is_opposite_to_border(ci))
				continue;
			const tm_type::point_type& p0 = tm->p_of_vi(tm->vi_of_ci(ci));
			const tm_type::point_type& p1 = tm->p_of_vi(tm->vi_of_ci(tm->next(ci)));
			const tm_type::point_type& p2 = tm->p_of_vi(tm->vi_of_ci(tm->prev(ci)));
			const tm_type::point_type& p3 = tm->p_of_vi(tm->vi_of_ci(tm->inv(ci)));
			tm_type::point_type c = tm_type::point_type((coord_type)(0.8*p0.x()+0.1*p1.x()+0.1*p2.x()),
													(coord_type)(0.8*p0.y()+0.1*p1.y()+0.1*p2.y()));
			tm_type::point_type d = tm_type::point_type((coord_type)(0.8*c.x()+0.2*p3.x()),
				                           (coord_type)(0.8*c.y()+0.2*p3.y()));
			glVertex2d(p0.x(),p0.y());
			glVertex2d(c.x(),c.y());
			glVertex2d(c.x(),c.y());
			glVertex2d(d.x(),d.y());
		}
		glEnd();
	}
	glLineWidth((float)primitive_scale);
	if (show_voronoi_edges && tm->get_nr_triangles() > 0) {
		unsigned int T = tm->get_nr_triangles();		
		glColor3f(0.0f, 1.0f, 0.5f);
		glVertexPointer(2, gl_traits<coord_type>::type, 0, &(vv[0].x()));
		glEnableClientState(GL_VERTEX_ARRAY);
		glBegin(GL_LINES);
		for (unsigned int ci=0; ci<tm->get_nr_corners(); ++ci)
			if (!tm->is_opposite_to_border(ci)) {
				unsigned int cj = tm->inv(ci);
				if (cj < ci)
					continue;
				glArrayElement(tm->ti_of_ci(ci));
				glArrayElement(tm->ti_of_ci(cj));
			}
		glEnd();
		glDisableClientState(GL_VERTEX_ARRAY);
		glBegin(GL_LINES);
		for (unsigned int ci=0; ci<tm->get_nr_corners(); ++ci)
			if (tm->is_opposite_to_border(ci)) {
				glVertex2d(vv[tm->ti_of_ci(ci)].x(),vv[tm->ti_of_ci(ci)].y());
				const tm_type::point_type& p0 = tm->p_of_vi(tm->vi_of_ci(tm->next(ci)));
				const tm_type::point_type& p1 = tm->p_of_vi(tm->vi_of_ci(tm->prev(ci)));
				glVertex4d(p1.y()-p0.y(),p0.x()-p1.x(),0,0);
			}
		glEnd();
	}
	glVertexPointer(2, gl_traits<coord_type>::type, 0, &(tm->p_of_vi(0).x()));
	glEnableClientState(GL_VERTEX_ARRAY);
	
	if (show_reconstruction && tm->get_nr_triangles() > 0 && first_other_vi != -1) {
		glColor3f(0.9f,0.7f,0.6f);
		glBegin(GL_LINES);
		for (unsigned ti=0; ti<tm->get_nr_triangles(); ++ti) {
			unsigned ci = tm->ci_of_ti(ti);
			unsigned v0 = tm->vi_of_ci(ci);
			unsigned v1 = tm->vi_of_ci(ci+1);
			unsigned v2 = tm->vi_of_ci(ci+2);
			if (v0 < v1 && v1 < (unsigned)first_other_vi)	draw_edge(ci+2);
			if (v1 < v2 && v2 < (unsigned)first_other_vi)	draw_edge(ci);
			if (v2 < v0 && v0 < (unsigned)first_other_vi)	draw_edge(ci+1);
		}
		glEnd();
	}

	if (debug_trace && vi < tm->get_nr_vertices() && vi > 0 && vc < tm->get_nr_vertices() && vc != vi-1) {
		// draw constraint edge
		glColor3f(0.5f,0.5f,0.5f);
		glBegin(GL_LINES);
		glArrayElement(vi-1);
		glArrayElement(vc);
		glEnd();

		// draw intersected edges
		unsigned int i;
		glColor3f(0.3f,0.6f,0.7f);
		glBegin(GL_LINES);
		for (i=0; i<trace_hits.size(); ++i) 
			if (trace_hits[i].type == tm_type::INTERSECTED_EDGE)
				draw_edge(trace_hits[i].ci_of_intersected_edge);
		glEnd();

		// draw incident edges
		glColor3f(0.9f,0.6f,0.3f);
		glBegin(GL_LINES);
		for (i=0; i<trace_hits.size(); ++i) 
			if (trace_hits[i].type == tm_type::INCIDENT_EDGE) 
				draw_edge(trace_hits[i].ci_of_incident_edge);
		glEnd();
	}
	if (debug_flipable) {
		// draw all not flipable edges
		glColor3f(0.7f, 0.6f, 0.0f);
		glBegin(GL_LINES);
		for (unsigned int ci=0; ci<tm->get_nr_corners(); ++ci) {
			if (!tm->is_opposite_to_border(ci) && tm->inv(ci) < ci)
				continue;
			if (tm->is_edge_flip_valid(ci))
				continue;
			glArrayElement(tm->vi_of_ci(tm->next(ci)));
			glArrayElement(tm->vi_of_ci(tm->prev(ci)));
		}
		glEnd();
	}
	if (debug_point_location && tm->get_nr_triangles() > 0)
		if (pli.is_outside) {
			glColor3f(0.0f,1.0f,1.0f);
			glBegin(GL_LINES);
				draw_edge(pli.ci);
			glEnd();
		}
		else {
			glColor3f(1.0f,0.0f,1.0f);
			glBegin(GL_TRIANGLES);
				draw_triangle(pli.ci);
			glEnd();
		}

	if (show_delaunay_triangles && tm->get_nr_triangles() > 0) {
		glColor3f(0.0f,0.0f,1.0f);
		glBegin(GL_TRIANGLES);
		for (unsigned int ci=0; ci<tm->get_nr_corners(); ++ci)
			glArrayElement(tm->vi_of_ci(ci));
		glEnd();
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_BLEND);
}

void gl_delaunay_mesh_draw::draw(context& ctx)
{
	if (!tm || tm->get_nr_vertices() == 0)
		return;

	// configure gl
	glPolygonMode(GL_FRONT, GL_LINE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_POINT_SMOOTH);
	glDisable(GL_LIGHTING);

	// compute necessary data
	compute_draw_data();

	// draw all text first
	draw_text_elements(ctx);

	// second draw all points
	draw_point_elements();

	// finally draw all lines
	draw_line_elements();

	clear_draw_data();

	glEnable(GL_LIGHTING);
	glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_POINT_SMOOTH);
	glPointSize(1.0f);
}

void gl_delaunay_mesh_draw::measure_time(unsigned int debug_point) 
{
	unsigned int m;
	if (vi < 3 || vi >= tm->get_nr_vertices()) {
		init_mesh(sample_size);
		m = tm->get_nr_vertices()/20;
	}
	else
		m = (tm->get_nr_vertices()-vi)/20;
	unsigned int n = tm->get_nr_vertices();
	if (m < 1000)
		m = 1000;
	if (debug_point != -1)
		n = debug_point;
	if (n == vi)
		return;
	std::cout << "test delaunay runtime" << std::endl;
	{
		cgv::utils::stopwatch w;
		if (vi == 0) {
			tm->add_triangle(0,1,2);
			vi = 3;
		}
		for (; vi < n; ++vi) {
			if (vi%m == 0)
				std::cout << vi << std::endl;
			tm->insert_vertex(vi);
		}
	}
	vi = 0;
	if (first_other_vi != -1) {
		show_reconstruction = true;
		update_member(&show_reconstruction);
	}
	post_redraw();
}

void gl_delaunay_mesh_draw::clear_mesh()
{
	tm->clear();
	post_redraw();
}

void gl_delaunay_mesh_draw::clear_all()
{
	tm->clear_triangles();
	post_redraw();
}

void gl_delaunay_mesh_draw::insert_triangle()
{
	unsigned int N = tm->get_nr_vertices();
	tm->add_triangle(vi, (vi+1)%N, (vi+2)%N);
	vi = (vi+3)%N;
	//tm->is_ci_of_vi_consistent(vi);
	post_redraw();
}

void gl_delaunay_mesh_draw::insert_point()
{
	if (tm_bak) {
		delete tm;
		tm = tm_bak;
		tm_bak = 0;
	}
//	if (fltk::event_state() != fltk::SHIFT)
		tm->insert_vertex(vi);
	if (++vi >= tm->get_nr_vertices())
		vi = 0;
	//tm->is_ci_of_vi_consistent(vi);
	post_redraw();
}

void gl_delaunay_mesh_draw::add_voronoi_points()
{
	if (tm->get_nr_triangles() > 0) {
		first_other_vi = vi = tm->get_nr_vertices();
		std::cout << "vi = " << vi << std::endl;
		unsigned int T = tm->get_nr_triangles();
		unsigned int ti;
		for (ti=0; ti<T; ++ti)
			tm->add_point(tm->compute_circum_center(tm->ci_of_ti(ti)));
		std::cout << "nr vertices = " << tm->get_nr_vertices() << std::endl;
	}
	post_redraw();
}

void gl_delaunay_mesh_draw::add_circle_center()
{
	for (unsigned i=0; i<nr_to_be_added; ++i) {
		if (i > 0)
			compute_largest_circle();
		if (ti_lc != -1) {
			tm->add_point(c_lc);
			tm->insert_vertex(tm->get_nr_vertices()-1);
		}
	}
	post_redraw();
}

void gl_delaunay_mesh_draw::create_gui()
{
	if (begin_tree_node("Sampling", set_random_seed, true)) {

		align("\a");

			add_control("set random seed", set_random_seed, "check");
			add_control("random seed", s);
			add_control("nr points", sample_size, "value_slider", "min=3;max=100000000;step=1;log=true");
			add_control("sampling", sample_sampling, "dropdown", "enums='regular,random,stratified'");
			add_control("distribution", sample_distribution, "dropdown", "enums='uniform,normal'");
			add_control("generator", sample_generator, "dropdown", "enums='random,pseudo random default,mt'");
			add_control("shape", sample_shape, "dropdown", "enums='square,triangle,circle,spiral,terrain'");
			add_control("strategy", sample_strategy, "dropdown", "enums='rejection,transform'");
			add_control("shuffle", sample_shuffle, "check");
			connect_copy(add_button("generate")->click, rebind(this, &gl_delaunay_mesh_draw::init_mesh, _r(sample_size)));
			connect_copy(add_button("add voronoi points")->click, rebind(this, &gl_delaunay_mesh_draw::add_voronoi_points));

		align("\b");

		end_tree_node(set_random_seed);
	}
	add_decorator("Delaunay", "heading");
	add_control("hierarchy fac", hierarchy_factor, "value_slider", "min=2;max=128;step=1;log=true");
	connect_copy(add_button("clear mesh")->click, rebind(this, &gl_delaunay_mesh_draw::clear_mesh));
	connect_copy(add_button("clear all")->click, rebind(this, &gl_delaunay_mesh_draw::clear_all));
	connect_copy(add_button("insert triangle")->click, rebind(this, &gl_delaunay_mesh_draw::insert_triangle));
	connect_copy(add_button("insert point")->click, rebind(this, &gl_delaunay_mesh_draw::insert_point));
	connect_copy(add_button("measure time")->click, rebind(this, &gl_delaunay_mesh_draw::measure_time,-1));

	add_decorator("Visualization", "heading");
	add_control("scale", primitive_scale,"value_slider", "min=0.1;max=10.0;step=0.001;log=true");
	add_control("corners", show_corner_ds, "check");
	add_control("points", show_points, "check");
	add_control("voronoi vertices", show_voronoi_vertices, "check");
	add_control("voronoi edges", show_voronoi_edges, "check");
	add_control("delaunay tgls", show_delaunay_triangles, "check");
	add_control("vertex labels", show_vertex_labels, "check");
	add_control("reconstruction", show_reconstruction, "check");

	add_decorator("Debug", "heading");
	add_control("point loc", debug_point_location, "check");
	add_control("flipable", debug_flipable, "check");
	add_control("trace", debug_trace, "check");
	add_control("nn", debug_nearest_neighbor, "check");
	add_control("next insert", debug_next_insertion, "check");
	add_control("largest circle", debug_largest_circle, "check");
	connect_copy(add_button("add circle center", "shortcut='i'")->click, rebind(this, &gl_delaunay_mesh_draw::add_circle_center));
	add_control("nr to be added", nr_to_be_added, "value_slider", "min=1;max=1000;log=true;ticks=true");

	connect_copy(find_control(primitive_scale)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(debug_next_insertion)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(debug_nearest_neighbor)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(debug_trace)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(debug_flipable)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(debug_point_location)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(debug_largest_circle)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(show_vertex_labels)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(show_delaunay_triangles)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(show_voronoi_edges)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(show_voronoi_vertices)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(show_points)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(find_control(show_corner_ds)->value_change, rebind(static_cast<drawable*>(this),&drawable::post_redraw));
}

/*
void gl_delaunay_mesh_draw::show_stats(int& y) const 
{
	const char* shape_strings[]    = { "square", "triangle", "circle" };
	const char* sampling_strings[] = { "random", "random high res", "uniform" };
	const char* bool_strings[] = { "false", "true" };

	gl_printf(10,y+=20,"Delaunay Mesh Drawable:");
	gl_printf(20,y+=20,"   sample: n=%d <N/n>, sampling=%s <s>, shape=%s <S>, shuffle=%s <u>, set seed=%s <0>", 
		sample_size, sampling_strings[sample_sampling], shape_strings[sample_shape], bool_strings[sample_shuffle], bool_strings[set_random_seed]);
	gl_printf(20,y+=20,"   |P|=%d, |T|=%d, vi=%d, vc=%d <c/C>, s=%d <R/r>, h=%d <H/h>", 
		tm->get_nr_vertices(), 
		tm->get_nr_triangles(), vi, vc, s, hierarchy_factor);
}

void gl_delaunay_mesh_draw::show_help(int& y) const 
{ 
	const char* toggle_strings[] = { "on ", "off" };
	gl_printf(10,y+=20,"Delaunay Mesh Drawable:");
	gl_printf(20,y+=20,"  toggle: vertex <l>abels %s, points %s <d>, delaunay %s <D>, c<o>rners %s",
		toggle_strings[show_vertex_labels], toggle_strings[show_points], toggle_strings[show_delaunay_triangles], toggle_strings[show_corner_ds]);
	gl_printf(20,y+=20,"          voronoi points %s <v>/diagram %s <V>, ",
		toggle_strings[show_voronoi_vertices], toggle_strings[show_voronoi_edges]);
	gl_printf(20,y+=20,"  debug:  next insertion %s <1>, point location %s <2>, nearest neighbor %s <3>",
		toggle_strings[debug_next_insertion], toggle_strings[debug_point_location], toggle_strings[debug_nearest_neighbor]);
	gl_printf(20,y+=20,"          flipable edges %s <4>, trace %s <5>",
		toggle_strings[debug_flipable], toggle_strings[debug_trace]);
	gl_printf(20,y+=20,"  point and line scale=%f <8/9>", primitive_scale);
	gl_printf(20,y+=20,"  clear triangles <x> /all <X>, init <i[small]/I>, add <[t]riangle/[p]oint>, skip <P>oint");
	gl_printf(20,y+=20,"  measure <m/M[reinit to small]");
}

gl_delaunay_mesh_draw::Result gl_delaunay_mesh_draw::handle_event(int event) 
{
	if (event == fltk::KEY) {
		switch (fltk::event_key()) {
		case 'f' :
			if (tm_bak) {
				delete tm;
				tm = tm_bak;
				tm_bak = 0;
			}
			else {
				// flip constraints
				std::vector<tm_type::trace_hit> trace_hits;
				tm->trace_segment(vc, vi-1, trace_hits);
				tm_bak = new tm_type(*tm);
				std::vector<unsigned int> intersected_edge_cis;
				for (unsigned int i=0; i<trace_hits.size(); ++i)
					if (trace_hits[i].type == tm_type::INTERSECTED_EDGE)
						intersected_edge_cis.push_back(trace_hits[i].ci_of_intersected_edge);
					else
						break;
				if (!intersected_edge_cis.empty())
					tm->flip_edges_to_insert_segment(vc, vi-1, intersected_edge_cis);
			}
			break;
		case 'c' :
			if (tm_bak) {
				delete tm;
				tm = tm_bak;
				tm_bak = 0;
			}
			if (fltk::event_state() == fltk::SHIFT) {
				if (vc > 0)
					--vc;
			}
			else {
				if (vc+1 < vi)
					++vc;
			}
			return HANDLED_AND_REDRAW;
	}
}
*/


#include <cgv/base/register.h>

factory_registration<gl_delaunay_mesh_draw> del_fac("New/Algorithms/Delaunay", 'D', true);

