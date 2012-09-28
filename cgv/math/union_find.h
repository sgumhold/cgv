#pragma once

namespace cgv{
	namespace math{
/**
* A union find data structure
*/
struct union_find
{
	int* sz;
	int* id;
	int components;
	int N;

	///N number of all elements 
	union_find(int N):N(N)
	{
		components=N;
		id = new int[N];
		sz = new int[N];
		for(int i = 0; i < N;i++)
		{
			id[i] = i;
			sz[i] = 1;
		}

	}
	///destructor
	~union_find()
	{
		delete[] sz;
		delete[] id;
	}

	///number of sets (initially number of all elements)
	 int num_of_components()
	 {
		 return components;
	 }

	 //return the number of elements in the set which contains x
	 int num_in_set(int x)
	 {
		int l= find(x);
		return sz[l];
	 }

	 //retruns label of the first set this can be used in combination with next_set to iterate over all sets
	 int first_set()
	 {
		 int x =0;
		 while(x < N && x != id[x])
			 x++;
		 return x;
	 }
	 //returns the next set of x or -1 if x is the last one
	 int next_set(int x)
	 {
		
		 x++;
		 while(x <N &&x != id[x])
			 x++;
		 if(x == N)
			 x= -1;
		 return x;

	 }

	 ///return label number of element x
	int find(int x)
	{
		while(x != id[x])
		{
			id[x] = id[id[x]];
			x=id[x];
		}
		return x;
	}

	

	///unite the set containing p with the set containing q (if p and q are in the same set, nothing is done)
	void unite(int p, int q)
	{
		int i = find(p);
        int j = find(q);
        if (i == j) return;
        if   (sz[i] < sz[j]) { id[i] = j; sz[j] += sz[i]; }
        else                 { id[j] = i; sz[i] += sz[j]; }
        components--;
	}

	///check wether p and q are in the same set
	bool find(int p, int q)
	{
		return find(p) == find(q);
	}

};

	}
}