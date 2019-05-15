
#include "surface_reconstructor.h"
#include <cgv/utils/progression.h>
#include <set>

void surface_reconstructor::increment_directed_edge(int vi, int vj, bool inc)
{
	int i = ng->find(vi,vj);
	if (i >= 0) {
		if (inc)
			++nr_triangles_per_edge[vi][i];
		else
			--nr_triangles_per_edge[vi][i];
	}
	else
		std::cerr << "did not find edge " << vi << "," << vj << std::endl;
}

bool surface_reconstructor::init_nr_triangles()
{
	unsigned int vi;
	// ensure that all is defined
	if (!ng || !pc)
		return false;
	neighbor_graph& NG = *ng;

	// init all counts to zero
	nr_triangles_per_vertex.resize(NG.size());
	std::fill(nr_triangles_per_vertex.begin(),
			    nr_triangles_per_vertex.end(), 0);
	nr_triangles_per_edge.resize(NG.size());
	for (vi=0; vi<NG.size(); ++vi) {
		nr_triangles_per_edge[vi].resize(NG[vi].size());
		std::fill(nr_triangles_per_edge[vi].begin(),
					 nr_triangles_per_edge[vi].end(), 0);
	}
	return true;
}

///
bool surface_reconstructor::can_add_triangle_manifold(unsigned int vi, unsigned int vj, unsigned int vk) const
{
	return add_triangle_check_edge(vi,vj) && 
		    add_triangle_check_edge(vj,vk) && 
			 add_triangle_check_edge(vk,vi);
}

bool surface_reconstructor::add_triangle_check_edge(unsigned int vi, unsigned int vj) const
{
	int j = ng->find(vi,vj);
	if (j == -1) {
		int i = ng->find(vj,vi);
		if (i == -1)
			return true;
		return nr_triangles_per_edge[vj][i] < 2;
	}
	return nr_triangles_per_edge[vi][j] < 2;
}

void surface_reconstructor::count_triangle(unsigned int vi, unsigned int vj, unsigned int vk, bool inc)
{
	if (inc) {
		++nr_triangles_per_vertex[vi];
		++nr_triangles_per_vertex[vj];
		++nr_triangles_per_vertex[vk];
	}
	else {
		--nr_triangles_per_vertex[vi];
		--nr_triangles_per_vertex[vj];
		--nr_triangles_per_vertex[vk];
	}
	increment_directed_edge(vi,vj,inc);
	increment_directed_edge(vj,vi,inc);
	increment_directed_edge(vi,vk,inc);
	increment_directed_edge(vj,vk,inc);
	increment_directed_edge(vk,vj,inc);
	increment_directed_edge(vk,vi,inc);
}

bool surface_reconstructor::compute_nr_triangles()
{
	if (!init_nr_triangles())
		return false;

	Idx vi, vj, vk, j, k;
	neighbor_graph& NG = *ng;
	// iterate all triangles
	for (vi=0; vi<(Idx)NG.size(); ++vi) {
		// refernce neighborhood Ni of vi
		const std::vector<Idx> &Ni = NG[vi];

		// add neighbor vertices of vi into set Si
		std::set<Idx> Si;
		for (j=0; j<(Idx)Ni.size(); ++j)
			Si.insert(Ni[j]);

		for (j=0; j<(Idx)Ni.size(); ++j) {
			vj = Ni[j];
			if (vj < vi)
				continue;
			// 
			const std::vector<Idx> &Nj = NG[vj];
			for (k=0; k<(Idx)Nj.size(); ++k) {
				vk = Nj[k];
				if (vk < vj)
					continue;
				if (Si.find(vk) == Si.end())
					continue;

				// we finally found a triangle vi vj vk
				count_triangle(vi,vj,vk);
			}
		}
	}
	ntpv.init();
	ntpe.init();
	// compute statistics
	for (vi=0; vi<(Idx)NG.size(); ++vi) {
		ntpv.update(nr_triangles_per_vertex[vi]);
		const std::vector<Idx> &Ni = NG[vi];
		for (j=0; j<(Idx)Ni.size(); ++j) {
			vj = Ni[j];
			if (vj < vi)
				continue;
			ntpe.update(nr_triangles_per_edge[vi][j]);				
		}
	}
	// show statistics
	std::cout << "nr tgls per vertex: " << ntpv << std::endl;
	std::cout << "nr tgls per edge:   " << ntpe << std::endl;
	return true;
}

bool surface_reconstructor::nr_triangles_computed() const
{
	return !nr_triangles_per_vertex.empty();
}

unsigned int surface_reconstructor::get_nr_triangles(unsigned int vi) const
{
	return nr_triangles_per_vertex[vi];
}

unsigned int surface_reconstructor::get_nr_triangles(unsigned int vi, unsigned int j) const
{
	return nr_triangles_per_edge[vi][j];
}

