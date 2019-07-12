
#include "surface_reconstructor.h"

int surface_reconstructor::find_corner(unsigned int vi, unsigned int vj, unsigned int vk) const
{
	neighbor_graph& NG = *ng;

	int j = NG.find(vi,vj);
	if (j == -1)
		return -1;
	int k = NG.find(vi,vk);
	if (k == -1)
		return -1;
	int n = (int)NG[vi].size();
	if (k == (j+1) % n)
		return j;
	if (use_orientation)
		return -1;
	if (j == (k+1) % n)
		return k;
	return -1;
}

bool surface_reconstructor::is_corner(unsigned int vi, unsigned int vj, unsigned int vk) const
{
	return find_corner(vi,vj,vk) != -1;
}



/// ensure that vertex info is allocated
void surface_reconstructor::ensure_vertex_info()
{
	vertex_info.resize(pc->get_nr_points());
	vertex_reference_length.resize(pc->get_nr_points());
	clear_vertex_info();
}
/// clear vertex info
void surface_reconstructor::clear_vertex_info()
{
	std::fill(vertex_info.begin(),vertex_info.end(),(unsigned char)0);
	std::fill(vertex_reference_length.begin(),vertex_reference_length.end(),(Crd)1);
}

///
void surface_reconstructor::clear_vertex_type_info()
{
	unsigned int n = (int)ng->size();
	for (unsigned int vi=0; vi<n; ++vi)
		set_vertex_type(vi, VT_NONE);
}
///
surface_reconstructor::VertexType surface_reconstructor::compute_vertex_type_info(unsigned int vi) const
{
	// reference neighborhood Ni of vi
	const std::vector<Idx> &Ni = ng->at(vi);
	unsigned int n = (int) Ni.size();
	// find first face corner
	unsigned int j0;
	for (j0=0; j0<n; ++j0)
		if (is_face_corner(vi,j0))
			break;
	// if no face corner skip this vertex
	if (j0 == n)
		return VT_NONE;

	bool last_is_face_corner = true;
	unsigned int j;
	unsigned int nr_holes = 0;
	for (j = (j0+1)%n; j != j0; j = (j+1)%n) {
		if (is_face_corner(vi,j))
			last_is_face_corner = true;
		else {
			if (last_is_face_corner)
				++nr_holes;
			last_is_face_corner = false;
		}
	}
	if (nr_holes == 0)
		return VT_ONE_RING;
	if (nr_holes == 1)
		return VT_BORDER;
	return VT_NM_BORDER;
}

surface_reconstructor::VertexType surface_reconstructor::get_vertex_type(unsigned int vi) const
{
	return (VertexType)((vertex_info[vi]/4)&3);
}

void surface_reconstructor::set_vertex_type(unsigned int vi, VertexType vt)
{
	vertex_info[vi] = (vertex_info[vi]&243)+4*(int)vt;
}


///
void surface_reconstructor::compute_vertex_type_info()
{
	neighbor_graph& NG = *ng;
	vertex_counts[0] = vertex_counts[1] = vertex_counts[2] = vertex_counts[3] = 0;
	for (unsigned int vi=0; vi<NG.size(); ++vi) {
		set_vertex_type(vi, compute_vertex_type_info(vi));
		++vertex_counts[get_vertex_type(vi)];
	}
}


surface_reconstructor::Crd surface_reconstructor::get_reference_length(unsigned int vi) const
{
	return vertex_reference_length[vi];
}

void surface_reconstructor::set_reference_length(unsigned int vi, const Crd& l)
{
	vertex_reference_length[vi] = l;
}

/// vertex border flag
bool surface_reconstructor::is_border(unsigned int vi) const
{
	return (vertex_info[vi]&1) != 0;
}

void surface_reconstructor::mark_as_border(unsigned int vi, bool flag)
{
	if (flag)
		vertex_info[vi] |= 1;
	else
		vertex_info[vi] &= 254;
}

void surface_reconstructor::ensure_directed_edge_info()
{
	if (!directed_edge_info.empty())
		return;
	if (ng == 0)
		return;
	neighbor_graph& NG = *ng;
	directed_edge_info.resize(NG.size());
	for (unsigned int vi=0; vi<NG.size(); ++vi)
		directed_edge_info[vi].resize(NG[vi].size());
	clear_directed_edge_info();
}

void surface_reconstructor::clear_directed_edge_info()
{
	if (ng == 0)
		return;
	neighbor_graph& NG = *ng;
	ensure_directed_edge_info();
	for (unsigned int vi=0; vi<NG.size(); ++vi)
		std::fill(directed_edge_info[vi].begin(),directed_edge_info[vi].end(),0);
}

///
void surface_reconstructor::clear_face_corner_info()
{
	if (ng == 0)
		return;
	neighbor_graph& NG = *ng;
	ensure_directed_edge_info();
	for (unsigned int vi=0; vi<NG.size(); ++vi) {
		std::vector<Idx> &Ni = ng->at(vi);
		for (unsigned int j = 0; j < Ni.size(); ++j)
			mark_as_face_corner(vi,j,false);
	}
}

///
void surface_reconstructor::clear_flag()
{
	if (ng == 0)
		return;
	neighbor_graph& NG = *ng;
	ensure_directed_edge_info();
	for (unsigned int vi=0; vi<NG.size(); ++vi) {
		std::vector<Idx> &Ni = ng->at(vi);
		for (unsigned int j = 0; j < Ni.size(); ++j)
			mark(vi,j,false);
	}
}


bool surface_reconstructor::is_face_corner(unsigned int vi, unsigned int j) const
{
	return (directed_edge_info[vi][j]&1) != 0;
}

bool surface_reconstructor::is_face_corner(unsigned int vi, unsigned int vj, unsigned int vk) const
{
	int j = find_corner(vi,vj,vk);
	if (j == -1)
		return false;
	return is_face_corner(vi,j);
}

bool surface_reconstructor::is_corner_or_non_face(unsigned int vi, unsigned int vj, unsigned int vk) const
{
	int j = find_corner(vi,vj,vk);
	if (j == -1)
		return true;
	return !is_face_corner(vi,j);
}


void surface_reconstructor::mark_as_face_corner(unsigned int vi, unsigned int j, bool flag)
{
	ensure_directed_edge_info();
	if (flag)
		directed_edge_info[vi][j] |= 1;
	else
		directed_edge_info[vi][j] &= 254;
}

void surface_reconstructor::mark_triangle_corner(unsigned int vi, unsigned int vj, unsigned int vk, bool flag)
{
	int j = find_corner(vi,vj,vk);
	if (j == -1) 
		std::cerr << "attempt to mark face corner " << vi << "," << vj << "," << vk << " without being one." << std::endl;
	else {
		mark_as_face_corner(vi,j,flag);
		if (flag && is_border(vi) && is_constraint(vi,j) && j > 0) {
			mark_as_border(vi,false);
			mark_as_constraint(vi,j,false);
		}
	}
}

void surface_reconstructor::mark_triangle(unsigned int vi, unsigned int vj, unsigned int vk, bool flag)
{
	mark_triangle_corner(vi,vj,vk,flag);
	mark_triangle_corner(vj,vk,vi,flag);
	mark_triangle_corner(vk,vi,vj,flag);
}

void surface_reconstructor::mark_triangular_faces(const std::vector<unsigned int>& T)
{
	ensure_directed_edge_info();
	const unsigned int *p = &T[0];
	for (unsigned int ti=0; ti < T.size(); ti += 3)
		mark_triangle(T[ti],T[ti+1],T[ti+2]);
}

bool surface_reconstructor::is_constraint(unsigned int vi, unsigned int j) const
{
	return (directed_edge_info[vi][j]&2) != 0;
}

void surface_reconstructor::mark_as_constraint(unsigned int vi, unsigned int j, bool flag)
{
	ensure_directed_edge_info();
	if (flag)
		directed_edge_info[vi][j] |= 2;
	else
		directed_edge_info[vi][j] &= 253;
}

/// simple marking of half edges
bool surface_reconstructor::is_marked(unsigned int vi, unsigned int j) const
{
	return (directed_edge_info[vi][j]&4) != 0;
}
void surface_reconstructor::mark(unsigned int vi, unsigned int j, bool flag)
{
	ensure_directed_edge_info();
	if (flag)
		directed_edge_info[vi][j] |= 4;
	else
		directed_edge_info[vi][j] &= 251;
}

void surface_reconstructor::remove_directed_edge(unsigned int vi, unsigned int vj)
{
	int j = ng->find(vi,vj);
	if (j == -1)
		return;
	std::vector<Idx> &Ni = ng->at(vi);
	Ni.erase(Ni.begin()+j);
	directed_edge_info[vi].erase(directed_edge_info[vi].begin()+j);
}
