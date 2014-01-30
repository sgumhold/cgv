#include "gl_mesh_drawable_base.h"

#include <cgv/base/attach_slot.h>
#include <cgv/base/find_action.h>

#include <cgv/gui/file_dialog.h>

#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>

#include <cgv_gl/gl/gl.h>
#include <cgv/render/view.h>
#include <cgv/utils/file.h>
#include <cgv/base/import.h>

using namespace cgv::base;
using namespace cgv::gui; 
using namespace cgv::utils::file;
using namespace cgv::media::illum;

#ifndef _MSC_VER
#define __stdcall
#endif

namespace cgv {
	namespace render {
		namespace gl {

struct vertex_data
{
	cgv::math::fvec<double,3> v;
	cgv::math::fvec<double,3> n;
	cgv::math::fvec<double,2> t;
	cgv::media::mesh::obj_loader::color_type c;
	bool has_tc;
	bool has_nml;
	bool has_clr;
};

void __stdcall vertexCallback(GLvoid *vertex)
{
	vertex_data* ptr = (vertex_data*) vertex;
	if (ptr->has_nml)
		glNormal3dv(ptr->n);
	if (ptr->has_tc)
		glTexCoord2dv(ptr->t);
	if (ptr->has_clr)
		glColor3fv(&ptr->c[0]);
	glVertex3dv(ptr->v);
}

typedef void (__stdcall *fn_ptr)();

unsigned render_faces_with_glu(const cgv::media::mesh::obj_loader& loader, const std::vector<unsigned>& fis, unsigned min_d)
{
	GLUtesselator* tobj = gluNewTess();
	if (!tobj)
		return 0;

	gluTessCallback(tobj, GLU_BEGIN, (fn_ptr)glBegin);
    gluTessCallback(tobj, GLU_VERTEX, (fn_ptr)vertexCallback);
    gluTessCallback(tobj, GLU_END, glEnd);

	unsigned nr = 0;
	vertex_data v;
	std::vector<vertex_data> V;
	for (unsigned i=0; i<fis.size(); ++i) {
		const cgv::media::mesh::face_info& fi = loader.faces[fis[i]];
		unsigned d  = fi.degree;
		if (d < min_d)
			continue;
		V.resize(d);
		gluTessBeginPolygon(tobj, NULL);
		gluTessBeginContour(tobj);
		++nr;
		// in case no normals are specified, render a face normal
		v.has_nml = false;
		if (fi.first_normal_index == -1) {
			unsigned hi=fi.first_vertex_index + d-1;
			cgv::media::mesh::obj_loader::v3d_type n(0,0,0);
			for (unsigned j=0; j<d; ++j) {
				unsigned hj = fi.first_vertex_index + j;
				n += 
					cross(loader.vertices[loader.vertex_indices[hi]],
						  loader.vertices[loader.vertex_indices[hj]]);
				hi = hj;
			}
			n.normalize();
			v.n = n;
			v.has_nml = true;
		}
		for (unsigned j=0; j<d; ++j) {
			v.has_tc = false;
			if (fi.first_texcoord_index != -1) {
				unsigned ht = j+fi.first_texcoord_index;
				v.t = loader.texcoords[loader.texcoord_indices[ht]];
				v.has_tc = true;
			}
			if (fi.first_normal_index != -1) {
				unsigned hn = j+fi.first_normal_index;
				v.n = loader.normals[loader.normal_indices[hn]];
				v.has_nml = true;
			}
			unsigned hv = j+fi.first_vertex_index;
			v.has_clr = loader.colors.size() == loader.vertices.size(); 
			if (v.has_clr)
				v.c = loader.colors[loader.vertex_indices[hv]];
			v.v = loader.vertices[loader.vertex_indices[hv]];
			V[j] = v;
			gluTessVertex(tobj, V[j].v, &V[j]);
			v.has_nml = false;
		}
		gluTessEndContour(tobj);
		gluTessEndPolygon(tobj);
		V.clear();
	}
	gluDeleteTess(tobj);
	return nr;
}


unsigned render_faces(const cgv::media::mesh::obj_loader& loader, const std::vector<unsigned>& fis, 
				  GLenum type, unsigned min_d)
{
	bool first = true;
	unsigned nr = 0;
	for (unsigned i=0; i<fis.size(); ++i) {
		const cgv::media::mesh::face_info& fi = loader.faces[fis[i]];
		unsigned d  = fi.degree;
		if (d < min_d || (min_d < 5 && d > min_d))
			continue;
		if (first)
			glBegin(type);
		++nr;
		// in case no normals are specified, render a face normal
		if (fi.first_normal_index == -1) {
			unsigned hi=fi.first_vertex_index + d-1;
			cgv::media::mesh::obj_loader::v3d_type n(0,0,0);
			for (unsigned j=0; j<d; ++j) {
				unsigned hj = fi.first_vertex_index + j;
				n += 
					cross(loader.vertices[loader.vertex_indices[hi]],
						  loader.vertices[loader.vertex_indices[hj]]);
				hi = hj;
			}
			n.normalize();
			glNormal3dv(n);
		}
		for (unsigned j=0; j<d; ++j) {
			if (fi.first_texcoord_index != -1) {
				unsigned ht = j+fi.first_texcoord_index;
				glTexCoord2dv(loader.texcoords[loader.texcoord_indices[ht]]);
			}
			if (fi.first_normal_index != -1) {
				unsigned hn = j+fi.first_normal_index;
				glNormal3dv(loader.normals[loader.normal_indices[hn]]);
			}
			unsigned hv = j+fi.first_vertex_index;
			if (loader.colors.size() == loader.vertices.size())
				glColor4fv(&loader.colors[loader.vertex_indices[hv]][0]);
			glVertex3dv(loader.vertices[loader.vertex_indices[hv]]);
		}
		if (min_d > 4)
			glEnd();
		else
			first = false;
	}
	if (!first && min_d < 5)
		glEnd();
	return nr;
}

/// construct from mtl info
material_info::material_info(const cgv::media::illum::obj_material& i) : 
	cgv::render::textured_material(i)
{
}

/// standard constructor
mesh_group_info::mesh_group_info(const group_info& g) : group_info(g), number_faces(0) 
{
}

/// construct from name which is necessary construction argument to node
gl_mesh_drawable_base::gl_mesh_drawable_base()
{
}

/// init textures
void gl_mesh_drawable_base::init_frame(context &ctx)
{
	for (unsigned i=0; i<materials.size(); ++i)
		materials[i].ensure_textures(ctx);
}

void gl_mesh_drawable_base::draw_mesh_group(context &ctx, unsigned gi, bool use_materials)
{
	for (unsigned i=0; i<materials.size(); ++i) {
		material_info& m = materials[i];

		if (use_materials)
			ctx.enable_material(m, MS_FRONT_AND_BACK);

		for (unsigned j=0; j<m.group_list_ids.size(); ++j) {
			unsigned gj = m.group_list_ids[j].first;
			if (gj != gi)
				continue;
			unsigned dl = m.group_list_ids[j].second;
			if (dl != -1)
				glCallList(dl);
		}
	
		if (use_materials)
			ctx.disable_material(m);
	}
}

void gl_mesh_drawable_base::draw_mesh(context &ctx, bool use_materials)
{
	for (unsigned i=0; i<materials.size(); ++i) {
		material_info& m = materials[i];

		if (use_materials)
			ctx.enable_material(m, MS_FRONT_AND_BACK);

		for (unsigned j=0; j<m.group_list_ids.size(); ++j) {
			unsigned gi = m.group_list_ids[j].first;
			unsigned dl = m.group_list_ids[j].second;
			if (dl != -1) {
				glCallList(dl);
			}
		}
	
		if (use_materials)
			ctx.disable_material(m);
	}
}

void gl_mesh_drawable_base::draw(context &ctx)
{
	draw_mesh(ctx);
}

/// load obj
bool gl_mesh_drawable_base::read_mesh(const std::string& _file_name)
{
	std::string fn = cgv::base::find_data_file(_file_name, "cpMD", "", model_path);
	if (fn.empty()) {
		std::cerr << "mesh file " << file_name << " not found" << std::endl;
		return false;
	}
	file_name = _file_name;

	cgv::base::push_file_parent(fn);

	loader.clear();
	std::string material_file = drop_extension(fn)+".mtl";
	if (exists(material_file)) {
		std::cout << "read mtl: " << material_file; std::cout.flush();
		loader.read_mtl(material_file);
		std::cout << " ... Done" << std::endl;
	}
	printf("read obj %s ", file_name.c_str());
	loader.read_obj(fn);
	printf("...Done.\n");

	cgv::base::pop_file_parent();

	unsigned vi, mi, gi, M = loader.materials.size(), G = loader.groups.size(), V = loader.vertices.size();
	// copy materials
	materials.clear();
	
	// copy groups
	for (gi=0; gi<G; ++gi)
		groups.push_back(mesh_group_info(loader.groups[gi]));

	if (get_context())
		get_context()->make_current();

	// compute box
	box.invalidate();
	for (vi=0; vi<V; ++vi)
		box.add_point(loader.vertices[vi]);

	// iterate materials
	for (mi=0; mi<M; ++mi) {
		std::vector<std::vector<unsigned> > grp_faces;
		grp_faces.resize(G);
		bool found_face = false;
		for (unsigned fi=0; fi<loader.faces.size(); ++fi) {
			// extract material faces sorted by group
			if (loader.faces[fi].material_index == mi) {
				gi = loader.faces[fi].group_index;
				grp_faces[gi].push_back(fi);
				found_face = true;
			}
		}
		if (!found_face)
			continue;

		materials.push_back(material_info(loader.materials[mi]));
		// compile display lists
		for (gi=0; gi<G; ++gi) {
			if (grp_faces[gi].size() == 0)
				continue;
			GLuint dl = glGenLists(1);
			materials.back().group_list_ids.push_back(
				std::pair<unsigned,unsigned>(gi,dl));
			glNewList(dl,GL_COMPILE);
			groups[gi].number_faces  = render_faces(loader, grp_faces[gi], GL_TRIANGLES, 3);
			groups[gi].number_faces += render_faces_with_glu(loader, grp_faces[gi], 4);
			glEndList();
		}
	}
	return true;
}

void gl_mesh_drawable_base::center_view()
{
	std::vector<cgv::render::view*> views;
	find_interface<cgv::render::view>(base_ptr(dynamic_cast<cgv::base::base*>(this)),views);
	for (unsigned i=0; i<views.size(); ++i) {
		views[i]->set_focus(box.get_center());
		views[i]->set_y_extent_at_focus(1.3*box.get_extent()(box.get_max_extent_coord_index()));
	}
	if (views.size() > 0)
		post_redraw();
}
/// return the axis aligned bounding box
const gl_mesh_drawable_base::box_type& gl_mesh_drawable_base::get_box() const
{
	return box;
}


		}
	}
}

