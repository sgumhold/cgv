#include "gl_point_cloud_drawable_base.h"
#include "ann_tree.h"
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/base/import.h>

using namespace std;
using namespace cgv::render;
using namespace cgv::utils::file;

gl_point_cloud_drawable_base::gl_point_cloud_drawable_base() : 
	ne(pc,ng), 	
	base_color(0.6f, 0.4f, 0.4f),
	nml_color(0.3f, 0.0f, 0.5f),
	box_color(0.0f,0.8f,0.9f)
{
	point_size = 3.0f;
	line_width = 1;
	nml_length = 0.5f;
	
	base_material.set_ambient(color_type(0.2f,0.2f,0.2f));
	base_material.set_diffuse(color_type(0.6f,0.4f,0.4f));
	base_material.set_specular(color_type(0.4f,0.4f,0.4f));
	base_material.set_shininess(20.0f);


	smooth_points = false;
	show_points = true;
	show_nmls = true;
	show_clrs = true;
	show_box = false;
	illum_points = true;
	show_neighbor_graph = false;

	k = 30;
	do_symmetrize = false;
	reorient_normals = true;
}

bool gl_point_cloud_drawable_base::read(const std::string& _file_name)
{
	std::string fn = cgv::base::find_data_file(_file_name, "cpD");
	if (fn.empty()) {
		cerr << "point cloud " << _file_name << " not found" << endl;
		return false;
	}
	if (!pc.read(fn)) {
		cerr << "could not read point cloud " << fn << endl;
		return false;
	}
	file_name = drop_extension(_file_name);
	post_redraw();
	return true;
}

bool gl_point_cloud_drawable_base::append(const std::string& _file_name)
{
	std::string fn = cgv::base::find_data_file(_file_name, "cpD");
	if (fn.empty()) {
		cerr << "point cloud " << _file_name << " not found" << endl;
		return false;
	}
	point_cloud pc1;
	if (!pc1.read(fn)) {
		cerr << "could not read point cloud " << fn << endl;
		return false;
	}
	if (!file_name.empty())
		file_name += " and "; 
	file_name += drop_extension(fn);
	pc.append(pc1);
	return true;
}

bool gl_point_cloud_drawable_base::write(const std::string& fn)
{
	if (!pc.write(fn)) {
		cerr << "could not write point cloud " << fn << endl;
		return false;
	}
	file_name = drop_extension(fn);
	return true;
}


void gl_point_cloud_drawable_base::draw_box(context& ctx, const Box& box)
{
	glDisable(GL_LIGHTING);
	glLineWidth((float)(2*line_width));
	
	glPushMatrix();
	glTranslatef(box.get_center()(0),box.get_center()(1),box.get_center()(2));
	glScalef(0.5f*box.get_extent()(0), 0.5f*box.get_extent()(1), 0.5f*box.get_extent()(2));

	GLboolean cull;
	glGetBooleanv(GL_CULL_FACE, &cull);
	if (cull)
		glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	ctx.tesselate_unit_cube();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (cull)
		glEnable(GL_CULL_FACE);
	
	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void gl_point_cloud_drawable_base::draw_box(context& ctx)
{
	if (!show_box)
		return;

	glColor3fv(&box_color[0]);
	draw_box(ctx, pc.box());	
}

void gl_point_cloud_drawable_base::draw_points(context& ctx)
{
	if (!show_points)
		return;
	if (point_size > 1 && smooth_points)
		glEnable(GL_POINT_SMOOTH);
	if (!illum_points)
		glDisable(GL_LIGHTING);
	glPolygonOffset(-2,-5);
	glEnable(GL_POLYGON_OFFSET_POINT);

		glColor3fv(&base_color[0]);
		glPointSize(point_size);

		int n = (int)pc.get_nr_points();
		glDrawArrays(GL_POINTS,0,n);

	glDisable(GL_POLYGON_OFFSET_POINT);
	if (!illum_points && pc.has_normals())
		glEnable(GL_LIGHTING);
	glDisable(GL_POINT_SMOOTH);
}

void gl_point_cloud_drawable_base::draw_normals(context& ctx)
{
	if (!show_nmls || !pc.has_normals())
		return;
	glDisable(GL_LIGHTING);
		glColor3fv(&nml_color[0]);
		glLineWidth(line_width);
		glBegin(GL_LINES);
		float nml_scale = (nml_length*pc.box().get_extent().length()/sqrt((float)pc.get_nr_points()));
		for (unsigned int i=0; i<pc.get_nr_points(); ++i) {
			Pnt dp = pc.pnt(i);
			glVertex3fv(dp);
			dp += nml_scale*pc.nml(i);
			glVertex3fv(dp);
		}
		glEnd();
	glEnable(GL_LIGHTING);
}

void gl_point_cloud_drawable_base::draw_edge_color(unsigned int vi, unsigned int j, bool is_symm, bool is_start) const
{
	if (is_symm)
		glColor3f(0.5f,0.5f,0.5f);
	else {
		if (is_start)
			glColor3f(1,0.5f,0.5f);
		else
			glColor3f(1,1,0.5f);
	}
}

void gl_point_cloud_drawable_base::draw_graph(context& ctx)
{
	if (!show_neighbor_graph)
		return;

	glDisable(GL_LIGHTING);
		glColor3f(0.5f,0.5f,0.5f);
		glLineWidth(line_width);
		glBegin(GL_LINES);
		for (unsigned int vi=0; vi<ng.size(); ++vi) {
			const std::vector<Idx> &Ni = ng[vi];
			for (unsigned int j=0; j<Ni.size(); ++j) {
				unsigned int vj = Ni[j];
				// check for symmetric case and only draw once
				if (ng.is_directed_edge(vj,vi)) {
					if (vi < vj) {
						draw_edge_color(vi,j,true,true);
						glArrayElement(vi);
						draw_edge_color(vi,j,true,false);
						glArrayElement(vj);
					}
				}
				else {
					draw_edge_color(vi,j,false,true);
					glArrayElement(vi);
					draw_edge_color(vi,j,false,false);
					glArrayElement(vj);
				}
			}
		}
		glEnd();
	glEnable(GL_LIGHTING);
}

void gl_point_cloud_drawable_base::draw(context& ctx)
{
	if (pc.get_nr_points() == 0)
		return;

	ctx.enable_material(base_material);
	glEnable(GL_COLOR_MATERIAL);

	draw_box(ctx);

	glVertexPointer(3, GL_FLOAT, 0, &(pc.pnt(0).x()));
	glEnableClientState(GL_VERTEX_ARRAY);
	draw_graph(ctx);
	glDisableClientState(GL_VERTEX_ARRAY);

	draw_normals(ctx);

	glVertexPointer(3, GL_FLOAT, 0, &(pc.pnt(0).x()));
	glEnableClientState(GL_VERTEX_ARRAY);
	if (pc.has_colors() && show_clrs) {
		glColorPointer(3, GL_FLOAT, 0, &(pc.clr(0)[0]));
		glEnableClientState(GL_COLOR_ARRAY);
	}
	if (pc.has_normals()) {
		glNormalPointer(GL_FLOAT, 0, &(pc.nml(0).x()));
		glEnableClientState(GL_NORMAL_ARRAY);
	}
	else
		glDisable(GL_LIGHTING);

	draw_points(ctx);

	if (pc.has_normals())
		glDisableClientState(GL_NORMAL_ARRAY);
	else
		glEnable(GL_LIGHTING);
	if (pc.has_colors() && show_clrs)
		glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_POINT_SMOOTH);
	glPointSize(1.0f);

	glEnable(GL_LIGHTING);
	glDisableClientState(GL_VERTEX_ARRAY);
	glLineWidth(1);
	ctx.disable_material(base_material);
}


void gl_point_cloud_drawable_base::clear()
{
	ng.clear();
}


void gl_point_cloud_drawable_base::build_neighbor_graph()
{
	clear();
	ann_tree T;
	T.build(pc);		
	cgv::utils::statistics he_stats;
	ng.build(pc.get_nr_points(), k, T, &he_stats);
	if (do_symmetrize)
		ng.symmetrize();
	std::cout << "half edge statistics " << he_stats << std::endl;
	cout << "v " << pc.get_nr_points()
		  << ", he = " << ng.nr_half_edges 
		  << " ==> " << (float)ng.nr_half_edges/((unsigned )(pc.get_nr_points())) << " half edges per vertex" << endl;
}

void gl_point_cloud_drawable_base::compute_normals()
{
	if (ng.empty())
		build_neighbor_graph();
	ne.compute_weighted_normals(reorient_normals && pc.has_normals());
	post_redraw();
}

void gl_point_cloud_drawable_base::recompute_normals()
{
	if (ng.empty())
		build_neighbor_graph();
	if (!pc.has_normals())
		compute_normals();
//	ne.compute_bilateral_weighted_normals(reorient_normals);
	ne.compute_plane_bilateral_weighted_normals(reorient_normals);
	post_redraw();
}

void gl_point_cloud_drawable_base::orient_normals()
{
	if (ng.empty())
		build_neighbor_graph();
	if (!pc.has_normals())
		compute_normals();
	ne.orient_normals();
	post_redraw();
}


bool gl_point_cloud_drawable_base::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 
		rh.reflect_member("data_path", data_path) &&
		rh.reflect_member("file_name", file_name) &&
		rh.reflect_member("point_size", point_size) &&
		rh.reflect_member("line_width", line_width) &&
		rh.reflect_member("nml_length", nml_length) &&
		rh.reflect_member("show_points", show_points) &&
		rh.reflect_member("show_nmls", show_nmls) &&
		rh.reflect_member("show_box", show_box) &&
		rh.reflect_member("illum_points", illum_points) &&
		rh.reflect_member("show_neighbor_graph", show_neighbor_graph) &&
		rh.reflect_member("k", k) &&
		rh.reflect_member("do_symmetrize", do_symmetrize) &&
		rh.reflect_member("reorient_normals", reorient_normals);
}

#include <cgv/base/find_action.h>
#include <cgv/render/view.h>

void gl_point_cloud_drawable_base::orient_normals_to_view_point()
{
	cgv::base::base_ptr bp(dynamic_cast<cgv::base::base*>(this));
	if (bp) {
		vector<cgv::render::view*> views;
		cgv::base::find_interface<cgv::render::view>(bp, views);
		if (views.empty())
			return;
		if (ng.empty())
			build_neighbor_graph();
		Pnt view_point = views[0]->get_eye();
		ne.orient_normals(view_point);
		post_redraw();
	}
}

