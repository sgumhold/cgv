
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <set>
#include "surface_reconstructor.h"
#include <cgv/math/functions.h>
#include <cgv/utils/progression.h>

/*
std::ostream& operator << (std::ostream& os, const grow_event& ge)
{
	return os << (ge.type == CORNER_GROW_EVENT ? "corner" : "edge") << " " << (ge.dir == BACKWARD ? "backward" : "forward") << "(vi=" << ge.vi << ",j=" << ge.j << ",k=" << ge.k << ", quality=" << ge.quality << ")";
}
*/

/// compute geometric quality of a triangle
float surface_reconstructor::compute_normal_quality(const Nml& n1, const Nml& n2) const
{
	float nml_comp;
	if (use_orientation)
		nml_comp = 0.5f*(dot(n1,n2)+1);
	else {
		nml_comp  = dot(n1,n2);
		nml_comp *= nml_comp;
	}
	return 0.5f*(cgv::math::erf((Crd)normal_quality_exp*(nml_comp-0.5f))+1.0f);
}

/// compute geometric quality of a triangle
float surface_reconstructor::compute_triangle_quality(unsigned int vi,unsigned int vj, unsigned int vk, unsigned int nr_insert, unsigned int nr_remove) const
{
	double as[3] = {
		compute_corner_angle(vi,vj,vk),
		compute_corner_angle(vj,vk,vi),
		compute_corner_angle(vk,vi,vj) 
	};
	if (use_orientation) {
		double tas[3] = {
			compute_tangential_corner_angle(vi,vj,vk),
			compute_tangential_corner_angle(vj,vk,vi),
			compute_tangential_corner_angle(vk,vi,vj) 
		};
		for (int ai=0; ai<3; ++ai) {
			if (tas[ai] > M_PI) {
				if (debug_events)
					std::cout << "   quality failed angle criterium: " 
					<< tas[0]*180/M_PI << "," << tas[1]*180/M_PI << "," << tas[2]*180/M_PI << std::endl;
				return 0;
			}
		}
	}
	float min_angle = (float)std::min(std::min(as[0],as[1]),as[2]);
//	if (min_angle < 0.05f)
//		return 0;

	Nml tgl_nml = compute_triangle_normal(vi,vj,vk);
	float nqs[3] = { 
		compute_normal_quality(tgl_nml,pc->nml(vi)),
		compute_normal_quality(tgl_nml,pc->nml(vj)),
		compute_normal_quality(tgl_nml,pc->nml(vk))
	};
	std::sort(nqs,nqs+3);
	float connectivity_term = 1.5f-0.5f*nr_insert;
//	std::cout << nr_insert << " inserts, " << nr_remove << " removes => " << connectivity_term << std::endl;
	return nqs[0]*min_angle+connectivity_term;
}


/// check consistency of corner grow operation
bool surface_reconstructor::can_extent_fan(
	unsigned int vi,unsigned int vj, unsigned int vk, Direction dir,
	unsigned int& nr_insert, unsigned int& nr_remove) const
{
	neighbor_graph& NG = *ng;
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int n = (int) Ni.size();
	int j = NG.find(vi,vj);
	int k = NG.find(vi,vk);
	if (debug_events)
		std::cout << "   check extent to fan " << vi << ",";
	if (dir == BACKWARD) {
		if (k == -1) {
			if (debug_events)
				std::cout << vj << ",?" << vk << " missing edge to vk" << std::endl;
			return false;		
		}
		if (j == -1) {
			j = find_surrounding_corner(vi,vj);
			++nr_insert;
		}
		else if (is_constraint(vi,k) && is_constraint(vi,j)) {
			if (debug_events)
				std::cout << vj << "," << vk << " would close border backwards" << std::endl;
			return false;
		}
		for (unsigned int l = j; l != k; l = (l+1)%n) {
			if (l!=j)
				++nr_remove;
			if (is_face_corner(vi,l)) {
				if (debug_events)
					std::cout << vj << "," << vk << " would collide backwards with faces" << std::endl;
				return false;
			}
		}
	}
	else {
		if (j == -1) {
			if (debug_events)
				std::cout << "?" << vj << "," << vk << " missing edge to vj" << std::endl;
			return false;
		}
		if (k == -1) {
			k = (find_surrounding_corner(vi,vk)+1)%n;
			++nr_insert;
		}
		else if (is_constraint(vi,k) && is_constraint(vi,j)) {
			if (debug_events)
				std::cout << vj << "," << vk << " would close border forwards" << std::endl;
			return false;
		}
		// check for overlapping face corners
		for (unsigned int l = j; l != k; l = (l+1)%n) {
			if (l != j)
				++nr_remove;
			if (is_face_corner(vi,l)) {
				if (debug_events)
					std::cout << vj << "," << vk << " would collide backwards with faces" << std::endl;
				return false;
			}
		}
	}
	if (debug_events)
		std::cout << " ok" << std::endl;
	return true;
}
/// check consistency of edge grow operation
bool surface_reconstructor::can_connect_to_fan(unsigned int vi,unsigned int vj, unsigned int vk, unsigned int& nr_insert, unsigned int& nr_remove) const
{
	if (debug_events)
		std::cout << "   check connect to fan " << vi << ",";
	neighbor_graph& NG = *ng;
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int n = (int) Ni.size();
	int j = NG.find(vi,vj);
	if (j == -1) {
		j = find_surrounding_corner(vi,vj);
		++nr_insert;
		if (debug_events)
			std::cout << "*";
	}
	if (debug_events)
		std::cout << vj << ",";
	int k = NG.find(vi,vk);
	if (k == -1) {
		k = (find_surrounding_corner(vi,vk)+1)%n;
		++nr_insert;
		if (debug_events)
			std::cout << "*";
	}
	if (debug_events)
		std::cout << vk;
	if (is_border(vi) && j >= k) {
		if (debug_events)
			std::cout << " would grow over border" << std::endl;
		return false;
	}
	// check for overlapping face corners
	for (unsigned int l = j; l != k; l = (l+1)%n) {
		if (l != j)
			++nr_remove;
		if (is_face_corner(vi,l)) {
			if (debug_events)
				std::cout << " would grow over faces" << std::endl;
			return false;
		}
	}
	if (debug_events)
		std::cout << " ok" << std::endl;
	return true;
}

/// check corner grow event and insert to queue
bool surface_reconstructor::is_valid_corner_grow_event(
	const grow_event& ge, unsigned int& nr_insert, unsigned int& nr_remove) const
{
	unsigned int vi = ge.vi;
	unsigned int j  = ge.j;
	unsigned int k  = ge.k;

	if (j == k) {
		std::cerr << "INVALID CORNER GROW EVENT" << std::endl;
		return false;
	}
	neighbor_graph& NG = *ng;
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int vj = Ni[j];
	unsigned int vk = Ni[k];
	if (debug_events)
		std::cout << "consider cornern grow event " << vi << "," << vj << "," << vk << std::endl;
	if (length(pc->pnt(vj)-pc->pnt(vk)) > valid_length_scale*get_reference_length(vi)) {
		if (debug_events)
			std::cout << "   failed reference length test" << std::endl;
		return false;
	}
	if (!can_add_triangle_manifold(vi,vj,vk)) {
		if (debug_events)
			std::cout << "   would create non manifold triangle" << std::endl;
		return false;
	}
	// check if inner angle of triangle is below 180 degrees
	if (compute_tangential_corner_angle(vi,vj,vk) > 0.99f*M_PI) {
		if (debug_events)
			std::cout << "   failed cornern angle test" << std::endl;
		return false;
	}
	/// check if we can extend the fans of the neighbor vertices
	if (!can_extent_fan(vj,vk,vi,BACKWARD,nr_insert,nr_remove) ||
		 !can_extent_fan(vk,vi,vj,FORWARD,nr_insert,nr_remove) )
			return false;
	return true;
}

/// check edge grow event and insert to queue
bool surface_reconstructor::is_valid_edge_grow_event(
	const grow_event& ge, unsigned int& nr_insert, unsigned int& nr_remove) const
{
	unsigned int vi = ge.vi;
	unsigned int j  = ge.j;
	unsigned int k  = ge.k;
	Direction dir   = ge.dir;

	if (j == k) {
		std::cerr << "INVALID CORNER GROW EVENT" << std::endl;
	}
	neighbor_graph& NG = *ng;
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int vj = Ni[j];
	unsigned int vk = Ni[k];
	if (debug_events)
		std::cout << "consider edge grow event " << vi << "," << vj << "," << vk << std::endl;
	if (!can_add_triangle_manifold(vi,vj,vk)) {
		if (debug_events)
			std::cout << "   would create non manifold triangle" << std::endl;
		return false;
	}
	if (dir == BACKWARD) {
		if (is_constraint(vi,k)) {
			if (debug_events)
				std::cout << "   would grow backwards over border" << std::endl;
			return false;
		}
		if (!can_connect_to_fan(vj,vk,vi,nr_insert,nr_remove) ||
			 !can_extent_fan(vk,vi,vj,FORWARD,nr_insert,nr_remove) )
			return false;
	}
	else {
		if (is_constraint(vi,j)) {
			if (debug_events)
				std::cout << "   would grow forwards over border" << std::endl;
			return false;
		}
		if (!can_extent_fan(vj,vk,vi,BACKWARD,nr_insert,nr_remove) ||
			 !can_connect_to_fan(vk,vi,vj,nr_insert,nr_remove) )
			return false;
	}
	return true;
}


bool surface_reconstructor::validate_event(grow_event& ge) const
{
	unsigned int nr_insert = 0, nr_remove = 0;
	if (ge.type == CORNER_GROW_EVENT) {
		if (!is_valid_corner_grow_event(ge,nr_insert,nr_remove))
			return false;
	}
	else {
		if (!is_valid_edge_grow_event(ge,nr_insert,nr_remove))
			return false;
	}
	ge.quality = compute_triangle_quality(ge.vi,ng->at(ge.vi)[ge.j],ng->at(ge.vi)[ge.k],nr_insert,nr_remove);
	if (ge.quality == 0) {
		if (debug_events)
			std::cout << "   has zero quality" << std::endl;
		return false;
	}
	return true;
}

void surface_reconstructor::add_grow_event(const grow_event& ge)
{
	if (debug_events) {
		std::cout << "add event " << ge << std::endl;
	}
	unsigned int gi = grow_events.insert(ge);
	if (ge.vi != grow_events[gi].vi) {
		std::cout << "ups add event of wrong vertex " << grow_events[gi].vi << " instead of " << ge.vi << std::endl;
	}
	if (first_grow_event[ge.vi] != -1 && ge.vi != grow_events[first_grow_event[ge.vi]].vi) {
		std::cout << "ups add event of wrong vertex " << grow_events[first_grow_event[ge.vi]].vi << " instead of " << ge.vi << std::endl;
	}
	grow_events[gi].next_grow_event_of_vertex = first_grow_event[ge.vi];
	first_grow_event[ge.vi] = gi;
}

/// check corner grow event and insert to queue
bool surface_reconstructor::consider_corner_grow_event(
	unsigned int vi,unsigned int j, unsigned int k)
{
	grow_event ge(vi,j,k,CORNER_GROW_EVENT);
	if (validate_event(ge))
		add_grow_event(ge);
	return true;
}

/// check edge grow event and insert to queue
bool surface_reconstructor::consider_edge_grow_event(
	unsigned int vi,unsigned int j, unsigned int k, Direction dir)
{
	grow_event ge(vi,j,k,EDGE_GROW_EVENT,dir);
	if (validate_event(ge))
		add_grow_event(ge);
	return true;
}

void surface_reconstructor::consider_grow_events(unsigned int vi)
{
	unsigned int vj, j;
	neighbor_graph& NG = *ng;
	// reference neighborhood Ni of vi
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int n = (int) Ni.size();

	// find first face corner
	unsigned int j0;
	for (j0=0; j0<n; ++j0)
		if (is_face_corner(vi,j0))
			break;
	// if no face corner skip this vertex
	if (j0 == n)
		return;

	bool last_is_face_corner = true;
	unsigned int block_end = (j0+1)%n;
	j = block_end;
	do {
		if (is_face_corner(vi,j)) {
			if (!last_is_face_corner) {
				consider_corner_grow_event(vi,block_end,j);
				// check backward if we also have to consider an edge event
				if (j != (block_end+1)%n) {
					// check forward if we also have to consider an edge event
					vj = Ni[j];
					unsigned int k = (j+n-1)%n;
					int jk = NG.find(vj,Ni[k]);
					if (jk == -1 || !is_face_corner(vj,jk))
						consider_edge_grow_event(vi,k,j,BACKWARD);
				}
			}
			block_end = (j+1)%n;
			last_is_face_corner = true;
		}
		else {
			if (last_is_face_corner) {
				// check forward if we can consider an edge event
				unsigned int k = (j+1)%n;
				if (!is_face_corner(vi,k)) {
					// check backward if we also have to consider an edge event
					vj = Ni[j];
					const std::vector<Idx> &Nj = NG[vj];
					unsigned int nj = (unsigned int) Nj.size();
					int jk = NG.find(vj,Ni[k]);
					if (jk == -1 || !is_face_corner(vj,(jk+nj-1)%nj))
						consider_edge_grow_event(vi,j,k,FORWARD);
				}
			}
			last_is_face_corner = false;
		}
		if (j == j0)
			break;
		j=(j+1)%n;
	} while (true);
}

void surface_reconstructor::build_grow_queue(const std::vector<unsigned int>& T)
{
	if (directed_edge_info.empty()) {
		std::cout << "growing only possible after construction of directed edge info" << std::endl;
		return;
	}
	init_nr_triangles();
	for (unsigned int i=0; i<T.size(); i+=3)
		count_triangle(T[i],T[i+1],T[i+2]);
	grow_events.clear();
	first_grow_event.resize(pc->get_nr_points());
	std::fill(first_grow_event.begin(),first_grow_event.end(),-1);

	// ensure that all is defined
	if (!ng || !pc)
		return;
	unsigned int n = (unsigned int) ng->size();
	cgv::utils::progression prog("build grow queue         ", n, 10);
	for (unsigned int vi=0; vi<n; ++vi) {
		prog.step();
		consider_grow_events(vi);
	}

	geqs.init();
	for (unsigned int i=0; i<grow_events.size(); ++i)
		geqs.update(grow_events[i].quality);
}


/// remove the grow events of a given vertex
void surface_reconstructor::remove_grow_events(unsigned int vi)
{
	if (debug_events)
		std::cout << "remove " << vi << " events:";
	int gi = first_grow_event[vi];
	int nr = 0;
	while (gi != -1) {
		int gj = gi;
		gi = grow_events[gi].next_grow_event_of_vertex;
		if (vi != grow_events[gj].vi) {
			std::cout << "ups removed event of wrong vertex " << grow_events[gj].vi << " instead of " << vi << std::endl;
		}
		if (debug_events) {
			std::cout << " " << grow_events[gj];
		}
		grow_events.remove(gj);
	}
	if (debug_events)
		std::cout << std::endl;
	first_grow_event[vi] = -1;
}

void surface_reconstructor::remove_directed_edges(unsigned int vi, unsigned int j, unsigned int k)
{
	if (j == k)
		return;

	neighbor_graph& NG = *ng;
	std::vector<Idx> &Ni = NG[vi];
	std::vector<unsigned char> &Ei = directed_edge_info[vi];
	std::vector<unsigned int> &Ti = nr_triangles_per_edge[vi];
	unsigned int n = (int) Ni.size();
	if (k > j) {
		Ni.erase(Ni.begin()+j,Ni.begin()+k);
		Ei.erase(Ei.begin()+j,Ei.begin()+k);
		Ti.erase(Ti.begin()+j,Ti.begin()+k);
	}
	else {
		Ni.erase(Ni.begin()+j,Ni.end());
		Ei.erase(Ei.begin()+j,Ei.end());
		Ti.erase(Ti.begin()+j,Ti.end());
		if (k > 0) {
			Ni.erase(Ni.begin(),Ni.begin()+k);
			Ei.erase(Ei.begin(),Ei.begin()+k);
			Ti.erase(Ti.begin(),Ti.begin()+k);
		}
	}
}

unsigned int surface_reconstructor::insert_directed_edge(unsigned int vi, unsigned int vj)
{
	neighbor_graph& NG = *ng;
	std::vector<Idx> &Ni = NG[vi];
	std::vector<unsigned char> &Ei = directed_edge_info[vi];
	std::vector<unsigned int> &Ti = nr_triangles_per_edge[vi];
	unsigned int n = (int) Ni.size();
	unsigned int j = find_surrounding_corner(vi,vj);
	if (j == n-1) {
		Ni.push_back(vj);
		Ei.push_back(0);
		Ti.push_back(0);
	}
	else {
		Ni.insert(Ni.begin()+j+1, vj);
		Ei.insert(Ei.begin()+j+1, 0);
		Ti.insert(Ti.begin()+j+1, 0);
	}
	return j+1;
}

/// extent a triangle fan in the 1-ring of vi
void surface_reconstructor::extent_fan(
	unsigned int vi,unsigned int vj, unsigned int vk, Direction dir)
{
	neighbor_graph& NG = *ng;
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int n = (int) Ni.size();
	int j = NG.find(vi,vj);
	int k = NG.find(vi,vk);
	if (dir == BACKWARD) {
		if (k == -1) {
			std::cerr << "could not find directed edge to vk in extent_fan method" << std::endl;
			exit(0);
		}
		if (j == -1) {
			j = insert_directed_edge(vi,vj);
			++n;
			if (k >= j)
				++k;
		}
	}
	else {
		if (j == -1) {
			std::cerr << "could not find directed edge to vj in extent_fan method" << std::endl;
			exit(0);
		}
		if (k == -1) {
			k = insert_directed_edge(vi,vk);
			++n;
			if (j >= k)
				++j;
		}
	}
	mark_as_face_corner(vi,j);
	remove_directed_edges(vi, (j+1)%n, k);
}
/// 
void surface_reconstructor::connect_to_fan(unsigned int vi,unsigned int vj, unsigned int vk)
{
	neighbor_graph& NG = *ng;
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int n = (int) Ni.size();
	int j = NG.find(vi,vj);
	int k = NG.find(vi,vk);
	if (j == -1) {
		j = insert_directed_edge(vi,vj);
		++n;
		if (k >= j)
			++k;
	}
	if (k == -1) {
		k = insert_directed_edge(vi,vk);
		++n;
		if (j >= k)
			++j;
	}
	mark_as_face_corner(vi,j);
	remove_directed_edges(vi, (j+1)%n, k);
}

/// perform grow event
void surface_reconstructor::perform_next_grow_event(std::vector<unsigned int>& T)
{
	if (grow_events.is_empty(grow_events.top())) {
		std::cout << "ATTEMPT TO PERFORM EMPTY GROW EVENT" << std::endl;
	}

	while (true) {
		grow_event& ge = grow_events[grow_events.top()];
		if (!validate_event(ge) || 
			 ( perform_intersection_tests &&
				  !can_create_triangle_without_self_intersections(
				  ge.vi,ng->at(ge.vi)[ge.j],ng->at(ge.vi)[ge.k]) ) ) {
		   // ensure that we remove top event from the event list of its vertex
			unsigned int vi = ge.vi;
			int* ge_idx_ref = &first_grow_event[vi];
			bool found = false;
			while (*ge_idx_ref != -1) {
				if (*ge_idx_ref == grow_events.top()) {
					*ge_idx_ref = ge.next_grow_event_of_vertex;
					found = true;
					break;
				}
				else {
					ge_idx_ref = &grow_events[*ge_idx_ref].next_grow_event_of_vertex;
				}
			}
			if (!found) {
				std::cout << "UPS could not find top event" << std::endl;
			}
			// before poping it
			grow_events.pop();
			if (grow_events.empty())
				return;
		}
		else
			break;
	}
	const grow_event& ge = grow_events[grow_events.top()];
	neighbor_graph& NG = *ng;
	unsigned int vi = ge.vi;
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int ni = (unsigned int) Ni.size();
	unsigned int j  = ge.j;
	unsigned int vj = Ni[j];
	const std::vector<Idx> &Nj = NG[vj];
	unsigned int nj = (unsigned int) Nj.size();
	unsigned int k  = ge.k;
	unsigned int vk = Ni[k];
	const std::vector<Idx> &Nk = NG[vk];
	unsigned int nk = (unsigned int) Nk.size();


	// collect all influenced vertices in the 1-ring of one of the triangles vertices
	std::set<unsigned int> VI;
	VI.insert(vi);
	VI.insert(vj);
	VI.insert(vk);
	unsigned int l;
	for (l=0;l<ni;++l)
		VI.insert(Ni[l]);
	for (l=0;l<nj;++l)
		VI.insert(Nj[l]);
	for (l=0;l<nk;++l)
		VI.insert(Nk[l]);

	if (ge.type == CORNER_GROW_EVENT) {
		mark_as_face_corner(vi,j);
		// check if we have to remove directed edges
		remove_directed_edges(vi,(j+1)%ni,k);
		extent_fan(vj,vk,vi,BACKWARD);
		extent_fan(vk,vi,vj,FORWARD);
	}
	else { // EDGE_GROW_EVENT
		mark_as_face_corner(vi,j);
		if (ge.dir == BACKWARD) {
			connect_to_fan(vj,vk,vi);
			extent_fan(vk,vi,vj,FORWARD);
		}
		else {
			extent_fan(vj,vk,vi,BACKWARD);
			connect_to_fan(vk,vi,vj);
		}
	}
	count_triangle(vi,vj,vk);

	// update priority queue
	for (std::set<unsigned int>::const_iterator iter = VI.begin(); iter != VI.end(); ++iter) {
		unsigned int vi = *iter;
		remove_grow_events(vi);
		consider_grow_events(vi);
	}
	// add new triangle
	T.push_back(vi);
	T.push_back(vj);
	T.push_back(vk);
}

/// perform grow events till no more events are left and add the generated triangles to T
unsigned int surface_reconstructor::grow_all(std::vector<unsigned int>& T)
{
	int iter = 0;
	while (!grow_events.empty()) {
		perform_next_grow_event(T);
		++iter;
	}
	return iter;
}
