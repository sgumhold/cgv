#include "simple_mesh.h"
#include "stl_reader.h"
#include "obj_loader.h"
#include <cgv/math/inv.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/media/mesh/obj_reader.h>
#include <cgv/math/bucket_sort.h>
#include <fstream>

namespace cgv {
	namespace media {
		namespace mesh {

std::string simple_mesh_base::get_attribute_name(attribute_type attr)
{
	const char* attribute_names[] = { "position", "texcoords", "normal", "tangent", "color" };
	return attribute_names[int(attr)];
}
simple_mesh_base::AttributeFlags simple_mesh_base::get_attribute_flag(attribute_type attr)
{
	AttributeFlags attribute_flags[] = { AF_position, AF_texcoords, AF_normal, AF_tangent, AF_color };
	return attribute_flags[int(attr)];
}
simple_mesh_base::simple_mesh_base() 
{
}
simple_mesh_base::simple_mesh_base(const simple_mesh_base& smb) :
	colored_model(smb),
	position_indices(smb.position_indices),
	normal_indices(smb.normal_indices),
	tex_coord_indices(smb.tex_coord_indices),
	faces(smb.faces),
	group_indices(smb.group_indices),
	group_names(smb.group_names),
	material_indices(smb.material_indices),
	materials(smb.materials)
{
}
simple_mesh_base::simple_mesh_base(simple_mesh_base&& smb) :
	colored_model(std::move(smb)),
	position_indices(std::move(smb.position_indices)),
	normal_indices(std::move(smb.normal_indices)),
	tex_coord_indices(std::move(smb.tex_coord_indices)),
	faces(std::move(smb.faces)),
	group_indices(std::move(smb.group_indices)),
	group_names(std::move(smb.group_names)),
	material_indices(std::move(smb.material_indices)),
	materials(std::move(smb.materials))
{
}
simple_mesh_base& simple_mesh_base::operator=(const simple_mesh_base& smb)
{
	colored_model::operator=(smb);
	position_indices=smb.position_indices;
	normal_indices=smb.normal_indices;
	tex_coord_indices=smb.tex_coord_indices;
	faces=smb.faces;
	group_indices=smb.group_indices;
	group_names=smb.group_names;
	material_indices=smb.material_indices;
	materials = smb.materials;
	return *this;
}
simple_mesh_base& simple_mesh_base::operator=(simple_mesh_base&& smb)
{
	colored_model::operator=(std::move(smb));
	position_indices=std::move(smb.position_indices);
	normal_indices=std::move(smb.normal_indices);
	tex_coord_indices=std::move(smb.tex_coord_indices);
	faces=std::move(smb.faces);
	group_indices=std::move(smb.group_indices);
	group_names=std::move(smb.group_names);
	material_indices=std::move(smb.material_indices);
	materials = std::move(smb.materials);
	return *this;
}
simple_mesh_base::idx_type simple_mesh_base::start_face()
{
	faces.push_back((cgv::type::uint32_type)position_indices.size());
	if (!materials.empty())
		material_indices.push_back(idx_type(materials.size()) - 1);
	if (!group_names.empty())
		group_indices.push_back(idx_type(group_names.size()) - 1);
	return idx_type(faces.size() - 1);
}
simple_mesh_base::idx_type simple_mesh_base::new_corner(idx_type position_index, idx_type normal_index,
														idx_type tex_coord_index)
{
	position_indices.push_back(position_index);
	if (normal_index != -1) //FIXME: -1 underflows unsigned int!
		normal_indices.push_back(normal_index);
	if (tex_coord_index != -1) //FIXME: -1 underflows unsigned int!
		tex_coord_indices.push_back(tex_coord_index);
	return idx_type(position_indices.size());
}
void simple_mesh_base::revert_face_orientation()
{
	bool nmls = position_indices.size() == normal_indices.size();
	bool tcs  = position_indices.size() == tex_coord_indices.size();
	for (idx_type fi = 0; fi < get_nr_faces(); ++fi) {
		idx_type ci = begin_corner(fi);
		idx_type cj = end_corner(fi);
		while (ci + 1 < cj) {
			--cj;
			std::swap(position_indices[ci], position_indices[cj]);
			if (nmls)
				std::swap(normal_indices[ci], normal_indices[cj]);
			if (tcs)
				std::swap(tex_coord_indices[ci], tex_coord_indices[cj]);
			++ci;
		}
	}
}
void simple_mesh_base::sort_faces(std::vector<idx_type>& perm, bool by_group, bool by_material) const
{
	if (by_group && by_material) {
		std::vector<idx_type> perm0;
		cgv::math::bucket_sort(group_indices, get_nr_groups(), perm0);
		cgv::math::bucket_sort(material_indices, get_nr_materials(), perm, &perm0);
	}
	else if (by_group)
		cgv::math::bucket_sort(group_indices, get_nr_groups(), perm);
	else
		cgv::math::bucket_sort(material_indices, get_nr_materials(), perm);
}
void simple_mesh_base::merge_indices(std::vector<idx_type>& indices, std::vector<idx4_type>& unique_quadruples, bool* include_tex_coords_ptr, bool* include_normals_ptr, bool* include_tangents_ptr) const
{
	bool include_tex_coords = false;
	if (include_tex_coords_ptr)
		*include_tex_coords_ptr = include_tex_coords = (tex_coord_indices.size() > 0) && *include_tex_coords_ptr;

	bool include_normals    = false;
	if (include_normals_ptr)
		*include_normals_ptr = include_normals = (normal_indices.size() > 0) && *include_normals_ptr;

	bool include_tangents   = false;
	if(include_tangents_ptr)
		*include_tangents_ptr = include_tangents = (tangent_indices.size() > 0) && *include_tangents_ptr;

	std::map<std::tuple<idx_type, idx_type, idx_type, idx_type>, idx_type> corner_to_index;
	for (idx_type ci = 0; ci < position_indices.size(); ++ci) {
		// construct corner
		idx4_type c(position_indices[ci], 
			    (include_tex_coords && ci < tex_coord_indices.size()) ? tex_coord_indices[ci] : 0, 
			    (include_normals && ci < normal_indices.size()) ? normal_indices[ci] : 0,
			    (include_tangents && ci < tangent_indices.size()) ? tangent_indices[ci] : 0);
		std::tuple<idx_type, idx_type, idx_type, idx_type> quadruple(c(0),c(1),c(2),c(3));
		// look corner up in map
		auto iter = corner_to_index.find(quadruple);
		// determine vertex index
		idx_type vi;
		if (iter == corner_to_index.end()) {
			vi = idx_type(unique_quadruples.size());
			corner_to_index[quadruple] = vi;
			unique_quadruples.push_back(c);
		}
		else
			vi = iter->second;

		indices.push_back(vi);
	}
}
void simple_mesh_base::extract_triangle_element_buffer(
	const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& triangle_element_buffer, 
	const std::vector<idx_type>* face_permutation_ptr, std::vector<idx3_type>* material_group_start_ptr) const
{
	idx_type mi = idx_type(-1);
	idx_type gi = idx_type(-1);
	// construct triangle element buffer
	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		idx_type fj = face_permutation_ptr ? face_permutation_ptr->at(fi) : fi;
		if (material_group_start_ptr) {
			if (mi != material_indices[fj] || gi != group_indices[fj]) {
				mi = material_indices[fj];
				gi = group_indices[fj];
				material_group_start_ptr->push_back(idx3_type(mi, gi, idx_type(triangle_element_buffer.size())));
			}
		}
		if (face_degree(fj) == 3) {
			for (idx_type ci = begin_corner(fj); ci < end_corner(fj); ++ci)
				triangle_element_buffer.push_back(vertex_indices.at(ci));
		}
		else {
			// in case of non triangular faces do simplest triangulation approach that assumes convexity of faces
			for (idx_type ci = begin_corner(fj) + 2; ci < end_corner(fj); ++ci) {
				triangle_element_buffer.push_back(vertex_indices.at(begin_corner(fj)));
				triangle_element_buffer.push_back(vertex_indices.at(ci - 1));
				triangle_element_buffer.push_back(vertex_indices.at(ci));
			}
		}
	}
}
void simple_mesh_base::extract_wireframe_element_buffer(const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& edge_element_buffer) const
{
	// map stores for each halfedge the number of times it has been seen before
	std::map<std::tuple<idx_type, idx_type>, idx_type> halfedge_to_count;
	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		idx_type last_vi = vertex_indices.at(end_corner(fi) - 1);
		for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			// construct halfedge with sorted vertex indices
			idx_type vi = vertex_indices.at(ci);
			std::tuple<idx_type, idx_type> halfedge(last_vi, vi);
			if (vi < last_vi)
				std::swap(std::get<0>(halfedge), std::get<1>(halfedge));

			// lookup corner in map
			auto iter = halfedge_to_count.find(halfedge);

			// determine vertex index
			if (iter == halfedge_to_count.end()) {
				halfedge_to_count[halfedge] = 1;
				edge_element_buffer.push_back(last_vi);
				edge_element_buffer.push_back(vi);
			}
			else
				++halfedge_to_count[halfedge];
			last_vi = vi;
		}
	}
}
simple_mesh_base::idx_type simple_mesh_base::extract_vertex_attribute_buffer_base(const std::vector<idx4_type>& unique_quadruples, AttributeFlags& flags, std::vector<uint8_t>& attrib_buffer) const
{
	// update flags of to be used attributes
	if (position_indices.empty() && (flags & AF_position))
		flags = AttributeFlags(flags & ~AF_position);
	if (tex_coord_indices.empty() && (flags & AF_texcoords))
		flags = AttributeFlags(flags & ~AF_texcoords);
	if (normal_indices.empty() && (flags & AF_normal))
		flags = AttributeFlags(flags & ~AF_normal);
	if (tangent_indices.empty() && (flags & AF_tangent))
		flags = AttributeFlags(flags & ~AF_tangent);
	if (!has_colors() && (flags & AF_color))
		flags = AttributeFlags(flags & ~AF_color);
	bool include_attribute[5] = { bool(flags&AF_position),bool(flags&AF_texcoords), 
		bool(flags&AF_normal),bool(flags&AF_tangent),bool(flags&AF_color) };
	// determine vertex size in bytes and allocate attribute buffer
	uint32_t cs = get_coord_size();
	uint32_t attribute_size[5] = { 3 * cs,2 * cs,3 * cs,3 * cs,uint32_t(get_color_size()) };
	uint32_t attribute_offset[5] = { 3 * cs,2 * cs,3 * cs,3 * cs,uint32_t(get_color_size() == 3 ? 4 : get_color_size()) };
	uint32_t vs = 0;
	for (int ai = 0; ai < 5; ++ai)
		if (include_attribute[ai])
			vs += attribute_offset[ai];
	attrib_buffer.resize(vs * unique_quadruples.size());
	// fill attribute buffer
	const uint8_t* attrib_ptrs[5] = {
		include_attribute[0] ? get_attribute_ptr(attribute_type::position) : nullptr,
		include_attribute[1] ? get_attribute_ptr(attribute_type::texcoords) : nullptr,
		include_attribute[2] ? get_attribute_ptr(attribute_type::normal) : nullptr,
		include_attribute[3] ? get_attribute_ptr(attribute_type::tangent) : nullptr,
		include_attribute[4] ? get_attribute_ptr(attribute_type::color) : nullptr
	};
	size_t loc = 0;
	for (auto t : unique_quadruples) {
		for (int ai = 0; ai < 5; ++ai)
			if (include_attribute[ai]) {
				const uint8_t* src_ptr = attrib_ptrs[ai] + attribute_size[ai] * t[ai & 3];
				std::copy(src_ptr, src_ptr + attribute_size[ai], &attrib_buffer[loc]);
				loc += attribute_offset[ai];
			}
	}
	return vs;
}
simple_mesh_base::idx_type simple_mesh_base::compute_inv(
	std::vector<idx_type>& inv,
	bool link_non_manifold_edges,
	std::vector<idx_type>* p2c_ptr,
	std::vector<idx_type>* next_ptr,
	std::vector<idx_type>* prev_ptr,
	std::vector<idx_type>* unmatched,
	std::vector<idx_type>* non_manifold,
	std::vector<idx_type>* unmatched_elements,
	std::vector<idx_type>* non_manifold_elements) const
{
	uint32_t fi, e = 0;
	if (p2c_ptr)
		p2c_ptr->resize(get_nr_positions());
	if (next_ptr)
		next_ptr->resize(get_nr_corners());
	if (prev_ptr)
		prev_ptr->resize(get_nr_corners());
	inv.resize(get_nr_corners(), uint32_t(-1));
	std::vector<idx_type> pis(get_nr_corners()), perm0, perm;
	// extract min position indices per corner
	//idx_type a = 0;
	//std::cout << "before sort" << std::endl;
	for (fi = 0; fi < get_nr_faces(); ++fi) {
		idx_type cb = begin_corner(fi), ce = end_corner(fi), cp = ce - 1, pp = c2p(cp);
		for (idx_type ci = cb; ci < ce; ++ci) {
			idx_type pi = c2p(ci);
			if (p2c_ptr)
				p2c_ptr->at(pi) = ci;
			if (next_ptr)
				next_ptr->at(cp) = ci;
			if (prev_ptr)
				prev_ptr->at(ci) = cp;
			pis[cp] = std::min(pp, pi);
			//if (++a < 20)
			//	std::cout << "   " << a - 1 << " : " << pp << "," << pi << std::endl;
			cp = ci;
			pp = pi;
		}
	}
	// sort corners by min position indices
	cgv::math::bucket_sort(pis, get_nr_positions(), perm0);
	//std::cout << "after min sort" << std::endl;
	//for (a=0; a < 20; ++a)
	//	std::cout << "   " << a << " : " << c2p(perm0[a]) << "," << c2p(next_ptr->at(perm0[a])) << " (" << pis[perm0[a]] << ")" << std::endl;
	// extract max position indices per corner and fill p2c, next and prev vectors
	for (fi = 0; fi < get_nr_faces(); ++fi) {
		idx_type cb = begin_corner(fi), ce = end_corner(fi), cp = ce - 1;
		for (idx_type ci = cb; ci < ce; ++ci) {
			idx_type pi = c2p(ci);
			pis[cp] = std::max(c2p(cp), pi);
			cp = ci;
		}
	}
	// sort corners by max position indices
	cgv::math::bucket_sort(pis, get_nr_positions(), perm, &perm0);
	//std::cout << "after max sort" << std::endl;
	//for (a = 0; a < 20; ++a)
	//	std::cout << "   " << a << " : " << c2p(perm[a]) << "," << c2p(next_ptr->at(perm[a])) << " (" << pis[perm[a]] << ")" << std::endl;
	// store target in pis
	for (fi = 0; fi < get_nr_faces(); ++fi) {
		idx_type cb = begin_corner(fi), ce = end_corner(fi), cp = ce - 1;
		for (idx_type ci = cb; ci < ce; ++ci) {
			pis[cp] = c2p(ci);
			cp = ci;
		}
	}
	perm0.clear();
	// finally perform matching
	idx_type i = 0;
	while (i < perm.size()) {
		idx_type ci = perm[i], pi0 = c2p(ci), pi1 = pis[ci];
		idx_type cnt = 1;
		while (i + cnt < perm.size()) {
			idx_type cj = perm[i + cnt], pj0 = c2p(cj), pj1 = pis[cj];
			if (std::min(pi0, pi1) == std::min(pj0, pj1) && std::max(pi0, pi1) == std::max(pj0, pj1)) {
				++cnt;
			}
			else
				break;
		}
		if (cnt == 1) {
			if (unmatched)
				unmatched->push_back(ci);
			if (unmatched_elements) {
				unmatched_elements->push_back(pi0);
				unmatched_elements->push_back(pi1);
			}
		}
		else if (cnt == 2) {
			inv[perm[i]] = perm[i + 1];
			inv[perm[i + 1]] = perm[i];
			++e;
		}
		else {
			if (link_non_manifold_edges) {
				idx_type cl = perm[i + cnt - 1];
				for (idx_type k = 0; k < cnt; ++k) {
					idx_type ck = perm[i + k];
					inv[cl] = ck;
					cl = ck;
				}
			}
			if (non_manifold)
				non_manifold->push_back(ci);
			if (non_manifold_elements) {
				non_manifold_elements->push_back(pi0);
				non_manifold_elements->push_back(pi1);
			}
		}
		i += cnt;
	}
	return e;
	/*
	std::map<std::pair<uint32_t, uint32_t>, uint32_t> pipj2ci;
	for (fi = 0; fi < get_nr_faces(); ++fi) {
		uint32_t prev_ci = end_corner(fi) - 1;
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			uint32_t pi = c2p(ci);
			if (p2c_ptr)
				p2c_ptr->at(pi) = ci;
			uint32_t next_ci = ci + 1 == end_corner(fi) ? begin_corner(fi) : ci + 1;
			if (next_ptr)
				next_ptr->at(ci) = next_ci;
			if (prev_ptr)
				prev_ptr->at(ci) = prev_ci;
			prev_ci = ci;
			uint32_t pj = c2p(next_ci);
			std::pair<uint32_t, uint32_t> pipj(std::min(pi, pj), std::max(pi, pj));
			if (pipj2ci.find(pipj) == pipj2ci.end())
				pipj2ci[pipj] = ci;
			else {
				uint32_t inv_ci = pipj2ci[pipj];
				inv[ci] = inv_ci;
				inv[inv_ci] = ci;
				pipj2ci.erase(pipj2ci.find(pipj));
			}
		}
	}
	*/
}
simple_mesh_base::idx_type simple_mesh_base::compute_c2e(const std::vector<uint32_t>& inv, std::vector<uint32_t>& c2e, std::vector<uint32_t>* e2c_ptr) const
{
	uint32_t e = 0;
	c2e.resize(get_nr_corners(), -1);
	if (e2c_ptr)
		e2c_ptr->resize(get_nr_corners() / 2);
	for (uint32_t fi = 0; fi < get_nr_faces(); ++fi) {
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			if (c2e[ci] == -1) {
				c2e[ci] = c2e[inv[ci]] = e;
				if (e2c_ptr)
					e2c_ptr->at(e) = ci;
				++e;
			}
		}
	}
	return e;
}
void simple_mesh_base::compute_c2f(std::vector<uint32_t>& c2f) const
{
	c2f.resize(get_nr_corners(), -1);
	for (uint32_t fi = 0; fi < get_nr_faces(); ++fi) {
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci)
			c2f[ci] = fi;
	}
}
template <typename T>
void simple_mesh<T>::construct(const obj_loader_generic<T>& loader, bool copy_grp_info, bool copy_material_info)
{
	for (unsigned vi = 0; vi < loader.vertices.size(); ++vi)
		new_position(loader.vertices[vi]);
	for (unsigned ni = 0; ni < loader.normals.size(); ++ni)
		new_normal(loader.normals[ni]);
	for (unsigned ti = 0; ti < loader.texcoords.size(); ++ti)
		new_tex_coord(loader.texcoords[ti]);
	if (copy_grp_info)
		for (unsigned gi = 0; gi < loader.groups.size(); ++gi)
			new_group(loader.groups[gi].name);
	if (copy_material_info)
		for (unsigned mi = 0; mi < loader.materials.size(); ++mi)
			ref_material(new_material()) = loader.materials[mi];	
	for (unsigned fi = 0; fi < loader.faces.size(); ++fi) {
		start_face();
		const auto& F = loader.faces[fi];
		if (copy_grp_info && !loader.groups.empty())
			group_index(fi) = F.group_index;
		if (copy_material_info && !loader.materials.empty())
			material_index(fi) = F.material_index;
		for (unsigned i = 0; i < F.degree; ++i) {
			new_corner(
				loader.vertex_indices[F.first_vertex_index + i], 
				F.first_normal_index != -1 ? loader.normal_indices[F.first_normal_index + i] : -1, 
				F.first_texcoord_index != -1 ? loader.texcoord_indices[F.first_texcoord_index + i] : -1
			);
		}
	}
}

template <typename T>
class simple_mesh_obj_reader : public obj_reader_generic<T>
{
public:
	/// type of coordinates
	typedef T coord_type;
	/// type used to store texture coordinates
	typedef cgv::math::fvec<T,2> vec2_type;
	/// type used to store positions and normal vectors
	typedef cgv::math::fvec<T,3> vec3_type;
	/// type used for rgba colors
	typedef illum::obj_material::color_type color_type;

	typedef typename simple_mesh<T>::idx_type idx_type;
protected:
	simple_mesh<T> &mesh;
public:
	simple_mesh_obj_reader(simple_mesh<T>& _mesh) : mesh(_mesh) {}
	/// overide this function to process a vertex
	void process_vertex(const vec3_type& p) { mesh.positions.push_back(p); }
	/// overide this function to process a texcoord
	void process_texcoord(const vec2_type& t) { mesh.tex_coords.push_back(vec2_type(t(0),t(1))); }
	/// overide this function to process a color (this called for vc prefixes which is is not in the standard but for example used in pobj-files)
	void process_color(const color_type& c) { mesh.resize_colors(mesh.get_nr_colors() + 1); mesh.set_color(mesh.get_nr_colors()-1, c); }
	/// overide this function to process a normal
	void process_normal(const vec3_type& n) { mesh.normals.push_back(n); }
	/// overide this function to process a face, the indices start with 0
	void process_face(unsigned vcount, int *vertices, int *texcoords, int *normals)
	{
		obj_reader_base::convert_to_positive(vcount, vertices, texcoords, normals, unsigned(mesh.positions.size()), unsigned(mesh.normals.size()), unsigned(mesh.tex_coords.size()));
		mesh.faces.push_back(idx_type(mesh.position_indices.size()));
		if (obj_reader_base::get_current_group() != -1)
			mesh.group_indices.push_back(obj_reader_base::get_current_group());
		if (obj_reader_base::get_current_material() != -1)
			mesh.material_indices.push_back(obj_reader_base::get_current_material());
		if (texcoords) {
			if (mesh.tex_coord_indices.size() < mesh.position_indices.size())
				mesh.tex_coord_indices.resize(mesh.position_indices.size(), 0);
		}
		if (normals) {
			if (mesh.normal_indices.size() < mesh.position_indices.size())
				mesh.normal_indices.resize(mesh.position_indices.size(), 0);
		}
		for (idx_type i = 0; i < vcount; ++i) {
			mesh.position_indices.push_back(idx_type(vertices[i]));
			if (texcoords)
				mesh.tex_coord_indices.push_back(idx_type(texcoords[i]));
			if (normals)
				mesh.normal_indices.push_back(idx_type(normals[i]));
		}
	}
	/// overide this function to process a group given by name and parameter string
	void process_group(const std::string& name, const std::string& parameters)
	{
		mesh.group_names.push_back(name);
	}
	/// process a material definition. If a material with a certain name is overwritten, it will receive the same index
	void process_material(const cgv::media::illum::obj_material& mtl, unsigned idx)
	{
		if (idx >= mesh.materials.size())
			mesh.materials.resize(idx+1);
		mesh.materials[idx] = mtl;
	}
};


/// copy constructor
template <typename T>
simple_mesh<T>::simple_mesh(const simple_mesh<T>& sm)
	: simple_mesh_base(sm), positions(sm.positions), normals(sm.normals), tex_coords(sm.tex_coords)
{
}

/// move constructor
template <typename T>
simple_mesh<T>::simple_mesh(simple_mesh<T>&& sm)
	: simple_mesh_base(std::move(sm)), positions(std::move(sm.positions)), normals(std::move(sm.normals)),
	  tex_coords(std::move(sm.tex_coords))
{
}

/// assignment operator
template <typename T>
simple_mesh<T>& simple_mesh<T>::operator = (const simple_mesh<T>& sm)
{
	simple_mesh_base::operator = (sm);
	positions = sm.positions;
	normals = sm.normals;
	tangents = sm.tangents;
	tex_coords = sm.tex_coords;
	return *this;
}

/// move assignment operator
template <typename T>
simple_mesh<T>& simple_mesh<T>::operator = (simple_mesh<T>&& sm)
{
	simple_mesh_base::operator = (std::move(sm));
	positions = std::move(sm.positions);
	normals = std::move(sm.normals);
	tangents = std::move(sm.tangents);
	tex_coords = std::move(sm.tex_coords);
	return *this;
}

/// clear simple mesh
template <typename T>
void simple_mesh<T>::clear() 
{
	positions.clear(); 
	normals.clear(); 
	tangents.clear();
	tex_coords.clear(); 
	position_indices.clear(); 
	tex_coord_indices.clear(); 
	normal_indices.clear(); 
	faces.clear();
	group_indices.clear();
	group_names.clear();
	material_indices.clear();
	materials.clear();
	destruct_colors();
}

template <typename T>
bool read_off(const std::string& file_name, 
	std::vector<cgv::math::fvec<T,3>>& positions, std::vector<cgv::rgba>& vertex_colors, 
	std::vector<std::vector<uint32_t>>& faces, std::vector<cgv::rgba>& face_colors)
{
	std::string content;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	std::vector<cgv::utils::line> lines;
	cgv::utils::split_to_lines(content, lines);
	if (!(lines[0] == "OFF")) {
		std::cerr << "WARNING: first line in OFF file " << file_name << " does not contain 'OFF'" << std::endl;
		return false;
	}
	unsigned real_li = 1;
	int v, f, e;
	for (unsigned li = 1; li < lines.size(); ++li) {
		if (lines[li].empty())
			continue;
		if (lines[li].begin[0] == '#')
			continue;
		++real_li;
		std::vector<cgv::utils::token> toks;
		cgv::utils::split_to_tokens(lines[li], toks, "");
		if (real_li == 2) {
			if (toks.size() != 3) {
				std::cerr << "WARNING: second line in OFF file " << file_name << " does provide 3 tokens" << std::endl;
				return false;
			}
			int I[3];
			for (int i = 0; i < 3; ++i) {
				if (!cgv::utils::is_integer(toks[i].begin, toks[i].end, I[i])) {
					std::cerr << "WARNING: token " << i << " on second line in OFF file " << file_name << " is not an integer value" << std::endl;
					return false;
				}
			}
			v = I[0]; f = I[1]; e = I[2];
			std::cout << "OFF file " << file_name << " found " << v << " vertices, " << f << " faces, and " << e << " edges." << std::endl;
			continue;
		}
		if (int(real_li) < v+3) {
			if (!(toks.size() == 3 || toks.size() == 6 || toks.size() == 7)) {
				std::cerr << "WARNING: line of vertex " << real_li - 3 << " contains " << toks.size() << " tokens instead of 3 or 7." << std::endl;
				return false;
			}
			double x[3];
			for (unsigned i = 0; i < 3; ++i) {
				if (!cgv::utils::is_double(toks[i].begin, toks[i].end, x[i])) {
					std::cerr << "WARNING: line of vertex " << real_li - 3 << " no double in XYZ component " << i << " but <" << toks[i] << ">." << std::endl;
					return false;
				}
			}
			positions.push_back(cgv::math::fvec<T, 3>(T(x[0]), T(x[1]), T(x[2])));
			if (toks.size() >= 6) {
				double c[4] = { 0,0,0,1 };
				for (unsigned i = 0; i+3 < toks.size(); ++i) {
					if (!cgv::utils::is_double(toks[i+3].begin, toks[i+3].end, c[i])) {
						std::cerr << "WARNING: line of vertex " << real_li - 3 << " no double in RGB[A] component " << i << " but <" << toks[i+3] << ">." << std::endl;
						return false;
					}
				}
				while (vertex_colors.size() + 1 < positions.size())
					vertex_colors.push_back(vertex_colors.empty() ? cgv::rgba(0.5, 0.5, 0.5, 1.0f) : vertex_colors.back());
				vertex_colors.push_back(cgv::rgba(float(c[0]), float(c[1]), float(c[2]), float(1.0-c[3])));
			}
		}
		else {
			int n;
			if (!cgv::utils::is_integer(toks[0].begin, toks[0].end, n)) {
				std::cerr << "WARNING: first token on face " << faces.size() << " is not of type integer " << std::endl;
				return false;
			}
			if (!(toks.size() == n + 1 || toks.size() == n + 4 || toks.size() == n + 5)) {
				std::cerr << "WARNING: line of face " << faces.size() << " contains " << toks.size() << " tokens instead of " << n+1 << " or " << n+5 << std::endl;
				return false;
			}
			faces.push_back({});
			auto& face = faces.back();
			for (int i = 0; i<n; ++i) {
				int pi;
				if (!cgv::utils::is_integer(toks[i+1].begin, toks[i + 1].end, pi)) {
					std::cerr << "WARNING: token " << i+1 << " on face " << faces.size()-1 << " is not of type integer " << std::endl;
					return false;
				}
				face.push_back(uint32_t(pi));
			}
			if (toks.size() >= n + 4) {
				double c[4] = { 0,0,0,1 };
				for (unsigned i = 0; i < toks.size()-n-1; ++i) {
					if (!cgv::utils::is_double(toks[i + n + 1].begin, toks[i + n + 1].end, c[i])) {
						std::cerr << "WARNING: line of vertex " << real_li - 3 << " no double in RGBA component " << i << " but <" << toks[i + n + 1] << ">." << std::endl;
						return false;
					}
				}
				while (face_colors.size()+1 < faces.size())
					face_colors.push_back(face_colors.empty() ? cgv::rgba(0.5, 0.5, 0.5, 1.0f) : face_colors.back());
				face_colors.push_back(cgv::rgba(float(c[0]), float(c[1]), float(c[2]), float(1.0 - c[3])));
			}
		}
	}
	return true;
}


/// read simple mesh from file
template <typename T>
bool simple_mesh<T>::read(const std::string& file_name)
{ 
	std::string ext = cgv::utils::to_lower(cgv::utils::file::get_extension(file_name));
	if (ext == "obj") {
		simple_mesh_obj_reader<T> reader(*this);
		return reader.read_obj(file_name);
	}
	if (ext == "stl") {
		try {
			stl_reader::StlMesh <T, unsigned> mesh(file_name);

			// copy vertices
			for (size_t vi = 0; vi < mesh.num_vrts(); ++vi)
				new_position(cgv::math::fvec<T, 3>(3, mesh.vrt_coords(vi)));

			// copy triangles and normals
			bool has_normals = mesh.raw_normals();
			for (size_t ti = 0; ti < mesh.num_tris(); ++ti) {
				if (has_normals)
					new_normal(cgv::math::fvec<T, 3>(3, mesh.tri_normal(ti)));
				start_face();
				for (size_t ci = 0; ci < 3; ++ci)
					new_corner(mesh.tri_corner_ind(ti, ci), has_normals ? (unsigned)ti : -1);
			}
			return true;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			return false;
		}
	}
	if (ext == "off") {
		ensure_colors(cgv::media::ColorType::CT_RGBA);
		auto& vertex_colors = *reinterpret_cast<std::vector<cgv::rgba>*>(ref_color_data_vector_ptr());
		std::vector<cgv::rgba> face_colors;
		std::vector<std::vector<idx_type>> faces;
		if (!read_off(file_name, ref_positions(), vertex_colors, faces, face_colors))
			return false;
		for (const auto& f : faces) {
			start_face();
			for (auto pi : f)
				new_corner(pi);
		}
		return true;
	}
	std::cerr << "unknown mesh file extension '*." << ext << "'" << std::endl;
	return false;
}

/// write simple mesh to file (currently only obj is supported)
template <typename T>
bool simple_mesh<T>::write(const std::string& file_name) const
{
	std::ofstream os(file_name);
	if (os.fail())
		return false;
	for (const auto& p : positions)
		os << "v " << p << std::endl;
	for (const auto& t : tex_coords)
		os << "vt " << t << std::endl;
	for (const auto& n : normals)
		os << "vn " << n << std::endl;

	bool nmls = position_indices.size() == normal_indices.size();
	bool tcs = position_indices.size() == tex_coord_indices.size();

	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		os << "f";
		for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			os << " " << position_indices[ci] + 1;
			if (!nmls && !tcs)
				continue;
			os << "/";
			if (tcs)
				os << tex_coord_indices[ci] + 1;
			if (nmls)
				os << "/" << normal_indices[ci] + 1;
		}
		os << "\n";
	}
	return true;
}

/// compute the axis aligned bounding box
template <typename T>
typename simple_mesh<T>::box_type simple_mesh<T>::compute_box() const
{
	box_type box;
	for (const auto& p : positions)
		box.add_point(p);
	return box;
}
/// compute vertex normals by averaging triangle normals
template <typename T>
void simple_mesh<T>::compute_vertex_normals(bool use_parallel_implementation)
{
	// clear previous normal info
	if (has_normals())
		normals.clear();
	// copy position indices to normals
	normal_indices = position_indices;
	// initialize normals to null vectors
	normals.resize(positions.size(), vec3_type(T(0)));
	if (use_parallel_implementation) {
#pragma omp parallel for
		for (int fi = 0; fi < int(get_nr_faces()); ++fi) {
			vec3_type nml;
			if (compute_face_normal(fi, nml))
				for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci)
					normal(normal_indices[ci]) += nml;
		}
#pragma omp parallel for
		for (int ni = 0; ni < int(normals.size()); ++ni)
			normals[ni].normalize();
	}
	else {
		vec3_type nml;
		for (idx_type fi = 0; fi < get_nr_faces(); ++fi)
			if (compute_face_normal(fi, nml))
				for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci)
					normal(normal_indices[ci]) += nml;
		for (auto& n : normals)
			n.normalize();
	}
}

template <typename T>
unsigned simple_mesh<T>::extract_vertex_attribute_buffer(const std::vector<idx4_type>& unique_quadruples,
														 bool include_tex_coords, bool include_normals,
														 bool include_tangents, std::vector<T>& attrib_buffer,
														 bool* include_colors_ptr, int* num_floats_in_vertex) const
{
	// correct inquiry in case data is missing
	include_tex_coords = include_tex_coords && !tex_coord_indices.empty() && !tex_coords.empty();
	include_normals = include_normals && !normal_indices.empty() && !normals.empty();
	include_tangents = include_tangents && !tangent_indices.empty() && !tangents.empty();
	bool include_colors = false;
	if (include_colors_ptr)
		*include_colors_ptr = include_colors = has_colors() && get_nr_colors() > 0 && *include_colors_ptr;

	// determine number floats per vertex
	unsigned nr_floats = 3;
	nr_floats += include_tex_coords ? 2 : 0;
	nr_floats += include_normals ? 3 : 0;
	nr_floats += include_tangents ? 3 : 0;
	unsigned color_increment = 0;
	if (include_colors) {
		color_increment = (int)ceil((float)get_color_size() / sizeof(T));
		nr_floats += color_increment;
	}

	if (num_floats_in_vertex)
		*num_floats_in_vertex = nr_floats;

	attrib_buffer.resize(nr_floats * unique_quadruples.size());
	T* data_ptr = &attrib_buffer.front();
	for (auto t : unique_quadruples) {
		*reinterpret_cast<vec3_type*>(data_ptr) = positions[t[0]];
		data_ptr += 3;
		if (include_tex_coords) {
			*reinterpret_cast<vec2_type*>(data_ptr) = tex_coords[t[1]];
			data_ptr += 2;
		}
		if (include_normals) {
			*reinterpret_cast<vec3_type*>(data_ptr) = normals[t[2]];
			data_ptr += 3;
		}
		if (include_tangents) {
			*reinterpret_cast<vec3_type*>(data_ptr) = tangents[t[3]];
			data_ptr += 3;
		}
		if (include_colors) {
			put_color(t[0], data_ptr);
			data_ptr += color_increment;
		}
	}
	return color_increment;
}

template <typename T> void simple_mesh<T>::transform(const mat3_type& linear_transformation, const vec3_type& translation)
{
	mat3_type inverse_linear_transform = inv(linear_transformation);
	transform(linear_transformation, translation, inverse_linear_transform);
}
template <typename T> void simple_mesh<T>::transform(const mat3_type& linear_transform, const vec3_type& translation, const mat3_type& inverse_linear_transform)
{
	for (auto& p : positions)
		p = linear_transform * p + translation;
	for (auto& n : normals)
		n = n * inverse_linear_transform;
	for(auto& t : tangents)
		t = t * inverse_linear_transform;
}

template <typename T> typename simple_mesh<T>::vec3_type simple_mesh<T>::compute_face_center(idx_type fi) const
{
	vec3_type ctr = vec3_type(0.0f);
	uint32_t nr = 0;
	for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
		ctr += position(c2p(ci));
		++nr;
	}
	ctr /= float(nr);
	return ctr;
}
template <typename T> bool simple_mesh<T>::compute_face_normal(idx_type fi, vec3_type& nml_out, bool normalize) const
{
	idx_type c0 = begin_corner(fi);
	idx_type ce = end_corner(fi);
	vec3_type p0 = position(position_indices[c0]);
	vec3_type dj = position(position_indices[c0 + 1]) - p0;
	vec3_type nml(0.0f);
	for (idx_type ci = c0 + 2; ci < ce; ++ci) {
		vec3_type di = position(position_indices[ci]) - p0;
		nml += cross(dj, di);
		dj = di;
	}
	if (!normalize) {
		nml_out = nml;
		return true;
	}
	T nl = nml.length();
	if (nl > T(1e-8f)) {
		nml *= T(1) / nl;
		nml_out = nml;
		return true;
	}
	return false;
}
template <typename T> void simple_mesh<T>::compute_face_normals(bool construct_normal_indices)
{
	if (construct_normal_indices)
		normal_indices.clear();
	normals.clear();
	// compute per face normals
	for (uint32_t fi = 0; fi < get_nr_faces(); ++fi) {
		vec3_type nml = vec3_type(T(1),0,0);
		compute_face_normal(fi, nml);
		uint32_t ni = new_normal(nml);
		if (construct_normal_indices) {
			for (idx_type ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
				normal_indices.push_back(ni);
			}
		}
	}
}
template <typename T> typename simple_mesh<T>::vec3_type simple_mesh<T>::compute_normal(const vec3_type& p0, const vec3_type& p1, const vec3_type& p2)
{
	return normalize(cross(p1 - p0, p2 - p0));
}
template <typename T> void simple_mesh<T>::compute_face_tangents(bool construct_tangent_indices) {
	// compute per face tangents
	if(!has_tex_coords())
		return;

	for(uint32_t fi = 0; fi < get_nr_faces(); ++fi) {
		std::vector<vec3_type> _P;
		std::vector<vec2_type> _T;
		vec3_type ctr(0.0f);
		uint32_t ci, nr = 0;
		for(ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			_P.push_back(position(c2p(ci)));
			_T.push_back(tex_coord(c2t(ci)));
			ctr += _P.back();
			++nr;
		}
		vec3_type tng(1.0f, 0.0f, 0.0f);
		// calculate tangents for faces with at least three corners
		// for more than 3 corners only use the first two edges and assume the face to be planar
		if(_P.size() > 2) {
			vec3_type edge0 = _P[1] - _P[0];
			vec3_type edge1 = _P[2] - _P[0];
			vec2_type delta_uv0 = _T[1] - _T[0];
			vec2_type delta_uv1 = _T[2] - _T[0];

			float dir_correction = (delta_uv1.x() * delta_uv0.y() - delta_uv1.y() * delta_uv0.x()) < 0.0f ? -1.0f : 1.0f;

			// when t1, t2, t3 in same position in UV space, just use default UV direction.
			if(delta_uv0.x() * delta_uv1.y() == delta_uv0.y() * delta_uv1.x()) {
				delta_uv0.x() = 0.0f;
				delta_uv0.y() = 1.0f;
				delta_uv1.x() = 1.0f;
				delta_uv1.y() = 0.0f;
			}

			tng.x() = delta_uv0.y() * edge1.x() - delta_uv1.y() * edge0.x();
			tng.y() = delta_uv0.y() * edge1.y() - delta_uv1.y() * edge0.y();
			tng.z() = delta_uv0.y() * edge1.z() - delta_uv1.y() * edge0.z();
			tng *= dir_correction;
		} else {
			std::cout << "could not compute tangent for non-triangular face" << std::endl;
		}

		tng.normalize();
		uint32_t ti = new_tangent(tng);
		if(construct_tangent_indices) {
			for(ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
				tangent_indices.push_back(ti);
			}
		}
	}
}

template <typename T> simple_mesh<T>::simple_mesh(const std::string& conway_notation)
{
	if (!conway_notation.empty())
		construct_conway_polyhedron(conway_notation);
}
template <typename T> void simple_mesh<T>::ambo()
{
	std::vector<uint32_t> c2e;
	std::vector<uint32_t> e2c;
	std::vector<uint32_t> inv;
	std::vector<uint32_t> prev;
	std::vector<uint32_t> p2c;
	compute_inv(inv, /*link_non_manifold_edges*/false, &p2c, 0, &prev);
	uint32_t e = compute_c2e(inv, c2e, &e2c);
	mesh_type new_M;
	// create one vertex per edge
	for (uint32_t ei = 0; ei < e; ++ei) {
		uint32_t pi = c2p(e2c[ei]);
		uint32_t pj = c2p(inv[e2c[ei]]);
		new_M.new_position(normalize(position(pi) + position(pj)));
	}
	// create one face for original faces
	for (uint32_t fi = 0; fi < get_nr_faces(); ++fi) {
		uint32_t new_fi = new_M.start_face();
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci)
			new_M.new_corner(c2e[ci], new_fi);
	}
	// create one face for original vertices
	for (uint32_t pi = 0; pi < get_nr_positions(); ++pi) {
		uint32_t c0 = p2c[pi];
		uint32_t ci = c0;
		uint32_t new_fi = new_M.start_face();
		do {
			new_M.new_corner(c2e[ci], new_fi);
			ci = prev[ci];
			ci = inv[ci];
		} while (ci != c0);
	}
	new_M.compute_face_normals();
	*this = new_M;
}
template <typename T> void simple_mesh<T>::truncate(T lambda)
{
	std::vector<uint32_t> inv;
	std::vector<uint32_t> prev;
	std::vector<uint32_t> p2c;
	compute_inv(inv, /*link_non_manifold_edges*/false, &p2c, 0, &prev);
	uint32_t c = get_nr_corners();
	mesh_type new_M;
	// create one vertex per corner
	for (uint32_t ci = 0; ci < c; ++ci) {
		uint32_t pi = c2p(ci);
		uint32_t pj = c2p(inv[ci]);
		new_M.new_position(normalize((1 - lambda) * position(pi) + lambda * position(pj)));
	}
	// create one face for original faces
	for (uint32_t fi = 0; fi < get_nr_faces(); ++fi) {
		uint32_t new_fi = new_M.start_face();
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			new_M.new_corner(ci, new_fi);
			new_M.new_corner(inv[ci], new_fi);
		}
	}
	// create one face for original vertices
	for (uint32_t pi = 0; pi < get_nr_positions(); ++pi) {
		uint32_t c0 = p2c[pi];
		uint32_t ci = c0;
		uint32_t new_fi = new_M.start_face();
		do {
			new_M.new_corner(ci, new_fi);
			ci = prev[ci];
			ci = inv[ci];
		} while (ci != c0);
	}
	new_M.compute_face_normals();
	*this = new_M;
}
template <typename T> void simple_mesh<T>::snub(T lambda)
{
	std::vector<uint32_t> inv;
	std::vector<uint32_t> prev;
	std::vector<uint32_t> p2c;
	compute_inv(inv, /*link_non_manifold_edges*/false, &p2c, 0, &prev);
	uint32_t c = get_nr_corners();
	mesh_type new_M;
	// create one vertex per corner
	for (uint32_t ci = 0; ci < c; ++ci) {
		uint32_t pi = c2p(ci);
		uint32_t pj = c2p(inv[ci]);
		new_M.new_position(normalize((1 - lambda) * position(pi) + lambda * position(pj)));
	}
	// create central face for original faces
	uint32_t fi;
	for (fi = 0; fi < get_nr_faces(); ++fi) {
		uint32_t new_fi = new_M.start_face();
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci)
			new_M.new_corner(ci, new_fi);
	}
	// create one per corner inside original faces
	for (fi = 0; fi < get_nr_faces(); ++fi) {
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			uint32_t new_fi = new_M.start_face();
			uint32_t prev_ci = prev[ci];
			new_M.new_corner(prev_ci, new_fi);
			new_M.new_corner(inv[prev_ci], new_fi);
			new_M.new_corner(ci, new_fi);
		}
	}
	// create one face for original vertices
	for (uint32_t pi = 0; pi < get_nr_positions(); ++pi) {
		uint32_t c0 = p2c[pi];
		uint32_t ci = c0;
		uint32_t new_fi = new_M.start_face();
		do {
			new_M.new_corner(ci, new_fi);
			ci = prev[ci];
			ci = inv[ci];
		} while (ci != c0);
	}
	new_M.compute_face_normals();
	*this = new_M;
}
template <typename T> void simple_mesh<T>::dual()
{
	std::vector<uint32_t> c2f;
	std::vector<uint32_t> p2c;
	std::vector<uint32_t> inv;
	std::vector<uint32_t> prev;
	compute_inv(inv, /*link_non_manifold_edges*/false, &p2c, 0, &prev);
	compute_c2f(c2f);
	uint32_t f = get_nr_faces();
	mesh_type new_M;
	// create one vertex per face
	for (uint32_t fi = 0; fi < f; ++fi) {
		vec3_type ctr = vec3_type(0.0f);
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci)
			ctr += position(c2p(ci));
		ctr.normalize();
		new_M.new_position(ctr);
	}
	// create one face for original vertices
	for (uint32_t pi = 0; pi < get_nr_positions(); ++pi) {
		uint32_t c0 = p2c[pi];
		uint32_t ci = c0;
		uint32_t new_fi = new_M.start_face();
		do {
			new_M.new_corner(c2f[ci], new_fi);
			ci = prev[ci];
			ci = inv[ci];
		} while (ci != c0);
	}
	new_M.compute_face_normals();
	*this = new_M;
}
template <typename T> void simple_mesh<T>::gyro(T lambda)
{
	std::vector<uint32_t> c2f;
	std::vector<uint32_t> p2c;
	std::vector<uint32_t> inv;
	std::vector<uint32_t> prev;
	compute_inv(inv, /*link_non_manifold_edges*/false, &p2c, 0, &prev);
	compute_c2f(c2f);
	uint32_t v = get_nr_positions();
	uint32_t f = get_nr_faces();
	uint32_t c = get_nr_corners();
	mesh_type new_M;
	// copy vertices
	for (uint32_t pi = 0; pi < v; ++pi)
		new_M.new_position(position(pi));
	// create one vertex per face
	for (uint32_t fi = 0; fi < f; ++fi) {
		vec3_type ctr = vec3_type(0.0f);
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci)
			ctr += position(c2p(ci));
		ctr.normalize();
		new_M.new_position(ctr);
	}
	// create one vertex per corner
	uint32_t ci;
	for (ci = 0; ci < c; ++ci) {
		uint32_t pi = c2p(ci);
		uint32_t pj = c2p(inv[ci]);
		new_M.new_position(normalize((1 - lambda) * position(pi) + lambda * position(pj)));
	}
	// create one face for per original corner
	for (ci = 0; ci < c; ++ci) {
		uint32_t inv_ci = inv[ci];
		uint32_t prev_inv_ci = inv[prev[ci]];
		uint32_t fi = c2f[ci];
		uint32_t pi = c2p(ci);
		uint32_t new_fi = new_M.start_face();
		new_M.new_corner(pi, new_fi);
		new_M.new_corner(ci + v + f, new_fi);
		new_M.new_corner(inv_ci + v + f, new_fi);
		new_M.new_corner(v + fi, new_fi);
		new_M.new_corner(prev_inv_ci + v + f, new_fi);
	}
	new_M.compute_face_normals();
	*this = new_M;
}
template <typename T> void simple_mesh<T>::join()
{
	std::vector<uint32_t> c2e;
	std::vector<uint32_t> e2c;
	std::vector<uint32_t> inv;
	std::vector<uint32_t> prev;
	std::vector<uint32_t> p2c;
	compute_inv(inv, /*link_non_manifold_edges*/false, &p2c, 0, &prev);
	uint32_t e = compute_c2e(inv, c2e, &e2c);
	std::vector<uint32_t> c2f;
	compute_c2f(c2f);

	uint32_t f = get_nr_faces();
	uint32_t v = get_nr_positions();
	mesh_type new_M;
	// copy vertices
	for (uint32_t pi = 0; pi < v; ++pi)
		new_M.new_position(position(pi));

	// append one vertex per face
	for (uint32_t fi = 0; fi < f; ++fi) {
		vec3_type ctr = vec3_type(0.0f);
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci)
			ctr += position(c2p(ci));
		ctr.normalize();
		new_M.new_position(ctr);
	}

	// create one face per edge
	for (uint32_t ei = 0; ei < e; ++ei) {
		uint32_t ci = e2c[ei];
		uint32_t fi = c2f[ci];
		uint32_t pi = c2p(ci);
		uint32_t cj = inv[ci];
		uint32_t fj = c2f[cj];
		uint32_t pj = c2p(cj);
		new_M.start_face();
		new_M.new_corner(fi + v, ei);
		new_M.new_corner(pi, ei);
		new_M.new_corner(fj + v, ei);
		new_M.new_corner(pj, ei);
	}
	new_M.compute_face_normals();
	*this = new_M;
}
template <typename T> void simple_mesh<T>::ortho()
{
	join();
	join();
}
template <typename T> void simple_mesh<T>::construct_conway_polyhedron(const std::string& conway_notation)
{
	if (conway_notation.back() == 'C' || conway_notation.back() == 'O') {
		// cube
		static float V[8 * 3] = { -1,-1,+1, +1,-1,+1, -1,+1,+1, +1,+1,+1, -1,-1,-1, +1,-1,-1, -1,+1,-1, +1,+1,-1 };
		static float N[6 * 3] = { -1,0,0, +1,0,0, 0,-1,0, 0,+1,0, 0,0,-1, 0,0,+1 };
		static int F[6 * 4] = { 0,2,6,4, 1,5,7,3, 0,4,5,1, 2,3,7,6, 4,6,7,5, 0,1,3,2 };
		for (int vi = 0; vi < 8; ++vi)
			new_position(normalize(vec3_type(3, &V[3 * vi])));
		for (int ni = 0; ni < 6; ++ni)
			new_normal(vec3_type(3, &N[3 * ni]));
		for (int fi = 0; fi < 6; ++fi) {
			start_face();
			for (int ci = 0; ci < 4; ++ci)
				new_corner(F[4 * fi + ci], fi);
		}
		if (conway_notation.back() == 'O')
			dual();
	}
	if (conway_notation.back() == 'T' || conway_notation.back() == 'I' || conway_notation.back() == 'D') {
		static const float a = float(1.0 / (2 * sqrt(3.0)));
		static const float b = float(1.0 / (3 * sqrt(3.0 / 2)));
		static const float V[4 * 3] = { -0.5f, -a, -b,  0.5f, -a, -b,  0,2 * a, -b,  0,  0,2 * b };
		static const int F[4 * 3] = { 0,2,1,3,2,0,3,0,1,3,1,2 };
		for (int vi = 0; vi < 4; ++vi)
			new_position(normalize(vec3_type(3, &V[3 * vi])));
		for (int fi = 0; fi < 4; ++fi) {
			start_face();
			new_normal(compute_normal(position(F[3 * fi]), position(F[3 * fi + 1]), position(F[3 * fi + 2])));
			for (int ci = 0; ci < 3; ++ci)
				new_corner(F[3 * fi + ci], fi);
		}
		if (conway_notation.back() != 'T') {
			snub();
			dual();
			if (conway_notation.back() == 'I')
				dual();
		}
	}
	for (int ci = (int)conway_notation.size() - 1; ci >= 0; --ci) {
		switch (conway_notation[ci]) {
		case 't': truncate(); break;
		case 'a': ambo(); break;
		case 'd': dual(); break;
		case 'j': join(); break;
		case 'o': ortho(); break;
		case 's': snub(); break;
		case 'g': gyro(); break;
		}
	}
}

template class simple_mesh<float>;
template class simple_mesh<double>;

		}
	}
}
