
#include "surface_reconstructor.h"
#include <cgv/utils/progression.h>

void surface_reconstructor::find_consistent_edges(std::vector<unsigned int>& E)
{
	E.clear();
	Idx vi, vj, j;
	// ensure that all is defined
	if (!ng || !pc)
		return;
	neighbor_graph& NG = *ng;
	for (vi=0; vi<(Idx)NG.size(); ++vi) {
		// refernce neighborhood Ni of vi
		const std::vector<Idx> &Ni = NG[vi];
		Cnt n = (Cnt) Ni.size();

		
		Idx k = n-1;
		for (j=0; j<(Idx)n; ++j) {
			if (!is_face_corner(vi,k) && !is_face_corner(vi,j)) {
				vj = Ni[j];
				if (vi < vj && NG.is_directed_edge(vj,vi)) {
					E.push_back(vi);
					E.push_back(vj);
				}
			}
			k = j;
		}		
	}
	std::cout << "found " << E.size()/2 << " consistent edges" << std::endl;
}


void surface_reconstructor::find_consistent_triangles(
	std::vector<unsigned int>* T)
{
	T[0].clear();
	T[1].clear();
	T[2].clear();
	Idx vi, vj, vk, j, k;
	// ensure that all is defined
	if (!ng || !pc)
		return;
	neighbor_graph& NG = *ng;
	cgv::utils::progression prog("find consistent triangles", (int)NG.size(), 10);
	for (vi=0; vi<(Idx)NG.size(); ++vi) {
		prog.step();
		// refernce neighborhood Ni of vi
		const std::vector<Idx> &Ni = NG[vi];

		// add neighbor vertices of vi into set Si
		unsigned int n = (int) Ni.size();
		for (j=0; j<(Idx)n; ++j) {
			k = (j+1) % n;
			vj = Ni[j];
			vk = Ni[k];
			bool tj = is_corner(vj,vk,vi);
			bool tk = is_corner(vk,vi,vj);
			if (tj && tk) {
				if (vi<vj && vi<vk) {
					T[0].push_back(vi);
					T[0].push_back(vj);
					T[0].push_back(vk);
				}
			}
			else if ((tj && vi<vj) || (tk && vi<vk)) {
				T[1].push_back(vi);
				T[1].push_back(vj);
				T[1].push_back(vk);
			}
			else {
				T[2].push_back(vi);
				T[2].push_back(vj);
				T[2].push_back(vk);
			}
		}
	}
/*	std::cout << "found " << T[0].size()/3 << " / " 
		                   << T[1].size()/3 << " / "
								 << T[2].size()/3 << " consistent triangles" << std::endl;*/
}



void surface_reconstructor::find_consistent_quads(std::vector<unsigned int>& T)
{
	Cnt N0 = (Cnt) T.size();
	Idx vi, vj, vk, vl;
	// ensure that all is defined
	if (!ng || !pc)
		return;
	neighbor_graph& NG = *ng;
	for (vi=0; vi<(Idx)NG.size(); ++vi) {
		// refernce neighborhood Ni of vi
		const std::vector<Idx> &Ni = NG[vi];
		Cnt ni = (Cnt) Ni.size();
	
		Idx ik = ni-1;
		for (unsigned ij=0; ij<ni; ++ij) {
			vj = Ni[ij];
			if (vi > vj)
				continue;
			int ji = NG.find(vj,vi);
			if (ji == -1)
				continue;
			vk = Ni[ik];
			int il = (ij+1)%ni;
			vl = Ni[il];

			int jk = NG.find(vj,vk);
			if (jk == -1)
				continue;
			int jl = NG.find(vj,vl);
			if (jl == -1)
				continue;
			unsigned int nj = (int) NG[vj].size();
			if (!(jk == (jl+2)%nj || jl == (jk+2)%nj))
				continue;


			int lj = NG.find(vl,vj);
			if (lj == -1)
				continue;
			int li = NG.find(vl,vi);
			if (li == -1)
				continue;
			unsigned int nl = (int) NG[vl].size();
			if ( !(li == (lj+2)%nl || li == (lj+1)%nl || lj == (li+2)%nl || lj == (li+1)%nl) )
				continue;


			int kj = NG.find(vk,vj);
			if (kj == -1)
				continue;
			int ki = NG.find(vk,vi);
			if (ki == -1)
				continue;
			unsigned int nk = (int) NG[vk].size();
			if ( !(ki == (kj+2)%nk || ki == (kj+1)%nk || kj == (ki+2)%nk || kj == (ki+1)%nk) )
				continue;
	
			int c = 0;
			if (NG.is_directed_edge(vk,vl)) {
				remove_directed_edge(vk,vl);
				c += 1;
			}
			if (NG.is_directed_edge(vl,vk)) {
				remove_directed_edge(vl,vk);
				c += 2;
			}
			if (c == 0) {
//				std::cerr << "IMPOSSIBLE CASE!!!!" << std::endl;
				continue;
			}
			if ( (c & 1) != 0) {
				T.push_back(vi);
				T.push_back(vk);
				T.push_back(vj);
				mark_triangle_corner(vi,vk,vj);
			}
			if ( (c & 1) != 0) {
				T.push_back(vi);
				T.push_back(vj);
				T.push_back(vl);
				mark_triangle_corner(vi,vj,vl);
			}
			ik = ij;
		}		
	}
	std::cout << "found another " << (T.size()-N0)/3 << " consistent triangles in quads" << std::endl;
}



bool surface_reconstructor::make_corner_consistent(unsigned int vi, unsigned int vj, unsigned int vk)
{
	if (is_corner(vi,vj,vk))
		return true;
	if (debug_mode == DBG_MAKE_CONSISTENT && debug_vi != -1 && debug_vi != vi)
		return true;
	
	neighbor_graph& NG = *ng;

	int j = NG.find(vi,vj);
	int k = NG.find(vi,vk);

	std::vector<Idx>& Ni = NG[vi];
	std::vector<Idx> Ni_tmp;
	std::vector<unsigned char>& Fi = directed_edge_info[vi];
	std::vector<unsigned char> Fi_tmp;
	if (debug_mode == DBG_MAKE_CONSISTENT) {
		Ni_tmp = Ni;
		Fi_tmp = Fi;
	}

	bool have_j = j != -1;
	bool have_k = k != -1;
	if (!have_j)
		Ni.push_back(vj);
	if (!have_k)
		Ni.push_back(vk);
	if (!have_j || !have_k) {
		sort_by_tangential_angle(vi);
		if (!have_j)
			j = NG.find(vi,vj);
		if (!have_k)
			k = NG.find(vi,vk);
	}
	if (j > k) {
		std::swap(j,k);
		std::swap(have_j, have_k);
	}
	if (!have_j)
		Fi.insert(Fi.begin()+j,0);
	if (!have_k)
		Fi.insert(Fi.begin()+k,0);

	int n = (int)Ni.size();
	bool success = true;
	if (k-j < j+n-k) {
		if (j+1 < k) {
			for (int i=j; i<k; ++i) 
				if (is_face_corner(vi,i)) {
					success = false;
					break;
				}
			if (success) {
				Ni.erase(Ni.begin()+j+1, Ni.begin()+k);
				Fi.erase(Fi.begin()+j+1, Fi.begin()+k);
			}
		}
	}
	else {
		if (k+1 < j+n) {
			int i;
			for (i=k;i<n;++i)
				if (is_face_corner(vi,i)) {
					success = false;
					break;
				}
			if (success)
				for (i=0;i<j;++i)
					if (is_face_corner(vi,i)) {
						success = false;
						break;
					}
			if (success) {
				if (k+1 < n) {
					Ni.erase(Ni.begin()+k+1,Ni.end());
					Fi.erase(Fi.begin()+k+1,Fi.end());
				}
				if (j > 0) {
					Ni.erase(Ni.begin(),Ni.begin()+j);
					Fi.erase(Fi.begin(),Fi.begin()+j);
				}
			}
		}
	}
	if (debug_mode == DBG_MAKE_CONSISTENT) {
		if (success) {
			draw_colored_edge(vi,vj,4,0,1,1,0,0,1);
			draw_colored_edge(vi,vk,4,0,1,1,0,0,1);
			for (unsigned int i=0; i<Ni.size(); ++i) {
				if (is_face_corner(vi,i))
					draw_colored_point(compute_corner_point(vi,i),4,0,0.5f,0.5f);
				draw_colored_edge(compute_corner_point(vi,i),compute_corner_point(vi,(i+1)%Ni.size()),2,0,1,0.5f);
			}
		}
		Ni = Ni_tmp;
		Fi = Fi_tmp;
	}
	return success;
}



void surface_reconstructor::make_triangles_consistent(
	const std::vector<unsigned int>& inconsistent_T, 
	std::vector<unsigned int>& consistent_T)
{
	if (directed_edge_info.empty()) {
		std::cout << "make_triangles_consistent of surface reconstructor only possible after construction of face info" << std::endl;
		return;
	}
	const unsigned int* tp = &inconsistent_T[0];
	unsigned int n = (unsigned int)inconsistent_T.size()/3;
	unsigned int nr_consistent = 0;
	for (unsigned int i = 0; i < n; ++i) {
		unsigned int vi = *tp++;
		unsigned int vj = *tp++;
		unsigned int vk = *tp++;
		if (is_corner_or_non_face(vi,vj,vk) &&
			 is_corner_or_non_face(vj,vk,vi) &&
			 is_corner_or_non_face(vk,vi,vj) &&
			 make_corner_consistent(vi,vj,vk) &&
			 make_corner_consistent(vj,vk,vi) &&
			 make_corner_consistent(vk,vi,vj) ) {
				 if (debug_mode != DBG_MAKE_CONSISTENT) {
					 consistent_T.push_back(vi);
					 consistent_T.push_back(vj);
					 consistent_T.push_back(vk);
					 mark_triangle_corner(vi,vj,vk);
					 mark_triangle_corner(vj,vk,vi);
					 mark_triangle_corner(vk,vi,vj);
				 }
				 else
					 draw_colored_triangle(vi,vj,vk,1,0,1);
			 ++nr_consistent;
		}
	}
	std::cout << "could make " << nr_consistent << " of " << (unsigned int)inconsistent_T.size()/3 << " triangles consistent " << std::endl;
}
