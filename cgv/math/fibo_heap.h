#pragma once
#include <string.h>


namespace cgv{
	namespace math{

/**
* A fibonacci heap. 
*/
template <typename key_type,typename value_type, int max_degrees=20>
class fibo_heap
{
	struct heap_node
	{
		heap_node* parent;
		heap_node* left;
		heap_node* right;
		heap_node* children[max_degrees];
		int num_children;
		int degree;
		key_type key;
		value_type value;
		
		heap_node(const key_type& _key, const value_type&_value)
		{
			left    = NULL;
			right   = NULL;
			parent  = NULL;
			key		= _key;
			value   =_value;
			degree  = 0;
			num_children   = 0;
			memset(children, 0, sizeof(children));
		}

		
	};
	public:
	///constructor
	fibo_heap()
	{
			root_heap = NULL;
			min_heap = NULL;
	}

	

	///destructor
	~fibo_heap()
	{
		heap_node* node = root_heap;
	        
		while (node != NULL)
		{
					heap_node* next_node = node->right;

					delete_tree(node);
					node = next_node;
			}

	}

	///insert key to heap
	void insert(const key_type& key, const value_type& value)
	{
		heap_node* new_node = new heap_node(key,value);
		add_to_root_heap(new_node);

	}

	///delete and return smallest key in heap
	value_type delete_min()
	{
		 assert(min_heap != NULL);

		 
		 value_type k = min_heap->value;
		 
		 // Move the children into root
		 level_up(min_heap);

		 // Remove the minimum node
		 remove_node(min_heap);
		 min_heap = NULL;

		 // Re-assign a new one for min_heap
		 assign_min_node();

		 // consolidate the heap trees
		 consolidate();
		return k;
	}

	bool empty()
	{
		return min_heap == NULL;
	}



private:
	heap_node* root_heap;
	heap_node* min_heap;


	void add_to_root_heap(heap_node* node)
	{
		// First time
	   if (root_heap == NULL)
			{
					root_heap = node;
					min_heap = node;
					return;
			}

			// Add to the most left of the root heap
			node->right = root_heap;
			root_heap->left = node;
			root_heap = node;

			// Check the minimum heap
			if (node->key < min_heap->key )
			{
					min_heap = node;
			}

	}

	void consolidate()
	{
		  // Make sure the degree of each tree is unique
			heap_node* degree_nodes[max_degrees];
			heap_node* node = root_heap;

			memset(degree_nodes, 0, sizeof(degree_nodes));
			while (node != NULL)
			{
					if (degree_nodes[node->degree] == NULL)
					{
							degree_nodes[node->degree] = node;
							node = node->right;
					}
					else    // merge the two trees
					{
							heap_node* pre_node = degree_nodes[node->degree];
	                        
							if (node->key <pre_node->key)
							{
									attach_tree(pre_node, node);
							}
							else
							{
									attach_tree(node, pre_node);
							}

							// Reset the search
							memset(degree_nodes, 0, sizeof(degree_nodes));                  
							node = root_heap;
					}
			}

	}

	void attach_tree(heap_node* from_node, heap_node* to_node)
	{
		 if (from_node == root_heap)
			{
					root_heap = root_heap->right;
			}
	        
			// Break the link of from_node
			if (from_node->left)  from_node->left->right = from_node->right;
			if (from_node->right) from_node->right->left = from_node->left;
			from_node->left = NULL;
			from_node->right = NULL;

			// Move the from_node under the to_node
			to_node->children[ to_node->num_children ] = from_node;
			from_node->parent = to_node;
			to_node->num_children++;
			to_node->degree++;

	}

	void level_up(heap_node* node)
	{
		 for (int i = 0; i < node->num_children; i++)
			{
					heap_node* child_node = node->children[i];

					add_to_root_heap(child_node);
					child_node->parent = NULL;
					node->children[i] = NULL;
			}

			node->num_children = 0;

	}

	void remove_node(heap_node* node)
	{
		  if (node == NULL) return;

			if (node == root_heap)
			{
					root_heap = root_heap->right;
			}
			if (node->left)  node->left->right = node->right;
			if (node->right) node->right->left = node->left;
			delete node;

	}

	void assign_min_node()
	{
		  heap_node* check_node = root_heap;

			while (check_node != NULL)
			{
					if (min_heap == NULL || check_node->key < min_heap->key )
					{
							min_heap = check_node;
					}
	                
					check_node = check_node->right;
			}

	}

	void delete_tree(heap_node* node)
	{
		  if (node == NULL) return;
	        
			for (int i = 0; i < node->num_children; i++)
			{
					heap_node* child_node = node->children[i];
					delete_tree(child_node);
			}

			delete node;

	}
};

	}
}




