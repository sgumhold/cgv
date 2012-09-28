#pragma once

#include <vector>
#include <cgv/utils/statistics.h>

#include "lib_begin.h"

struct CGV_API knn_info
{
	virtual ~knn_info();
	virtual unsigned get_nr_vertices() const = 0;
	virtual void append_neighbors(unsigned i, unsigned k, std::vector<unsigned>& neighbor_list) const = 0;
};


struct graph_location
{
	unsigned int vi;
	unsigned int ni;
	bool outwards;
	graph_location(unsigned int _vi = 0, unsigned int _ni = 0, bool _outwards = true) 
		: vi(_vi), ni(_ni), outwards(_outwards) {}
	bool operator == (const graph_location& gl) const {
		return vi == gl.vi && ni == gl.ni && outwards == gl.outwards;
	}
};

struct CGV_API neighbor_graph : public std::vector<std::vector<unsigned int> >
{
	unsigned int nr_half_edges;

	neighbor_graph();
	/// reimplement clear function to also clear nr_half_edges
	void clear();
	/**@name queries*/
	//@{
	/// find index of vj in neighbors of  vi and return -1 if not found
	int find(unsigned int vi, unsigned int vj) const;
	/// check if the directed edge from  vi to vj is contained in the neighbor graph
	bool is_directed_edge(unsigned int vi, unsigned int vj) const;
	/// collect a closed loop starting it outward direction at vi towards neighbor ni 
	void collect_cycle(unsigned int vi, unsigned int ni, 
		std::vector<graph_location>& cycle, int max_cycle_length) const;

	/**@name navigation*/
	//@{
	/// return 
	unsigned int vi(const graph_location& gl) const;
	graph_location inv(const graph_location& gl) const;
	graph_location follow_edge(const graph_location& gl) const;
	graph_location follow_wedge(const graph_location& gl) const;
	graph_location next(const graph_location& gl) const;
	//@}

	/**@name construction */
	//@{
	void build(unsigned k, const knn_info& knn, cgv::utils::statistics* he_stats = 0);
	void symmetrize();
	//@}
};

#include <cgv/config/lib_end.h>
