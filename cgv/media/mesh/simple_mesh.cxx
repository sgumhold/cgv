#include "simple_mesh.h"
#include <cgv/math/inv.h>
#include <cgv/media/mesh/obj_reader.h>
#include <cgv/math/bucket_sort.h>
#include <fstream>

namespace cgv {
	namespace media {
		namespace mesh {

/// default constructor
simple_mesh_base::simple_mesh_base() 
{

}
/// copy constructor
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
/// assignment operator
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


/// create a new empty face to which new corners are added and return face index
simple_mesh_base::idx_type simple_mesh_base::start_face()
{
	faces.push_back((cgv::type::uint32_type)position_indices.size());
	if (!materials.empty())
		material_indices.push_back(idx_type(materials.size()) - 1);
	if (!group_names.empty())
		group_indices.push_back(idx_type(group_names.size()) - 1);
	return idx_type(faces.size() - 1);
}
/// create a new corner from position, optional normal and optional tex coordinate indices and return corner index
simple_mesh_base::idx_type simple_mesh_base::new_corner(idx_type position_index, idx_type normal_index, idx_type tex_coord_index)
{
	position_indices.push_back(position_index);
	if (normal_index != -1)
		normal_indices.push_back(normal_index);
	if (tex_coord_index != -1)
		tex_coord_indices.push_back(tex_coord_index);
	return idx_type(position_indices.size());
}

/// revert face orientation
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

/// sort faces by group and material indices with two bucket sorts
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

/// merge the three indices into one index into a vector of unique index triples
void simple_mesh_base::merge_indices(std::vector<idx_type>& indices, std::vector<vec3i>& unique_triples, bool* include_tex_coords_ptr, bool* include_normals_ptr) const
{
	bool include_tex_coords = false;
	if (include_tex_coords_ptr)
		*include_tex_coords_ptr = include_tex_coords = (tex_coord_indices.size() > 0) && *include_tex_coords_ptr;

	bool include_normals    = false;
	if (include_normals_ptr)
		*include_normals_ptr = include_normals = (normal_indices.size() > 0) && *include_normals_ptr;

	std::map<std::tuple<idx_type, idx_type, idx_type>, idx_type> corner_to_index;
	for (idx_type ci = 0; ci < position_indices.size(); ++ci) {
		// construct corner
		vec3i c(position_indices[ci], 
			    (include_tex_coords && ci < tex_coord_indices.size()) ? tex_coord_indices[ci] : 0, 
			    (include_normals && ci < normal_indices.size()) ? normal_indices[ci] : 0);
		std::tuple<idx_type, idx_type, idx_type> triple(c(0),c(1),c(2));
		// look corner up in map
		auto iter = corner_to_index.find(triple);
		// determine vertex index
		idx_type vi;
		if (iter == corner_to_index.end()) {
			vi = idx_type(unique_triples.size());
			corner_to_index[triple] = vi;
			unique_triples.push_back(c);
		}
		else
			vi = iter->second;

		indices.push_back(vi);
	}
}

/// extract element array buffers for triangulation
void simple_mesh_base::extract_triangle_element_buffer(
	const std::vector<idx_type>& vertex_indices, std::vector<idx_type>& triangle_element_buffer, 
	const std::vector<idx_type>* face_perm_ptr, std::vector<vec3i>* material_group_start_ptr) const
{
	idx_type mi = idx_type(-1);
	idx_type gi = idx_type(-1);
	// construct triangle element buffer
	for (idx_type fi = 0; fi < faces.size(); ++fi) {
		idx_type fj = face_perm_ptr ? face_perm_ptr->at(fi) : fi;
		if (material_group_start_ptr) {
			if (mi != material_indices[fj] || gi != group_indices[fj]) {
				mi = material_indices[fj];
				gi = group_indices[fj];
				material_group_start_ptr->push_back(vec3i(mi, gi, idx_type(triangle_element_buffer.size())));
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

/// extract element array buffers for edges in wireframe
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

/// compute a index vector storing the inv corners per corner and optionally index vectors with per position corner index, per corner next and or prev corner index (implementation assumes closed manifold connectivity)
void simple_mesh_base::compute_inv(std::vector<uint32_t>& inv, std::vector<uint32_t>* p2c_ptr, std::vector<uint32_t>* next_ptr, std::vector<uint32_t>* prev_ptr)
{
	uint32_t fi, e = 0;
	if (p2c_ptr)
		p2c_ptr->resize(get_nr_positions());
	if (next_ptr)
		next_ptr->resize(get_nr_corners());
	if (prev_ptr)
		prev_ptr->resize(get_nr_corners());
	inv.resize(get_nr_corners());
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
}
/// given the inv corners compute index vector per corner its edge index and optionally per edge its corner index (implementation assumes closed manifold connectivity)
uint32_t simple_mesh_base::compute_c2e(const std::vector<uint32_t>& inv, std::vector<uint32_t>& c2e, std::vector<uint32_t>* e2c_ptr)
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
/// compute index vector with per corner its face index
void simple_mesh_base::compute_c2f(std::vector<uint32_t>& c2f)
{
	c2f.resize(get_nr_corners(), -1);
	for (uint32_t fi = 0; fi < get_nr_faces(); ++fi) {
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci)
			c2f[ci] = fi;
	}
}

template <typename T>
class simple_mesh_obj_reader : public obj_reader_generic<T>
{
public:
	/// type of coordinates
	typedef T crd_type;
	/// type used to store texture coordinates
	typedef cgv::math::fvec<T,2> v2d_type;
	/// type used to store positions and normal vectors
	typedef cgv::math::fvec<T,3> v3d_type;
	/// type used for rgba colors
	typedef illum::obj_material::color_type color_type;

	typedef typename simple_mesh<T>::idx_type idx_type;
protected:
	simple_mesh<T> &mesh;
public:
	simple_mesh_obj_reader(simple_mesh<T>& _mesh) : mesh(_mesh) {}
	/// overide this function to process a vertex
	void process_vertex(const v3d_type& p) { mesh.positions.push_back(p); }
	/// overide this function to process a texcoord
	void process_texcoord(const v2d_type& t) { mesh.tex_coords.push_back(v2d_type(t(0),t(1))); }
	/// overide this function to process a color (this called for vc prefixes which is is not in the standard but for example used in pobj-files)
	void process_color(const color_type& c) { mesh.resize_colors(mesh.get_nr_colors() + 1); mesh.set_color(mesh.get_nr_colors()-1, c); }
	/// overide this function to process a normal
	void process_normal(const v3d_type& n) { mesh.normals.push_back(n); }
	/// overide this function to process a face, the indices start with 0
	void process_face(unsigned vcount, int *vertices, int *texcoords, int *normals)
	{
		convert_to_positive(vcount, vertices, texcoords, normals, unsigned(mesh.positions.size()), unsigned(mesh.normals.size()), unsigned(mesh.tex_coords.size()));
		mesh.faces.push_back(idx_type(mesh.position_indices.size()));
		if (get_current_group() != -1)
			mesh.group_indices.push_back(get_current_group());
		if (get_current_material() != -1)
			mesh.material_indices.push_back(get_current_material());
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


/// construct from string corresponding to Conway notation (defaults to empty mesh)
template <typename T>
simple_mesh<T>::simple_mesh(const simple_mesh<T>& sm) : simple_mesh_base(sm), positions(sm.positions), normals(sm.normals), tex_coords(sm.tex_coords)
{
}

/// construct from string corresponding to Conway notation (defaults to empty mesh)
template <typename T>
simple_mesh<T>& simple_mesh<T>::operator = (const simple_mesh<T>& sm)
{
	simple_mesh_base::operator = (sm);
	positions = sm.positions;
	normals = sm.normals;
	tex_coords = sm.tex_coords;
	return *this;
}

/// clear simple mesh
template <typename T>
void simple_mesh<T>::clear() 
{
	positions.clear(); 
	normals.clear(); 
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

/// read simple mesh from file
template <typename T>
bool simple_mesh<T>::read(const std::string& file_name)
{ 
	simple_mesh_obj_reader<T> reader(*this);
	return reader.read_obj(file_name);
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
void simple_mesh<T>::compute_vertex_normals()
{
	// clear previous normal info
	if (has_normals())
		normals.clear();
	// copy position indices to normals
	normal_indices = position_indices;
	// initialize normals to null vectors
	normals.resize(positions.size(), vec3(0.0f));
	// iterate faces
	for (idx_type fi = 0; fi < get_nr_faces(); ++fi) {
		idx_type c0 = begin_corner(fi);
		idx_type ce = end_corner(fi);
		vec3 p0 = position(position_indices[c0]);
		vec3 dj = position(position_indices[c0 + 1]) - p0;
		vec3 nml(0.0f);
		for (idx_type ci = c0+2; ci < ce; ++ci) {
			vec3 di = position(position_indices[ci]) - p0;
			nml += cross(dj, di);
			dj = di;
		}
		T nl = nml.length();
		if (nl > 1e-8f) {
			nml *= 1.0f/nl;
			for (idx_type ci = c0; ci < ce; ++ci)
				normal(normal_indices[ci]) += nml;
		}
	}
	for (auto& n : normals)
		n.normalize();
}

/// extract vertex attribute array and element array buffers for triangulation and edges in wireframe
template <typename T>
unsigned simple_mesh<T>::extract_vertex_attribute_buffer(
	const std::vector<idx_type>& vertex_indices, 
	const std::vector<vec3i>& unique_triples, 
	bool include_tex_coords, bool include_normals, 
	std::vector<T>& attrib_buffer, bool* include_colors_ptr) const
{
	// correct inquiry in case data is missing
	include_tex_coords = include_tex_coords && !tex_coord_indices.empty() && !tex_coords.empty();
	include_normals = include_normals && !normal_indices.empty() && !normals.empty();
	bool include_colors = false;
	if (include_colors_ptr)
		*include_colors_ptr = include_colors = has_colors() && *include_colors_ptr;

	// determine number floats per vertex
	unsigned nr_floats = 3;
	nr_floats += include_tex_coords ? 2 : 0;
	nr_floats += include_normals ? 3 : 0;
	unsigned color_increment = 0;
	if (include_colors) {
		color_increment = (int)ceil((float)get_color_size() / sizeof(T));
		nr_floats += color_increment;
	}

	attrib_buffer.resize(nr_floats*unique_triples.size());
	T* data_ptr = &attrib_buffer.front();
	for (auto t : unique_triples) {
		*reinterpret_cast<vec3*>(data_ptr) = positions[t[0]];
		data_ptr += 3;
		if (include_tex_coords) {
			*reinterpret_cast<vec2*>(data_ptr) = tex_coords[t[1]];
			data_ptr += 2;
		}
		if (include_normals) {
			*reinterpret_cast<vec3*>(data_ptr) = normals[t[2]];
			data_ptr += 3;
		}
		if (include_colors) {
			put_color(t[0], data_ptr);
			data_ptr += color_increment;
		}
	}
	return color_increment;
}

/// apply transformation to mesh
template <typename T>
void simple_mesh<T>::transform(const mat3& linear_transformation, const vec3& translation)
{
	mat3 inverse_linear_transform = inv(linear_transformation);
	transform(linear_transformation, translation, inverse_linear_transform);
}

/// apply transformation to mesh with given inverse linear transformation
template <typename T>
void simple_mesh<T>::transform(const mat3& linear_transform, const vec3& translation, const mat3& inverse_linear_transform)
{
	for (auto& p : positions)
		p = linear_transform * p + translation;
	for (auto& n : normals)
		n = n * inverse_linear_transform;
}

/// construct from string corresponding to conway notation (defaults to empty mesh)
template <typename T>
simple_mesh<T>::simple_mesh(const std::string& conway_notation)
{
	if (!conway_notation.empty())
		construct_conway_polyhedron(conway_notation);
}

template <typename T> void simple_mesh<T>::compute_face_normals()
{
	// compute per face normals
	for (uint32_t fi = 0; fi < get_nr_faces(); ++fi) {
		std::vector<vec3> P;
		vec3 ctr = vec3(0.0f);
		uint32_t nr = 0;
		for (uint32_t ci = begin_corner(fi); ci < end_corner(fi); ++ci) {
			P.push_back(position(c2p(ci)));
			ctr += P.back();
			++nr;
		}
		ctr /= float(nr);
		vec3 prev_p = P.back() - ctr;
		vec3 nml = vec3(0.0f);
		for (uint32_t i = 0; i < P.size(); ++i) {
			vec3 p = P[i] - ctr;
			nml += cross(prev_p, p);
			prev_p = p;
		}
		nml.normalize();
		new_normal(nml);
	}
}

template <typename T> void simple_mesh<T>::ambo()
{
	std::vector<uint32_t> c2e;
	std::vector<uint32_t> e2c;
	std::vector<uint32_t> inv;
	std::vector<uint32_t> prev;
	std::vector<uint32_t> p2c;
	compute_inv(inv, &p2c, 0, &prev);
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
	compute_inv(inv, &p2c, 0, &prev);
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
	compute_inv(inv, &p2c, 0, &prev);
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
	compute_inv(inv, &p2c, 0, &prev);
	compute_c2f(c2f);
	uint32_t f = get_nr_faces();
	mesh_type new_M;
	// create one vertex per face
	for (uint32_t fi = 0; fi < f; ++fi) {
		vec3 ctr = vec3(0.0f);
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
	compute_inv(inv, &p2c, 0, &prev);
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
		vec3 ctr = vec3(0.0f);
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

template <typename T> typename simple_mesh<T>::vec3 simple_mesh<T>::compute_normal(const vec3& p0, const vec3& p1, const vec3& p2)
{
	return normalize(cross(p1 - p0, p2 - p0));
}

template <typename T> void simple_mesh<T>::join()
{
	std::vector<uint32_t> c2e;
	std::vector<uint32_t> e2c;
	std::vector<uint32_t> inv;
	std::vector<uint32_t> prev;
	std::vector<uint32_t> p2c;
	compute_inv(inv, &p2c, 0, &prev);
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
		vec3 ctr = vec3(0.0f);
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
			new_position(normalize(vec3(3, &V[3 * vi])));
		for (int ni = 0; ni < 8; ++ni)
			new_normal(vec3(3, &N[3 * ni]));
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
			new_position(normalize(vec3(3, &V[3 * vi])));
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
