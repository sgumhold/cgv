#include "neighbor_graph.h"
#include <cgv/utils/progression.h>
#include <algorithm>

using namespace std;

neighbor_graph::neighbor_graph() : nr_half_edges(0) {}

void neighbor_graph::clear()
{
	std::vector<std::vector<Idx> >::clear();
	nr_half_edges = 0;
}

int neighbor_graph::find(Idx vi, Idx vj) const
{
	const vector<Idx>& Ni = at(vi);
	for (Idx j=0; j<(Idx)Ni.size(); ++j) {
		if (Ni[j] == vj)
			return j;
	}
	return -1;
}

neighbor_graph::Idx neighbor_graph::vi(const graph_location& gl) const
{
	return at(gl.vi)[gl.ni];
}

graph_location neighbor_graph::inv(const graph_location& gl) const
{
	return graph_location(gl.vi,gl.ni,!gl.outwards);
}

graph_location neighbor_graph::follow_edge(const graph_location& gl) const
{
	Idx vj = at(gl.vi)[gl.ni];
	Idx nj = find(vj,gl.vi);
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

void neighbor_graph::collect_cycle(Idx vi, Idx ni, std::vector<graph_location>& cycle, int max_cycle_length) const
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

bool neighbor_graph::is_directed_edge(Idx vi, Idx vj) const
{
	return std::find(at(vi).begin(), at(vi).end(),vj) != at(vi).end();
}

void neighbor_graph::symmetrize()
{
	cgv::utils::progression prog("symmetrize neighbor graph", (unsigned)size(), 10);
	for (Idx i = 0; i < (Idx) size(); ++i) {
		prog.step();
		vector<Idx>& ni = at(i);
		for (Idx j = 0; j < (Idx)ni.size(); ++j) {
			vector<Idx>& nj = at(ni[j]);
			if (std::find(nj.begin(),nj.end(),i) == nj.end()) {
				nj.push_back(i);
				++nr_half_edges;
			}
		}
	}
}
