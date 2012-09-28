#pragma once
#include <cgv/math/adjacency_list.h>
#include <cgv/math/union_find.h>
#include <cgv/math/fibo_heap.h>


namespace cgv{
	namespace math{




template <typename v_type>
void mst_prim(adjacency_list<v_type> &graph, adjacency_list<v_type> &mst)
{
	if(graph.nverts() == 0)
		return;

	std::vector<int> *vflags = new std::vector<int>(graph.nverts(),0);
	
	mst.resize(graph.nverts());
	mst.directed=false;

	
	fibo_heap<double,adjacency_list<v_type>::edge_type*> heap;
	
	(*vflags)[0]=1;
	for(unsigned ei = 0; ei < graph.vertex(0).edges.size(); ei++)
	{
		adjacency_list<v_type>::edge_type *e = &(graph.vertex(0).edges[ei]);	
		heap.insert(e->weight,e);	
	}

	while(!heap.empty())
	{
		adjacency_list<v_type>::edge_type *e = heap.delete_min();

		if((*vflags)[e->end] == 0)
		{

			mst.add_edge(*e);
			(*vflags)[e->end] = 1;
			for(unsigned ei = 0; ei < graph.vertex(e->end).edges.size(); ei++)
			{
				adjacency_list<v_type>::edge_type *e2 = &(graph.vertex(e->end).edges[ei]);
				if( (*vflags)[e2->end] == 0)
					heap.insert(e2->weight,e2);	
			}
		}
	}

	delete vflags;

	

	
	
}



	}
}