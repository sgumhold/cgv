#include "gl_point_cloud_drawable_base.h"
#include "ann_tree.h"
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
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
	view_ptr = 0;

	outline_width_from_pixel = 1.5f;
	percentual_outline_width = 0.0f;
	point_size = 3.0f;
	line_width = 1;
	nml_length = 0.5f;

	show_point_step = 1;
	show_point_begin = 0;
	show_point_end = 0;
	orient_splats = true;

	base_material.set_ambient(color_type(0.2f,0.2f,0.2f));
	base_material.set_diffuse(color_type(0.6f,0.4f,0.4f));
	base_material.set_specular(color_type(0.4f,0.4f,0.4f));
	base_material.set_shininess(20.0f);

	sort_points = true;
	smooth_points = true;
	show_points = true;
	show_nmls = true;
	show_clrs = true;
	show_box = false;
	illum_points = true;
	show_neighbor_graph = false;
	use_point_shader = true;	

	blend_points = true;
	backface_cull_points = true;

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
	show_point_begin = 0;
	show_point_end = pc.get_nr_points();

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
	show_point_begin = 0;
	show_point_end = pc.get_nr_points();
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
	if (!ensure_view_pointer())
		exit(0);

	if (!show_points)
		return;
	if (backface_cull_points) {
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
	}
	else
		glDisable(GL_CULL_FACE);
	if (point_size > 1 && smooth_points) {
		glEnable(GL_POINT_SMOOTH);
		if (blend_points) {
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}
	if (!illum_points)
		glDisable(GL_LIGHTING);
	glPolygonOffset(-2, -5);
	glEnable(GL_POLYGON_OFFSET_POINT);

	glColor3fv(&base_color[0]);
	glPointSize(point_size);

	if (use_point_shader && pc_prog.is_linked()) {
		// compute reference point size
		Pnt e = pc.box().get_extent();
		Crd A = 2.0f*(e(0)*e(1) + e(1)*e(2) + e(2)*e(0));
		Crd reference_point_size = 0.1f*sqrt(A/pc.get_nr_points());
		cgv::render::gl::set_lighting_parameters(ctx, pc_prog);
		pc_prog.set_uniform(ctx, "has_normals", pc.has_normals());
		pc_prog.set_uniform(ctx, "cull_backfacing", backface_cull_points);
		pc_prog.set_uniform(ctx, "smooth_points", smooth_points);
		pc_prog.set_uniform(ctx, "illum_points", illum_points);
		pc_prog.set_uniform(ctx, "orient_splats", orient_splats);
		pc_prog.set_uniform(ctx, "point_size", reference_point_size*point_size*sqrt((float)show_point_step));
		pc_prog.set_uniform(ctx, "width", ctx.get_width());
		pc_prog.set_uniform(ctx, "height", ctx.get_height());
		float pixel_extent_per_depth = (float)(2.0*tan(0.5*0.0174532925199*view_ptr->get_y_view_angle())/ctx.get_height());
		pc_prog.set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
		pc_prog.set_uniform(ctx, "outline_width_from_pixel", outline_width_from_pixel);
		pc_prog.set_uniform(ctx, "percentual_outline_width", 0.01f*percentual_outline_width);
		pc_prog.enable(ctx);
	}
	std::size_t n = (show_point_end - show_point_begin) / show_point_step;
	GLint offset = show_point_begin / show_point_step;

	if (sort_points && ensure_view_pointer()) {
		struct sort_pred {
			const point_cloud& pc;
			const Pnt& view_dir;
			unsigned show_point_step;
			bool operator () (GLint i, GLint j) const {
				return dot(pc.pnt(show_point_step*i), view_dir) > dot(pc.pnt(show_point_step*j), view_dir);
			}
			sort_pred(const point_cloud& _pc, const Pnt& _view_dir, unsigned _show_point_step) : pc(_pc), view_dir(_view_dir), show_point_step(_show_point_step) {}
		};
		Pnt view_dir = view_ptr->get_view_dir();
		std::vector<GLint> indices;
		indices.resize(n);
		size_t i;
		for (i = 0; i < indices.size(); ++i)
			indices[i] = (GLint)i + offset;
		std::sort(indices.begin(), indices.end(), sort_pred(pc, view_dir, show_point_step));

		glDepthFunc(GL_ALWAYS);
		glDrawElements(GL_POINTS, n, GL_UNSIGNED_INT, &indices[0]);
		glDepthFunc(GL_LESS);
	}
	else {
		glDrawArrays(GL_POINTS, offset, n);
	}

	if (use_point_shader && pc_prog.is_linked())
		pc_prog.disable(ctx);

	glDisable(GL_POLYGON_OFFSET_POINT);
	if (!illum_points && pc.has_normals())
		glEnable(GL_LIGHTING);
	if (point_size > 1 && smooth_points) {
		glDisable(GL_POINT_SMOOTH);
		if (blend_points) {
			glDisable(GL_BLEND);
		}
	}
	if (backface_cull_points) {
		glDisable(GL_CULL_FACE);
	}
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

void gl_point_cloud_drawable_base::init_frame(cgv::render::context& ctx)
{
	if (!pc_prog.is_created()) {
		if (!pc_prog.build_program(ctx, "pc.glpr", true)) {
			exit(0);
		}
	}
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

	glVertexPointer(3, GL_FLOAT, sizeof(Pnt)*show_point_step, &(pc.pnt(0).x()));
	glEnableClientState(GL_VERTEX_ARRAY);
	if (pc.has_colors() && show_clrs) {
		glColorPointer(3, GL_FLOAT, sizeof(Clr)*show_point_step, &(pc.clr(0)[0]));
		glEnableClientState(GL_COLOR_ARRAY);
	}
	if (pc.has_normals()) {
		glNormalPointer(GL_FLOAT, sizeof(Nml)*show_point_step, &(pc.nml(0).x()));
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

void gl_point_cloud_drawable_base::orient_normals_to_view_point()
{
	if (ensure_view_pointer()) {
		if (ng.empty())
			build_neighbor_graph();
		Pnt view_point = view_ptr->get_eye();
		ne.orient_normals(view_point);
		post_redraw();
	}
}


#include <cgv/base/find_action.h>
#include <cgv/render/view.h>

bool gl_point_cloud_drawable_base::ensure_view_pointer()
{
	cgv::base::base_ptr bp(dynamic_cast<cgv::base::base*>(this));
	if (bp) {
		vector<cgv::render::view*> views;
		cgv::base::find_interface<cgv::render::view>(bp, views);
		if (!views.empty()) {
			view_ptr = views[0];
			return true;
		}
	}
	return false;
}

#ifdef REGISTER_SHADER_FILES
#include <cgv/base/register.h>
#include <point_cloud_shader_inc.h>
#endif
