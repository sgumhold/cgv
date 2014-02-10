#pragma once

#include <vector>

namespace cgv {
	namespace data {
/** IndexHeap is a heap that allows changing of the elements */
template <typename T>
class dynamic_priority_queue
{
public:
	typedef T element_type;
protected:
	/// store per element, whether it is empty and a link to the next empty element or to the heap location
	struct extended_element
	{
		element_type element;
		bool is_empty : 1;
		unsigned int link : 31;
	};
	/// container for elements
	std::vector<extended_element> elements;
	/// store index of last empty element or -1 if there is non
	int last_empty_element;
	/// store indices to the elements on the heap
	std::vector<unsigned int> heap;
	/// check if a position is outside of range
	bool valid(unsigned int hp) const                  { return hp < heap.size(); }
	/// make sure that the link for hp is ok
	void updateLink(unsigned int hp)                   { elements[heap[hp]].link = hp; }
	/// check if hp1 is less than hp2, if true, exchange elements
	bool isBetter(unsigned int hp1, unsigned int hp2) const { 
		return elements[heap[hp1]].element<elements[heap[hp2]].element; 
	}
	/// check if hp1 is less than hp2, if true, exchange elements
	bool exchangeIfBetter(unsigned int hp1, unsigned int hp2)
	{
		if (isBetter(hp1,hp2)) {
			std::swap(heap[hp1], heap[hp2]);
			updateLink(hp1);
			updateLink(hp2);
			return true;
		}
		return false;
	}
	///
	void heapify(unsigned int hp)           { if (!positionUpward(hp)) positionDownward(hp); }
	/// move one element to another position, that should be empty
	void move(unsigned int hp1, unsigned int hp2)
	{
		heap[hp2] = heap[hp1];
		updateLink(hp2);
	}
	/// remove an element on a certain heap pos
	void removeHeap(unsigned int hp)
	{
		if (hp != heap.size()-1) {
			move((unsigned int)heap.size()-1,hp);
			heap.pop_back();
			heapify(hp);
		}
		else 
			heap.pop_back();
	}
	/// move an element downward to the correct position
	void positionDownward(unsigned int& hp)
	{
		unsigned int child;
		while (valid(child = 2*hp+1)) {
			unsigned int tmp = child+1;
			if (valid(tmp) && isBetter(tmp,child)) child = tmp;
			if (!exchangeIfBetter(child, hp)) break;
			hp = child;
		}
	}
	/// move an element upward to the correct position
	bool positionUpward(unsigned int& hp)
	{
		bool changed = false;
		while (hp != 0) {
			unsigned int pa = (hp-1)/2;
			if (!exchangeIfBetter(hp, pa)) break;
			hp = pa;
			changed = true;
		}
		return changed;
	}
public:
	/// empty construction
	dynamic_priority_queue() : last_empty_element(-1) {}
	/// remove all elements
	void clear()        { elements.clear(); heap.clear(); last_empty_element = -1; }
	/// check if heap is empty
	bool empty() const { return heap.size() == 0; }
	/// return the number of elements in the heap
	size_t size() const { return heap.size(); }
	/// return the number of elements in the heap
	size_t size_of_element_container() const { return elements.size(); }

	/**@name queue interface */
	//@{
	/// return the top element
	unsigned int top() const { return heap[0]; }
	/// remove top element
	void pop() { remove(top()); }
	/// extract top element
	int extract()
	{
		unsigned int tmp = top();
		pop();
		return tmp;
	}
	//@}

	/**@name dynamic element access*/
	//@{
	///
	bool is_empty(unsigned int idx) const { return elements[idx].is_empty; }
	///
	const element_type& operator [] (unsigned int idx) const { return elements[idx].element; }
	element_type& operator [] (unsigned int idx) { return elements[idx].element; }
	/// insert the given element and return its index in the element container
	unsigned int insert(const element_type& e)
	{
		unsigned int idx;
		if (last_empty_element != -1) {
			idx = last_empty_element;
			if (elements[idx].link == idx)
				last_empty_element = -1;
			else
				last_empty_element = elements[idx].link;
		}
		else {
			idx = (unsigned int) elements.size();
			elements.resize(idx+1);
		}
		elements[idx].element  = e;
		elements[idx].is_empty = false;

		unsigned int hp = (unsigned int) heap.size();
		heap.push_back(idx);
		updateLink(hp);
		positionUpward(hp);

		return idx;
	}
	/// update after a change to the predicate
	void update(unsigned int idx) 
	{
		if (!is_empty(idx)) 
			heapify(elements[idx].link); 
	}
	/// remove element at location idx
	void remove(unsigned int idx) 
	{
		if (is_empty(idx))
			return;
		removeHeap(elements[idx].link);
		elements[idx].is_empty = true;
		if (last_empty_element == -1)
			elements[idx].link = idx;
		else
			elements[idx].link = last_empty_element;
		last_empty_element = idx;
	}
};

	}
}