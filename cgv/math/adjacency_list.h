#pragma once
#include<vector>
#include<assert.h>


namespace cgv{
	namespace math{


//a basic graph edge type
struct edge
{
	//index of start and end vertex	
	unsigned start,end; 
};


struct weighted_edge : public edge
{
	/// edge weight
	double weight;
	int flag;
	
	///standard constructor
	weighted_edge() : weight(1.0) {}
	weighted_edge(double w) : weight(w) {}
};

//a basic graph node type
template <typename ET>
struct vertex 
{
	typedef typename ET edge_type;
	//incident edges
	std::vector<edge_type> edges;
	
	unsigned nedges()
	{
		return edges.size();
	}

};


/**
* A graph represented in an adjacency list.
*
* To create a basic graph without extra information stored per vertex or edge
* use the predefined type cgv::math::graph:
* cgv::math::graph g;
*
* To create a basic weighted graph with an additional weight attribute per edge use
* the predefined type cgv::math::weighted_graph;
* cgv::math::weighted_graph wg;
* 
* To create a graph with extra attributes per edge and vertex:
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
* typedef cgv::math::adjacency_list< my_vertex > my_graph;
* my_graph g;
* ...
*/
template<typename v_type  >
class adjacency_list
{
public:
	typedef typename v_type vertex_type;
	typedef typename v_type::edge_type edge_type;
	
	
	///vertices
	std::vector<vertex_type> *vertices;
	///flag indicating a directed/undirected graph
	bool directed;


	adjacency_list()
	{
		vertices = NULL;
		directed = true;
	}

	///creates a graph with vnum vertices and zero edges
	///the graph is directed if the flag directed is true
	adjacency_list(const unsigned vnum, bool directed=true) : directed(directed)
	{
		vertices = new std::vector<vertex_type>(vnum);
		this->directed = directed;
	}


	
	///copy constructor
	adjacency_list(const adjacency_list& al)
	{
		if (vertices)
			delete vertices;
		directed = al.is_directed();
		vertices = new std::vector<vertex_type>(*(al.vertices));
	}
	
	///assignment operator
	adjacency_list& operator=(const adjacency_list& al)
	{
		if(&al == this)
			return *this;
		if (vertices)
			delete vertices;
		directed = al.is_directed();
		vertices = new std::vector<vertex_type>(*(al.vertices));
		return *this;
	}

	///destructor of adjacency_list
	virtual ~adjacency_list()
	{
		if(vertices)
			delete vertices;
	}

	///resize number of vertices, all edge data is removed
	void resize(const unsigned& vnum)
	{
		if(!vertices)
			vertices = new std::vector<vertex_type>(vnum);
		else
		{
			remove_all_edges();
			vertices->resize(vnum);
		}
	}

	// clear graph
	void clear()
	{
		if(vertices)
		{
			delete vertices;
			vertices = NULL;
		}
	}


	///return number of vertices
	const unsigned nverts() const
	{
		assert(vertices != NULL);		
		return vertices->size(); 
	}

	///removes all edges
	void remove_all_edges()
	{
		for(unsigned vi = 0; vi < nverts(); vi++)
		{
			vertex(vi).edges.clear();
		}
	}

	///access vertex i
	vertex_type& vertex(unsigned i)
	{
		return (*vertices)[i];
	}

	///access const vertex i
	const vertex_type& vertex(unsigned i)const 
	{
		return (*vertices)[i];
	}

	

	///returns true if the graph is a directed one
	bool is_directed() const
	{
		return directed;
	}

	
	/// checks if edge is already in list
	bool edge_exists(int start, int end) const
	{
		for (unsigned  i=0; i < vertex(start).edges.size(); i++)
		{
			if(vertex(start).edges[i].end == end)
			{
				//std::cout<<"Edge already in list"<<std::endl;
				return true;
			}
		}
		return false; 
	}

	
	
	/// adds an edge to the list definded by the start and end vertex
	bool add_edge(const edge_type& e)
	{
		
		if(edge_exists(e.start, e.end))
			return false;


		if( e.start < nverts() && e.end < nverts())
		{

					

			(*vertices)[e.start].edges.push_back(e); 

			if(!directed)
			{
				edge_type e2=e;
				e2.start = e.end;
				e2.end = e.start;
				(*vertices)[e.end].edges.push_back(e2); 
			}
			return true;
		}
		return false;
	}
	

	/// adds an edge to the list definded by the start and end vertex
	bool add_edge(unsigned int start, unsigned int end)
	{
		
		if(edge_exists(start, end))
			return false;


		if( start < nverts() && end < vertices->size())
		{

			edge_type e;
			e.start=start;
			e.end = end;
		

			(*vertices)[start].edges.push_back(e); 

			if(!directed)
			{
				edge_type e2;
				e2.start=end;
				e2.end = start;
				
				(*vertices)[end].edges.push_back(e2); 
			}
			return true;
		}
		return false;
	}

	///add new vertex to graph
	void add_vertex(const vertex_type& v)
	{
		vertices->push_back(v);
	}

	/*/// adds an edge to the list
	void add_edge(unsigned start,unsigned end,const edge_type& e)
	{
		if(edge_exists(start, end))
			return false;


		if( start < vertices->size() && end < vertices->size())
		{

			edge_type e;
			e.start = start;
			e.end = end;
		

			(*vertices)[start].edges.push_back(e); 

			if(!directed)
			{
				edge_type e2;
				e2.start = end;
				e2.end = start;
				
				(*vertices)[end].edges.push_back(e); 
			}
			return true;
		}
		return false;
	}*/

/*	/// adds an vertex with n zero filled edges to the list
	void add(int n){
	
		vertex v;
			for(int i=0;i<n;i++){
				edge a;
				a.start = 0;
				a.target   = 0;
				v.edges.push_back(a);
			}
		vertices->push_back(v);

	}

	/// adds an vertex to the list
	void add(vertex v){
		std::cout<<"not tested"<<std::endl;
		vertices->push_back(v);
	}
	

	

	/// check if an edge is in the list
 	bool contains(edge e){
 
		for (std::vector<vertex>::iterator it = vertices.begin(); it != vertices.target(); ++it) {
			for (std::vector<vertex>::iterator i = it.edges.begin(); i != it.edges.target(); ++i) {
				if ( e == *i )
					return true;
			}
		}
 		return false;
	}

	/// check if the vertex is in the list
 	bool contains(vertex v){
 
		for (unsigned int i = 0; i < vertices->size(); i ++) {
			if ( v == (*vertices)[i])
 				return true;
		}
 		return false;
	}

	


	// outputs the list as String 
	void to_String(){
		for (unsigned int i = 0; i < vertices->size(); i++){
			std::cout<<i<<":[ "; // index of Vertex
			for (unsigned int j = 0 ; j < (*vertices)[i].edges.size();j++){
				std::cout<<(*vertices)[i].edges[j].start << "," << (*vertices)[i].edges[j].target << " "; // start and end of edges
			}
			std::cout<<"]"<<std::endl;
		}
	}
	
	

	/// removes an edge from the list
	void remove(int start, int target){

			for (unsigned int i = 0;i< (*vertices)[start].edges.size(); i++){
				if((*vertices)[start].edges[i].target == target){
					std::vector<edge>::iterator it = (*vertices)[start].edges.begin() + i;
					(*vertices)[start].edges.erase(it);
				}
			}
	}

	/// removes an vertex from the list
	void remove(vertex e){
		std::cout<<"not implemented"<<std::endl;
	}

	/// removes an vertex from the list
	void remove(int n){
		// delete edges to n (all from n are deleted itself)
		for(unsigned int i = 0; i< vertices->size(); i++){
			for (unsigned int j = 0;j< (*vertices)[i].edges.size(); j++){
				if((*vertices)[i].edges[j].target == n){
					std::vector<edge>::iterator it = (*vertices)[i].edges.begin() + j;
					(*vertices)[i].edges.erase(it);
				}
		// and decrese all edges with start or end > n
				if((*vertices)[i].edges[j].target > n){
					(*vertices)[i].edges[j].target -= 1; 
				}
				if((*vertices)[i].edges[j].start > n){
					(*vertices)[i].edges[j].start -= 1; 
				}
			}
		}

		std::vector<vertex>::iterator it = vertices->begin() + n;
		vertices->erase(it);


	}*/

};

typedef  adjacency_list<vertex<edge> > base_graph;
typedef  adjacency_list<vertex<weighted_edge> > weighted_graph;



	}
}