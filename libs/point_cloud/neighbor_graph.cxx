#include "neighbor_graph.h"
#include <cgv/utils/progression.h>
#include <algorithm>

using namespace std;

knn_info::~knn_info()
{
}

neighbor_graph::neighbor_graph() : nr_half_edges(0) {}

void neighbor_graph::clear()
{
	std::vector<std::vector<unsigned int> >::clear();
	nr_half_edges = 0;
}

int neighbor_graph::find(unsigned int vi, unsigned int vj) const
{
	const vector<unsigned int>& Ni = at(vi);
	for (unsigned int j=0; j<Ni.size(); ++j) {
		if (Ni[j] == vj)
			return j;
	}
	return -1;
}

unsigned int neighbor_graph::vi(const graph_location& gl) const
{
	return at(gl.vi)[gl.ni];
}

graph_location neighbor_graph::inv(const graph_location& gl) const
{
	return graph_location(gl.vi,gl.ni,!gl.outwards);
}

graph_location neighbor_graph::follow_edge(const graph_location& gl) const
{
	unsigned int vj = at(gl.vi)[gl.ni];
	unsigned int nj = find(vj,gl.vi);
	if (nj == at(vj).size()) {
		std::cerr << "did not find edge in neighbor ring" << std::endl;
		exit(0);
	}
	return graph_location(vj,nj,!gl.outwards);
}

graph_location neighbor_graph::follow_wedge(const graph_location& gl) const 
{
	if (gl.outwards) {
		std::cerr << "attempt to follow wedge from outward locations" << std::endl;
		exit(0);
	}
	return graph_location(gl.vi,(gl.ni+1)%at(gl.vi).size(),true);
}

graph_location neighbor_graph::next(const graph_location& gl) const
{
	return follow_wedge(follow_edge(gl));
}

void neighbor_graph::collect_cycle(unsigned int vi, unsigned int ni, std::vector<graph_location>& cycle, int max_cycle_length) const
{
	graph_location gl0(vi,ni);
	cycle.push_back(gl0);
	graph_location gl = gl0;
	do {
		gl = next(gl);
		if (gl == gl0)
			break;
		cycle.push_back(gl);
		if (max_cycle_length != -1 && (int)cycle.size() >= max_cycle_length) {
			std::cerr << "infinite cycle!!!" << std::endl;
			break;
		}
	} while (true);
}

bool neighbor_graph::is_directed_edge(unsigned int vi, unsigned int vj) const
{
	return std::find(at(vi).begin(), at(vi).end(),vj) != at(vi).end();
}

void neighbor_graph::build(unsigned k, const knn_info& knn, cgv::utils::statistics* he_stats) 
{
	if (he_stats)
		he_stats->init();
	clear();
	unsigned int n = knn.get_nr_vertices();
	resize(n);
	nr_half_edges = 0;
	for (unsigned int i = 0; i < n; ++i) {
		if (i % 10000 == 0)
			cout << "build ng " << i << " nr half edges = " << nr_half_edges << endl;
		knn.append_neighbors(i, k, at(i));
		if (he_stats)
			he_stats->update(k);
		nr_half_edges += k;
	}
}

void neighbor_graph::symmetrize()
{
	cgv::utils::progression prog("symmetrize neighbor graph", (unsigned int)size(), 10);
	for (unsigned int i = 0; i < size(); ++i) {
		prog.step();
		vector<unsigned int>& ni = at(i);
		for (unsigned int j = 0; j < ni.size(); ++j) {
			vector<unsigned int>& nj = at(ni[j]);
			if (std::find(nj.begin(),nj.end(),i) == nj.end()) {
				nj.push_back(i);
				++nr_half_edges;
			}
		}
	}
}
