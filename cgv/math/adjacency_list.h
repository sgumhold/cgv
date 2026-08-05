#pragma once

#include <algorithm>
#include <numeric>
#include <queue>
#include <utility>
#include <vector>

#include <assert.h>

namespace cgv{
namespace math{

/// a basic graph edge type
struct edge
{
	/// the index of the start vertex
	size_t start = 0;
	/// the index of the end vertex
	size_t end = 0;
};

template<typename T>
struct weighted_edge : public edge
{
	using weight_type = T;

	/// the edge weight
	T weight = {};
};

/// a basic graph node type
template<typename EdgeT>
struct vertex
{
	/// the used edge type
	using edge_type = EdgeT;

	/// incident edges
	std::vector<edge_type> edges;
};

/// the graph edge orientation
enum class EdgeOrientation {
	Undirected,	// the graph contains only undirected edges
	Directed,	// the graph contains only directed edges; inverse directed edges are allowed
};

/**
* A graph represented as an adjacency list.
*
* To create a basic graph without extra information stored per vertex or edge
* use the predefined type cgv::math::graph:
* cgv::math::graph g;
*
* To create a basic weighted graph with an additional weight attribute per edge use
* the predefined template cgv::math::weighted_graph and specify the weight type;
* cgv::math::weighted_graph<weight_type> wg;
*
* To create a graph with user-defined attributes per edge and vertex:
*
* struct my_edge: public cgv::math::edge
* {
*  	double my_extra_edge_attr;
* };
*
* struct my_vertex : public cgv::math::vertex<my_edge>
* {
*	int my_extra_vertex_attr;
* };
*
* using my_graph = cgv::math::adjacency_list< my_vertex >;
* my_graph g;
* ...
*/
template<typename VertexT>
class adjacency_list {
public:
	using vertex_type = VertexT;
	using edge_type = typename VertexT::edge_type;

	/// create a graph with the given edge_orientation
	adjacency_list(EdgeOrientation edge_orientation = EdgeOrientation::Undirected) : _edge_orientation(edge_orientation) {}

	/// create a graph with the given edge_orientation and vertex_count default-initialized vertices and zero edges
	adjacency_list(size_t vertex_count, EdgeOrientation edge_orientation = EdgeOrientation::Undirected) : _edge_orientation(edge_orientation) {
		_vertices.resize(vertex_count);
	}

	/// resize number of vertices, all edge data is removed
	void resize(size_t vertex_count) {
		remove_all_edges();
		_vertices.resize(vertex_count);
	}

	/// return true if the graph is directed
	bool is_directed() const {
		return _edge_orientation == EdgeOrientation::Directed;
	}

	/// clear graph
	void clear() {
		_vertices.clear();
	}

	/// return true if the graph does not contain any vertices
	bool empty() const {
		return _vertices.empty();
	}

	/// return the number of vertices, i.e. the order of the graph
	size_t vertex_count() const {
		return _vertices.size();
	}

	/// return the number of edges, i.e. the size of the graph
	size_t edge_count() const {
		size_t count = std::accumulate(_vertices.begin(), _vertices.end(), 0, [](const vertex_type& vertex, size_t count) { return count + vertex.edges.size(); });
		// Undirected edges are stored twice so we need to divide the count by 2
		if(!is_directed())
			return count / 2;
		return count;
	}

	/// removes all edges
	void remove_all_edges() {
		std::for_each(_vertices.begin(), _vertices.end(), [](vertex_type& vertex) { vertex.edges.clear(); });
	}

	/// access to vertex i
	vertex_type& vertex(size_t i) {
		return _vertices[i];
	}

	/// const access to vertex i
	const vertex_type& vertex(size_t i) const {
		return _vertices[i];
	}

	/// return a list of all edges in no particular order
	const std::vector<edge_type> to_edge_list() const {
		std::vector<edge_type> edges;
		for(const auto& vertex : _vertices)
			std::copy(vertex.edges.begin(), vertex.edges.end(), std::inserter(edges, edges.end()));
		return edges;
	}

	/// add a new vertex to graph and return its index
	size_t add_vertex(const vertex_type& vertex) {
		_vertices.push_back(vertex);
		return _vertices.size() - 1;
	}

	/// add an edge to the list; return false if the edge already exists, true otherwise
	bool add_edge(const edge_type& edge) {
		if(edge_exists(edge.start, edge.end))
			return false;

		if(edge.start < vertex_count() && edge.end < vertex_count()) {
			vertex(edge.start).edges.push_back(edge);
			if(!is_directed()) {
				// build a reverse edge by copying the new edge and its properties and swapping its start and end indices
				edge_type reverse_edge = edge;
				std::swap(reverse_edge.start, reverse_edge.end);
				vertex(edge.end).edges.push_back(reverse_edge);
			}
			return true;
		}
		return false;
	}

	/// add a default-initialized edge definded by the start and end vertex to the list; return false if the edge already exists, true otherwise
	bool add_edge(size_t start, size_t end) {
		return add_edge({ start, end });
	}

	/// check if edge is already in list
	bool edge_exists(size_t start, size_t end) const {
		for(const edge_type& edge : vertex(start).edges) {
			if(edge.end == end)
				return true;
		}
		return false;
	}

	/// return true if the graph contains at least one cycle 
	bool is_cyclic() const {
		if(is_directed()) {
			// If a directed graph cannot be sorted topologically it contains a cycle
			return !topological_sort_impl();
		} else {
			// Keep track of visited vertices
			std::vector<bool> visited(vertex_count(), false);

			// Perform BFS from every unvisited node
			for(size_t i = 0; i < vertex_count(); ++i) {
				if(!visited[i]) {
					// If BFS finds a cycle
					if(has_cycle_undirected_breadth_first(i, visited))
						return true;
				}
			}

			// If no cycle is found in any component
			return false;
		}
	}

	// return the graph vertex indices in topological order; only works for directed graphs; if the graph is undirected or contains a cycle an empty list is returned
	std::vector<size_t> topological_sort() const {
		std::vector<size_t> vertices;
		if(!topological_sort_impl(&vertices))
			return {};
		return vertices;
	}

private:
	// search for a cycle from a given start vertex in an undirected graph and return true if the graph contains a cycle
	bool has_cycle_undirected_breadth_first(size_t start, std::vector<bool>& visited) const {
		// Queue stores { current vertex, parent vertex }
		std::queue<std::pair<size_t, size_t>> edge_queue;

		// maker for invalid indices akin to std::string::npos
		constexpr size_t nindex = static_cast<size_t>(-1);

		// Start node has no parent
		edge_queue.push({ start, nindex });
		visited[start] = true;

		while(!edge_queue.empty()) {
			size_t node = edge_queue.front().first;
			size_t parent = edge_queue.front().second;
			edge_queue.pop();
			
			// Traverse all neighbors of current node
			for(const edge_type& edge : vertex(node).edges) {

				// If neighbor is not visited, mark it visited and push to queue
				if(!visited[edge.end]) {
					visited[edge.end] = true;
					edge_queue.push({ edge.end, node });
				} else if(edge.end != parent) {
					// If neighbor is visited and not parent, a cycle is detected
					return true;
				}
			}
		}

		// No cycle found starting from this node
		return false;
	}

	// return true if the graph vertices can be sorted in topological order; only works for directed graphs; if the graph is undirected or contains a cycle false is returned;
	// if ordered_vertices is supplied it will be filled with the vertex indices in topological order if the function returned true
	bool topological_sort_impl(std::vector<size_t>* ordered_vertices = nullptr) const {
		if(!is_directed())
			return false;

		// Array to store in-degree of each vertex
		std::vector<size_t> in_degree(vertex_count(), 0);

		// Compute in-degrees of all vertices
		for(const vertex_type& vertex : _vertices) {
			for(const edge_type& edge : vertex.edges)
				in_degree[edge.end]++;
		}

		std::queue<size_t> vertex_queue;

		// Add all vertices with in-degree 0 to the queue
		for(size_t i = 0; i < in_degree.size(); ++i) {
			if(in_degree[i] == 0)
				vertex_queue.push(i);
		}

		if(ordered_vertices)
			ordered_vertices->reserve(vertex_count());

		// Count of visited (processed) nodes
		size_t visited_count = 0;

		// Perform breadth first serach (Topological Sort)
		while(!vertex_queue.empty()) {
			size_t i = vertex_queue.front();
			vertex_queue.pop();
			visited_count++;
			if(visited_count > vertex_count())
				return false;

			// Add the vertex to the output list
			if(ordered_vertices)
				ordered_vertices->push_back(i);

			// Reduce in-degree of neighbors
			for(const auto& edge : vertex(i).edges) {
				in_degree[edge.end]--;
				if(in_degree[edge.end] == 0) {
					// Add to queue when in-degree becomes 0
					vertex_queue.push(edge.end);
				}
			}
		}

		// If visited nodes != total nodes, a cycle exists
		return visited_count == vertex_count();
	}

	/// vertices
	std::vector<vertex_type> _vertices;
	/// the type of the graph edges
	EdgeOrientation _edge_orientation = EdgeOrientation::Undirected;
};

using graph = adjacency_list<vertex<edge>>;
template<typename T>
using weighted_graph = adjacency_list<vertex<weighted_edge<T>>>;

} // namespace math
} // namespace cgv
