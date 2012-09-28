#pragma once
#include  <cgv/math/fibo_heap.h>

void test_fibo_heap()
{
	using namespace cgv::math;

	cgv::math::fibo_heap<int,int> heap;

	heap.insert(4,4);
	heap.insert(100,100);
	heap.insert(1,1);
	heap.insert(3,3);
	heap.insert(5,5);
	heap.insert(1,1);
	heap.insert(4,4);
	heap.insert(3,3);
	
	assert(heap.delete_min()==1);
	assert(heap.delete_min()==1);
	assert(heap.delete_min()==3);
	assert(heap.delete_min()==3);
	assert(heap.delete_min()==4);
	assert(heap.empty() == false);
	assert(heap.delete_min()==4);
	assert(heap.delete_min()==5);
	assert(heap.delete_min()==100);
	assert(heap.empty() == true);
	
}
