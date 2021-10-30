#include "HeapFactory.h"

void HeapFactory::Init()
{
	defaultHeap = new Heap("DefaultHeap");
}

Heap* HeapFactory::GetDefaultHeap()
{
	return defaultHeap;
}

Heap* HeapFactory::defaultHeap;

