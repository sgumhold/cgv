#pragma once

#include <vector>
#include <iostream>
#include <cgv/utils/statistics.h>
#include <cgv/type/standard_types.h>

#include "lib_begin.h"

/// struct representing a directed half-edge in a knn graph
struct graph_location
{
	typedef cgv::type::int32_type Idx;
	typedef cgv::type::uint32_type Cnt;
	/// index of point
	Idx vi;
	/// index of knn neighbor, 0 <= ni < k
	Idx ni;
	/// orientation of edge
	bool outwards;
	/// construct from all necessary information
	graph_location(Idx _vi = 0, Idx _ni = 0, bool _outwards = true) 
		: vi(_vi), ni(_ni), outwards(_outwards) {}
	/// compare for equality
	bool operator == (const graph_location& gl) const {
		return vi == gl.vi && ni == gl.ni && outwards == gl.outwards;
	}
};

/** Data structure used to store a knn-neighbor graph. */
struct CGV_API neighbor_graph : public std::vector<std::vector<graph_location::Idx> >
{
	/// index type
	typedef graph_location::Idx Idx;
	/// count type
	typedef graph_location::Cnt Cnt;
	/// store number of entries in the 
	Cnt nr_half_edges;
	/// construct empty neighbor graph
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
	//@}

	/**@name graph location based navigation*/
	//@{
	/// return 
	Idx vi(const graph_location& gl) const;
	/// toggle outwards flag
	graph_location inv(const graph_location& gl) const;
	/// move to neighbor along edge and toggle outwards flag
	graph_location follow_edge(const graph_location& gl) const;
	/// cycle around corner of vertex (goes to next neighbor and toggles outwards flag)
	graph_location follow_wedge(const graph_location& gl) const;
	/// follow edge and then follow wedge
	graph_location next(const graph_location& gl) const;
	//@}

	/**@name construction */
	//@{
	/// build a knn neighbor graph for n points from a data structure that provides the method extract_neighbors(i, k, vector<Idx>&).
	template <typename knn_info>
	void build(Cnt n, Cnt k, const knn_info& knn, cgv::utils::statistics* he_stats = 0) {
		if (he_stats)
			he_stats->init();
		clear();
		resize(n);
		nr_half_edges = 0;
		for (Idx i = 0; i < (Idx)n; ++i) {
			/*if (i % 10000 == 0)
				std::cout << "build ng " << i << " nr half edges = " << nr_half_edges << std::endl;*/
			knn.extract_neighbors(i, k, at(i));
			if (he_stats)
				he_stats->update(k);
			nr_half_edges += k;
		}
	}
	/// ensure the neighbor graph to be symmetric
	void symmetrize();
	//@}
};

#include <cgv/config/lib_end.h>
