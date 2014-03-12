#pragma once

#include <vector>
#include <iostream>
#include <cgv/utils/statistics.h>

#include "lib_begin.h"

struct CGV_API knn_info
{
	typedef int Idx;
	typedef unsigned Cnt;
	virtual ~knn_info();
	virtual Cnt get_nr_vertices() const = 0;
	virtual void append_neighbors(Idx i, Idx k, std::vector<Idx>& neighbor_list) const = 0;
};


struct graph_location
{
	typedef knn_info::Idx Idx;
	typedef knn_info::Cnt Cnt;

	Idx vi;
	Idx ni;
	bool outwards;
	graph_location(Idx _vi = 0, Idx _ni = 0, bool _outwards = true) 
		: vi(_vi), ni(_ni), outwards(_outwards) {}
	bool operator == (const graph_location& gl) const {
		return vi == gl.vi && ni == gl.ni && outwards == gl.outwards;
	}
};

struct CGV_API neighbor_graph : public std::vector<std::vector<knn_info::Idx> >
{
	typedef knn_info::Idx Idx;
	typedef knn_info::Cnt Cnt;

	unsigned int nr_half_edges;

	neighbor_graph();
	/// reimplement clear function to also clear nr_half_edges
	void clear();
	/**@name queries*/
	//@{
	/// find index of vj in neighbors of  vi and return -1 if not found
	int find(Idx vi, Idx vj) const;
	/// check if the directed edge from  vi to vj is contained in the neighbor graph
	bool is_directed_edge(Idx vi, Idx vj) const;
	/// collect a closed loop starting it outward direction at vi towards neighbor ni 
	void collect_cycle(Idx vi, Idx ni, std::vector<graph_location>& cycle, int max_cycle_length = -1) const;

	/**@name navigation*/
	//@{
	/// return 
	Idx vi(const graph_location& gl) const;
	graph_location inv(const graph_location& gl) const;
	graph_location follow_edge(const graph_location& gl) const;
	graph_location follow_wedge(const graph_location& gl) const;
	graph_location next(const graph_location& gl) const;
	//@}

	/**@name construction */
	//@{
	template <typename knn_info>
	void build(Cnt n, Cnt k, const knn_info& knn, cgv::utils::statistics* he_stats = 0) {
		if (he_stats)
			he_stats->init();
		clear();
		resize(n);
		nr_half_edges = 0;
		for (Idx i = 0; i < (Idx)n; ++i) {
			if (i % 10000 == 0)
				std::cout << "build ng " << i << " nr half edges = " << nr_half_edges << std::endl;
			knn.extract_neighbors(i, k, at(i));
			if (he_stats)
				he_stats->update(k);
			nr_half_edges += k;
		}
	}
	void symmetrize();
	//@}
};

#include <cgv/config/lib_end.h>
