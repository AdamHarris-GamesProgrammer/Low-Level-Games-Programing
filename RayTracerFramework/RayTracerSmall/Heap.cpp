#include "Heap.h"

void Heap::AllocateMemory(int size)
{
	_totalAllocated += size;
}

void Heap::DeallocateMemory(int size)
{
	_totalAllocated -= size;
}



int Heap::GetAmountAllocated()
{
	return _totalAllocated;
}
